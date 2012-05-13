
// UIDlg.h : 头文件
//

#pragma once


// CUIDlg 对话框
class CUIDlg : public CDialogEx
{
// 构造
public:
	CUIDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_UI_DIALOG };

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
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLbnSelchangeList3();
	afx_msg void OnEnChangeMfceditbrowse1();
	afx_msg void OnBnClickedMfcmenubutton2();
	afx_msg void OnBnClickedMfcmenubutton4();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnTvnSelchangedMfcshelltree1(NMHDR *pNMHDR, LRESULT *pResult);
};
