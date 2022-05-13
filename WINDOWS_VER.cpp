#include <stdlib.h>
#include <iostream>
#include <stdio.h.>
#include <WinSock2.h>
#include <direct.h>
#include <io.h>

#define BUFSIZE 4096

#pragma comment(lib,"Ws2_32.lib")
using namespace std;

class infor {
private:
	char ip[11];
	int port;
public:
	void set_infor(char a[11], int b)
	{
		strcpy(ip, a);
		port = b;
	}

	char *get_ip() {
		return ip;
	}

	int get_port() {
		return port;
	}
};


int isDir(char* s) {
	_finddatai64_t c_file;
	int hFile, result;

	if ((hFile = _findfirsti64(s, &c_file)) == -1L)
		result = 0; // 파일 또는 디렉토리가 없으면 0 반환
	else if (c_file.attrib & _A_SUBDIR)
		result = 1; // 디렉토리면 1 반환

	_findclose(hFile);
	return result;
}


int isFile(char* s) {
	_finddatai64_t c_file;
	int hFile, result;

	if ((hFile = _findfirsti64(s, &c_file)) == -1L)
		result = 0; // 파일 또는 디렉토리가 없으면 0 반환
	else if (c_file.attrib & _A_SUBDIR)
		result = 0; // 디렉토리면 0 반환
	else
		result = 1; // 그밖의 경우는 "존재하는 파일"이기에 0 반환

	_findclose(hFile);
	return result;
}

char &set_file() {
	cout << "파일의 경로를 입력하여 주십시오. (파일명 포함)" << endl;
	char a[256];
	while (1) {
		cin >> a;
		if (isFile(a) == 1)
		{
			break;
		}
		else
		{
			cout << "잘못된 파일 경로 입니다. 다시 입력하여 주십시오. (파일명 포함)" << endl;
		}
	}
	return *a;
}

char &set_dir() {
	cout << "다운로드 경로를 입력하여 주십시오. (각 드라이브의 최상위 경로 EX : C:\ 지정 불가능)" << endl;
	char a[256];
	while (1) {
		cin >> a;
		if (isDir(a) == 1)
		{

			break;
		}
		else
		{
			cout << "잘못된 경로 입니다. 다시 입력하여 주십시오." << endl;
		}
	}
	return *a;
}


int datareceive(SOCKET s, char *buf, int len, int flags) {
	int rec;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		rec = recv(s, ptr, left, flags);
		if (rec == 0)
			break;
		left -= rec;
		ptr += rec;
	}
	return (len - left);
}

int Receive() {
	int retval;
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	infor a;
	cout << "사용하실 포트를 입력하여 주세요." << endl;
	int port;
	cin >> port;
	a.set_infor("",port);


	SOCKADDR_IN servaddr;
	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(a.get_port());
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listen_sock, (SOCKADDR *)&servaddr, sizeof(servaddr));

	listen(listen_sock, SOMAXCONN);

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE];

	while (1) {

		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);

		cout << "클라이언트 접속 IP : " << inet_ntoa(clientaddr.sin_addr) << " 포트 : " << a.get_port() << endl;

		char filename[256];
		ZeroMemory(filename, 256);
		datareceive(client_sock, filename, 256, 0);

		cout << "받을 파일 이름 : " << filename << endl;

		int fsize;
		datareceive(client_sock, (char*)&fsize, sizeof(fsize), 0);
		cout << "받을 파일 크기 : " << fsize << endl;
		FILE *fp;

		char downloaded_file[256];
		cout << "파일을 저장할 경로와 이름을 입력하여 주십시오." << endl;
		cin >> downloaded_file;
		send(client_sock, "fin", sizeof("fin"), 0);
		fopen_s(&fp, downloaded_file, "wb");

		if (fp == NULL)
		{
			cout << "파일 개방 실패!" << endl;
			return 0;
		}
		else
		{
			int fnum = 0;
			while (1) {
				retval = datareceive(client_sock, buf, BUFSIZE, 0);

				if (retval == 0)
				{
					break;
				}
				else
				{
					fwrite(buf, 1, retval, fp);
					fnum += retval;
				}
			}
			fclose(fp);
			if (fnum == fsize)
				cout << "파일 전송 성공" << endl;
			else
				cout << "파일 전송 실패" << endl;
			closesocket(client_sock);
			cout << "클라이언트 종료 IP = " << inet_ntoa(clientaddr.sin_addr) << " 포트 : " << ntohs(clientaddr.sin_port) << endl;
		}
		closesocket(listen_sock);
		WSACleanup();
		return 0;
	}
}


int Send()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	char ip[11];
	int port;
	cout << "접속 할 서버의 ip를 입력하여 주세요." << endl;
	cin >> ip;
	cout << "접속 할 서버의 포트를 입력하여 주세요." << endl;
	cin >> port;
	infor serv;
	serv.set_infor(ip, port);

	SOCKADDR_IN servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(serv.get_port());
	servaddr.sin_addr.s_addr = inet_addr(serv.get_ip());


	char myFile[256] = "";
	strcpy(myFile, &set_file());

	connect(sock, (SOCKADDR*)&servaddr, sizeof(servaddr));
	cout << "서버에 접속 성공:IP = " << ip << ", " << "Port = " << port << endl;

	FILE* fp = fopen(myFile, "rb");
	char filename[256];
	ZeroMemory(filename, 256);
	sprintf(filename, myFile);

	send(sock, filename, 256, 0);
	fseek(fp, 0, SEEK_END);
	int totalbytes = ftell(fp);
	send(sock, (char*)&totalbytes, sizeof(totalbytes), 0);

	char buf[BUFSIZE];
	int numread;
	int numtotal = 0;

	rewind(fp);
	char isfin[4];
	recv(sock, isfin, sizeof("fin"), 0);
	if (strcmp(isfin, "fin") == 0)
	{


		while (1) {
			numread = fread(buf, 1, BUFSIZE, fp);
			if (numread > 0) {
				send(sock, buf, numread, 0);
				numtotal += numread;
			}
			else if (numread == 0 && numtotal == totalbytes) {
				cout << "총" << numtotal << "바이트 파일 전송을 완료했습니다." << endl;
				break;
			}
		}
	}
	else
	{
		Sleep(10);
	}
	fclose(fp);

	closesocket(sock);
	cout << "서버에 접속 종료: IP = " << inet_ntoa(servaddr.sin_addr) << "," << "Port = " << ntohs(servaddr.sin_port) << endl;

	WSACleanup();
	return 0;
}

int main(void)
{
	while (1)
	{
		cout << "커맨드를 입력하여 주세요" << endl << "Send : 파일 송신하기" << endl << "Receive : 파일 수신하기" << endl << "Set_dir : 파일의 수신 경로지정하기" << endl << "Quit : 종료" << endl;
		char Selection[10];
		cin >> Selection;
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
			chdir(&set_dir());
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