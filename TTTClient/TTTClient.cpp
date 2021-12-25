/****************************************************************************************
	- ƽ���� ����
	- �ۼ��� TCP ����/Ŭ���̾�Ʈ ���α׷��� �ڵ带 ������� ����
	- ������ Ŭ���̾�Ʈ�� ������ �õ��� Ƚ���� �ְ� ������ ǥ���� �� �ִ�.
	- ������ Ȱ���� �ʿ�x
	- �迭��(0���� 8) �� �ϳ��� �Է��Ͽ� ������ S, Ŭ���̾�Ʈ�� X�� �����ǿ� ǥ���Ѵ�
	- ƽ���� ��Ģ�� ���� ������ �¸��ϸ� ���� +1, Ŭ���̾�Ʈ�� �¸��ϸ� ���� +1
	- ���� �õ��� Ƚ�� +1
*****************************************************************************************/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#define SERV_PORT 5000
#define BUFSIZE 20

#define WHO_SERVER 1    // ����    
#define WHO_CLIENT 2    // Ŭ���̾�Ʈ

#define NOT_WINNER 0    // ���� �º� �ȳ�
#define WIN_SERVER 1    // ���� ��
#define WIN_CLIENT 2    // Ŭ���̾�Ʈ ��
#define DEAD_HEAT 3     // ���º�

void check_who(char num, int who);   // �̱�� �ִ� ������ �迭�� ���� üũ�ߴ��� �Ǵ�
int who_win(void);					 // ���� �̰���� ������ �Ǵ�
void print_game(void);               // ������ ���
void print_result(void);             // ��� ���

char mark[9][3];					 // �̱�� �ִ� ��� �迭�� ����
int cnt = 0;						 // ����̳� üũ �ƴ��� �Ǵ�
char check[10] = "ooooooooo";		 // ������ �迭

int nServerScore = 0;			// ���� ����
int nClientScore = 0;           // Ŭ���̾�Ʈ ����
int nClientTryCount = 0;        // Ŭ���̾�Ʈ �õ� Ƚ��

int main(int argc, char** argv)
{
	SOCKET sock;
	struct sockaddr_in servAddr;
	char servIP[15];                // ���� �ּ�
	char message[BUFSIZE];          // �޽���(����)
	int str_len;
	char hello[] = "O\n";			// Ŭ���� ��Ʈ�� �����ߴٴ� ǥ�� ����
	int who = 1;					// ���� üũ �ߴ��� ǥ��
	char num;					    // ������ ��� üũ���� �Ǵ�

	int maxLen = sizeof(message); // Max number of bytes to receive
	WSADATA wsaData;                 // Structure for WinSock setup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Load Winsock 2.2 DLL
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	printf("������ IP �ּҸ� �Է��ϼ��� : ");
	scanf("%s", servIP);
	rewind(stdin);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servIP);
	servAddr.sin_port = htons(SERV_PORT);

	// INVALID_SOCKET
	// -> ������ �����ڵ�, ���� �� INVALID_SOCKET ��ȯ
	// Ŭ���̾�Ʈ ����
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Error: socket() failed");
		exit(1);
	}

	// SOCKET_ERROR
	// ->���� �Լ��� ���Ǵ� listen, connect, bind, closesocket �Լ����� ���� �� ��ȯ ��
	// �غ��� �����ּҸ� Ŭ���̾�Ʈ ���Ͽ� ����õ�
	if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		printf("ERROR: connect() failed");
		exit(1);
	}

	printf("���� ���� ����!\n");

	// EndGame���� ���� �̻� ���� ����
	// ������ �����ϱ� �� ���� ���� ���θ� ���´�
	// ���� ���� ����(Y or N) �Է��ϸ� ������ message�� ������ ���� ����
	// �������� ƽ���並 �����Ѵ�
	bool bEndGame = false;
	while (bEndGame == false) {
		fputs("������ ������ �����Ͻðڽ��ϱ�(Y/N) : ", stdout); // ���� ����
		fgets(message, sizeof(message), stdin);
		rewind(stdin);
		send(sock, message, sizeof(message), 0); // Ŭ���̾�Ʈ�� �����Ѱ� ������ ����
		if (message[0] == 'y' || message[0] == 'Y')
		{
			nClientTryCount++;
			fputs("=========== ���� ���� ===========\n", stdout);
			while (1) {
				// ������ ���� �޽��� ���� ���
				fputs("���� �����Դϴ�.\n", stdout); // ���� ����
				str_len = recv(sock, message, sizeof(message), 0);
				if (0 < str_len) {
					printf("���� : %s", message);
					cnt++;

					who = WHO_SERVER;
					num = message[0];

					check_who(num, who); //������ ������ üũ�� �� �Ǵ�
					print_game(); // �������� �����
					// �������� ������Ʈ �� ������ �̱�� ��찡 ������� �Ǵ�
					int whowin = who_win();
					// �ºΰ� ��
					if (whowin != NOT_WINNER)
						break;
				}

				fputs("Ŭ���̾�Ʈ �����Դϴ� : ", stdout); // Ŭ���̾�Ʈ ����
				memset(message, 0, sizeof(message));
				fgets(message, BUFSIZE, stdin);
				rewind(stdin);
				send(sock, message, sizeof(message), 0); // Ŭ���̾�Ʈ�� �����Ѱ� ������ ����
				
				cnt++;
				who = WHO_CLIENT;
				num = message[0];

				// �����ǿ� üũ�� �� �Ǵ�
				check_who(num, who);
				// ������ ����
				print_game();
				int whowin = who_win();
				// �ºΰ� ���� loop Ż�� �� ����
				if (whowin != NOT_WINNER)
					break;
			}
			fputs("=========== ���� ���� ===========\n", stdout);
		}
		else
			break;
	}

	print_result(); // ��� ���
	fputs("=========== ���α׷� ���� ===========\n", stdout);

	closesocket(sock); // ���� �ݾ���
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

// �̱�� ��� �迭�� ���� ǥ�� �ߴ��� �Ǵ�
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