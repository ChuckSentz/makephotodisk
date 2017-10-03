/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




***************************************************************************************/
#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "ImageProperties.h"
#include "afxdialogex.h"
#include "DocRecords.h"


// CImageProperties dialog

IMPLEMENT_DYNAMIC(CImageProperties, CDialogEx)

CImageProperties::CImageProperties( const PhotoImage * image, CWnd* pParent ) 
	: CDialogEx(CImageProperties::IDD, pParent)
{
	m_image = image ;
}

CImageProperties::~CImageProperties()
{
}

void CImageProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_NAME, m_file_name);
	DDX_Control(pDX, IDC_PATH, m_path_name);
}


BEGIN_MESSAGE_MAP(CImageProperties, CDialogEx)
END_MESSAGE_MAP()


// CImageProperties message handlers


BOOL CImageProperties::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString		temp ;

	m_path_name.SetWindowText( m_image->GetSourceFilePath( ) ) ;

	// the "name" in the m_image is the filename without the extension - simplest way to get extension is to just take the last element of the fully qualified path 
	temp = m_image->GetSourceFilePath( ) ;
	temp.MakeReverse( ) ;
	if ( -1 != temp.FindOneOf( _T( "/\\" ) ) ) 
		temp = temp.Left( temp.FindOneOf( _T( "/\\" ) ) ) ;
	temp.MakeReverse( ) ;

	m_file_name.SetWindowText( temp ) ;

	WIN32_FILE_ATTRIBUTE_DATA	find_data ;

	if ( GetFileAttributesEx( m_image->GetSourceFilePath( ), GetFileExInfoStandard, &find_data ) )
	{
		CString		number_text ;
		UINT64		file_size ;

		DisplayTimeField( find_data.ftCreationTime, IDC_CREATED_TIME ) ;
		DisplayTimeField( find_data.ftLastWriteTime, IDC_MODIFIED_TIME ) ;
		DisplayTimeField( find_data.ftLastAccessTime, IDC_ACCESSED_TIME ) ;

		file_size = MAKEUINT64( find_data.nFileSizeLow, find_data.nFileSizeHigh ) ;

		if ( file_size / 1000000000 )
			number_text.Format( _T( "%3I64d.%1I64d Gb" ), file_size / 1000000000, ( ( ( file_size % 1000000000 ) + 50000000 ) / 1000000000 ) % 10 ) ;
		else if ( file_size / 1000000 ) 
			number_text.Format( _T( "%3I64d.%1I64d Mb" ), file_size / 1000000, ( ( ( file_size % 1000000 ) + 50000 ) / 100000 ) % 10 ) ;
		else if ( file_size / 1000 )
			number_text.Format( _T( "%3I64d.%1I64d Kb" ), file_size / 1000, ( ( ( file_size % 1000 ) + 50 ) / 100 ) % 10 ) ;
		else
			number_text.Format( _T( "%I64d bytes" ), file_size ) ;

		GetDlgItem( IDC_FILE_SIZE )->SetWindowText( number_text ) ;
	}

	return TRUE ;
}







void CImageProperties::DisplayTimeField( FILETIME & ft, int control_ID ) 
{
	SYSTEMTIME	sys_time ;
	TCHAR		formatted_date[ 128 ] ;
	TCHAR		formatted_time[ 128 ] ;
	CString		date_time ;

	FileTimeToSystemTime( &ft, &sys_time ) ;

	date_time = _T( "Date/time unavailable" ) ;

	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &sys_time, _T( "dddd dd MMMM yyyy" ), formatted_date, sizeof( formatted_date ) / sizeof( formatted_date[ 0 ] ) ) )
	{
		if ( GetTimeFormat( LOCALE_USER_DEFAULT, 0, &sys_time, _T( "hh:mm tt" ), formatted_time, sizeof( formatted_time ) / sizeof( formatted_time[ 0 ] ) ) )
		{
			date_time.Format( _T( "%s at %s" ), formatted_date, formatted_time ) ;
		}
	}

	GetDlgItem( control_ID )->SetWindowText( date_time ) ;
}
