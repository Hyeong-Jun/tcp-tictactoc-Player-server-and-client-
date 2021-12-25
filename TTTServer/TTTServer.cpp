/****************************************************************************************
    - ƽ���� ����
    - �ۼ��� TCP ����/Ŭ���̾�Ʈ ���α׷��� �ڵ带 ������� ����
    - ������ Ŭ���̾�Ʈ�� ������ �õ��� Ƚ���� �ְ� ������ ǥ���� �� �ִ�.
    - ������ Ȱ���� �ʿ�x
    - �迭��(0���� 8) �� �ϳ��� �Է��Ͽ� ������ S, Ŭ���̾�Ʈ�� X�� �����ǿ� ǥ���Ѵ�
    - ƽ���� ��Ģ�� ���� ������ �¸��ϸ� ���� +1, �÷��̾ �¸��ϸ� ���� +1
    - ���� �õ��� Ƚ�� +1
*****************************************************************************************/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <time.h>
#define SERV_PORT 5000
#define MAXPENDING 1    // �ִ� �÷��̾� ��
#define BUFSIZE 20

#define WHO_SERVER 1    // ����    
#define WHO_CLIENT 2    // Ŭ���̾�Ʈ

#define NOT_WINNER 0    // ���� �º� �ȳ�
#define WIN_SERVER 1    // ���� ��
#define WIN_CLIENT 2    // Ŭ���̾�Ʈ ��
#define DEAD_HEAT 3     // ���º�

void check_who(char num, int who);   // �̱�� �ִ� ������ �迭�� ���� üũ�ߴ��� �Ǵ�
int who_win(void);				     // ���� �̰���� ������ �Ǵ�
void print_game(void);               // ������ ���
void print_result(void);             // ��� ���

char mark[9][3];					 // �̱�� �ִ� ��� �迭�� ����
int cnt = 0;						 // ����̳� üũ �ƴ��� �Ǵ�
char check[10] = "ooooooooo";		 // ������ �迭

int nServerScore = 0;           // ���� ����
int nClientScore = 0;           // Ŭ���̾�Ʈ ����
int nClientTryCount = 0;        // Ŭ���̾�Ʈ �õ� Ƚ��

int main(int argc, char** argv)
{
    SOCKET servSock;
    SOCKET clntSock;
    struct sockaddr_in servAddr;
    struct sockaddr_in clntAddr;
    int clntLen;
    int str_len;
    char message[BUFSIZE];          // �޽���(����)
    int who = WHO_SERVER;			// ���� üũ �ߴ��� ǥ��
    char num;					    // ������ ��� üũ���� �Ǵ�
    WSADATA wsaData;


    // ��¥, �ð� ǥ��
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Load Winsock 2.2 DLL 
        fprintf(stderr, "WSAStartup() failed");
        exit(1);
    }

    // INVALID_SOCKET
    // -> ������ �����ڵ�, ���� �� INVALID_SOCKET ��ȯ
    //���� ����
    servSock = socket(PF_INET, SOCK_STREAM, 0);
    if (servSock == INVALID_SOCKET) {
        printf("Error: socket() failed");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET; // AF_INET : ip
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // IPǥ��
    servAddr.sin_port = htons(SERV_PORT); // Port�� s / 

    // SOCKET_ERROR
    // ->���� �Լ��� ���Ǵ� listen, connect, bind, closesocket �Լ����� ���� �� ��ȯ ��
    // ���Ͽ� �ּ��Ҵ�
    if (bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        printf("Error: bind() failed");
        exit(1);
    }

    printf("���� ���� %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    // ���� ��û ��� ���·� ����
    if (listen(servSock, MAXPENDING) == SOCKET_ERROR) {
        printf("Error: listen() failed");
        exit(1);
    }
    
    // bind -> listen -> accept -> 

    // Ŭ���̾�Ʈ�� ���� Connect ��û�� �� ������ ������ ���
    while (1) {
        fputs("Ŭ���̾�Ʈ ������ ��ٸ��ϴ�.\n", stdout);

        // ���� ��û ����
        clntLen = sizeof(clntAddr);
        clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntLen);
        if (clntSock == INVALID_SOCKET) {
            printf("Error: accept() failed");
            break;
        }
        fputs("Ŭ���̾�Ʈ ����!\n", stdout);
        nServerScore = 0;           // ���� ����
        nClientScore = 0;           // Ŭ���̾�Ʈ ����
        nClientTryCount = 0;        // Ŭ���̾�Ʈ �õ� Ƚ��

        // EndGame���� ���� �̻� ���� ����
        // Ŭ���̾�Ʈ�� ���� ���� ���� ����(Y or N) ������ ���� ����
        // �������� ƽ���並 �����Ѵ�
        bool bEndGame = false;
        while (bEndGame == false) {
            fputs("Ŭ���̾�Ʈ���� ���� ���� ���θ� ���� �ֽ��ϴ�.\n", stdout);
            str_len = recv(clntSock, message, sizeof(message), 0);
            if (str_len == SOCKET_ERROR) {
                fputs("Ŭ���̾�Ʈ ���� ����\n", stdout);
                break;
            }
            else if (0 == str_len) {
                fputs("Ŭ���̾�Ʈ ���� ����\n", stdout);
                break;
            }
            else if (0 < str_len && (message[0] == 'n' || message[0] == 'N'))
                break;
            else if (0 < str_len && (message[0] == 'y' || message[0] == 'Y')) {
                nClientTryCount++; // print_result()�� ���� Ƚ���� �����Ѵ�

                fputs("=========== ���� ���� ===========\n", stdout);
                // ������ ������ ��� ����
                while (1) {
                    fputs("���� �����Դϴ� : ", stdout); // ���� ����
                    fgets(message, sizeof(message), stdin);
                    rewind(stdin);
                    // '0' ~ '8' �Է¸� ����
                    if (!('0' <= message[0] && message[0] <= '8'))
                    {
                        fputs("'0' ~ '8' �Է¸� �����մϴ�.\n", stdout); // ���� ����
                        continue;
                    }
                    send(clntSock, message, sizeof(message), 0); // ������ �����Ѱ� Ŭ���̾�Ʈ�� ����
                    
                    cnt++;
                    who = WHO_SERVER;
                    num = message[0];

                    // �����ǿ� ���� or Ŭ���̾�Ʈ �� ǥ��
                    check_who(num, who);
                    // ������ ����
                    print_game();
                    int whowin = who_win();
                    // �ºΰ� ��
                    if (whowin != NOT_WINNER)
                        break;

                    // Ŭ���̾�Ʈ�� ���� �޽��� ���� ���
                    fputs("Ŭ���̾�Ʈ �����Դϴ�.\n", stdout); // Ŭ���̾�Ʈ ����
                    memset(message, 0, sizeof(message));
                    str_len = recv(clntSock, message, sizeof(message), 0);
                    if (str_len == SOCKET_ERROR)
                    {
                        fputs("Ŭ���̾�Ʈ ���� ����\n", stdout);
                        bEndGame = true;
                        break;
                    }
                    if (0 < str_len) {
                        printf("Ŭ���̾�Ʈ : %s", message);
                        cnt++;

                        who = WHO_CLIENT;
                        num = message[0];
                        // ������ üũ�� �� �Ǵ�
                        check_who(num, who);
                        print_game();
                        // �������� ������Ʈ �� ������ �̱�� ��찡 ������� �Ǵ�
                        int whowin = who_win();
                        // �ºΰ� ���� loop Ż�� �� ����
                        if (whowin != NOT_WINNER)
                            break;
                    }
                }
                fputs("=========== ���� ���� ===========\n", stdout);
            }
        }

        print_result(); // ��� ���
    }

    closesocket(clntSock); // ���� �ݾ���
    //WSACleanup(); // ���� ���̺귯�� ����

    system("pause");

    return 0;
}

// �¹��� ����
int who_win(void)
{
    int iRes = NOT_WINNER;
    for (int i = 1; i <= 8; i++)
    {
        if (mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 1) {
            printf("���� ��!\n");
            nServerScore++;
            iRes = WIN_SERVER;
        }
        else if (mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 2) {
            printf("Ŭ���̾�Ʈ ��!\n");
            nClientScore++;
            iRes = WIN_CLIENT;
        }

        if (cnt == 9 && i == 8 && !(mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 1)
            && !(mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 2)) {
            printf("���º�\n");
            iRes = DEAD_HEAT;
        }
    }

    if (iRes != NOT_WINNER)
    {
        memset(mark, 0, sizeof(mark));
        cnt = 0;
        memcpy(check, "ooooooooo", sizeof(check));
    }
    return iRes;
}

// �迭�� ���� ǥ�� �ߴ��� �Ǵ�
// 3x3 Tic-Tac-Toe���� 8������ �¸���찡 ����
void check_who(char num, int who) {
    int who_char;

    if (who == 1)
        who_char = 'S'; // Server�� ƽ���� ǥ��
    else
        who_char = 'C'; // Client�� ƽ���� ǥ��

    check[atoi(&num)] = who_char; // �����ǿ� üũ�� ��� ǥ��

    // �������� ���� ��ġ�� ���� �̱�� ���
    switch (num) {
    case '0': {
        mark[1][0] = who;
        mark[4][0] = who;
        mark[7][0] = who;
        break;
    }
    case '1': {
        mark[2][0] = who;
        mark[4][1] = who;
        break;
    }
    case '2': {
        mark[3][0] = who;
        mark[4][2] = who;
        mark[8][0] = who;
        break;
    }
    case '3': {
        mark[1][1] = who;
        mark[5][0] = who;
        break;
    }
    case '4': {
        mark[2][1] = who;
        mark[5][1] = who;
        mark[8][1] = who;
        mark[7][1] = who;
        break;
    }
    case '5': {
        mark[3][1] = who;
        mark[5][2] = who;
        break;
    }
    case '6': {
        mark[1][2] = who;
        mark[6][0] = who;
        mark[8][2] = who;
        break;
    }
    case '7': {
        mark[2][2] = who;
        mark[6][1] = who;
        break;
    }
    case '8': {
        mark[3][2] = who;
        mark[6][2] = who;
        mark[7][2] = who;
        break;
    }
    }
}

void print_game(void)
{
    // ������(0~8��)
    // 0 1 2
    // 3 4 5
    // 6 7 8
    for (int j = 0; j <= 8; j++) {
        printf("%c ", check[j]);
        if (j == 2 || j == 5 || j == 8) printf("\n");
    }
}

void print_result(void)
{
    printf("���� ���ھ�: %d\n", nServerScore); // ���� ����
    printf("Ŭ���̾�Ʈ ���ھ�: %d\n", nClientScore); // Ŭ���̾�Ʈ ����
    printf("Ŭ���̾�Ʈ �õ� Ƚ��: %d\n", nClientTryCount); // Ŭ���̾�Ʈ �õ� Ƚ��
}