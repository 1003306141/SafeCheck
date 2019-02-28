
// MainClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MainClient.h"
#include "MainClientDlg.h"
#include "afxdialogex.h"
#include "Network.h"

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
END_MESSAGE_MAP()


// CMainClientDlg 消息处理程序

BOOL CMainClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetDlgItem(IDC_IPADDRESS1)->SetWindowTextA("114.115.244.171");

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
		MessageBox("IP地址不能为空！", 0, 0);
		return FALSE;
	}
	GetDlgItem(IDC_USERNAME)->GetWindowTextA(str);
	if (str == "")
	{
		MessageBox("用户名不能为空！", 0, 0);
		return FALSE;
	}
	GetDlgItem(IDC_PASSWORD)->GetWindowTextA(str);
	if (str == "")
	{
		MessageBox("密码不能为空！", 0, 0);
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
	{
		MessageBox("注册成功", 0, 0);
		return TRUE;
	}

	return FALSE;
}