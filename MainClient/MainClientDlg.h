
// MainClientDlg.h: 头文件
//

#pragma once
#define WM_SHOWTASK WM_USER+1


// CMainClientDlg 对话框
class CMainClientDlg : public CDialogEx
{
// 构造
public:
	CMainClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRegister();
	afx_msg void OnBnClickedExit();

public:
	NOTIFYICONDATA m_nid;
	LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);
	bool CheckInput();
	bool CheckUser();
	void InitTray(int n);
	afx_msg void OnDestroy();
	//管理员登录
	afx_msg void On32771();
	//用户登录
	afx_msg void On32772();
};
