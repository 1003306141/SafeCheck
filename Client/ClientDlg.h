
// ClientDlg.h: 头文件
//

#pragma once

enum ALB_SOCK_RET
{
	NONSENSE = 0,
	CONNECT_FAILED = 10,
	CONNECT_SUCCESS,
	USERNAME_NOT_EXIST,
	INVALID_PASSWD,
	ALREADY_LOGIN,
	NOT_SPECIFIC_MAC,
	NO_LOGIN,			// 不允许登陆
	ALREADY_ONLINE,		// 该账号已经在线
	INVALID_CLIENT,
};

// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
// 构造
public:
	CClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
public:
	//检测用户输入是否符合规格
	int CheckUserInput();
	//检测IP地址是否为服务器IP地址
	int CheckServerIP();
	//将IP地址等信息写入配置文件
	int WriteConfig();
	//使用用户名密码登录服务器
	int ServerLogin();
};
