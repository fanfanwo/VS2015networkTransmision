// server2.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
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

//�����׽���
SOCKET createSocket();
//�ж��ļ��Ƿ����
BOOL IsDirExist(char cdir[]);
//��ȡ�ļ��洢��·��
void piturePath(char name[],char **path);
//����ͼƬ�ļ�����Ž��յ�ͼƬ
void savePicture(FILE* &p, char path[], SOCKET sSocket);
//��ʾ���յ���ͼƬ
void showPicture(char *path);
//������
void errlog(SOCKET sSocket);

struct Picture
{
	long length;
	char buffer[4096];
	int flag;
}picture;

int main() {

	//�����������׽���
	SOCKET sSocket = createSocket();

	//�����ṹ��ַ����
	sockaddr_in serveraddr;
	sockaddr_in clientaddr;
	int slen = sizeof(serveraddr);
	int clen = sizeof(clientaddr);

	//���÷������ĵ�ַ
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	//serveraddr.sin_addr.S_un.S_addr = inet_addr("192.168.0.114");
	serveraddr.sin_addr.S_un.S_addr = inet_addr("192.168.10.52");

	//�ѵ�ַ�󶨵�������
	int ret = ::bind(sSocket, (SOCKADDR*)&serveraddr, slen);
	if (ret == SOCKET_ERROR)
	{
		cout << "bind failed " << endl;
		closesocket(sSocket);
		WSACleanup();
		cout << "2s���˳�����̨��" << endl;
		Sleep(2000);
		exit(0);
	}

	
	char data[200];                        //���ն����ݵĻ�����
	memset(data, 0, sizeof(data));         //��ʼ��������
	char begin[] = "�õģ�׼������ͼƬ��"; //��ʼ��־��Ϣ
	char end[] = "����ͼƬ��ɡ�";         //������־��Ϣ
	int iRcv;                              //����״̬
	int iSend;                             //����״̬
	FILE *p;                               //�ļ�ָ��

										   /* ���͵İ��ϴ󣬳��������߻��浼�¶�����
										   ������mtu size�������������udp�����ܻᳬ�������ߵĻ��壬���¶�����
										   ���������������socket���ջ��塣 */
										   //int nRecvBuf = 128 * 1024;//����Ϊ128K
	// setsockopt(sSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

	//ѭ������ͼƬ
	while (1) {
		cout << "=================================================Server===============================================" << endl;

		//���տͻ��˵Ŀ�ʼ��Ϣ
		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "������Ϣʧ�ܣ�" << endl;
			errlog(sSocket);
		}
		cout << "Client: ip��" << inet_ntoa(clientaddr.sin_addr)<<"port��"<<ntohs(clientaddr.sin_port)<< ">>data��"<<  data << endl;
		memset(data, 0, sizeof(data));

		//���ͻ�����Ϣ���ͻ���
		iSend = sendto(sSocket, begin, strlen(begin), 0, (SOCKADDR*)&clientaddr, clen);
		if (iSend == SOCKET_ERROR) {
			cout << "������Ϣʧ�ܣ�" << endl;
			errlog(sSocket);
		}
		cout << "Server: " << begin << endl;

		//����ͼƬ����
		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "������Ϣʧ�ܣ�" << endl;
			errlog(sSocket);

		}

		cout << "���յ�ͼƬ����: " << data << endl;
		//��ȡͼƬ����ڱ����ľ���·��
		char *path =NULL;
		piturePath(data,&path);
		cout << "path= " << path << endl;
		memset(data, 0, sizeof(data));

		//��ͼƬ
		savePicture(p, path, sSocket);

		cout << "�������������С�������" << endl;
		picture.flag = 0;

		while (!picture.flag) {
			//cout << count << endl;
			//count++;
			memset(picture.buffer, 0, sizeof(picture.buffer));
			iRcv = recvfrom(sSocket, (char*)&picture, sizeof(struct Picture), 0, (SOCKADDR*)&clientaddr, &clen);
			if (iRcv == SOCKET_ERROR) {
				cout << "����ͼƬʧ�ܣ�" << endl;
				errlog(sSocket);
				return -8;
			}
			if (iRcv == 0) //�ͻ����Ѿ��ر�����
			{
				cout<<"Client has closed the connection!"<<endl;
				return 0;
			}

			fwrite(picture.buffer, picture.length, 1, p);

			//����һ�ΰ���ȷ��һ��
			char success[] = "success";
			iSend = sendto(sSocket, success, strlen(success), 0, (SOCKADDR*)&clientaddr, clen);
			if (iSend == SOCKET_ERROR) {
				cout << "������Ϣʧ�ܣ�" << endl;
				errlog(sSocket);
				return -10;
			}
			memset(success, 0, sizeof(success));
			
		}
		cout << "�������������С�������" << endl;
		cout << "��������������ɡ�������" << endl;
		cout << endl;
		Sleep(2000);
		//չʾ���յ���ͼƬ
		showPicture(path);





		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "������Ϣʧ�ܣ�" << endl;
			errlog(sSocket);
			return -9;

		}
		cout << "Client: " << data << endl;
		memset(data, 0, sizeof(data));

		iSend = sendto(sSocket, end, strlen(end), 0, (SOCKADDR*)&clientaddr, clen);
		if (iSend == SOCKET_ERROR) {
			cout << "������Ϣʧ�ܣ�" << endl;
			errlog(sSocket);
			return -10;
		}
		//cout << "Server: " << end << endl;

		iRcv = recvfrom(sSocket, data, sizeof(data), 0, (SOCKADDR*)&clientaddr, &clen);
		if (iRcv == SOCKET_ERROR) {
			cout << "������Ϣʧ�ܣ�" << endl;
			errlog(sSocket);
			return -11;
		}
		cout << "Client: " << data << endl;
		//cout << strcmp(data, "byebye") << endl;
		if (!(strcmp(data, "byebye"))) {
			cout << "2m��رշ��������ӣ�";
			Sleep(200000);
			break;
		}
		memset(data, 0, sizeof(data));
		fclose(p);
		p = NULL;
		cout << endl;
	}

	//�رգ�����
	closesocket(sSocket);
	WSACleanup();
	return 0;

}
//����socket
SOCKET createSocket() {
	WORD version = MAKEWORD(2, 2);
	WSADATA wsadata;
	if (WSAStartup(version, &wsadata))
	{
		cout << "WSAStartup failed " << endl;
		cout << "2s�����̨����رգ�" << endl;
		Sleep(2000);
		exit(0);
	}
	//�жϰ汾
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		cout << "wVersion not 2.2" << endl;
		cout << "2s�����̨����رգ�" << endl;
		Sleep(2000);
		exit(0);
	}

	SOCKET sSocket;
	sSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (SOCKET_ERROR == sSocket)
	{
		cout << "socket failed" << endl;
		cout << "2s�����̨����رգ�" << endl;
		Sleep(2000);
		exit(0);
	}
	else {
		return sSocket;
	}
}
void errlog(SOCKET sSocket)
{
	cout << "2s���˳�����̨��" << endl;
	closesocket(sSocket);
	WSACleanup();
	Sleep(2000);
	exit(0);
}

// �ж��ļ����Ƿ����
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

//ͼƬ�洢��·��
void piturePath(char name[], char **path)
{
	char dir[50] = { 0 };
	while (true)
	{
		cout << "������ͼƬ���·��: " << endl;
		cin >> dir;
		if (IsDirExist(dir)) {
			break;
		}
		else {
			cout << "�ļ�Ŀ¼�����ڣ�" << endl;
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

//��ʾ���յ���ͼƬ
void showPicture(char *path)
{
	Mat recvPicture;
	recvPicture = imread("E:/VS2015Code/h.jpg");
	imshow("���յ���ͼƬ", recvPicture);
	waitKey();
}

void savePicture(FILE* &p, char path[], SOCKET sSocket) {
	// �Զ� / д��ʽ�򿪻���һ���������ļ����������д��
	if (!(p = fopen(path, "wb+"))) {
		cout << "ͼƬ���·������" << endl;
		cout << "2s���˳�����̨��" << endl;
		closesocket(sSocket);
		WSACleanup();
		Sleep(20000);
		exit(0);
	}
	cout << "���ͼƬ�ľ���·��: " << path << endl;
	cout << endl;

}