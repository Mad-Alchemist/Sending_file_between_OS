#include <stdlib.h>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

// 버퍼 크기 정의
#define BUFSIZE 4096



using namespace std;

class infor {  // 정보
private: // private 접근지정자 선언
	char ip[16];   // ip 주소  
	int port;      // 포트 번호
public:
	void set_infor(char a[16], int b) // Private 접근 지정이 된 요소를 변경하기 위해 메소드 함수 선언
	{
		strcpy(ip, a);
		port = b;
	}

	char *get_ip() {  // 문자열 ip의 포인터를 반환하는 메소드 함수
		return ip;
	}

	int get_port() { // Port의 값을 출력하기 위한 메소드 함수 
		return port;
	}
};

char* set_dir() {  // 디렉토리 설정 함수
	cout << "다운로드 경로를 입력하여 주십시오. (각 드라이브의 최상위 경로 EX : C:\ 지정 불가능)" << endl;
	char a[256];
	while (1) {
		cin >> a;
		struct stat dir;                // 파일의 정보를 담고 있는 stat 구조체 dir 선언
		lstat(a,&dir);                  // a에 입력받은 문자열의 경로의 파일을 stst 구조체 dir에 입력하는 함수 lstat 호출
		if (S_ISDIR(dir.st_mode)==0)    // 디렉토리인 경우 0이 아닌 값을 리턴하는 S_ISDIR 사용 (UNIX 매크로)
		{
			break;
		}
		else
		{
			cout << "잘못된 경로 입니다. 다시 입력하여 주십시오." << endl;
		}
	}
	return a;
}

void set_file(char *d) {                                                 // 파일의 경로를 설정하는 함수
	cout << "파일의 경로를 입력하여 주십시오. (파일명 포함)" << endl;
	char a[256];
	memset(a, 0, 256);                                                   // 256 바이트 만큼 a를 0으로 초기화
	while (1) {
		cin >> a;
		struct stat b;
		lstat(a, &b);
		if (S_ISREG(b.st_mode))                                         // a가 파일인지 검사하는 매크로 S_ISREG 사용 (UNIX 매크로)
		{
			break;
		}
		else
		{
			cout << "잘못된 파일 경로 입니다. 다시 입력하여 주십시오. (파일명 포함)" << endl;
		}
	}
	strcpy(d, a);
}

int datareceive(int s, char *buf, int len) { // 데이터가 끝날때 까지 데이터를 수신하는 함수 선언
	// 통신에 필요한 변수 선언
	int rec;
	char *ptr = buf;
	int left = len;

	// 남아 있는 데이터가 있는 경우 반복적으로 실행
	while (left > 0) {
		rec = recv(s, ptr, left,0);
		// 더이상 데이터를 받아오지 못하는 경우
		if (rec == 0) 
			break;
		// 받아올 데이터가 존재하는 경우
		left -= rec;
		ptr += rec;
	}
	return (len - left); // 더 받을 수 있는 데이터 길이를 반환(데이터를 한 번이라도 받았으면 0이 아닌 수를 반환하게 된다.)
}

int Receive() { // 데이터를 수신하는 함수인 Receive() 함수 선언
	
	int retval; // 데이터 크기를 담을 변수 
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0); // 연결용 소켓을 생성 Linux에서는 SOCKET형 구조체가 아닌 int형 사용

	infor a;
	cout << "사용하실 포트를 입력하여 주세요." << endl;
	int port;
	cin >> port;
	a.set_infor("",port);

	// 바인딩 처리 변수를 초기화
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(a.get_port());
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listen_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)); // 바인딩

	listen(listen_sock, SOMAXCONN); // 연결 함수 시작

	// 데이터 통신에 사용할 변수 
	int client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char buf[BUFSIZE];

	// 반복적으로 클라이언트로부터 소켓 요청을 받음
	while (1) {

		// 클라이언트 접속을 받음
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);

		// 접속한 클라이언트 정보를 출력
		cout << "클라이언트 접속 IP : " << inet_ntoa(clientaddr.sin_addr) << " 포트 : " << a.get_port() << endl;
		
		// 파일 이름을 받기 
		char filename[256];
		memset(filename, 0, 256);
		datareceive(client_sock, filename, 256);

		cout << "받을 파일 이름 : " << filename << endl; // 받을 파일 이름을 출력

		// 파일 크기를 받기 
		int fsize;
		datareceive(client_sock, (char*)&fsize, sizeof(fsize));
		cout << "받을 파일 크기 : " << fsize << endl; // 받을 파일 크기를 출력
		FILE *fp;
		char downloaded_file[256]; // 다운로드 받을 파일의 이름을 저장할 변수 선언
		cout << "파일을 저장할 경로와 이름을 입력하여 주십시오." << endl; // 파일을 저장할 경로와 이름을 출력
		cin >> downloaded_file;
		
		send(client_sock, "fin", sizeof("fin"), 0); // 클라이언트 소켓으로 파일 이름을 수신
		fp=fopen(downloaded_file, "wb"); // 다운로드 파일을 열기 
		
		// 다운로드한 데이터 파일을 정상적으로 개방에 실패할 경우
		if (fp == NULL)
		{
			cout << "파일 개방 실패!" << endl;
			return 0;
		}
		else
		{
			// 파일 데이터 받기 
			int fnum = 0;
			while (1) {
				retval = datareceive(client_sock, buf, BUFSIZE);

				// 더 받을 데이터가 없을 때 
				if (retval == 0)
				{
					break;
				}
				else
				{
					fwrite(buf, 1, retval, fp);
					// 받은 데이터 크기만큼 변수에 더해줌
					fnum += retval;
				}
			}
			fclose(fp);

			// 전송 결과 
			if (fnum == fsize)
				cout << "파일 전송 성공" << endl;
			else
				cout << "파일 전송 실패" << endl;
			// 해당 클라이언트 소켓을 폐기 
			close(client_sock);
			cout << "클라이언트 종료 IP = " << inet_ntoa(clientaddr.sin_addr) << " 포트 : " << ntohs(clientaddr.sin_port) << endl;
		}
		// 서버 소켓을 폐기
		close(listen_sock);;
		return 0;
	}
}


int Send() // 데이터를 전송할 Send() 함수 호출
{

	// 클라이언트 소켓 초기화
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	char ip[16]; // 문자열 ip 주소 선언
	int port; // 포트 번호 선언
	cout << "접속 할 서버의 ip를 입력하여 주세요." << endl;
	cin >> ip;
	cout << "접속 할 서버의 포트를 입력하여 주세요." << endl;
	cin >> port;
	infor serv;
	serv.set_infor(ip, port);

	// 클라이언트 소켓을 서버에 연결
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(serv.get_port());
	servaddr.sin_addr.s_addr = inet_addr(serv.get_ip());

	// 보낼 파일을 설정
	char myFile[256] = "";
	set_file(myFile);

	// 서버에 연결을 시도
	connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
	cout << "서버에 접속 성공:IP = " << ip << ", " << "Port = " << port << endl;

	// 파일 열기
	FILE* fp = fopen(myFile, "rb");
	// 파일 이름 변수 
	char filename[256];
	memset(filename, 0, 256);
	sprintf(filename, myFile);

	// 소켓으로 파일 이름을 전송
	send(sock, filename, 256, 0);
	// 파일 크기를 얻기 
	fseek(fp, 0, SEEK_END);
	int totalbytes = ftell(fp);
	// 파일 크기 소켓으로 보내기 
	send(sock, (char*)&totalbytes, sizeof(totalbytes), 0);

	// 파일 데이터 전송에 사용할 변수 
	char buf[BUFSIZE];
	int numread;
	int numtotal = 0;

	// 파일 포인터를 제일 앞으로 이동
	rewind(fp);
	char isfin[4];
	recv(sock, isfin, sizeof("fin"), 0);
	while(1)
	{
	if (strcmp(isfin, "fin") == 0)
	{

		// 반복적으로 파일 데이터 보내기 
		while (1) {
			// 파일의 내용을 버퍼에 담음
			numread = fread(buf, 1, BUFSIZE, fp);
			// 파일 데이터가 조금이라도 남은 경우
			if (numread > 0) {
				send(sock, buf, numread, 0);
				numtotal += numread;
			}
			// 파일을 모두 전송한 경우 
			else if (numread == 0 && numtotal == totalbytes) {
				cout << "총" << numtotal << "바이트 파일 전송을 완료했습니다." << endl;
				break;
			}
		}
		break;
	}
	
	else
	{
		sleep(1);
	}
	}
	fclose(fp);

	// 소켓을 폐기 
	close(sock);
	cout << "서버에 접속 종료: IP = " << inet_ntoa(servaddr.sin_addr) << "," << "Port = " << ntohs(servaddr.sin_port) << endl;

	return 0;
}

// 메인 함수 
int main(void)
{
	while (1) // 반복문 실행
	{
		cout << "커맨드를 입력하여 주세요" << endl << "Send : 파일 송신하기" << endl << "Receive : 파일 수신하기" << endl << "Set_dir : 파일의 수신 경로지정하기" << endl << "Quit : 종료" << endl;
		char Selection[10]; // 커맨드를 선택하기 위해 문자열 선언
		cin >> Selection;
		// 문자열을 비교하기 위해서 strcmp() 함수 호출
		if (strcmp(Selection, "Send") == 0)
		{
			Send();
		}
		else if (strcmp(Selection, "Receive") == 0)
		{
			Receive();
		}
		else if (strcmp(Selection, "Set_dir") == 0)
		{
			chdir(set_dir()); // 현재 작업 디렉토리를 변경하기 위해서 chdir() 함수 호출
		}
		else if (strcmp(Selection,"Quit") == 0)
		{
			break;
		}
		else
		{
			cout << "정확한 커맨드를 입력하여 주세요" << endl;
		}
	}
	return 0;
}