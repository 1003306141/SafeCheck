#pragma once
//**********************************头文件包含
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

//**********************************库文件包含
#pragma comment(lib, "libcrypto32MT.lib")	// ssl 加密函数
#pragma comment(lib, "libssl32MT.lib")		// ssl 安全信道 
#pragma comment(lib, "ws2_32.lib")		
#pragma comment(lib,"Iphlpapi.lib")

#pragma comment(lib,"Sensapi.lib")  

//**********************************宏定义
#define CONNECT_TIMEOUT		1000
#define MAX_RETRY_TINE		3

#define SSL_CHANNEL_ON		0
#define SSL_CHANNEL_OFF		1
#define SSL_WORKING			0
#define SSL_NOT_WORKING		1

#define MAXBUF				1280
#define CMD_SIZE			3
#define	HEAD_SIZE			7

//**********************************结构体定义

struct SSL_Handler
{
	SSL*     ssl;
	SSL_CTX* ctx;
	SOCKET   sock;
	char     buf[MAXBUF];
};
// 通信结构体
struct HeadPacket
{
	char	cmd[CMD_SIZE];
	char	text[MAXBUF];
};

//**********************************类定义


//**********************************函数声明
//获取有线网卡地址
bool GetWiredMAC_IP(char* wiredMAC, char* wiredIP);

//初始化SSL
int InitSSL(const char *ip, int port);

//向服务器发送信息
bool SendInfo(const char* cmdType, const char* text);

//结束SSL
int EndSSL();

//从服务器获取信息
bool GetReplyInfo(char* info);

//用户认证
bool Authentication(char* ServerIP, char* username, char* password);

//断线重连
bool AutoAuthentication();

//客户端注册
bool RegisterClient();

//从服务器获取指令
bool GetFromServer();

//判断服务器任务队列是否适合上传文件
bool areYouReady(SOCKET& sock, int seq);

//初始化上传文件SOCK
bool initSock(SOCKET &sclient, const char* host, int port);

//上传文件
bool UploadFile(SOCKET& sock, char* filename);

//从服务器换取关键字文件
bool GetKeyFile();

//执行远程命令，全盘扫描多线程
bool RemoteAllScan(char* filename);

//执行远程命令，全盘扫描单线程
bool RemoteAllScan1(char* filename);

//执行远程命令，快速扫描
bool RemoteFastScan(char* filename);

//检测网络是否连通
bool CheckInternet();

//获取服务器IP，用户名等配置信息
bool GetConfig();

//执行远程命令，卸载自身
void RemoteRemoveSelf();

//获取并处理服务器控制命令线程
DWORD _stdcall GetServerCommand(LPVOID);

//心跳测试专用线程
DWORD _stdcall HeartBeat(LPVOID Dlg);




















