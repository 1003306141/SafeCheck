#pragma once
//**********************************ͷ�ļ�����
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#include "Scan.h"
#include "MainClientDlg.h"

#include <WTypes.h>  
#include <Sensapi.h>  

//**********************************���ļ�����
#pragma comment(lib, "libcrypto32MT.lib")	// ssl ���ܺ���
#pragma comment(lib, "libssl32MT.lib")		// ssl ��ȫ�ŵ� 
#pragma comment(lib, "ws2_32.lib")		
#pragma comment(lib,"Iphlpapi.lib")

#pragma comment(lib,"Sensapi.lib")  

//**********************************�궨��
#define CONNECT_TIMEOUT		1000
#define MAX_RETRY_TINE		3

#define SSL_CHANNEL_ON		0
#define SSL_CHANNEL_OFF		1
#define SSL_WORKING			0
#define SSL_NOT_WORKING		1

#define MAXBUF				1280
#define CMD_SIZE			3
#define	HEAD_SIZE			7

//**********************************�ṹ�嶨��

struct SSL_Handler
{
	SSL*     ssl;
	SSL_CTX* ctx;
	SOCKET   sock;
	char     buf[MAXBUF];
};
// ͨ�Žṹ��
struct HeadPacket
{
	char	cmd[CMD_SIZE];
	char	text[MAXBUF];
};

//**********************************�ඨ��


//**********************************��������
//��ȡ����������ַ
bool GetWiredMAC_IP(char* wiredMAC, char* wiredIP);

//��ʼ��SSL
int InitSSL(const char *ip, int port);

//�������������Ϣ
bool SendInfo(const char* cmdType, const char* text);

//����SSL
int EndSSL();

//�ӷ�������ȡ��Ϣ
bool GetReplyInfo(char* info);

//�û���֤
bool Authentication(char* ServerIP, char* username, char* password);

//��������
bool AutoAuthentication();

//�ͻ���ע��
bool RegisterClient();

//�ӷ�������ȡָ��
bool GetFromServer();

//�жϷ�������������Ƿ��ʺ��ϴ��ļ�
bool areYouReady(SOCKET& sock, int seq);

//��ʼ���ϴ��ļ�SOCK
bool initSock(SOCKET &sclient, const char* host, int port);

//�ϴ��ļ�
bool UploadFile(SOCKET& sock, char* filename);

//�ӷ�������ȡ�ؼ����ļ�
bool GetKeyFile();

//ִ��Զ�����ȫ��ɨ����߳�
bool RemoteAllScan(char* filename);

//ִ��Զ�����ȫ��ɨ�赥�߳�
bool RemoteAllScan1(char* filename);

//ִ��Զ���������ɨ��
bool RemoteFastScan(char* filename);

//��������Ƿ���ͨ
bool CheckInternet();

//��ȡ������IP���û�����������Ϣ
bool GetConfig();

//ִ��Զ�����ж������
void RemoteRemoveSelf();

//��ȡ��������������������߳�
DWORD _stdcall GetServerCommand(LPVOID);

//��������ר���߳�
DWORD _stdcall HeartBeat(LPVOID Dlg);




















