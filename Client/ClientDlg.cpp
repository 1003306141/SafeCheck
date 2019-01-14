
// ClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LOGIN_TIMEOUT	9 * 1000



//*******************全局变量声明处************************
static bool isTcpChannelUseful = false;
static int  albSockRet;
SOCKET slisten;
bool CreateTCPServer();
int Talk2Client();
DWORD _stdcall Func(void*)
{
	Talk2Client();
	return 0;
}


//*******************全局变量声明处*************************

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CClientDlg 对话框

CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CClientDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CClientDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

void CClientDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CheckUserInput() == FALSE)
		return;
	SetDlgItemTextA(IDC_STATUS, "正在验证服务器IP ...");
	if (CheckServerIP() == -1)
	{
		SetDlgItemTextA(IDC_STATUS, "服务器IP地址错误...");
		return;
	}
	SetDlgItemTextA(IDC_STATUS, "服务器IP地址正确，验证成功");
	if (WriteConfig() == FALSE)
	{
		SetDlgItemTextA(IDC_STATUS, "配置文件写入失败！");
		return;
	}
	SetDlgItemTextA(IDC_STATUS, "配置文件写入成功！");
	if (ServerLogin() == FALSE)
	{
		SetDlgItemTextA(IDC_STATUS, "账号密码验证失败，请重新输入！");
		return;
	}
	//MessageBox("登录成功！", "成功", 0);
}
//检测用户输入是否符合规格
int CClientDlg::CheckUserInput()
{
	char username[100] = { 0 };
	char password[100] = { 0 };
	DWORD server_ip = 0;
	GetDlgItem(IDC_EDIT2)->GetWindowTextA(password, 100);
	GetDlgItem(IDC_EDIT1)->GetWindowTextA(username, 100);
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1))->GetAddress(server_ip);
	if (server_ip == 0)
	{
		MessageBoxA("请输入服务器IP地址！", "错误", 0);
		return FALSE;
	}
	if (username[0] == NULL && password[0] == NULL)
	{
		MessageBoxA("用户名和密码不能为空！", "错误", 0);
		return FALSE;
	}
	if (username[0] == NULL)
	{
		MessageBoxA("用户名不能为空！", "错误", 0);
		return FALSE;
	}
	if (password[0] == NULL)
	{
		MessageBoxA("密码不能为空！", "错误", 0);
		return FALSE;
	}
	return TRUE;
}
//检测IP地址是否为服务器IP地址
int CClientDlg::CheckServerIP()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKADDR_IN addrSrv;
	int err;
	int ret = 0;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(50005);

	DWORD server_ip = 0;
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1))->GetAddress(server_ip);
	char serverStr[256] = { '\0' };
	unsigned char* pIP = (unsigned  char*)&server_ip;
	char serverIP_[32] = { '\0' };
	snprintf(serverIP_, 32, "%u.%u.%u.%u", *(pIP + 3), *(pIP + 2), *(pIP + 1), *pIP);

	addrSrv.sin_addr.S_un.S_addr = inet_addr(serverIP_);

	timeval tm;
	fd_set set;
	unsigned long ul = 1;
	ioctlsocket(sockClient, FIONBIO, &ul); //设置为非阻塞模式

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
				ret = -1;
		}
		else
			ret = -1;
	}


	closesocket(sockClient);
	WSACleanup();
	return ret;
}
//将IP地址等信息写入配置文件
int CClientDlg::WriteConfig()
{
	DWORD server_ip = 0;
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1))->GetAddress(server_ip);
	char serverStr[256] = { '\0' };
	unsigned char* pIP = (unsigned  char*)&server_ip;
	char serverIP_[32] = { '\0' };
	snprintf(serverIP_, 32, "%u.%u.%u.%u", *(pIP + 3), *(pIP + 2), *(pIP + 1), *pIP);

	FILE* fp = fopen("etc\\config.ini", "w");
	if (fp == NULL)
	{
		return FALSE;
		fclose(fp);
	}
	fprintf(fp, "IS_WRITE_IP 1\r\n");
	fprintf(fp, "SERVER_ADDR %s\r\n", serverIP_);
	fprintf(fp, "SERVER_PORT 50005\r\n");
	fprintf(fp, "UPDATE_ADDR %s\r\n", serverIP_);
	fprintf(fp, "LOCAL_PORT 50005\r\n");
	fprintf(fp, "WORK_IN_LAN 0");
	fclose(fp);
	return TRUE;
}
//使用用户名密码登录服务器
int CClientDlg::ServerLogin()
{
	if (!CreateTCPServer())
	{
		MessageBox("创建本地tcp通道失败", "失败", 0);
		return FALSE;
	}
	CreateThread(NULL, 0, Func, NULL, 0, NULL);
	if (albSockRet == NONSENSE)
		MessageBox("正在认证用户名和秘密...");
	if (albSockRet == CONNECT_FAILED)
		MessageBox("连接服务器失败！");
	if (albSockRet == INVALID_CLIENT)
		MessageBox("用户名或密码错误！");
	if (albSockRet == ALREADY_ONLINE)
		MessageBox("该账号已经在其它机器上登录，不允许重复登录！");
	if (albSockRet == NO_LOGIN)
		MessageBox("该账号已被管理员禁止登录！");
	if (albSockRet == NOT_SPECIFIC_MAC)
		MessageBox("当前登录所使用的用户名已经与其它电脑绑定！");
	if (albSockRet == INVALID_PASSWD)
		MessageBox("用户名或密码错误！");
	if (albSockRet == ALREADY_LOGIN)
		MessageBox("已经登录，不可重复登录！");

	

	return TRUE;
}
bool CreateTCPServer()
{
	if (isTcpChannelUseful)
	{
		return true;
	}

	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return false;
	}

	//创建套接字
	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return false;
	}

	//绑定IP和端口
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(159);
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
		return false;
	}

	//开始监听
	if (listen(slisten, 1) == SOCKET_ERROR)
	{
		printf("listen error !");
		return false;
	}

	isTcpChannelUseful = true;
	return true;
}

int Talk2Client()
{
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[32];

	memset(revData, 0, sizeof(revData));

	static	FD_SET fdRead;
	int		nRet = 0;//记录发送或者接受的字节数

	static TIMEVAL	tv = { (LOGIN_TIMEOUT / 1000), 0 };//设置超时等待时间


	FD_ZERO(&fdRead);
	FD_SET(slisten, &fdRead);

	//只处理read事件
	nRet = select(0, &fdRead, NULL, NULL, &tv);

	if (nRet == 0)
	{
		//没有连接或者没有读事件
		closesocket(slisten);
	}
	else if (nRet > 0)
	{
		sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
	}

	//接收数据
	recv(sClient, revData, sizeof(revData) - 1, 0);

	closesocket(sClient);
	closesocket(slisten);

	WSACleanup();
	isTcpChannelUseful = false;
	return atoi(revData);
}




