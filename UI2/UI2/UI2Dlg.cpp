
// UI2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UI2.h"
#include "UI2Dlg.h"
#include "afxdialogex.h"

#include "tslib/read_file.h"
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

	ON_BN_CLICKED(IDC_BUTTON5, &CUI2Dlg::OnBnClickedButton5)//复制

	ON_BN_CLICKED(IDC_BUTTON6, &CUI2Dlg::OnBnClickedButton6)//打开
	ON_BN_CLICKED(IDC_BUTTON3, &CUI2Dlg::OnBnClickedButton3)//打开目录
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



//copy
void CUI2Dlg::OnBnClickedButton5()
{
	CString title("提示");
	CString str1,str2;
GetDlgItemText(IDC_EDIT1, str1);
GetDlgItemText(IDC_EDIT2, str2);
USES_CONVERSION; //将cstring转化为char所用的宏，以下将cstring转化为char
	char *addr1=T2A(str1.GetBuffer());
	char *addr2=T2A(str2.GetBuffer());

//内存
	char buff[10240];
	//读文件啦
init_read_file_from_disk();
long a=read_file_from_disk( addr1,buff,sizeof(buff));
//判断是否成功打开
if(a<0)
{
CString text1("文件复制失败！");

MessageBox(text1,title,MB_OK);
}
else{
//打开新文件
FILE* fp;
 fp=fopen(addr2,"wb+");// 读写打开或建立一个二进制文件，允许读和写
 fwrite(buff,sizeof(buff),1,fp);
fclose(fp); //关闭文件
CString text2("文件复制成功！");
MessageBox(text2,title,MB_OK);
}
}


//打开文件
void CUI2Dlg::OnBnClickedButton6()
{
	CString title("提示");
	CString str3;
GetDlgItemText(IDC_EDIT1, str3);

USES_CONVERSION; //将cstring转化为char所用的宏，以下将cstring转化为char
char *addr3=T2A(str3.GetBuffer());

	//SetDlgItemText(IDC_EDIT2, str);看看对不对
//内存
	char buff2[10240];
	//读文件啦
init_read_file_from_disk();
long b=read_file_from_disk( addr3,buff2,sizeof(buff2));
if(b<0)
{
	CString text2("文件打开失败！");
MessageBox(text2,title,MB_OK);
}
CString str4("C:/Users/user/Desktop");
CString str=str4+str3;//临时文件地址
char *addr=T2A(str.GetBuffer());
//打开新文件
FILE* fp;
 fp=fopen(addr,"wb+");// 读写打开或建立一个二进制文件，允许读和写
 fwrite(buff2,sizeof(buff2),1,fp);
fclose(fp); //关闭文件
CString dir("open");
ShellExecute(NULL,dir,str,NULL,NULL,SW_SHOWNORMAL);
}


//打开目录
void CUI2Dlg::OnBnClickedButton3()
{
	// TODO: ÔÚ´ËÌí¼Ó¿Ø¼þÍ¨Öª´¦Àí³ÌÐò´úÂë
}
