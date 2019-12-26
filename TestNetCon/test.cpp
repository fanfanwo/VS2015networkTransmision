#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#define _CRL_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
using namespace std;
/*服务器端*/
int main()
{
	//初始化WinSock库
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsadata;
	if (WSAStartup(sockVersion, &wsadata) != 0)
	{
		cout << "WinSock2 initalise failed!" << endl;
		return 0;
	}
	
	/*创建套接字，用来监听客户端的发送信息状态*/
	//SOCKET slisten = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SOCKET slisten = socket(AF_INET, SOCK_DGRAM, 0);
	if (slisten<0)
	{
		cout << "socket creat fail" << endl;
		exit(1);
	}
	//填充网络信息结构体sockaddr_in
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	//sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.114");
	/*将该套接字绑定到一个sockaddr_in的结构体*/
	if (bind(slisten, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		cout << "bind socket error !" << endl;
		system("pause");
		return 0;
	}
	//请求连接服务器的客户端的网络信息结构体
	sockaddr_in remoteaddr;
	int nlen = sizeof(remoteaddr);
	
	/*进入循环监听状态，使客户端一直处于循环监听客户端，并且接受由客户端发送的数据*/
	while (true)
	{
		char recdata[256] = "";
		int ret = recvfrom(slisten, recdata, sizeof(recdata), 0, (SOCKADDR*)&remoteaddr, &nlen);
		if (ret<0)
		{
			perror("fail to recv");
			exit(1);
		}
		cout << "接受一个连接：ip:" << inet_ntoa(remoteaddr.sin_addr) << ",port:" << ntohs(remoteaddr.sin_port) << endl;
		cout << "data : " << recdata << endl;
		strcat(recdata, "*_*");
		if (sendto(slisten,recdata,strlen(recdata),0,(SOCKADDR*)&remoteaddr, nlen)<0)
		{
			cout << "send back failed!" << endl;
			exit(1);
		}
	}
	closesocket(slisten);
	WSACleanup();
	system("pause");
	return 0;
}
