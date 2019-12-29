// server2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
//#include <WS2tcpip.h>
#include <string.h>
#include <opencv.hpp>

#pragma comment(lib,"ws2_32.lib")
#define _CRL_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
using namespace std;
using namespace cv;

//创建套接字
SOCKET createSocket();
//判断文件是否存在
BOOL IsDirExist(char cdir[]);
//获取文件存储的路径
void piturePath(char name[],char **path);
//创建图片文件，存放接收的图片
void savePicture(FILE* &p, char path[], SOCKET sSocket);
//显示接收到的图片
void showPicture(char *path);
//错误处理
void errlog(SOCKET sSocket);

struct Picture
{
	long length;
	char buffer[4096];
	int flag;
}picture;

int main() {

	//创建服务器套接字
	SOCKET sSocket = createSocket();

	//创建结构地址变量
	sockaddr_in serveraddr;
	sockaddr_in clientaddr;
	int slen = sizeof(serveraddr);
	int clen = sizeof(clientaddr);

	//设置服务器的地址
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	//serveraddr.sin_addr.S_un.S_addr = inet_addr("192.168.0.114");
	serveraddr.sin_addr.S_un.S_addr = inet_addr("192.168.10.52");

	//把地址绑定到服务器
	int ret = ::bind(sSocket, (SOCKADDR*)&serveraddr, slen);
	if (ret == SOCKET_ERROR)
	{
		cout << "bind failed " << endl;
		closesocket(sSocket);
		WSACleanup();
		cout << "2s后退出控制台！" << endl;
		Sleep(2000);
		exit(0);
	}

	
	char data[200];                        //接收短数据的缓冲区
	memset(data, 0, sizeof(data));         //初始化缓冲区
	char begin[] = "好的，准备接受图片。"; //开始标志消息
	char end[] = "接收图片完成。";         //结束标志消息
	int iRcv;                              //接受状态
	int iSend;                             //发送状态
	FILE *p;                               //文件指针

										   /* 发送的包较大，超过接受者缓存导致丢包：
										   包超过mtu size数倍，几个大的udp包可能会超过接收者的缓冲，导致丢包。
										   这种情况可以设置socket接收缓冲。 */
										   //int nRecvBuf = 128 * 1024;//设置为128K
	// setsockopt(sSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

	//循环接收图片
	while (1) {
		cout << "=================================================Server===============================================" << endl;

		//接收客户端的开始消息
		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "接受信息失败！" << endl;
			errlog(sSocket);
		}
		cout << "Client: ip：" << inet_ntoa(clientaddr.sin_addr)<<"port："<<ntohs(clientaddr.sin_port)<< ">>data："<<  data << endl;
		memset(data, 0, sizeof(data));

		//发送回馈消息给客户端
		iSend = sendto(sSocket, begin, strlen(begin), 0, (SOCKADDR*)&clientaddr, clen);
		if (iSend == SOCKET_ERROR) {
			cout << "发送信息失败！" << endl;
			errlog(sSocket);
		}
		cout << "Server: " << begin << endl;

		//接收图片名字
		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "接受信息失败！" << endl;
			errlog(sSocket);

		}

		cout << "接收的图片名字: " << data << endl;
		//获取图片存放在本机的绝对路径
		char *path =NULL;
		piturePath(data,&path);
		cout << "path= " << path << endl;
		memset(data, 0, sizeof(data));

		//打开图片
		savePicture(p, path, sSocket);

		cout << "····接收中····" << endl;
		picture.flag = 0;

		while (!picture.flag) {
			//cout << count << endl;
			//count++;
			memset(picture.buffer, 0, sizeof(picture.buffer));
			iRcv = recvfrom(sSocket, (char*)&picture, sizeof(struct Picture), 0, (SOCKADDR*)&clientaddr, &clen);
			if (iRcv == SOCKET_ERROR) {
				cout << "接受图片失败！" << endl;
				errlog(sSocket);
				return -8;
			}
			if (iRcv == 0) //客户端已经关闭连接
			{
				cout<<"Client has closed the connection!"<<endl;
				return 0;
			}

			fwrite(picture.buffer, picture.length, 1, p);

			//接受一次包就确认一次
			char success[] = "success";
			iSend = sendto(sSocket, success, strlen(success), 0, (SOCKADDR*)&clientaddr, clen);
			if (iSend == SOCKET_ERROR) {
				cout << "发送信息失败！" << endl;
				errlog(sSocket);
				return -10;
			}
			memset(success, 0, sizeof(success));
			
		}
		cout << "····接收中····" << endl;
		cout << "····接收完成····" << endl;
		cout << endl;
		Sleep(2000);
		//展示接收到的图片
		showPicture(path);





		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "接受信息失败！" << endl;
			errlog(sSocket);
			return -9;

		}
		cout << "Client: " << data << endl;
		memset(data, 0, sizeof(data));

		iSend = sendto(sSocket, end, strlen(end), 0, (SOCKADDR*)&clientaddr, clen);
		if (iSend == SOCKET_ERROR) {
			cout << "发送信息失败！" << endl;
			errlog(sSocket);
			return -10;
		}
		//cout << "Server: " << end << endl;

		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "接受信息失败！" << endl;
			errlog(sSocket);
			return -11;
		}
		cout << "Client: " << data << endl;
		//cout << strcmp(data, "byebye") << endl;
		if (!(strcmp(data, "byebye"))) {
			cout << "2m后关闭服务器连接！";
			Sleep(200000);
			break;
		}
		memset(data, 0, sizeof(data));
		fclose(p);
		p = NULL;
		cout << endl;
	}

	//关闭，清理
	closesocket(sSocket);
	WSACleanup();
	return 0;

}
//创建socket
SOCKET createSocket() {
	WORD version = MAKEWORD(2, 2);
	WSADATA wsadata;
	if (WSAStartup(version, &wsadata))
	{
		cout << "WSAStartup failed " << endl;
		cout << "2s后控制台将会关闭！" << endl;
		Sleep(2000);
		exit(0);
	}
	//判断版本
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		cout << "wVersion not 2.2" << endl;
		cout << "2s后控制台将会关闭！" << endl;
		Sleep(2000);
		exit(0);
	}

	SOCKET sSocket;
	sSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (SOCKET_ERROR == sSocket)
	{
		cout << "socket failed" << endl;
		cout << "2s后控制台将会关闭！" << endl;
		Sleep(2000);
		exit(0);
	}
	else {
		return sSocket;
	}
}
void errlog(SOCKET sSocket)
{
	cout << "2s后退出控制台！" << endl;
	closesocket(sSocket);
	WSACleanup();
	Sleep(2000);
	exit(0);
}

// 判断文件夹是否存在
BOOL IsDirExist(char cdir[])
{
	string dir(cdir);
	size_t origsize = dir.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(dir.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, dir.c_str(), _TRUNCATE);
	DWORD dwAttrib = GetFileAttributes(wcstring);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//图片存储的路径
void piturePath(char name[], char **path)
{
	char dir[50] = { 0 };
	while (true)
	{
		cout << "请输入图片存放路径: " << endl;
		cin >> dir;
		if (IsDirExist(dir)) {
			break;
		}
		else {
			cout << "文件目录不存在！" << endl;
		}
	}
	int k = 0;
	int nlen = strlen(name);
	int dlen = strlen(dir);

	*path = (char *)malloc(dlen);
	memcpy(*path,dir, dlen);
	memcpy(*path+dlen, name, nlen);

	cout <<"path:" <<*path<< endl;
}

//显示接收到的图片
void showPicture(char *path)
{
	Mat recvPicture;
	recvPicture = imread("E:/VS2015Code/h.jpg");
	imshow("接收到的图片", recvPicture);
	waitKey();
}

void savePicture(FILE* &p, char path[], SOCKET sSocket) {
	// 以读 / 写方式打开或建立一个二进制文件，允许读和写。
	if (!(p = fopen(path, "wb+"))) {
		cout << "图片存放路径出错！" << endl;
		cout << "2s后退出控制台！" << endl;
		closesocket(sSocket);
		WSACleanup();
		Sleep(20000);
		exit(0);
	}
	cout << "存放图片的绝对路径: " << path << endl;
	cout << endl;

}