/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



Dialog box which displays the status of the process of refreshing jpegs and html for the project 

***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "GdiPlusBitmap.h"
#include "RefreshDialog.h"
#include "afxdialogex.h"

using namespace Gdiplus;


// CRefreshDialog dialog

IMPLEMENT_DYNAMIC(CRefreshDialog, CDialogEx)
								
/******************************************************************************
CRefreshDialog::CRefreshDialog


******************************************************************************/
CRefreshDialog::CRefreshDialog( 
	CMakePhotoDisk3Doc * doc,				// I - document 
	int refresh_file_ct,					// I - total number of image files to generate 
	CWnd* pParent							// I - parent CWnd. Default NULL 
	)
	:	CDialogEx(CRefreshDialog::IDD, pParent),
		m_jpeg_update_worker( doc, refresh_file_ct ) 
{
	m_refresh_file_ct = refresh_file_ct ;
	m_doc = doc ;
	m_bar_pos = 0 ;
	m_status_image_resource_id = 0 ;

	m_timer_ID = 0 ;
	m_current_photo = NULL ;
}




CRefreshDialog::~CRefreshDialog()
{


}



void CRefreshDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CURRENT_FILE, m_current_file);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progress_bar);
}


BEGIN_MESSAGE_MAP(CRefreshDialog, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CRefreshDialog::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()







/******************************************************************************
CRefreshDialog::OnInitDialog

	Just display a progress dialog while Photoshop processes our photos 

******************************************************************************/
BOOL CRefreshDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_progress_bar.SetRange( 0, m_refresh_file_ct ) ;
	m_progress_bar.SetPos( 0 ) ;

	// timer for UI updates 
	m_timer_ID = SetTimer( StatusTimerID, 250, NULL ) ;

	return TRUE ;
}






/******************************************************************************
CRefreshDialog::OnBnClickedCancel

	Just do default handling of cancel 

******************************************************************************/
void CRefreshDialog::OnBnClickedCancel()
{
	m_jpeg_update_worker.CancelProcessing( ) ;

	// FIXME!!! Need to figure out how to queue up the cancel command 

	// can't do this immediately - need to give thread chance to clean up and exit 
	// CDialogEx::OnCancel();
}








/******************************************************************************
CRefreshDialog::OnTimer

	Process the timer message: 

	1. check the pipe for any new filename. If it is a new filenmae, be sure and 
	invalidate the dialog box so we get a WM_PAINT message 

	2. update the progress bar after each individual jpeg generated.  

******************************************************************************/
void CRefreshDialog::OnTimer(
	UINT_PTR nIDEvent					// I - timer event ID. We only process our own timer ID 
)
{
	if ( nIDEvent == m_timer_ID ) 
	{
		CString			line ;
		CString			fq_source_name ;
		bool			new_source ;

		if ( m_jpeg_update_worker.IsFinished( ) )
		{
			// display the finished checkmark 
			m_status_image_resource_id = IDB_UPDATE_DONE ;
			m_current_photo = NULL ;
			KillTimer( m_timer_ID ) ;
			Invalidate( ) ;

			// change cancel button to an OK button 
			GetDlgItem( IDOK )->EnableWindow( ) ;
			GetDlgItem( IDCANCEL )->EnableWindow( FALSE ) ;

			// hide the filename & progress bar 
			GetDlgItem( IDC_CURRENT_FILE )->ShowWindow( SW_HIDE ) ;
			GetDlgItem( IDC_PROGRESS_BAR_CAPTION )->ShowWindow( SW_HIDE ) ;
			GetDlgItem( IDC_PROGRESS_BAR )->ShowWindow( SW_HIDE ) ;
		}
		else if ( new_source = m_jpeg_update_worker.GetStatus( m_current_photo, m_bar_pos ) )
		{
			fq_source_name = m_current_photo->GetSourceFilePath( ) ;
			Invalidate( ) ;

			// We reverse the string b/c there is no "CString::ReverseFindOneOf" function 
			fq_source_name.MakeReverse( ) ;
			if ( -1 != fq_source_name.FindOneOf( _T( "/\\" ) ) )
				fq_source_name = fq_source_name.Left( fq_source_name.FindOneOf( _T( "/\\" ) ) ) ;
			fq_source_name.MakeReverse( ) ;

			m_current_file.SetWindowText( fq_source_name ) ;
			m_progress_bar.SetPos( m_bar_pos ) ;
		}
		else
		{
			// working on another jpeg
			m_progress_bar.SetPos( m_bar_pos ) ;
		}
	}

	CDialogEx::OnTimer( nIDEvent ) ;
}





/******************************************************************************
CRefreshDialog::OnPaint

	The only odd thing we do here is to use the GDI+ functions to paint a representation of the 
	current slide, or a status image

******************************************************************************/
void CRefreshDialog::OnPaint()
{
	CPaintDC	dc(this); // device context for painting
	Graphics	graphics( dc.GetSafeHdc( ) ) ;
	Image *		preview_jpeg = NULL ;
	CRect		frame_rect ;
	CRect		image_rect( 0, 0, 0, 0 ) ;
	Rect		dest_rect ;
	CGdiPlusBitmapResource	status_image ;

	GetDlgItem( IDC_PICTURE_FRAME )->GetWindowRect( &frame_rect ) ;
	ScreenToClient( &frame_rect ) ;
	dc.FillSolidRect( &frame_rect, RGB( 200, 200, 200 ) ) ;

	// now figure out what to paint - 
	if ( m_current_photo ) 
	{
		preview_jpeg = Image::FromFile( m_current_photo->GetPreviewJpeg( ) ) ;

		// horizontal or vertical? 
		if ( preview_jpeg->GetWidth( ) > preview_jpeg->GetHeight( ) )
		{
			image_rect.right = 80 * frame_rect.Width( ) / 100 ;
			image_rect.bottom = preview_jpeg->GetHeight( ) * image_rect.Width( ) / preview_jpeg->GetWidth( ) ;
		}
		else
		{
			image_rect.bottom = 80 * frame_rect.Height( ) / 100 ;
			image_rect.right = preview_jpeg->GetWidth( ) * image_rect.Height( ) / preview_jpeg->GetHeight( ) ;
		}

		dest_rect.X = frame_rect.left + ( frame_rect.Width( ) - image_rect.Width( ) ) / 2 ;
		dest_rect.Y = frame_rect.top + ( frame_rect.Height( ) - image_rect.Height( ) ) / 2 ;
		dest_rect.Height = image_rect.Height( ) ;
		dest_rect.Width = image_rect.Width( ) ;

		graphics.DrawImage( preview_jpeg, dest_rect ) ;
	}
	else if ( m_status_image_resource_id ) 
	{	
		// print an icon based on status - either cancelled, error, or finished OK 
		status_image.Load( m_status_image_resource_id, _T( "PNG" ) )  ;

		dest_rect.X = frame_rect.left ;
		dest_rect.Y = frame_rect.top ;
		dest_rect.Height = frame_rect.Height( ) ;
		dest_rect.Width = frame_rect.Width( ) ;

		graphics.DrawImage( (Bitmap *) status_image, dest_rect ) ;
	}
}


void CRefreshDialog::OnDestroy()
{
	CDialogEx::OnDestroy();

	if ( m_timer_ID )
		KillTimer( m_timer_ID ) ;
}
