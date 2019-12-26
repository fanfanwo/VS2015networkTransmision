#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#define _CRL_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
using namespace std;
/*��������*/
int main()
{
	//��ʼ��WinSock��
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsadata;
	if (WSAStartup(sockVersion, &wsadata) != 0)
	{
		cout << "WinSock2 initalise failed!" << endl;
		return 0;
	}
	
	/*�����׽��֣����������ͻ��˵ķ�����Ϣ״̬*/
	//SOCKET slisten = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SOCKET slisten = socket(AF_INET, SOCK_DGRAM, 0);
	if (slisten<0)
	{
		cout << "socket creat fail" << endl;
		exit(1);
	}
	//���������Ϣ�ṹ��sockaddr_in
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	//sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.114");
	/*�����׽��ְ󶨵�һ��sockaddr_in�Ľṹ��*/
	if (bind(slisten, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		cout << "bind socket error !" << endl;
		system("pause");
		return 0;
	}
	//�������ӷ������Ŀͻ��˵�������Ϣ�ṹ��
	sockaddr_in remoteaddr;
	int nlen = sizeof(remoteaddr);
	
	/*����ѭ������״̬��ʹ�ͻ���һֱ����ѭ�������ͻ��ˣ����ҽ����ɿͻ��˷��͵�����*/
	while (true)
	{
		char recdata[256] = "";
		int ret = recvfrom(slisten, recdata, sizeof(recdata), 0, (SOCKADDR*)&remoteaddr, &nlen);
		if (ret<0)
		{
			perror("fail to recv");
			exit(1);
		}
		cout << "����һ�����ӣ�ip:" << inet_ntoa(remoteaddr.sin_addr) << ",port:" << ntohs(remoteaddr.sin_port) << endl;
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
