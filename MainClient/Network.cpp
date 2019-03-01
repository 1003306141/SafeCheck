#include "stdafx.h"
#include "network.h"


static SSL_Handler hdl = { 0 };

bool GetWiredMAC_IP(char* wiredMAC, char* wiredIP)
{
	//申请10个网卡空间
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO[10];
	unsigned long stSize = sizeof(IP_ADAPTER_INFO) * 10;
	//获取所有网卡信息，参数二为输入输出参数
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	//空间不足
	if (nRel == ERROR_BUFFER_OVERFLOW)
	{
		if (pIpAdapterInfo != NULL)
			delete[] pIpAdapterInfo;
		return FALSE;
	}
	PIP_ADAPTER_INFO cur = pIpAdapterInfo;
	while (cur)
	{
		//printf("网卡描述：%s\n", cur->Description);
		switch (cur->Type)//00-E0-7D-68-00-06
		{
		case MIB_IF_TYPE_OTHER:break;
		case MIB_IF_TYPE_ETHERNET:
		{
			sprintf(wiredMAC, "%02X:%02X:%02X:%02X:%02X:%02X",
				cur->Address[0], cur->Address[1],
				cur->Address[2], cur->Address[3],
				cur->Address[4], cur->Address[5]);
			sprintf(wiredIP, "%s", cur->IpAddressList.IpAddress.String);
			if (pIpAdapterInfo != NULL)
				delete[] pIpAdapterInfo;
			return TRUE;
			//printf("有线网卡\n");
			//printf("IP地址：%s\n", cur->IpAddressList.IpAddress.String);
			//printf("子网掩码：%s\n", cur->IpAddressList.IpMask.String);
			//printf("MAC地址：%02X:%02X:%02X:%02X:%02X:%02X\n",cur->Address[0], cur->Address[1], cur->Address[2], cur->Address[3], cur->Address[4], cur->Address[5]);
		}break;
		case MIB_IF_TYPE_TOKENRING:break;
		case MIB_IF_TYPE_FDDI:break;
		case MIB_IF_TYPE_PPP:break;
		case MIB_IF_TYPE_LOOPBACK:break;
		case MIB_IF_TYPE_SLIP:break;
		default://无线网卡或者Unknown type
		{
		}break;
		}
		cur = cur->Next;
		//printf("--------------------------\n");
	}
	return FALSE;
}

int InitSSL(const char *ip, int port)
{
	int ret;
	int cnt;

	SOCKADDR_IN addrSrv;

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	hdl.ctx = SSL_CTX_new(SSLv23_client_method());

	if (hdl.ctx == NULL)
	{
		ERR_print_errors_fp(stdout);
		exit(1);
	}

	/* start initialize windows socket library */
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0) {
		return SSL_CHANNEL_OFF;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return SSL_CHANNEL_OFF;
	}
	/* end initialize windows socket library */

	/* start  establish a traditional TCP connection to server */
	hdl.sock = socket(AF_INET, SOCK_STREAM, 0);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(ip);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	//    printf("socket created\n");
	//printf("connecting to %s:%d\n", SERV_ADDR, SERV_PORT);

	cnt = 0;
	while (cnt < MAX_RETRY_TINE) {
		ret = connect(hdl.sock, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (ret < 0)
		{
			//ERR_print_errors_fp(stdout);
			printf("connect error : %d\n", WSAGetLastError());
			return -1;
		}
		else if (ret != 0)
		{
			cnt += 1;
			printf("Connect Failed\n");
			ret = CONNECT_TIMEOUT * cnt;
			Sleep(ret);
			printf("reconnect to %s:%d %d/%d times...\n", ip, port, cnt, MAX_RETRY_TINE);
		}
		else {
			break;
		}
	}
	if (cnt >= MAX_RETRY_TINE) {
		//printf("remote server is not listening on %s:%d\n", GS_acfg.ServAddr, GS_acfg.ServPort);
		printf("remote server is not listening on XXXXXXX\n");
		//InformUser(CONNECT_FAILED);本地TCP通知窗口程序连接失败
		return SSL_CHANNEL_OFF;
	}
	else {
		printf("ssl channel established successfully!\n");
	}
	/* TCP connection established */


	/* start establish a SSL channel upon the previous TCP connection */
	hdl.ssl = SSL_new(hdl.ctx);
	SSL_set_fd(hdl.ssl, hdl.sock);
	if (SSL_connect(hdl.ssl) == -1) {
		ERR_print_errors_fp(stderr);
	}
	/* end create SSL channel */

	return SSL_CHANNEL_ON;
}

bool SendInfo(const char* cmdType, const char* text)
{
	int pktSize = HEAD_SIZE + strlen(text);
	char* tmpBuf = (char*)malloc(100);

	sprintf(tmpBuf, "%s0000", cmdType);
	sprintf(tmpBuf + HEAD_SIZE, "%s", text);

	int ret = SSL_write(hdl.ssl, tmpBuf, pktSize);
	if (ret <= 0)
	{
		printf("sendinfo error!\n");
		free(tmpBuf);
		return FALSE;
	}
	free(tmpBuf);
	return TRUE;
}

bool GetReplyInfo(char* info)
{
	int receivedSize = 0;
	int restPktSize = 0;
	char command[HEAD_SIZE] = { 0 };


	receivedSize = SSL_read(hdl.ssl, command, HEAD_SIZE);
	if (receivedSize == -1)
		printf("错误代码是%d\n", WSAGetLastError());
	if (receivedSize == 0)
		return FALSE;
	if (receivedSize != HEAD_SIZE)
	{
		printf("Receiving Failed!\n");
		return FALSE;
	}
	
	restPktSize = *(unsigned char*)(command + HEAD_SIZE - 1);
	char* content = (char*)malloc(restPktSize + 1);
	memset(content, 0, restPktSize + 1);
	SSL_read(hdl.ssl, content, restPktSize);
	sprintf(info, "%s\0", content);

	free(content);
	return TRUE;
}

bool Authentication(char* ServerIP, char* username, char* password)
{
	char info[30];

	if (InitSSL(ServerIP, 50005) == -1)
	{
		MessageBox(0, "连接失败！", 0, 0);
		return FALSE;
	}
	GetReplyInfo(info);
	if (strcmp(info, "WHO ARE YOU") != 0)
		return FALSE;
	
	//构造指定格式的认证信息
	char wiredMAC[20], wiredIP[20], ATHinfo[50];
	if (!GetWiredMAC_IP(wiredMAC, wiredIP))
		return FALSE;
	sprintf(ATHinfo, "%s\n%s\n%s\n%s", username, password, wiredMAC, wiredIP);
	SendInfo("ATH", ATHinfo);

	GetReplyInfo(info);

	//检查当前账号是否存在
	if (strcmp(info, "INVALID CLIENT") == 0)
	{
		MessageBox(0, "INVALID CLIENT", 0, 0);
		return FALSE;
	}
	//检查当前账号是否在线
	if (strcmp(info, "ALREADY LOGIN") == 0)
	{
		MessageBox(0, "ALREADY LOGIN", 0, 0);
		return FALSE;
	}
	//检查密码
	if (strcmp(info, "WRONG PASSWD") == 0)
	{
		MessageBox(0, "WRONG PASSWD", 0, 0);
		return FALSE;
	}
	//检查 mac 是否与注册时的一致
	if (strcmp(info, "NO_LOGIN") == 0)
	{
		MessageBox(0, "NO_LOGIN", 0, 0);
		return FALSE;
	}
	//检查 mac 是否与注册时的一致
	if (strcmp(info, "MAC_DIFF") == 0)
	{
		MessageBox(0, "MAC_DIFF", 0, 0);
		return FALSE;
	}
	// 首次登陆，需要注册
	if (strcmp(info, "NEED REGISTER") == 0)
	{
		if (!RegisterClient())
		{
			MessageBox(0, "注册失败", 0, 0);
			return FALSE;
		}
	}

	//获取服务器发送的”文件上传服务器信息“---------其实已经没什么吊用了---------114.215.19.63 50007 3
	GetReplyInfo(info);
	return TRUE;
}

bool RegisterClient()
{
	char info[50];
	char wiredMAC[20], wiredIP[20];
	char athInfo[100] = { 0 };

	GetWiredMAC_IP(wiredMAC, wiredIP);
	sprintf(athInfo, "MAC:1 %s-wired\nHDS:2 16L782DFS-HDS1 20180108GK2301A097-HDS2\n",wiredMAC);

	SendInfo("ATH", athInfo);
	GetReplyInfo(info);
	if (strcmp("OK", info) == 0)
		return TRUE;
	return FALSE;
	/*
	MAC:1 00:E0:7D:68:00:06-wired
	HDS:2 17L782DFS-HDS1 20190108GK2301A097-HDS2

	MAC:1 0A:0B:0C:0D:0E:0F-wired\nHDS:2 16L782DFS-HDS1 20180108GK2301A097-HDS2\n

	SerialNumber
	17L782DFS
	20190108GK2301A097
	*/
}

bool GetFromServer(char* username)
{
	static	FD_SET fdRead;

	static TIMEVAL	tv = { 0, 2000 };//设置超时等待时间

	FD_ZERO(&fdRead);
	FD_SET(hdl.sock, &fdRead);

	//只处理read事件，不过后面还是会有读写消息发送的
	int nRet = select(0, &fdRead, NULL, NULL, &tv);

	if (nRet == 0)
	{
		//没有连接或者没有读事件
		return false;
	}

	if (nRet > 0)
	{
		char info[50];
		GetReplyInfo(info);
		//全盘扫描
		if (strcmp(info, "003#") == 0)
		{
			SendInfo("COK", "executing task");
			char filename[40] = { 0 };
			sprintf(filename, "first-%s.rlog", username);
			RemoteAllScan(filename);
		}
		//快速扫描
		if (strcmp(info, "006#") == 0)
		{
			SendInfo("COK", "executing task");
			char filename[40] = { 0 };
			sprintf(filename, "second-%s.rlog", username);
			RemoteFastScan(filename);
		}
		//远程卸载
		if (strcmp(info, "005#") == 0)
		{
			MessageBox(0, "远程卸载", 0, 0);
		}
	}

	return false;
}

bool initSock(SOCKET &sclient, const char* host, int port)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return false;
	}

	sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return false;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(host);
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		perror(host);
		closesocket(sclient);
		return false;
	}

	return true;
}

bool areYouReady(SOCKET& sock, int seq)
{
	static char *host = "114.115.244.171";
	static int port = 50007;
	static char buf[8];
	int waitingTasks = 5;
	bool bRet = false;

	initSock(sock, host, port);

	// 发送自己的任务编号
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", seq);
	send(sock, buf, strlen(buf), 0);

	// 获取等待任务数量
	memset(buf, 0, sizeof(buf));
	recv(sock, buf, sizeof(buf), 0);
	waitingTasks = atoi(buf);

	// 如果等待任务数量为 0，说明服务器此时空闲，可以文件上传
	if (waitingTasks <= 0)
	{
		bRet = true;
	}
	else
	{
		closesocket(sock);
	}

	return bRet;
}

bool UploadFile(SOCKET& sock,char* filename)
{
	static char tmpBuf[2048];
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return FALSE;
	fseek(fp, 0, SEEK_END);
	int restSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// 调试,计算发送次数
	int cnt = 0;
	int readSize = 0;

	while (restSize > 0)
	{
		memset(tmpBuf, 0, sizeof(tmpBuf));
		readSize = fread(tmpBuf, 1, MAXBUF, fp);
		send(sock, tmpBuf, readSize, 0);
		cnt += 1;
		restSize -= readSize;
	}
	fclose(fp);

	return true;
}

bool RemoteAllScan(char* filename)
{
	char info[50];
	Scaner::alldiskscan();
	MoveFile("first.rlog", filename);
	SendInfo("UPD", filename);
	SendInfo("RPL", "12345678123456781234567812345678");
	GetReplyInfo(info);

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return FALSE;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp);
	char buf[100] = { 0 };
	sprintf(buf, "%d default_pass", size);
	SendInfo("RPL", buf);
	GetReplyInfo(info);

	//上传文件
	SOCKET sock;
	if (areYouReady(sock, 1))      
	{
		UploadFile(sock, filename);
		remove(filename);
	}
}

bool RemoteFastScan(char* filename)
{
	char info[50];
	Scaner::fastscan();
	MoveFile("second.rlog", filename);
	SendInfo("UPD", filename);
	SendInfo("RPL", "12345678123456781234567812345678");
	GetReplyInfo(info);

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return FALSE;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp);
	char buf[100] = { 0 };
	sprintf(buf, "%d default_pass", size);
	SendInfo("RPL", buf);
	GetReplyInfo(info);

	//上传文件
	SOCKET sock;
	if (areYouReady(sock, 1))
	{
		UploadFile(sock, filename);
		remove(filename);
	}
}

DWORD _stdcall GerServerCommand(LPVOID username)
{
	//循环从服务器获取命令
	while (1)
	{
		GetFromServer((char*)username);
		Sleep(1000);
	}
}






















