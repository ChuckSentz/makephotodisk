/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
// MultiSelectTimeAdjust.cpp : implementation file
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MultiSelectTimeAdjust.h"
#include "afxdialogex.h"
#include "MakePhotoDisk3Doc.h"



// CMultiSelectTimeAdjust dialog

IMPLEMENT_DYNAMIC(CMultiSelectTimeAdjust, CDialogEx)

CMultiSelectTimeAdjust::CMultiSelectTimeAdjust(CMakePhotoDisk3Doc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiSelectTimeAdjust::IDD, pParent)
	, m_set_adjust_time(FALSE)
	, m_adjust_time_string(_T(""))
{
	m_document = pDoc ;
	m_adjust_seconds = 0 ;
}

CMultiSelectTimeAdjust::~CMultiSelectTimeAdjust()
{
}

void CMultiSelectTimeAdjust::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ADJUST_TIME_CHECK, m_set_adjust_time);
	DDX_Control(pDX, IDC_ADJUST_TIME_EDIT, m_adjust_time_edit);
	DDX_Text(pDX, IDC_ADJUST_TIME_EDIT, m_adjust_time_string);
}


BEGIN_MESSAGE_MAP(CMultiSelectTimeAdjust, CDialogEx)
	ON_BN_CLICKED(IDC_ADJUST_TIME_CHECK, &CMultiSelectTimeAdjust::OnBnClickedAdjustTimeCheck)
	ON_BN_CLICKED(IDOK, &CMultiSelectTimeAdjust::OnBnClickedOk)
END_MESSAGE_MAP()


// CMultiSelectTimeAdjust message handlers


void CMultiSelectTimeAdjust::OnBnClickedAdjustTimeCheck()
{
	UpdateData( TRUE ) ;
	GetDlgItem( IDC_ADJUST_TIME_EDIT )->EnableWindow( m_set_adjust_time ) ; 
}


void CMultiSelectTimeAdjust::OnBnClickedOk()
{
	UpdateData( TRUE ) ;

	if ( PhotoImage::ParseAdjustTimeString( m_adjust_time_string, m_adjust_seconds ) )
		CDialogEx::OnOK();
	else
	{
		AfxMessageBox( _T( "Invalid time string" ), MB_OK ) ;
		GetDlgItem( IDC_ADJUST_TIME_EDIT )->SetFocus( ) ;
		GetDlgItem( IDC_ADJUST_TIME_EDIT )->SendMessage( EM_SETSEL, 0, -1 ) ;
	}
}




BOOL CMultiSelectTimeAdjust::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	PhotoImage *		photo ;
	POSITION			pos ;
	INT32				adjust_seconds ;
	INT32				tmp ;
	bool				any_override = false ;
	bool				multi_adjust = false ;

	if ( photo = m_document->GetFirstSelectedPhoto( pos ) )
	{
		if ( photo->GetCreateTimeAdjust( adjust_seconds ) )
			any_override = true ;

		while ( photo = m_document->GetNextSelectedPhoto( pos ) ) 
		{
			if ( photo->GetCreateTimeAdjust( tmp ) )
			{
				any_override = true ;

				if ( tmp != adjust_seconds ) 
					multi_adjust = true ;
			}
			else if ( any_override ) 
				multi_adjust = true ;
		}

		if ( any_override && !multi_adjust ) 
		{
			m_set_adjust_time = TRUE ;
			m_adjust_time_string = PhotoImage::FormatAdjustTime( adjust_seconds ) ;
			m_adjust_seconds = adjust_seconds ;

			GetDlgItem( IDC_ADJUST_TIME_EDIT )->EnableWindow( TRUE ) ; 
		}

		UpdateData( FALSE ) ;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
