// VideoManage.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "VideoManage.h"
#include "afxdialogex.h"


// VideoManage 对话框


CVideoManage::CVideoManage(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoManage::IDD, pParent)
{

}

CVideoManage::~CVideoManage()
{
}

void CVideoManage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoManage, CDialogEx)
END_MESSAGE_MAP()


// VideoManage 消息处理程序
