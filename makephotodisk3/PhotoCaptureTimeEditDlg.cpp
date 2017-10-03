/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Dialog box for creation time, photog credit, camera
***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "PhotoCaptureTimeEditDlg.h"
#include "afxdialogex.h"


// static helper functions 


// CPhotoCaptureTimeEditDlg dialog

IMPLEMENT_DYNAMIC(CPhotoCaptureTimeEditDlg, CDialogEx)

CPhotoCaptureTimeEditDlg::CPhotoCaptureTimeEditDlg(PhotoImage * image, CWnd* pParent /*=NULL*/)
	: CDialogEx(CPhotoCaptureTimeEditDlg::IDD, pParent)
	, m_override_create_time(FALSE)
	, m_adjust_time(FALSE)
	, m_override_camera_ID(FALSE)
	, m_override_photog(FALSE)
	, m_create_time(COleDateTime::GetCurrentTime())
	, m_camera_ID(_T(""))
	, m_photog(_T(""))
	, m_adjust_time_string(_T(""))
	, m_preview_string(_T(""))
{
	m_original_image = image ;
	m_image = *image ;
}


CPhotoCaptureTimeEditDlg::~CPhotoCaptureTimeEditDlg()
{
}

void CPhotoCaptureTimeEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CREATE_DATE_TIME, m_date_time_picker);
	DDX_Control(pDX, IDC_CAMERA_NAME_EDIT, m_camera_name_edit);
	DDX_Control(pDX, IDC_PHOTOGRAPHER_EDIT, m_photo_credit_edit);
	DDX_Check(pDX, IDC_OVERWRIDE_CREATE_TIME, m_override_create_time);
	DDX_Check(pDX, IDC_ADJUST_TIME_CHECK, m_adjust_time);
	DDX_Check(pDX, IDC_EDIT_CAMERA_NAME, m_override_camera_ID);
	DDX_Check(pDX, IDC_PHOTOGRAPHER_CREDIT_CHECK, m_override_photog);
	DDX_DateTimeCtrl(pDX, IDC_CREATE_DATE_TIME, m_create_time);
	DDX_Text(pDX, IDC_CAMERA_NAME_EDIT, m_camera_ID);
	DDX_Text(pDX, IDC_PHOTOGRAPHER_EDIT, m_photog);
	DDX_Control(pDX, IDC_ADJUST_TIME, m_adjust_time_edit);
	DDX_Text(pDX, IDC_ADJUST_TIME, m_adjust_time_string);
	DDX_Text(pDX, IDC_CAPTION_PREVIEW, m_preview_string);
}


BEGIN_MESSAGE_MAP(CPhotoCaptureTimeEditDlg, CDialogEx)
	ON_BN_CLICKED(IDC_OVERWRIDE_CREATE_TIME, &CPhotoCaptureTimeEditDlg::OnBnClickedOverrideCreateTime)
	ON_BN_CLICKED(IDC_ADJUST_TIME_CHECK, &CPhotoCaptureTimeEditDlg::OnBnClickedAdjustTimeCheck)
	ON_BN_CLICKED(IDC_EDIT_CAMERA_NAME, &CPhotoCaptureTimeEditDlg::OnBnClickedEditCameraName)
	ON_BN_CLICKED(IDC_PHOTOGRAPHER_CREDIT_CHECK, &CPhotoCaptureTimeEditDlg::OnBnClickedPhotographerCreditCheck)
	ON_NOTIFY(NM_KILLFOCUS, IDC_CREATE_DATE_TIME, &CPhotoCaptureTimeEditDlg::OnNMKillfocusCreateDateTime)
	ON_EN_KILLFOCUS(IDC_ADJUST_TIME, &CPhotoCaptureTimeEditDlg::OnEnKillfocusAdjustTime)
	ON_EN_KILLFOCUS(IDC_PHOTOGRAPHER_EDIT, &CPhotoCaptureTimeEditDlg::OnEnKillfocusPhotographerEdit)
	ON_EN_KILLFOCUS(IDC_CAMERA_NAME_EDIT, &CPhotoCaptureTimeEditDlg::OnEnKillfocusCameraNameEdit)
END_MESSAGE_MAP()





// CPhotoCaptureTimeEditDlg message handlers


BOOL CPhotoCaptureTimeEditDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// setup the date/time picker - 
	m_date_time_picker.SetFormat( _T( "ddd MM/dd/yyyy  hh:mm:ss tt" ) ) ;

	SYSTEMTIME		create_time ;

	if ( !( m_override_create_time = m_image.GetOverrideCreateTime( create_time ) ) )
		if ( !m_image.GetExtractedCreateTime( create_time ) ) 
			GetLocalTime( &create_time ) ;
	m_create_time = COleDateTime( create_time ) ;

	if ( m_override_create_time ) 
		m_date_time_picker.EnableWindow( TRUE ) ;


	// setup the adjust seconds - 
	int adjust_seconds = 0 ;

	if ( m_adjust_time = m_image.GetCreateTimeAdjust( adjust_seconds ) )
	{
		ASSERT( !m_override_create_time ) ;
		m_adjust_time_edit.EnableWindow( TRUE ) ;
	}
	m_adjust_time_string = PhotoImage::FormatAdjustTime( adjust_seconds ) ;

	// setup the photographer override 
	if ( m_override_photog = m_image.GetOverridePhotographer( m_photog ) )
		m_photo_credit_edit.EnableWindow( TRUE ) ;

	if ( m_override_camera_ID = m_image.GetOverrideCameraName( m_camera_ID ) )
		m_camera_name_edit.EnableWindow( TRUE ) ;

	m_preview_string = m_image.GetCreationSubCaption( ) ;

	UpdateData( FALSE ) ;

	return TRUE ;
}





void CPhotoCaptureTimeEditDlg::OnBnClickedOverrideCreateTime()
{
	UpdateData( TRUE ) ;

	if ( m_override_create_time ) 
	{
		m_adjust_time = FALSE ;
		m_adjust_time_edit.EnableWindow( FALSE ) ;
		m_date_time_picker.EnableWindow( TRUE ) ;
	}
	else
	{
		SYSTEMTIME	sys_time ;

		m_date_time_picker.EnableWindow( FALSE ) ;
		ZeroMemory( &sys_time, sizeof( sys_time ) ) ;
		m_image.SetOverrideCreateTime( sys_time ) ;
	}

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetCreationSubCaption( ) ;
	UpdateData( FALSE ) ;
}








void CPhotoCaptureTimeEditDlg::OnNMKillfocusCreateDateTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateData( TRUE ) ;

	UpdateImageCopy( ) ;

	UpdateData( FALSE ) ;
	*pResult = 0;
}

void CPhotoCaptureTimeEditDlg::UpdateOverrideTimes( ) 
{
	SYSTEMTIME	sys_time ;

	sys_time.wYear = m_create_time.GetYear( ) ;
	sys_time.wMonth = m_create_time.GetMonth( ) ;
	sys_time.wDay = m_create_time.GetDay( ) ;
	sys_time.wHour = m_create_time.GetHour( ) ;
	sys_time.wMinute = m_create_time.GetMinute( ) ;
	sys_time.wSecond = m_create_time.GetSecond( ) ;
	sys_time.wMilliseconds = 0 ;
	sys_time.wDayOfWeek = 0 ;
	
	m_image.SetOverrideCreateTime( sys_time ) ;
	m_preview_string = m_image.GetCreationSubCaption( ) ;
}







void CPhotoCaptureTimeEditDlg::OnBnClickedAdjustTimeCheck()
{
	UpdateData( TRUE ) ;

	if ( m_adjust_time ) 
	{
		m_override_create_time = FALSE ;
		m_date_time_picker.EnableWindow( FALSE ) ;
		m_adjust_time_edit.EnableWindow( TRUE ) ;

		if ( 0 == m_adjust_time_string.GetLength( ) ) 
			m_adjust_time_string = _T( "0s" ) ;
	}
	else
	{
		m_adjust_time_edit.EnableWindow( FALSE ) ;
	}

	UpdateImageCopy( ) ;

	m_preview_string = m_image.GetCreationSubCaption( ) ;
	UpdateData( FALSE ) ;
}









void CPhotoCaptureTimeEditDlg::OnEnKillfocusAdjustTime()
{
	CString		old_adjust_str ;

	old_adjust_str = m_adjust_time_string ;

	UpdateData( TRUE ) ;

	if ( 0 != m_adjust_time_string.CompareNoCase( old_adjust_str ) ) 
	{
		INT32	adjust_seconds ;

		if ( PhotoImage::ParseAdjustTimeString( m_adjust_time_string, adjust_seconds ) )
			m_image.SetCreateTimeAdjust( adjust_seconds ) ;

		m_preview_string = m_image.GetCreationSubCaption( ) ;

		UpdateData( FALSE ) ;
	}
}









void CPhotoCaptureTimeEditDlg::OnBnClickedPhotographerCreditCheck()
{
	UpdateData( TRUE ) ;

	if ( m_override_photog ) 
	{
		m_photo_credit_edit.EnableWindow( TRUE ) ;
	}
	else
	{
		m_photo_credit_edit.EnableWindow( FALSE ) ;
	}

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetCreationSubCaption( ) ;
	UpdateData( FALSE ) ;
}



void CPhotoCaptureTimeEditDlg::OnEnKillfocusPhotographerEdit()
{
	UpdateData( TRUE ) ;
	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetCreationSubCaption( ) ;
	UpdateData( FALSE ) ;
}







void CPhotoCaptureTimeEditDlg::OnBnClickedEditCameraName()
{
	UpdateData( TRUE ) ;
	m_camera_name_edit.EnableWindow( m_override_camera_ID ) ;

	if ( m_override_camera_ID ) 
	{
		m_camera_name_edit.EnableWindow( TRUE ) ;
	}
	else
	{
		m_camera_name_edit.EnableWindow( FALSE ) ;
	}

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetCreationSubCaption( ) ;
	UpdateData( FALSE ) ;
}





void CPhotoCaptureTimeEditDlg::OnEnKillfocusCameraNameEdit()
{
	UpdateData( TRUE ) ;

	UpdateImageCopy( ) ;
	m_preview_string = m_image.GetCreationSubCaption( ) ;

	UpdateData( FALSE ) ;
}










void CPhotoCaptureTimeEditDlg::OnOK()
{
	UpdateData( TRUE ) ;

	UpdateImageCopy( ) ;
	m_image.SetHasDirtyEdits( ) ;
	*m_original_image = m_image ;

	CDialogEx::OnOK();
}







void CPhotoCaptureTimeEditDlg::UpdateImageCopy( )
{
	if ( m_override_create_time ) 
	{
		ASSERT( FALSE == m_adjust_time ) ;

		m_image.SetCreateTimeAdjust( 0 ) ;

		UpdateOverrideTimes( ) ;
	}
	else
	{
		SYSTEMTIME	sys_time ;

		ZeroMemory( &sys_time, sizeof( sys_time ) ) ;
		m_image.SetOverrideCreateTime( sys_time ) ;
	}


	if ( m_adjust_time )
	{
		int	adjust_time = 0 ;
	
		ASSERT( FALSE == m_override_create_time ) ;

		SYSTEMTIME	sys_time ;
		ZeroMemory( &sys_time, sizeof( sys_time ) ) ;
		m_image.SetOverrideCreateTime( sys_time ) ;

		if ( PhotoImage::ParseAdjustTimeString( m_adjust_time_string, adjust_time ) )
			m_image.SetCreateTimeAdjust( adjust_time ) ;
	}
	else
	{
		m_image.SetCreateTimeAdjust( 0 ) ;
	}


	if ( m_override_photog ) 
	{
		m_image.SetOverridePhotographer( m_photog ) ;
	}
	else
	{
		m_image.SetOverridePhotographer( _T( "" ) ) ;
	}


	if ( m_override_camera_ID ) 
	{
		m_image.SetOverrideCameraName( m_camera_ID ) ;
	}
	else
	{
		m_image.SetOverrideCameraName( _T( "" ) ) ;
	}
}


