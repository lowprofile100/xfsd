
// UI2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UI2.h"
#include "UI2Dlg.h"
#include "afxdialogex.h"

#include "tslib/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

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
	ON_BN_CLICKED(IDC_BUTTON10, &CUI2Dlg::OnBnClickedButton10)//mount

	ON_BN_CLICKED(IDC_BUTTON2, &CUI2Dlg::OnBnClickedButton2)//卸载
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
	CString text1("文件复制失败！");
	CString str1,str2;
GetDlgItemText(IDC_EDIT1, str1);
GetDlgItemText(IDC_EDIT2, str2);
USES_CONVERSION; //将cstring转化为char所用的宏，以下将cstring转化为char
	char *addr1=T2A(str1.GetBuffer());
	char *addr2=T2A(str2.GetBuffer());

//内存
	char buff[10240];
	//读文件啦
long a=read_file_from_disk( addr1,buff,sizeof(buff));
//判断是否成功打开
if(a<0)
{

MessageBox(text1,title,MB_OK);
}
else{
//打开新文件
FILE* fp;
errno_t err;
err=fopen_s(&fp,addr2,"wb+");// 读写打开或建立一个二进制文件，允许读和写
if(err<0)//打开文件失败鸟
{
MessageBox(text1,title,MB_OK);
} 
else{
 fwrite(buff,sizeof(buff),1,fp);
fclose(fp); //关闭文件
CString text2("文件复制成功！");
MessageBox(text2,title,MB_OK);
}
}
}


//打开文件
void CUI2Dlg::OnBnClickedButton6()
{
	CString str3;
	CString title("提示");	
	CString text2("文件打开失败！");
GetDlgItemText(IDC_EDIT1, str3);

USES_CONVERSION; //将cstring转化为char所用的宏，以下将cstring转化为char
char *addr3=T2A(str3.GetBuffer());

	//SetDlgItemText(IDC_EDIT2, str);看看对不对
//内存
	char buff2[10240];
	//读文件啦
long b=read_file_from_disk( addr3,buff2,sizeof(buff2));
if(b<0)//读文件失败
{
MessageBox(text2,title,MB_OK);
}
else{
CString str4("C:/Users/user/Desktop");
int lon=str3.ReverseFind('/');
CString strname=str3.Mid(lon);

CString str=str4+strname;//临时文件地址
char *addr=T2A(str.GetBuffer());
//打开新文件
FILE* fp;
errno_t err;
 err=fopen_s(&fp,addr,"wb+");// 读写打开或建立一个二进制文件，允许读和写
if(err<0)//打开临时文件失败鸟
{
MessageBox(text2,title,MB_OK);
} 
else{
fwrite(buff2,sizeof(buff2),1,fp);
fclose(fp); //关闭文件
CString dir("open");
//打开临时文件
ShellExecute(NULL,dir,str,NULL,NULL,SW_SHOWNORMAL);
}
}
}


//打开目录
void CUI2Dlg::OnBnClickedButton3()
{
	int  a=0;
	char buff[10240];
	CString title("提示");
	CString str,strlist;
	CString huiche("\r\n");
GetDlgItemText(IDC_EDIT1, str);

USES_CONVERSION; //将cstring转化为char所用的宏，以下将cstring转化为char
char *addr=T2A(str.GetBuffer());
//num=3;
//buff="a.h\0b.h\0c.h\0";

//获取文件目录

a=list_file( addr, buff);
if(a<0)
{
	CString text2("目录打开失败！");
MessageBox(text2,title,MB_OK);

}
else{
	//char转换string并分行
int i=0;
int j=0;
for(i=0;i<a;j++)
{
	if(buff[j]!='\0')
	{
	strlist+= buff[j];
	}
	else
	{
		i++;
		strlist=strlist + huiche;
    }
}
SetDlgItemText(IDC_EDIT3, strlist);
}
}

//加载
void CUI2Dlg::OnBnClickedButton10()
{
CString title("提示");
CString str1;
GetDlgItemText(IDC_EDIT4, str1);

//创建文件目录
CString list("tslib");
CreateDirectory(list, NULL);
//复制文件系统
CString dispath("tslib/xfs.lib");
CopyFile(str1, dispath, 0);
int   nError   =   GetLastError(); 
if( nError!=0)
{
	CString text1("文件系统加载失败！");
MessageBox(text1,title,MB_OK);

}
else{
//加载咧
init_read_file_from_disk();

CString text2("文件系统加载成功！");
MessageBox(text2,title,MB_OK);
CString str2("文件系统成功加载");
	str1=str1+str2;
SetDlgItemText(IDC_EDIT4, str1);
}
}




//卸载
void CUI2Dlg::OnBnClickedButton2()
{
CString title("提示");

CString dispath("tslib/xfs.lib");
int nError=DeleteFile(dispath);
//卸载的调用啊喂！

if( nError==0)
{
	CString text1("文件系统卸载失败！");
MessageBox(text1,title,MB_OK);

}
else{

CString text2("文件系统已卸载！");
MessageBox(text2,title,MB_OK);
CString str2("文件系统已卸载");
SetDlgItemText(IDC_EDIT4, str2);
}
}
