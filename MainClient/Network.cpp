#include "stdafx.h"
#include "network.h"

//����ֻ�ܷ��������֪��Ϊʲô������ͷ�ļ���ͻ��ظ�����
#include <openssl/applink.c>

SSL_Handler hdl = { 0 };

char serverIP[40] = { 0 };
char Port[10] = { 0 };
char username[40] = { 0 };
bool isConnect;
bool isScan = FALSE;

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
		//InformUser(CONNECT_FAILED);����TCP֪ͨ���ڳ�������ʧ��
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

int EndSSL()
{
	if (NULL == hdl.ssl) {
		return 0;
	}
	SSL_shutdown(hdl.ssl);
	SSL_free(hdl.ssl);
	closesocket(hdl.sock);
	WSACleanup();   //end socket
	SSL_CTX_free(hdl.ctx);
	memset(&hdl, 0, sizeof(hdl));
	return 0;
}

bool SendInfo(const char* cmdType, const char* text)
{
	int pktSize = HEAD_SIZE + strlen(text);
	char* tmpBuf = (char*)malloc(200);

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

	if (hdl.ssl == NULL)
		return FALSE;
	receivedSize = SSL_read(hdl.ssl, command, HEAD_SIZE);
	if (receivedSize == -1)
		printf("���������%d\n", WSAGetLastError());
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

bool AutoAuthentication()
{
	char info[30];

	if (InitSSL(serverIP, 50005) == -1)
		return FALSE;

	GetReplyInfo(info);
	if (strcmp(info, "WHO ARE YOU") != 0)
		return FALSE;

	//����ָ����ʽ����֤��Ϣ
	char wiredMAC[20], wiredIP[20], ATHinfo[50];
	if (!GetWiredMAC_IP(wiredMAC, wiredIP))
		return FALSE;
	sprintf(ATHinfo, "%s\n%s\n%s\n%s", username, "1234567", wiredMAC, wiredIP);
	SendInfo("ATH", ATHinfo);

	GetReplyInfo(info);

	//��鵱ǰ�˺��Ƿ����
	if (strcmp(info, "INVALID CLIENT") == 0)
		return FALSE;

	//��鵱ǰ�˺��Ƿ�����
	if (strcmp(info, "ALREADY LOGIN") == 0)
		return FALSE;

	//�������
	if (strcmp(info, "WRONG PASSWD") == 0)
		return FALSE;

	//��� mac �Ƿ���ע��ʱ��һ��
	if (strcmp(info, "NO_LOGIN") == 0)
		return FALSE;

	//��� mac �Ƿ���ע��ʱ��һ��
	if (strcmp(info, "MAC_DIFF") == 0)
		return FALSE;

	//---------�����ʵ�Ѿ�ûʲô����---------114.215.19.63 50007 3
	GetReplyInfo(info);
	return TRUE;
}

VOID GetWiredIp(char* wired_ip)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD computer_name_size = sizeof(computer_name);
	GetComputerName(computer_name, &computer_name_size);

	hostent* host_info = gethostbyname(computer_name);
	strcpy(
		wired_ip,
		inet_ntoa(*(struct in_addr *)*host_info->h_addr_list)
	);

	WSACleanup();
}

bool Authentication(char* ServerIP, char* username, char* password)
{
	char info[30];

	if (InitSSL(ServerIP, 50005) == -1)
	{
		MessageBox(0, "����ʧ�ܣ�", "ʧ��", 0);
		return FALSE;
	}

	GetReplyInfo(info);
	if (strcmp(info, "WHO ARE YOU") != 0)
		return FALSE;
	
	//����ָ����ʽ����֤��Ϣ
	char wiredMAC[20], wiredIP[20], ATHinfo[50];
	if (!GetWiredMAC_IP(wiredMAC, wiredIP))
		return FALSE;
	GetWiredIp(wiredIP);

	sprintf(ATHinfo, "%s\n%s\n%s\n%s", username, password, wiredMAC, wiredIP);
	SendInfo("ATH", ATHinfo);

	GetReplyInfo(info);

	//��鵱ǰ�˺��Ƿ����
	if (strcmp(info, "INVALID CLIENT") == 0)
	{
		MessageBox(0, "�û���������", "ʧ��", 0);
		return FALSE;
	}
	//��鵱ǰ�˺��Ƿ�����
	if (strcmp(info, "ALREADY LOGIN") == 0)
	{
		MessageBox(0, "�Ѿ���¼", "ʧ��", 0);
		return FALSE;
	}
	//�������
	if (strcmp(info, "WRONG PASSWD") == 0)
	{
		MessageBox(0, "�������", "ʧ��", 0);
		return FALSE;
	}
	//��� mac �Ƿ���ע��ʱ��һ��
	if (strcmp(info, "NO_LOGIN") == 0)
	{
		MessageBox(0, "NO_LOGIN", "ʧ��", 0);
		return FALSE;
	}
	//��� mac �Ƿ���ע��ʱ��һ��
	if (strcmp(info, "MAC_DIFF") == 0)
	{
		MessageBox(0, "MAC��ַ��������˲�ƥ��", "ʧ��", 0);
		return FALSE;
	}
	// �״ε�½����Ҫע��
	if (strcmp(info, "NEED REGISTER") == 0)
	{
		if (!RegisterClient())
		{
			MessageBox(0, "ע��ʧ��", "ʧ��", 0);
			return FALSE;
		}
	}

	//��ȡ���������͵ġ��ļ��ϴ���������Ϣ��---------��ʵ�Ѿ�ûʲô������---------114.215.19.63 50007 3
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

bool GetWiredMAC_IP(char* wiredMAC, char* wiredIP)
{
	//����10�������ռ�
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO[10];
	unsigned long stSize = sizeof(IP_ADAPTER_INFO) * 10;
	//��ȡ����������Ϣ��������Ϊ�����������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	//�ռ䲻��
	if (nRel == ERROR_BUFFER_OVERFLOW)
	{
		if (pIpAdapterInfo != NULL)
			delete[] pIpAdapterInfo;
		return FALSE;
	}
	PIP_ADAPTER_INFO cur = pIpAdapterInfo;
	while (cur)
	{
		//printf("����������%s\n", cur->Description);
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
			if (strcmp(wiredIP, "0.0.0.0") == 0)
				strcpy(wiredIP, "192.168.1.1");
			if (pIpAdapterInfo != NULL)
				delete[] pIpAdapterInfo;
			return TRUE;
			//printf("��������\n");
			//printf("IP��ַ��%s\n", cur->IpAddressList.IpAddress.String);
			//printf("�������룺%s\n", cur->IpAddressList.IpMask.String);
			//printf("MAC��ַ��%02X:%02X:%02X:%02X:%02X:%02X\n",cur->Address[0], cur->Address[1], cur->Address[2], cur->Address[3], cur->Address[4], cur->Address[5]);
		}break;
		case MIB_IF_TYPE_TOKENRING:break;
		case MIB_IF_TYPE_FDDI:break;
		case MIB_IF_TYPE_PPP:break;
		case MIB_IF_TYPE_LOOPBACK:break;
		case MIB_IF_TYPE_SLIP:break;
		default://������������Unknown type
		{
		}break;
		}
		cur = cur->Next;
		//printf("--------------------------\n");
	}
	return FALSE;
}

bool GetFromServer()
{
	static	FD_SET fdRead;

	static TIMEVAL	tv = { 0, 500 };//���ó�ʱ�ȴ�ʱ��

	FD_ZERO(&fdRead);
	FD_SET(hdl.sock, &fdRead);

	//ֻ����read�¼����������滹�ǻ��ж�д��Ϣ���͵�
	int nRet = select(0, &fdRead, NULL, NULL, &tv);

	if (nRet == 0)
	{
		//û�����ӻ���û�ж��¼�
		return false;
	}

	if (nRet > 0)
	{
		char info[50];
		GetReplyInfo(info);
		//ȫ��ɨ��
		if (strcmp(info, "003#") == 0)
		{
			isScan = TRUE;
			SendInfo("COK", "executing task");
			char filename[40] = { 0 };
			sprintf(filename, "first-%s.rlog", username);
			if (isMultiple == 1)
				RemoteAllScan(filename);//���߳�
			else RemoteAllScan1(filename);//���߳�
		}
		//����ɨ��
		if (strcmp(info, "006#") == 0)
		{
			isScan = TRUE;
			SendInfo("COK", "executing task");
			char filename[40] = { 0 };
			sprintf(filename, "second-%s.rlog", username);
			RemoteFastScan(filename);
		}
		//Զ��ж��
		if (strcmp(info, "005#") == 0)
		{
			EndSSL();
			RemoteRemoveSelf();
		}
	}
	return FALSE;
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
	static char *host = serverIP;
	static int port = 50007;
	static char buf[8];
	int waitingTasks = 5;
	bool bRet = false;

	initSock(sock, host, port);

	// �����Լ���������
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", seq);
	send(sock, buf, strlen(buf), 0);

	// ��ȡ�ȴ���������
	memset(buf, 0, sizeof(buf));
	recv(sock, buf, sizeof(buf), 0);
	waitingTasks = atoi(buf);

	// ����ȴ���������Ϊ 0��˵����������ʱ���У������ļ��ϴ�
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

	// ����,���㷢�ʹ���
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

bool GetKeyFile()
{
	char info[100];
	SendInfo("DNF", "Specialkeywords.txt");
	GetReplyInfo(info);

	//�жϷ������Ƿ�����ļ�
	if (strcmp(info, "OK") != 0)
		return FALSE;

	//��ȡ�ļ���ϣ
	GetReplyInfo(info);

	//����ָ��Ҫ���ȡ�ļ�
	SendInfo("RPL", "BEGIN");

	//��ȡ�ļ���С
	GetReplyInfo(info);
	int restSize = atoi(info);
	int recvSize = 0;

	//�ӷ�������ȡ�ؼ����ļ�
	FILE* fp = fopen("Fullkeywords.txt", "wb");
	if (fp == NULL)
		return FALSE;

	char tmpBuf[2048] = { 0 };
	while (restSize > 0)
	{
		memset(tmpBuf, 0, sizeof(tmpBuf));
		recvSize = SSL_read(hdl.ssl, tmpBuf, MAXBUF);
		fwrite(tmpBuf, 1, recvSize, fp);
		restSize -= recvSize;
	}
	fclose(fp);
	return TRUE;
}

bool CheckInternet()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKADDR_IN addrSrv;
	int err;
	int ret = TRUE;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return FALSE;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return FALSE;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(50005);

	addrSrv.sin_addr.S_un.S_addr = inet_addr(serverIP);

	timeval tm;
	fd_set set;
	unsigned long ul = 1;
	ioctlsocket(sockClient, FIONBIO, &ul); //����Ϊ������ģʽ

	if (connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == -1)
	{
		tm.tv_sec = 1;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockClient, &set);
		if (select(sockClient + 1, NULL, &set, NULL, &tm) > 0)
		{

			int error = -1;
			int len = sizeof(int);
			getsockopt(sockClient, SOL_SOCKET, SO_ERROR, (char *)&error, /*(socklen_t *)*/&len);
			if (error != 0)
				ret = FALSE;
		}
		else
			ret = FALSE;
	}


	closesocket(sockClient);
	WSACleanup();
	return ret;
}

bool GetConfig()
{
	FILE* fp = fopen("config.ini", "r");
	if (fp == NULL)
		return FALSE;
	fscanf(fp, "%s%s", serverIP, Port);
	fscanf(fp, "%s", username);
	fclose(fp);
}

void RemoteRemoveSelf()
{
	char dirPath[1024];
	GetCurrentDirectory(1024, dirPath);
	char rmCommand[1024];
	sprintf(rmCommand, "rmdir /s /q \"%s\"", dirPath);

	FILE* fp = fopen("remove.bat", "w");
	fprintf(fp, "taskkill /f /im mainclient.exe\ntaskkill /f /im userui.exe\n%s\nattrib -h -s -r -a %0\ndel %% 0", rmCommand);
	fclose(fp);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cbReserved2 = NULL;
	si.lpReserved2 = NULL;
	CreateProcess("remove.bat", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	CloseHandle(pi.hProcess);

	/*
	[���������ɾ������]
	attrib -h -s -r -a %0
	del %0
	*/
}

//ȫ�̶��߳�ɨ��
bool RemoteAllScan(char* filename)
{
	//ɨ��֮ǰɾ����һ��ɨ����ļ�
	remove(filename);
	//�ӷ�������ȡ�ؼ�����Ϣ
	if (!GetKeyFile())
		return FALSE;

	char info[50];
	Scaner::alldiskscan();
	isScan = FALSE;
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

	//�ϴ��ļ�
	SOCKET sock;
	if (areYouReady(sock, 1))
	{
		UploadFile(sock, filename);
	}
}

//ȫ�̵��߳�ɨ��
bool RemoteAllScan1(char* filename)
{
	//ɨ��֮ǰɾ����һ��ɨ����ļ�
	remove(filename);
	//�ӷ�������ȡ�ؼ�����Ϣ
	if (!GetKeyFile())
		return FALSE;

	char info[50];
	Scaner::alldiskscannormal();
	isScan = FALSE;
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

	//�ϴ��ļ�
	SOCKET sock;
	if (areYouReady(sock, 1))
	{
		UploadFile(sock, filename);
	}
}

//����·����ɨ��
bool RemoteFastScan(char* filename)
{
	//ɨ��֮ǰɾ����һ��ɨ����ļ�
	remove(filename);
	//�ӷ�������ȡ�ؼ�����Ϣ
	if (!GetKeyFile())
		return FALSE;

	char info[50];
	Scaner::fastscan();
	isScan = FALSE;
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

	//�ϴ��ļ�
	SOCKET sock;
	if (areYouReady(sock, 1))
	{
		UploadFile(sock, filename);
	}
}

DWORD _stdcall GetServerCommand(LPVOID Dlg)
{
	GetConfig();
	if (isTray == 1)
	{
		Shell_NotifyIcon(NIM_DELETE, &((CMainClientDlg*)Dlg)->m_nid);
		((CMainClientDlg*)Dlg)->InitTray(1);
	}
	isConnect = TRUE;

	while (1)
	{
		if (isConnect == TRUE)
		{
			GetFromServer();
		}
		else if (isConnect == FALSE)
		{
			if (AutoAuthentication())
			{
				if (isTray == 1)
				{
					Shell_NotifyIcon(NIM_DELETE, &((CMainClientDlg*)Dlg)->m_nid);
					((CMainClientDlg*)Dlg)->InitTray(1);
				}
				isConnect = TRUE;
			}
		}
		Sleep(3000);
	}
}
//GetReplyInfo()����������Ϣ��ʱ�򣬽��ܵ����Ƿ������ȷ��͵�
//�п��ܳ��ִ������������������ҷ���ɨ������Ҹ����������������ԣ�Ȼ�������������ܵ���ɨ��ָ�������ɨ��ָ��ͽ��ܲ����ˡ�
//ͬһ�߳��������Ե�������Ǹ��߳�����ɨ����Ƿ��Ͳ�����������

DWORD _stdcall HeartBeat(LPVOID Dlg)
{
	while (1)
	{
		//����Ͽ����ӣ���ȴ�5���ٴ��ж�
		if (isConnect == FALSE)
		{
			Sleep(5000);
			continue;
		}
		//�ж��Ƿ������磬û����������Ϊ����
		if (!CheckInternet())
		{
			isConnect = FALSE;
			if (isTray == 1)
			{
				Shell_NotifyIcon(NIM_DELETE, &((CMainClientDlg*)Dlg)->m_nid);
				((CMainClientDlg*)Dlg)->InitTray(0);
			}
			EndSSL();
			Sleep(5000);
			continue;
		}
		//��ʱ��������û����ɨ��
		if (isScan == FALSE)
		{
			Sleep(60000);
			char info[50];
			SendInfo("HBT", "HBT");
			int nRet = GetReplyInfo(info);
			if (nRet == FALSE)
			{
				isConnect = FALSE;
				if (isTray == 1)
				{
					Shell_NotifyIcon(NIM_DELETE, &((CMainClientDlg*)Dlg)->m_nid);
					((CMainClientDlg*)Dlg)->InitTray(0);
				}
				EndSSL();
			}
		}
		else if (isScan == TRUE)
		{
			//��ʱ����������ɨ��
			char info[50];
			SendInfo("HBT", "HBT");
			Sleep(1000);
			GetReplyInfo(info);
			if (strcmp(info, "HBT") != 0)
			{
				isConnect = FALSE;
				if (isTray == 1)
				{
					Shell_NotifyIcon(NIM_DELETE, &((CMainClientDlg*)Dlg)->m_nid);
					((CMainClientDlg*)Dlg)->InitTray(0);
				}
				EndSSL();
			}
			if (strcmp(info, "WHO ARE YOU") == 0)
			{
				isConnect = FALSE;
				if (isTray == 1)
				{
					Shell_NotifyIcon(NIM_DELETE, &((CMainClientDlg*)Dlg)->m_nid);
					((CMainClientDlg*)Dlg)->InitTray(0);
				}
				EndSSL();
			}
			//ɨ��ʱ10��һ����������
			Sleep(10000);
		}
	}
}















