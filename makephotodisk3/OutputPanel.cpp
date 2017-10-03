/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


View for the output panel below the light table 

***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MakePhotoDisk3Doc.h"
#include "OutputPanel.h"
#include "PhotoTitleDescrEditDlg.h"
#include "PhotoCaptureTimeEditDlg.h"


// COutputPanel
IMPLEMENT_DYNCREATE( COutputPanel, CFormView )

COutputPanel::COutputPanel(	)
	: CFormView( COutputPanel::IDD )
	, m_enable_extended_comment(FALSE)
	, m_comment(_T(""))
{
}

COutputPanel::~COutputPanel()
{
}


void COutputPanel::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	// DDX_Control(pDX, IDC_TITLE_AND_DESCRIPTION, m_title_subtitle);
	// DDX_Control(pDX, IDC_CREATED_WITH_CAMERA_ON_DATE, m_creation_static);
	DDX_Control(pDX, IDC_EXPOSURE_DATA, m_exposure_static);
	DDX_Check(pDX, IDC_ENABLE_EXTENDED_COMMENT, m_enable_extended_comment);
	DDX_Control(pDX, IDC_EXTENDED_EDIT, m_comment_edit_box);
	DDX_Control(pDX, IDC_ENABLE_EXTENDED_COMMENT, m_enable_extended_checkbox);
	DDX_Text(pDX, IDC_EXTENDED_EDIT, m_comment);
}




BEGIN_MESSAGE_MAP(COutputPanel, CFormView )
	ON_WM_PAINT()
	ON_STN_CLICKED(IDC_TITLE_AND_DESCRIPTION, &COutputPanel::OnStnClickedTitleAndDescription)
	ON_STN_CLICKED(IDC_CREATED_WITH_CAMERA_ON_DATE, &COutputPanel::OnStnClickedCreatedWithCameraOnDate)
	ON_STN_CLICKED(IDC_EXPOSURE_DATA, &COutputPanel::OnStnClickedExposureData)
	ON_BN_CLICKED(IDC_ENABLE_EXTENDED_COMMENT, &COutputPanel::OnBnClickedEnableExtendedComment)
END_MESSAGE_MAP()


#ifdef _DEBUG

CMakePhotoDisk3Doc* COutputPanel::GetDocument( ) const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMakePhotoDisk3Doc)));
	return (CMakePhotoDisk3Doc*)m_pDocument;
}

#endif





// COutputPanel message handlers


/**************************************************************************************************

**************************************************************************************************/
void COutputPanel::OnPaint()
{
	CPaintDC dc(this); // device context for painting
/*
	CRect		client_rect ;
	CString		work ;
	CFont		bold_font ;
	CFont		reg_font ;
	CFont		ital_font ;
	CFont *		dc_font ;
	int			line_spacing ;

	bold_font.CreateFont( -12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial" ) ) ;
	reg_font.CreateFont(  -12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial" ) ) ;
	ital_font.CreateFont(  -12, 0, 0, 0, FW_REGULAR, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial" ) ) ;

	GetClientRect( &client_rect ) ;
	dc.FillSolidRect( &client_rect, GetSysColor( COLOR_WINDOW ) ) ;

	dc.SetTextColor( RGB( 0, 0, 0 ) ) ;
	dc_font = dc.SelectObject( &bold_font ) ;

	if ( GetDocument( )->GetSelectedCollection( ) )
	{
		const PhotoImage * selected_image ;

		if ( selected_image = GetDocument( )->GetSelectedImage( ) ) 
		{
			// get the title & description
			work = selected_image->GetFormattedCaption( ) ;
			line_spacing = dc.DrawText( work, &client_rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX ) ;
			client_rect.top += 2 * line_spacing ;
			dc.SelectObject( &reg_font ) ;

				// get the sub-caption, if any - 
			work = selected_image->GetCreationSubCaption( ) ;
			if ( work.GetLength( ) )
			{
				line_spacing = dc.DrawText( work, &client_rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX ) ;
				client_rect.top += line_spacing ;
			}

				// get the expossure string, if any 
			work = selected_image->GetExposureSubCaption( ) ;
			if ( work.GetLength( ) )
			{
				line_spacing = dc.DrawText( work, &client_rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX ) ;
				client_rect.top += line_spacing ;
			}
		}
		else
		{
			work = GetDocument( )->GetSelectedCollection( )->GetName( ) ;
			line_spacing = dc.DrawText( work, &client_rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX ) ;
			client_rect.top += line_spacing ;
			dc.SelectObject( &reg_font ) ;

			if ( GetDocument( )->GetSelectedCollection( )->GetIsCollectionSet( ) )
				work = _T( "A collection of collections." ) ;
			else
			{
				work.Format( _T( "A collection of %d images." ), GetDocument( )->GetPhotoListSize( ) ) ;
			}
			dc.DrawText( work, &client_rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX ) ;
		}
	}
	dc.SelectObject( dc_font ) ;
*/
}




/**************************************************************************************************
Invoked via CDocument::UpdateAllViews. update_type tells us why we're being updated. We don't 
need the other arguments 

**************************************************************************************************/
void COutputPanel::OnUpdate(CView* /*pSender*/, LPARAM update_type, CObject* /*pHint*/)
{
	CString	work ;
	PhotoImage * selected_image ;

	switch ( update_type ) 
	{
		case CMakePhotoDisk3Doc::CatalogSelected :
			ScrollToTopOfForm( ) ;

			break ;

		case CMakePhotoDisk3Doc::CollectionSelected : 
			ScrollToTopOfForm( ) ;

			m_title_subtitle.SetInteractiveMode( false ) ;
			m_creation_static.SetInteractiveMode( false ) ;
			m_title_subtitle.SetText( GetDocument( )->GetSelectedCollection( )->GetName( ), FALSE ) ;
			// m_title_subtitle.SetFont( &m_bold_font ) ;			

			if ( GetDocument( )->GetSelectedCollection( )->GetIsCollectionSet( ) )
				work = _T( "A collection of collections." ) ;
			else
			{
				work.Format( _T( "A collection of %d images." ), GetDocument( )->GetPhotoListSize( ) ) ;
			}
			m_creation_static.SetText( work, false) ;
			m_exposure_static.SetWindowText( _T( "" ) ) ;
			m_enable_extended_checkbox.EnableWindow(FALSE);
			m_enable_extended_checkbox.ShowWindow(SW_HIDE);
			m_comment_edit_box.EnableWindow(FALSE);
			break;

		case CMakePhotoDisk3Doc::ImageSelected :
			ScrollToTopOfForm( ) ;

			if ( 1 == GetDocument( )->GetSelectionCount( ) )
			{
				if ( selected_image = GetDocument( )->GetLastSelectedImage( ) ) 
				{
					// get the title & description
					m_title_subtitle.SetText( selected_image->GetFormattedCaption( ), selected_image->GetFormattedCaptionOverrides( ) ) ;
					m_title_subtitle.SetInteractiveMode( ) ;

					// get the sub-caption, if any - 
					m_creation_static.SetText( selected_image->GetCreationSubCaption( ), selected_image->GetCreationSubCaptionOverrides( ) ) ;
					m_creation_static.SetInteractiveMode( ) ;

					// get the expossure string, if any 
					work = selected_image->GetExposureSubCaption( ) ;
					m_exposure_static.SetWindowText( work ) ;
				}
				else
					ASSERT( FALSE ) ;

				m_title_subtitle.SetInteractiveMode( true ) ;
				m_creation_static.SetInteractiveMode( true ) ;
				m_enable_extended_checkbox.EnableWindow( TRUE ) ;
				m_enable_extended_checkbox.ShowWindow( SW_SHOW ) ;

				if (selected_image->GetExtendedComment( work ))
				{
					m_enable_extended_comment = TRUE ;
					PhotoImage::RemoveHTMLCoding( work ) ;
					m_comment = work ;
					m_comment_edit_box.EnableWindow( TRUE ) ;
					m_comment_edit_box.ShowWindow( SW_SHOW ) ;
				}
				else
				{
					m_enable_extended_comment = FALSE ;
					m_comment = _T("") ;
					m_comment_edit_box.EnableWindow( FALSE ) ;
					m_comment_edit_box.ShowWindow( SW_HIDE ) ;
				}

				UpdateData(FALSE);
			}
			else
			{
				m_title_subtitle.SetInteractiveMode( false ) ;
				m_creation_static.SetInteractiveMode( false ) ;
				m_creation_static.SetText( _T( "" ), false ) ;
				m_exposure_static.SetWindowText( _T( "" ) ) ;
				m_enable_extended_checkbox.EnableWindow( FALSE );
				m_enable_extended_checkbox.ShowWindow( SW_HIDE );
				m_comment_edit_box.EnableWindow( FALSE ) ;
				m_comment_edit_box.ShowWindow( SW_HIDE ) ;
				CString	multi_select_caption ;

				multi_select_caption.Format( _T( "%d images selected" ), GetDocument( )->GetSelectionCount( ) ) ;
				m_title_subtitle.SetText( multi_select_caption, false ) ;
			}
			break ;

		case CMakePhotoDisk3Doc::ImageDeselected : 
			ScrollToTopOfForm( ) ;

			if (1 == GetDocument()->GetSelectionCount())
			{
				if (selected_image = GetDocument()->GetLastSelectedImage())
				{
					UpdateData( TRUE ) ;
					
					if ( m_enable_extended_comment && m_comment.GetLength( ) )
					{
						PhotoImage::AddHTMLCoding( m_comment ) ;
						selected_image->SetExtendedComment( m_comment ) ;
					}
					else
						selected_image->SetExtendedComment( _T( "" ) ) ;
				}
			}

			m_title_subtitle.SetInteractiveMode( false ) ;
			m_creation_static.SetInteractiveMode( false ) ;
			m_enable_extended_checkbox.EnableWindow(FALSE);
			m_enable_extended_checkbox.ShowWindow(SW_HIDE);
			m_comment_edit_box.EnableWindow(FALSE);
			break;

		default :
			break ;
	}
}



/*********************************************************************************************
Just a little helper-function to scroll the view back to the home position - in case user has 
scrolled down to view edit-box for extended comment. 

*********************************************************************************************/
void COutputPanel::ScrollToTopOfForm( )
{
	POINT	origin ;

	origin.x = origin.y = 0 ;
	ScrollToPosition( origin ) ;
}






/**************************************************************************************************
In the future, we will need this in order to scroll the 


**************************************************************************************************/
BOOL COutputPanel::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	return TRUE ;
}






void COutputPanel::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	PhotoImage *	selected_image ;

	if ( 1 == GetDocument( )->GetSelectionCount( ) ) 
		selected_image = GetDocument( )->GetLastSelectedImage( ) ;
	else
		selected_image = NULL ;

/*
	// create our custom fonts 
	CFont * title_font = m_title_subtitle.GetFont( ) ;
	LOGFONT	original_log_font ;
	LOGFONT	work_log_font ;

	title_font->GetLogFont( &original_log_font ) ;
	work_log_font = original_log_font ;
	work_log_font.lfWeight = FW_BOLD ;
//	m_bold_font.CreateFontIndirect( &work_log_font ) ;
*/

	m_creation_static.SubclassDlgItem( IDC_CREATED_WITH_CAMERA_ON_DATE, this ) ;
//	m_creation_static.ModifyStyle( 0, SS_NOTIFY ) ;

	m_title_subtitle.SubclassDlgItem( IDC_TITLE_AND_DESCRIPTION, this ) ;
//	m_title_subtitle.ModifyStyle( 0, SS_NOTIFY ) ;

	m_exposure_static.SetWindowText( _T( "" )) ;

	if ( selected_image ) 
		m_creation_static.SetText( selected_image->GetCreationSubCaption( ), selected_image->GetCreationSubCaptionOverrides( ) ) ;

	GetDlgItem( IDC_EXPOSURE_DATA )->ModifyStyle( 0, SS_NOTIFY ) ;

	// TODO: Add your specialized code here and/or call the base class
}


void COutputPanel::OnStnClickedTitleAndDescription()
{
	PhotoImage *		selected_image ;

	if ( 1 == GetDocument( )->GetSelectionCount( ) && ( selected_image = GetDocument( )->GetLastSelectedImage( ) ) )
	{
		CPhotoTitleDescrEditDlg		dlg( selected_image, this ) ;

		if ( dlg.DoModal( ) == IDOK ) 
		{
			m_title_subtitle.SetText( selected_image->GetFormattedCaption( ), selected_image->GetFormattedCaptionOverrides( ) ) ;
			GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::MetadataUpdated ) ;
		}
	}
}


void COutputPanel::OnStnClickedCreatedWithCameraOnDate()
{
	PhotoImage *				selected_image ;

	if ( 1 == GetDocument( )->GetSelectionCount( ) && ( selected_image = GetDocument( )->GetLastSelectedImage( ) ) )
	{
		CPhotoCaptureTimeEditDlg	dlg( selected_image, this ) ;
		SYSTEMTIME					before_time ;

		before_time = selected_image->GetCreateTime( ) ;

		if ( dlg.DoModal( ) == IDOK ) 
		{
			m_creation_static.SetText( selected_image->GetCreationSubCaption( ), selected_image->GetCreationSubCaptionOverrides( ) ) ;

			if ( selected_image->CompareCreateTime( before_time ) && GetDocument( )->GetSortByCaptureTime( ) )
				GetDocument( )->ChangedCreateTime( selected_image, selected_image->CompareCreateTime( before_time ), true ) ;

			GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::MetadataUpdated ) ;
		}
	}
}



void COutputPanel::OnStnClickedExposureData()
{
	// TODO: Add your control notification handler code here
}




void COutputPanel::OnBnClickedEnableExtendedComment()
{
	PhotoImage *				selected_image ;

	if ( 1 == GetDocument( )->GetSelectionCount( ) && ( selected_image = GetDocument( )->GetLastSelectedImage( ) ) )
	{
		selected_image->SetHasDirtyEdits( ) ;

		UpdateData( TRUE ) ;

		m_comment_edit_box.EnableWindow( m_enable_extended_comment ) ;
		m_comment_edit_box.ShowWindow( m_enable_extended_comment ? SW_SHOW : SW_HIDE ) ;
	}
}
