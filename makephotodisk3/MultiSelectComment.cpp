/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
// MultiSelectComment.cpp : implementation file
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MultiSelectComment.h"
#include "afxdialogex.h"
#include "MakePhotoDisk3Doc.h"


// CMultiSelectComment dialog

IMPLEMENT_DYNAMIC(CMultiSelectComment, CDialogEx)

CMultiSelectComment::CMultiSelectComment(CMakePhotoDisk3Doc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiSelectComment::IDD, pParent)
	, m_add_comment(FALSE)
	, m_comment(_T(""))
{
	m_document = pDoc ;
}

CMultiSelectComment::~CMultiSelectComment()
{
}

void CMultiSelectComment::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ADD_COMMENT, m_add_comment);
	DDX_Control(pDX, IDC_COMMENT, m_comment_edit);
	DDX_Text(pDX, IDC_COMMENT, m_comment);
}


BEGIN_MESSAGE_MAP(CMultiSelectComment, CDialogEx)
	ON_BN_CLICKED(IDC_ADD_COMMENT, &CMultiSelectComment::OnBnClickedAddComment)
	ON_BN_CLICKED(IDOK, &CMultiSelectComment::OnBnClickedOk)
END_MESSAGE_MAP()


// CMultiSelectComment message handlers


void CMultiSelectComment::OnBnClickedAddComment()
{
	UpdateData( TRUE ) ;
	m_comment_edit.EnableWindow( m_add_comment ) ;
}


void CMultiSelectComment::OnBnClickedOk()
{
	UpdateData( TRUE ) ;
	PhotoImage::AddHTMLCoding( m_comment ) ;
	UpdateData( FALSE ) ;
	CDialogEx::OnOK();
}


BOOL CMultiSelectComment::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	POSITION		pos ;
	PhotoImage *	image ;
	CString			first_comment ;
	CString			tmp ;
	bool			any_comment = false ;
	bool			comments_different = false ;


	if ( image = m_document->GetFirstSelectedPhoto( pos ) )
	{
		if ( image->GetExtendedComment( first_comment ) ) 
			any_comment = true ;

		while ( image = m_document->GetNextSelectedPhoto( pos ) ) 
		{
			if ( image->GetExtendedComment( tmp ) )
				any_comment = true ;

			if ( 0 != tmp.Compare( first_comment ) ) 
				comments_different = true ;
		}
	}

	if ( any_comment && !comments_different ) 
	{
		PhotoImage::RemoveHTMLCoding( first_comment ) ;
		m_comment = first_comment ;

		m_add_comment = TRUE ;
		m_comment_edit.EnableWindow( TRUE ) ;
		UpdateData( FALSE ) ;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}




