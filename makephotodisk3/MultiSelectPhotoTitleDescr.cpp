/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
// MultiSelectPhotoTitleDescr.cpp : implementation file
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MultiSelectPhotoTitleDescr.h"
#include "MakePhotoDisk3Doc.h"
#include "afxdialogex.h"


// CMultiSelectPhotoTitleDescr dialog

IMPLEMENT_DYNAMIC(CMultiSelectPhotoTitleDescr, CDialogEx)

CMultiSelectPhotoTitleDescr::CMultiSelectPhotoTitleDescr(CMakePhotoDisk3Doc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiSelectPhotoTitleDescr::IDD, pParent)
	, m_title(_T(""))
	, m_descr(_T(""))
	, m_set_title(FALSE)
	, m_set_descr(FALSE)
{
	m_document = pDoc ;
}

CMultiSelectPhotoTitleDescr::~CMultiSelectPhotoTitleDescr()
{
}

void CMultiSelectPhotoTitleDescr::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE_EDIT, m_title_edit_control);
	DDX_Text(pDX, IDC_TITLE_EDIT, m_title);
	DDX_Control(pDX, IDC_DESCR_EDIT, m_descr_edit_control);
	DDX_Text(pDX, IDC_DESCR_EDIT, m_descr);
	DDX_Check(pDX, IDC_CUSTOM_TITLE_CHECK, m_set_title);
	DDX_Check(pDX, IDC_CUSTOM_DESCR_CHECK, m_set_descr);
}


BEGIN_MESSAGE_MAP(CMultiSelectPhotoTitleDescr, CDialogEx)
	ON_BN_CLICKED(IDC_CUSTOM_TITLE_CHECK, &CMultiSelectPhotoTitleDescr::OnBnClickedCustomTitleCheck)
	ON_BN_CLICKED(IDC_CUSTOM_DESCR_CHECK, &CMultiSelectPhotoTitleDescr::OnBnClickedCustomDescrCheck)
	ON_BN_CLICKED(IDOK, &CMultiSelectPhotoTitleDescr::OnBnClickedOk)
END_MESSAGE_MAP()


// CMultiSelectPhotoTitleDescr message handlers


void CMultiSelectPhotoTitleDescr::OnBnClickedCustomTitleCheck()
{
	UpdateData( TRUE ) ;
	GetDlgItem( IDC_TITLE_EDIT )->EnableWindow( m_set_title ) ;
}


void CMultiSelectPhotoTitleDescr::OnBnClickedCustomDescrCheck()
{
	UpdateData( TRUE ) ;
	GetDlgItem( IDC_DESCR_EDIT )->EnableWindow( m_set_descr ) ;
}


BOOL CMultiSelectPhotoTitleDescr::OnInitDialog()
{
	POSITION		pos ;
	PhotoImage *	photo ;
	CString			title ;
	CString			descr ;
	CString			tmp ;
	bool			any_title_override = false ;
	bool			multi_title = false ;
	bool			any_descr_override = false ;
	bool			multi_descr = false ;

	CDialogEx::OnInitDialog();

	ASSERT( m_document->GetSelectionCount( ) > 1 ) ;

	if ( photo = m_document->GetFirstSelectedPhoto( pos ) )
	{
		any_title_override = photo->GetOverrideTitle( title ) ;
		any_descr_override = photo->GetOverrideDescr( descr ) ;

		while ( photo = m_document->GetNextSelectedPhoto( pos ) ) 
		{
			if ( photo->GetOverrideTitle( tmp ) )
				any_title_override = true ;
			if ( 0 != tmp.Compare( title ) ) 
				multi_title = true ;

			if ( photo->GetOverrideDescr( tmp ) )
				any_descr_override = true ;
			if ( 0 != tmp.Compare( descr ) )
				multi_descr = true ;
		}
	}

	if ( any_title_override )
		if ( !multi_title ) 
		{
			m_title = title ;
			m_set_title = TRUE ;
			GetDlgItem( IDC_TITLE_EDIT )->EnableWindow( m_set_title ) ;
		}
		else
			m_title = _T( "Multiple title overrides" ) ;


	if ( any_descr_override ) 
		if ( !multi_descr ) 
		{
			m_descr = descr ;
			m_set_descr = TRUE ;
			GetDlgItem( IDC_DESCR_EDIT )->EnableWindow( m_set_descr ) ;
		}
		else
			m_descr = _T( "Multiple description overrides" ) ;

	UpdateData( FALSE ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMultiSelectPhotoTitleDescr::OnBnClickedOk()
{
	UpdateData( TRUE ) ;
	CDialogEx::OnOK();
}
