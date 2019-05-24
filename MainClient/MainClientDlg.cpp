//MFC支持
#include "stdafx.h"
#include "MainClient.h"
#include "MainClientDlg.h"
#include "afxdialogex.h"

//库
#include "Network.h"//网络功能
#include "Scan.h"//扫描功能
#include "Monitor.h"//动态监控
#include <Dbt.h>//检测USB插入消息

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainClientDlg 对话框

CMainClientDlg::CMainClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAINCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMainClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMainClientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_REGISTER, &CMainClientDlg::OnBnClickedRegister)
	ON_BN_CLICKED(IDC_EXIT, &CMainClientDlg::OnBnClickedExit)
	ON_MESSAGE(WM_SHOWTASK, OnShowTask)
	ON_WM_DESTROY()
	ON_COMMAND(ID_32771, &CMainClientDlg::On32771)
	ON_COMMAND(ID_32772, &CMainClientDlg::On32772)
END_MESSAGE_MAP()

LRESULT  CMainClientDlg::OnShowTask(WPARAM wparam, LPARAM lparam)
{
	//wParam接收的是图标的ID，而lParam接收的是鼠标的行为
	if (wparam != IDR_MAINFRAME) return  1;
	switch (lparam)
	{
	case  WM_RBUTTONUP://右键起来时弹出快捷菜单，这里只有一个关闭
	{
		CPoint pos;
		CMenu menu;
		//从资源文件中添加一个响应菜单
		GetCursorPos(&pos);
		menu.LoadMenu(IDR_MENU1);
		SetForegroundWindow();//放置在前面
		CMenu* pmenu;    //定义右键菜单指针
		pmenu = menu.GetSubMenu(0);      //该函数取得被指定菜单激活的下拉式菜单或子菜单的句柄
		ASSERT(pmenu != NULL);
		pmenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, pos.x, pos.y, this);   //在指定位置显示右键快捷菜单
		pmenu->Detach();
		pmenu->DestroyMenu();
	}
	break;
	case  WM_LBUTTONDBLCLK://双击左键的处理
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		CreateProcess(".\\userUI.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
	break;
	}
	return 0;
}

// CMainClientDlg 消息处理程序

BOOL CMainClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	if (AutoStart())
	{
		//任务栏不显示
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		//主窗口不显示
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		wp.flags = WPF_RESTORETOMAXIMIZED;
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(&wp);
		if(isTray == 1)
			InitTray(0);

		while (1)
		{
			GetConfig();
			if (CheckInternet())
			{
				OnBnClickedRegister();
				break;
			}
			Sleep(5000);
		}
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMainClientDlg::OnPaint()
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
HCURSOR CMainClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CMainClientDlg::AutoStart()
{
	FILE* fp = fopen("config.ini", "r");
	if (fp == NULL)
		return FALSE;
	char ServerIP[20], username[20], password[20], Port[10];
	fscanf(fp, "%s %s", ServerIP, Port);
	fscanf(fp, "%s", username);
	fscanf(fp, "%s", password);
	fclose(fp);

	CString str1(ServerIP);
	CString str2(username);
	CString str3(password);
	CString str4(Port);
	GetDlgItem(IDC_IPADDRESS1)->SetWindowTextA(str1);
	GetDlgItem(IDC_USERNAME)->SetWindowTextA(str2);
	GetDlgItem(IDC_PASSWORD)->SetWindowTextA(str3);
	GetDlgItem(IDC_PORT)->SetWindowTextA(str4);

	return TRUE;
}

void CMainClientDlg::InitTray(int n)
{
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;             // 自定义的消息名称
	if(n == 1)
		m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	else m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME1));
	strcpy(m_nid.szTip, "保密自查客户端");                // 信息提示条为"服务器程序"，VS2008 UNICODE编码用wcscpy_s()函数
	Shell_NotifyIcon(NIM_ADD, &m_nid);                // 在托盘区添加图标
}

void CMainClientDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}

void CMainClientDlg::OnBnClickedCancel()
{
	//CDialogEx::OnCancel();
}

void CMainClientDlg::OnBnClickedRegister()
{
	if (!CheckInput())
		return;
	if (!CheckUser())
		return;
	if (!CreateConfig())
		return;

	ShowWindow(SW_HIDE);
	GetKeyFile();

	CreateThread(NULL, 0, GetServerCommand, (LPVOID)this, 0, NULL);
	CreateThread(NULL, 0, HeartBeat, (LPVOID)this, 0, NULL);
	CreateThread(NULL, 0, MonitorStart, NULL, 0, NULL);
}

void CMainClientDlg::OnBnClickedExit()
{
	OnCancel();
}

bool CMainClientDlg::CheckInput()
{
	CString str;
	GetDlgItem(IDC_IPADDRESS1)->GetWindowTextA(str);
	if (str == "0.0.0.0")
	{
		MessageBox("IP地址不能为空！", "失败", 0);
		return FALSE;
	}
	GetDlgItem(IDC_USERNAME)->GetWindowTextA(str);
	if (str == "")
	{
		MessageBox("用户名不能为空！", "失败", 0);
		return FALSE;
	}
	GetDlgItem(IDC_PASSWORD)->GetWindowTextA(str);
	if (str == "")
	{
		MessageBox("密码不能为空！", "失败", 0);
		return FALSE;
	}
	return TRUE;
}

bool CMainClientDlg::CheckUser()
{
	CString str;
	char ServerIP[20], username[20], password[20];

	GetDlgItem(IDC_IPADDRESS1)->GetWindowTextA(str);
	strcpy(ServerIP, str);

	GetDlgItem(IDC_USERNAME)->GetWindowTextA(str);
	strcpy(username, str);

	GetDlgItem(IDC_PASSWORD)->GetWindowTextA(str);
	strcpy(password, str);

	if (Authentication(ServerIP, username, password))
		return TRUE;

	return FALSE;
}

bool CMainClientDlg::CreateConfig()
{
	CString str;
	char ServerIP[20], username[20], password[20], Port[10];

	GetDlgItem(IDC_IPADDRESS1)->GetWindowTextA(str);
	strcpy(ServerIP, str);

	GetDlgItem(IDC_USERNAME)->GetWindowTextA(str);
	strcpy(username, str);

	GetDlgItem(IDC_PASSWORD)->GetWindowTextA(str);
	strcpy(password, str);

	GetDlgItem(IDC_PORT)->GetWindowTextA(str);
	if (str != "")
		strcpy(Port, str);
	else strcpy(Port, "80");
	FILE* fp = fopen("config.ini", "w");
	if (fp == NULL)
	{
		MessageBox("配置文件生成失败", "失败", 0);
		return FALSE;
	}
	fprintf(fp, "%s %s\n%s\n%s\n", ServerIP, Port, username, password);
	fclose(fp);
	return TRUE;
}

void CMainClientDlg::OnDestroy()
{
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	return CDialogEx::OnDestroy();
}

void CMainClientDlg::On32771()
{
	char ServerIP[20], Port[10];
	char webaddr[128];
	FILE* fp = fopen("config.ini", "r");
	if (fp == NULL)
		MessageBox("缺少配置文件", "失败", 0);
	fscanf(fp, "%s %s", ServerIP, Port);
	fclose(fp);
	sprintf(webaddr, "http://%s:%s/", ServerIP, Port);
	ShellExecute(NULL, _T("open"), webaddr, NULL, NULL, SW_SHOWNORMAL);
}

void CMainClientDlg::On32772()
{
	char ServerIP[20], Port[10];
	char webaddr[128];
	FILE* fp = fopen("config.ini", "r");
	if (fp == NULL)
		MessageBox("缺少配置文件", "失败", 0);
	fscanf(fp, "%s %s", ServerIP, Port);
	fclose(fp);
	sprintf(webaddr, "http://%s:%s/loginScanSelf", ServerIP, Port);
	ShellExecute(NULL, _T("open"), webaddr, NULL, NULL, SW_SHOWNORMAL);
}

LRESULT CMainClientDlg::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case DBT_DEVICEARRIVAL:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		if (DBT_DEVTYP_VOLUME == lpdb->dbch_devicetype)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			DWORD dwDriverMask = lpdbv->dbcv_unitmask;
			DWORD dwTemp = 1;
			char szDriver[4] = "A:\\";
			for (szDriver[0] = 'A'; szDriver[0] <= 'Z'; szDriver[0]++)
			{
				if (0 < (dwTemp & dwDriverMask))
				{
					::MessageBox(NULL, szDriver, "设备已插入", MB_OK);
				}
				dwTemp = (dwTemp << 1);
			}
		}
	}break;

	case DBT_DEVICEREMOVECOMPLETE:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		if (DBT_DEVTYP_VOLUME == lpdb->dbch_devicetype)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			DWORD dwDriverMask = lpdbv->dbcv_unitmask;
			DWORD dwTemp = 1;
			char szDriver[4] = "A:\\";
			for (szDriver[0] = 'A'; szDriver[0] <= 'Z'; szDriver[0]++)
			{
				if (0 < (dwTemp & dwDriverMask))
				{
					::MessageBox(NULL, szDriver, "设备已拔出", MB_OK);
				}
				dwTemp = (dwTemp << 1);
			}
		}
	}break;

	default:
		break;
	}
	return 0;
}