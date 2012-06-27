
// UI2Dlg.h : 头文件
//

#pragma once


// CUI2Dlg 对话框
class CUI2Dlg : public CDialogEx
{
// 构造
public:
	CUI2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_UI2_DIALOG };

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
	afx_msg void OnEnChangeMfceditbrowse1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton3();
};
