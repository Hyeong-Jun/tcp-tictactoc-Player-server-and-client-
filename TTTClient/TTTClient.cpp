/****************************************************************************************
	- 틱택토 게임
	- 작성한 TCP 서버/클라이언트 프로그래밍 코드를 기반으로 진행
	- 서버는 클라이언트가 게임을 시도한 횟수와 최고 점수를 표시할 수 있다.
	- 스레드 활용할 필요x
	- 배열값(0부터 8) 중 하나를 입력하여 서버면 S, 클라이언트면 X를 게임판에 표시한다
	- 틱택토 규칙에 따라 서버가 승리하면 점수 +1, 클라이언트가 승리하면 점수 +1
	- 게임 시도한 횟수 +1
*****************************************************************************************/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#define SERV_PORT 5000
#define BUFSIZE 20

#define WHO_SERVER 1    // 서버    
#define WHO_CLIENT 2    // 클라이언트

#define NOT_WINNER 0    // 아직 승부 안남
#define WIN_SERVER 1    // 서버 승
#define WIN_CLIENT 2    // 클라이언트 승
#define DEAD_HEAT 3     // 무승부

void check_who(char num, int who);   // 이길수 있는 경우들의 배열에 누가 체크했는지 판단
int who_win(void);					 // 누가 이겻는지 비겼는지 판단
void print_game(void);               // 게임판 출력
void print_result(void);             // 결과 출력

char mark[9][3];					 // 이길수 있는 경우 배열로 선언
int cnt = 0;						 // 몇번이나 체크 됐는지 판단
char check[10] = "ooooooooo";		 // 게임판 배열

int nServerScore = 0;			// 서버 점수
int nClientScore = 0;           // 클라이언트 점수
int nClientTryCount = 0;        // 클라이언트 시도 횟수

int main(int argc, char** argv)
{
	SOCKET sock;
	struct sockaddr_in servAddr;
	char servIP[15];                // 서버 주소
	char message[BUFSIZE];          // 메시지(버퍼)
	int str_len;
	char hello[] = "O\n";			// 클라이 언트가 접속했다는 표시 문자
	int who = 1;					// 누가 체크 했는지 표시
	char num;					    // 게임판 어디에 체크할지 판단

	int maxLen = sizeof(message); // Max number of bytes to receive
	WSADATA wsaData;                 // Structure for WinSock setup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Load Winsock 2.2 DLL
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	printf("서버의 IP 주소를 입력하세요 : ");
	scanf("%s", servIP);
	rewind(stdin);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servIP);
	servAddr.sin_port = htons(SERV_PORT);

	// INVALID_SOCKET
	// -> 성공시 소켓핸들, 실패 시 INVALID_SOCKET 반환
	// 클라이언트 소켓
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Error: socket() failed");
		exit(1);
	}

	// SOCKET_ERROR
	// ->소켓 함수에 사용되는 listen, connect, bind, closesocket 함수들의 실패 시 반환 값
	// 준비한 서버주소를 클라이언트 소켓와 연결시도
	if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		printf("ERROR: connect() failed");
		exit(1);
	}

	printf("서버 연결 성공!\n");

	// EndGame이지 않은 이상 연결 유지
	// 게임이 시작하기 전 게임 시작 여부를 묻는다
	// 게임 시작 여부(Y or N) 입력하면 서버로 message를 보내고 게임 시작
	// 서버부터 틱택토를 시작한다
	bool bEndGame = false;
	while (bEndGame == false) {
		fputs("서버와 게임을 시작하시겠습니까(Y/N) : ", stdout); // 서버 차례
		fgets(message, sizeof(message), stdin);
		rewind(stdin);
		send(sock, message, sizeof(message), 0); // 클라이언트가 선택한것 서버로 전송
		if (message[0] == 'y' || message[0] == 'Y')
		{
			nClientTryCount++;
			fputs("=========== 게임 시작 ===========\n", stdout);
			while (1) {
				// 서버로 부터 메시지 수신 대기
				fputs("서버 차례입니다.\n", stdout); // 서버 차례
				str_len = recv(sock, message, sizeof(message), 0);
				if (0 < str_len) {
					printf("서버 : %s", message);
					cnt++;

					who = WHO_SERVER;
					num = message[0];

					check_who(num, who); //서버가 게임판 체크한 곳 판단
					print_game(); // 게임판을 찍어줌
					// 게임판이 업데이트 될 때마다 이기는 경우가 생겼는지 판단
					int whowin = who_win();
					// 승부가 남
					if (whowin != NOT_WINNER)
						break;
				}

				fputs("클라이언트 차례입니다 : ", stdout); // 클라이언트 차례
				memset(message, 0, sizeof(message));
				fgets(message, BUFSIZE, stdin);
				rewind(stdin);
				send(sock, message, sizeof(message), 0); // 클라이언트가 선택한것 서버로 전송
				
				cnt++;
				who = WHO_CLIENT;
				num = message[0];

				// 게임판에 체크한 곳 판단
				check_who(num, who);
				// 게임판 생성
				print_game();
				int whowin = who_win();
				// 승부가 나면 loop 탈출 후 종료
				if (whowin != NOT_WINNER)
					break;
			}
			fputs("=========== 게임 종료 ===========\n", stdout);
		}
		else
			break;
	}

	print_result(); // 결과 출력
	fputs("=========== 프로그램 종료 ===========\n", stdout);

	closesocket(sock); // 소켓 닫아줌
	//WSACleanup(); // 소켓 라이브러리 해제

	system("pause");

	return 0;
}

// 승무패 조건
int who_win(void)
{
	int iRes = NOT_WINNER;
	for (int i = 1; i <= 8; i++)
	{
		if (mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 1) {
			printf("서버 승!\n");
			nServerScore++;
			iRes = WIN_SERVER;
		}
		else if (mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 2) {
			printf("클라이언트 승!\n");
			nClientScore++;
			iRes = WIN_CLIENT;
		}

		if (cnt == 9 && i == 8 && !(mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 1)
			&& !(mark[i][0] == mark[i][1] && mark[i][1] == mark[i][2] && mark[i][2] == 2)) {
			printf("무승부\n");
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

// 이기는 경우 배열에 누가 표시 했는지 판단
// 3x3 Tic-Tac-Toe에는 8가지의 승리경우가 있음
void check_who(char num, int who) {
	int who_char;

	if (who == 1)
		who_char = 'S'; // Server측 틱택토 표시
	else
		who_char = 'C'; // Client측 틱택토 표시

	check[atoi(&num)] = who_char; // 게임판에 체크한 사람 표시

	// 게임판의 선택 위치에 따라 이기는 경우
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
	// 게임판(0~8번)
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
	printf("서버 스코어: %d\n", nServerScore); // 서버 점수
	printf("클라이언트 스코어: %d\n", nClientScore); // 클라이언트 점수
	printf("클라이언트 시도 횟수: %d\n", nClientTryCount); // 클라이언트 시도 횟수
}