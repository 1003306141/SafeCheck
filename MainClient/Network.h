#pragma once
//**********************************ͷ�ļ�����
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

//**********************************���ļ�����
#pragma comment(lib, "libcrypto32MT.lib")	// ssl ���ܺ���
#pragma comment(lib, "libssl32MT.lib")		// ssl ��ȫ�ŵ� 
#pragma comment(lib, "ws2_32.lib")		
#pragma comment(lib,"Iphlpapi.lib")

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

//�ӷ�������ȡ��Ϣ
bool GetReplyInfo(char* info);

//�û���֤
bool Authentication(char* ServerIP, char* username, char* password);

//�ͻ���ע��
bool RegisterClient();






















