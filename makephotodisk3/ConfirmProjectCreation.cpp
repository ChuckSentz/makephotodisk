/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Dialog box displayed to confirm user's choices before creating a project's directory and
copying the initial template files. Really is nothing more than a glorified message box. 

***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "ConfirmProjectCreation.h"
#include "afxdialogex.h"


// CConfirmProjectCreation dialog

IMPLEMENT_DYNAMIC(CConfirmProjectCreation, CDialog)

CConfirmProjectCreation::CConfirmProjectCreation(CWnd* pParent /*=NULL*/)
	: CDialog(CConfirmProjectCreation::IDD, pParent)
	, m_catalog_name(_T(""))
	, m_collection_name(_T(""))
	, m_directory_name(_T(""))
	, m_disk_type(_T(""))
	, m_photo_count(_T(""))
{

}

CConfirmProjectCreation::~CConfirmProjectCreation()
{
}

void CConfirmProjectCreation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CATALOG_NAME, m_catalog_name);
	DDX_Text(pDX, IDC_COLLECTION_NAME, m_collection_name);
	DDX_Text(pDX, IDC_DIRECTORY_NAME, m_directory_name);
	DDX_Text(pDX, IDC_DISK_TYPE, m_disk_type);
	DDX_Text(pDX, IDC_PHOTO_COUNT, m_photo_count);
}


BEGIN_MESSAGE_MAP(CConfirmProjectCreation, CDialog)
END_MESSAGE_MAP()


// CConfirmProjectCreation message handlers


