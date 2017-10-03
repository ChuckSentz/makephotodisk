/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
// MultiSelectPhotogCamera.cpp : implementation file
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MultiSelectPhotogCamera.h"
#include "afxdialogex.h"
#include "MakePhotoDisk3Doc.h"


// CMultiSelectPhotogCamera dialog

IMPLEMENT_DYNAMIC(CMultiSelectPhotogCamera, CDialogEx)

CMultiSelectPhotogCamera::CMultiSelectPhotogCamera(CMakePhotoDisk3Doc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiSelectPhotogCamera::IDD, pParent)
	, m_override_photog(FALSE)
	, m_photographer(_T(""))
	, m_override_camera(FALSE)
	, m_camera(_T(""))
{
	m_document = pDoc ;
}

CMultiSelectPhotogCamera::~CMultiSelectPhotogCamera()
{
}

void CMultiSelectPhotogCamera::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CUSTOM_PHOTOG_CHECK, m_override_photog);
	DDX_Control(pDX, IDC_PHOTOG_EDIT, m_photog_edit);
	DDX_Text(pDX, IDC_PHOTOG_EDIT, m_photographer);
	DDX_Check(pDX, IDC_CUSTOM_CAMERA_CHECK, m_override_camera);
	DDX_Control(pDX, IDC_CAMERA_EDIT, m_camera_edit);
	DDX_Text(pDX, IDC_CAMERA_EDIT, m_camera);
}


BEGIN_MESSAGE_MAP(CMultiSelectPhotogCamera, CDialogEx)
	ON_BN_CLICKED(IDC_CUSTOM_PHOTOG_CHECK, &CMultiSelectPhotogCamera::OnBnClickedCustomPhotogCheck)
	ON_BN_CLICKED(IDC_CUSTOM_CAMERA_CHECK, &CMultiSelectPhotogCamera::OnBnClickedCustomCameraCheck)
	ON_BN_CLICKED(IDOK, &CMultiSelectPhotogCamera::OnBnClickedOk)
END_MESSAGE_MAP()


// CMultiSelectPhotogCamera message handlers


BOOL CMultiSelectPhotogCamera::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString		photog ;
	CString		camera ;
	CString		tmp ;
	bool		any_photog = false ;
	bool		multi_photog = false ; 
	bool		any_camera = false ;
	bool		multi_camera = false ;
	POSITION	pos ;
	PhotoImage *	photo ;

	if ( photo = m_document->GetFirstSelectedPhoto( pos ) )
	{
		any_photog = photo->GetOverridePhotographer( photog ) ;
		any_camera = photo->GetOverrideCameraName( camera ) ;

		while ( photo = m_document->GetNextSelectedPhoto( pos ) )
		{
			if ( photo->GetOverridePhotographer( tmp ) )
				any_photog = true ;
			if ( 0 != tmp.Compare( photog ) )
				multi_photog = true ;

			if ( photo->GetOverrideCameraName( tmp ) )
				any_camera = true ;
			if ( 0 != tmp.Compare( camera ) ) 
				multi_camera = true ;
		}
	}
		
	if ( any_photog && !multi_photog ) 
	{
		m_override_photog = TRUE ;
		m_photographer = photog ;
		m_photog_edit.EnableWindow( TRUE ) ;
	}

	if ( any_camera && !multi_camera ) 
	{
		m_override_camera = TRUE ;
		m_camera = camera ;
		m_camera_edit.EnableWindow( TRUE ) ;
	}

	UpdateData( FALSE ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMultiSelectPhotogCamera::OnBnClickedCustomPhotogCheck()
{
	UpdateData( TRUE ) ;
	m_photog_edit.EnableWindow( m_override_photog ) ;
}


void CMultiSelectPhotogCamera::OnBnClickedCustomCameraCheck()
{
	UpdateData( TRUE ) ;
	m_camera_edit.EnableWindow( m_override_camera ) ;

}


void CMultiSelectPhotogCamera::OnBnClickedOk()
{
	UpdateData( TRUE ) ;
	CDialogEx::OnOK();
}
