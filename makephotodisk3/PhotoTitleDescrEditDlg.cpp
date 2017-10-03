/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Dialog box for editing title and description
***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "PhotoTitleDescrEditDlg.h"
#include "afxdialogex.h"


// CPhotoTitleDescrEditDlg dialog

IMPLEMENT_DYNAMIC(CPhotoTitleDescrEditDlg, CDialogEx)

CPhotoTitleDescrEditDlg::CPhotoTitleDescrEditDlg(PhotoImage * image, CWnd* pParent /*=NULL*/)
	: CDialogEx(CPhotoTitleDescrEditDlg::IDD, pParent)
	, m_override_title(FALSE)
	, m_override_description(FALSE)
	, m_title_string(_T(""))
	, m_description_string(_T(""))
	, m_preview_string(_T(""))
{
	m_original_image = image ;
	m_image = *image ;
}

CPhotoTitleDescrEditDlg::~CPhotoTitleDescrEditDlg()
{
}

void CPhotoTitleDescrEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CUSTOM_TITLE_CHECK, m_override_title);
	DDX_Check(pDX, IDC_CUSTOM_DESCR_CHECK, m_override_description);
	DDX_Control(pDX, IDC_TITLE_EDIT, m_title_edit_ctrl);
	DDX_Text(pDX, IDC_TITLE_EDIT, m_title_string);
	DDX_Control(pDX, IDC_DESCR_EDIT, m_description_edit_ctrl);
	DDX_Text(pDX, IDC_DESCR_EDIT, m_description_string);
	DDX_Text(pDX, IDC_CAPTION_PREVIEW, m_preview_string);
}


BEGIN_MESSAGE_MAP(CPhotoTitleDescrEditDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CUSTOM_TITLE_CHECK, &CPhotoTitleDescrEditDlg::OnBnClickedCustomTitleCheck)
	ON_BN_CLICKED(IDC_CUSTOM_DESCR_CHECK, &CPhotoTitleDescrEditDlg::OnBnClickedCustomDescrCheck)
	ON_BN_CLICKED(IDOK, &CPhotoTitleDescrEditDlg::OnBnClickedOk)
	ON_EN_KILLFOCUS(IDC_TITLE_EDIT, &CPhotoTitleDescrEditDlg::OnEnKillfocusTitleEdit)
	ON_EN_KILLFOCUS(IDC_DESCR_EDIT, &CPhotoTitleDescrEditDlg::OnEnKillfocusDescrEdit)
END_MESSAGE_MAP()



BOOL CPhotoTitleDescrEditDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog( ) ;

	m_preview_string = m_image.GetFormattedCaption( ) ;

	if ( m_override_title = m_image.GetOverrideTitle( m_title_string ) )
		m_title_edit_ctrl.EnableWindow( TRUE ) ;

	if ( m_override_description = m_image.GetOverrideDescr( m_description_string ) )
		m_description_edit_ctrl.EnableWindow( TRUE ) ;

	UpdateData( FALSE ) ;

	return TRUE ; 
}




// CPhotoTitleDescrEditDlg message handlers


void CPhotoTitleDescrEditDlg::OnBnClickedCustomTitleCheck()
{
	UpdateData( TRUE ) ;

	if ( m_override_title ) 
	{
		m_title_edit_ctrl.EnableWindow( TRUE ) ;

		if ( 0 == m_title_string.GetLength( ) ) 
		{
			m_image.GetExtractedTitle( m_title_string ) ;
		}
	}
	else
	{
		m_title_edit_ctrl.EnableWindow( FALSE ) ;
	}

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetFormattedCaption( ) ;
	UpdateData( FALSE ) ;
}










void CPhotoTitleDescrEditDlg::OnEnKillfocusTitleEdit()
{
	UpdateData( TRUE ) ;

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetFormattedCaption( ) ;

	UpdateData( FALSE ) ;
}








void CPhotoTitleDescrEditDlg::OnBnClickedCustomDescrCheck()
{
	UpdateData( TRUE ) ;

	if ( m_override_description ) 
	{
		m_description_edit_ctrl.EnableWindow( TRUE ) ;
		if ( 0 == m_description_string.GetLength( ) )
			m_image.GetExtractedDescr( m_description_string ) ;
	} 
	else
	{
		m_description_edit_ctrl.EnableWindow( FALSE ) ;
	}

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetFormattedCaption( ) ;
	UpdateData( FALSE ) ;
}






void CPhotoTitleDescrEditDlg::OnEnKillfocusDescrEdit()
{
	UpdateData( TRUE ) ;
	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetFormattedCaption( ) ;
	UpdateData( FALSE ) ;
}







void CPhotoTitleDescrEditDlg::UpdateImageCopy( ) 
{
	if ( m_override_title ) 
	{
		m_image.SetOverrideTitle( m_title_string ) ;
	}
	else
	{
		m_image.SetOverrideTitle( _T( "" ) ) ;
	}


	if ( m_override_description ) 
	{
		m_image.SetOverridedDescr( m_description_string ) ;
	} 
	else
	{
		m_image.SetOverridedDescr( _T( "" ) ) ;
	}
}




void CPhotoTitleDescrEditDlg::OnBnClickedOk()
{
	UpdateData( TRUE ) ;
	UpdateImageCopy( ) ;

	m_image.SetHasDirtyEdits( ) ;
	*m_original_image = m_image ;
	CDialogEx::OnOK();
}



