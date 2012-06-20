
// UI2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UI2.h"
#include "UI2Dlg.h"
#include "afxdialogex.h"

#include "fs/tslib/read_file.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUI2Dlg 对话框




CUI2Dlg::CUI2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUI2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUI2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUI2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT2, &CUI2Dlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON5, &CUI2Dlg::OnBnClickedButton5)
	ON_EN_CHANGE(IDC_EDIT1, &CUI2Dlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CUI2Dlg 消息处理程序

BOOL CUI2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUI2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUI2Dlg::OnPaint()
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
HCURSOR CUI2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}






//目标地址对话框
void CUI2Dlg::OnEnChangeEdit2()
{
	
}

//copy
void CUI2Dlg::OnBnClickedButton5()
{
	
	CString str1,str2;
GetDlgItemText(IDC_EDIT1, str1);
GetDlgItemText(IDC_EDIT2, str2);
USES_CONVERSION; //将cstring转化为char所用的宏，以下将cstring转化为char
	char *addr1=T2A(str1.GetBuffer());
	char *addr2=T2A(str2.GetBuffer());

//这个内存是这么设的吗？可是这样的话后面用sizeof取不到长度。
	char buff[1024];
	//这块跑不起来不知道为什么。。。。
init_read_file()
read_file( addr1,buff,sizeof(buff));

//以下的代码测试正常
FILE* fp;
 fp=fopen(addr2,"wb+");// 读写打开或建立一个二进制文件，允许读和写
 fwrite(&buff,sizeof(buff),1,fp);
fclose(fp); //关闭文件


}

//源地址对话框
void CUI2Dlg::OnEnChangeEdit1()
{

}
