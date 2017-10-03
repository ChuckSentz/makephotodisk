/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	
	The "panel" is the pane on the left of the application window which is changed
	out as the user moves through the process of defining the application.

	At present, there are three panels and they're displayed in the following 
	order with a new project:

	Panel					Resource ID						Class
	----------------------	------------------------------	----------------------
	Select collection		IDD_SELECT_CATALOG_COLLECTION	CPanelSelectCollection
	Define project			IDD_DEFINE_PROJECT				CPanelDefineProject
	Update/Create project	IDD_UPDATE_PROJECT				CPanelEditUpdateDisk

	If the user loads a project, he starts on Update/Create project.

***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MakePhotoDisk3Doc.h"
#include "SplitterWndEx.h"
#include "PanelEditUpdateDisk.h"
#include "RefreshDialog.h" 
#include "UpdateHTMLWorker.h"


// static helper function 
static bool GetDreamweaverPath( CString & dreamweaver_exe ) ;



// CPanelEditUpdateDisk

IMPLEMENT_DYNCREATE(CPanelEditUpdateDisk, CFormView)

CPanelEditUpdateDisk::CPanelEditUpdateDisk()
	: CFormView(CPanelEditUpdateDisk::IDD)
{
	m_disk_icon_image = NULL ;
	m_key_pic_image = NULL ;
	ZeroMemory( &m_disk_icon_time, sizeof( m_disk_icon_time ) ) ;
	ZeroMemory( &m_key_pic_time, sizeof( m_key_pic_time ) ) ;
}

CPanelEditUpdateDisk::~CPanelEditUpdateDisk()
{
}

void CPanelEditUpdateDisk::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DISK_TITLE_EDIT, m_disk_title_edit);
	DDX_Control(pDX, IDC_DISK_REVISION_EDIT, m_revision_edit);
	DDX_Control(pDX, IDC_DISK_TYPE_TEXT, m_disk_type_text);
}






/******************************************************************************
******************************************************************************/
BEGIN_MESSAGE_MAP(CPanelEditUpdateDisk, CFormView)
	ON_BN_CLICKED(IDC_UDPATE_FILES, &CPanelEditUpdateDisk::OnBnClickedUdpateFiles)
	ON_BN_CLICKED(IDC_EDIT_START_PAGE, &CPanelEditUpdateDisk::OnBnClickedEditStartPage)
//	ON_STN_CLICKED(IDC_DISK_TYPE_TEXT, &CPanelEditUpdateDisk::OnStnClickedDiskTypeText)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_LARGE_PRESENTATION, &CPanelEditUpdateDisk::OnBnClickedLargePresentation)
	ON_BN_CLICKED(IDC_COMPRESSED, &CPanelEditUpdateDisk::OnBnClickedCompressed)
	ON_BN_CLICKED(IDC_FACEBOOK, &CPanelEditUpdateDisk::OnBnClickedFacebook)
	ON_BN_CLICKED(IDC_FULL_SIZE, &CPanelEditUpdateDisk::OnBnClickedFullSize)
	ON_BN_CLICKED(IDC_WATERMARK_PRESENTATION, &CPanelEditUpdateDisk::OnBnClickedWatermarkPresentation)
	ON_BN_CLICKED(IDC_WATERMARK_COMPRESSED, &CPanelEditUpdateDisk::OnBnClickedWatermarkCompressed)
	ON_BN_CLICKED(IDC_WATERMARK_FACEBOOK, &CPanelEditUpdateDisk::OnBnClickedWatermarkFacebook)
	ON_BN_CLICKED(IDC_WATERMARK_FULL_SIZE, &CPanelEditUpdateDisk::OnBnClickedWatermarkFullSize)
END_MESSAGE_MAP()


// CPanelEditUpdateDisk diagnostics

#ifdef _DEBUG
void CPanelEditUpdateDisk::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPanelEditUpdateDisk::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif

CMakePhotoDisk3Doc* CPanelEditUpdateDisk::GetDocument( ) const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMakePhotoDisk3Doc)));
	return (CMakePhotoDisk3Doc*)m_pDocument;
}

#endif //_DEBUG





// CPanelEditUpdateDisk message handlers


/******************************************************************************
CPanelEditUpdateDisk::OnInitialUpdate
	
	Actually a virtual override. Our splitter window class sends the WM_INITIALUPDATE 
	message whenever this view-class is swapped into a panel  

******************************************************************************/
void CPanelEditUpdateDisk::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
 
	CRect dlg_rect( 0, 0, 206, 380 ) ;
	CSplitterWndFlex * parent_splitter ;

	MapDialogRect( m_hWnd, &dlg_rect ) ;
	parent_splitter = dynamic_cast< CSplitterWndFlex *>( GetParent( ) ) ;
	parent_splitter->SetColumnInfo( 0, dlg_rect.Width( ), dlg_rect.Width( ) ) ;
	parent_splitter->RecalcLayout( ) ;

	CString	disk_descr_str ;
	CString	disk_type_str ; 
	CString catalog_path ;

	catalog_path = GetDocument( )->GetCatalog( ) ;

	// display the selected catalog and collection. If the catalog path is too long to fit in the edit box, we make sure it's scrolled all the way to the right. The user can click on the edit box 
	// and select the whole contents, or scroll left if desired, etc. 
	GetDlgItem( IDC_CATALOG )->SetWindowText( catalog_path ) ;
	( (CEdit *) GetDlgItem( IDC_CATALOG ) )->SetSel( catalog_path.GetLength( ), catalog_path.GetLength( ) ) ;
	GetDlgItem( IDC_COLLECTION )->SetWindowText( GetDocument( )->GetSelectedCollection( )->GetName( ) ) ;

	switch ( GetDocument( )->GetBasicDiskType( ) )
	{
		case CMakePhotoDisk3Doc::FamilyAndFriendsDisk :
			disk_type_str = _T( "Family && Friends disk" ) ;
			break ;

		case CMakePhotoDisk3Doc::AssignmentProduct : 
			disk_type_str = _T( "Assignment disk" ) ;
			break ;

		case CMakePhotoDisk3Doc::ClientDisk : 
			disk_type_str = _T( "Client disk" ) ;
			break ;

		case CMakePhotoDisk3Doc::SamplerDisk : 
			disk_type_str = _T( "Sampler disk" ) ;
			break ;
				
		default : 
			ASSERT( FALSE ) ;
			break ;
	}
	disk_descr_str.Format( _T( "%d photos in a %s" ), GetDocument( )->GetPhotoListSize( ), (const TCHAR *) disk_type_str ) ;
	m_disk_type_text.SetWindowText( disk_descr_str ) ;
	m_disk_title_edit.SetWindowText( GetDocument( )->GetDiskTitle( ) ) ;

	CString version_no ;
	version_no.Format( _T( "%d.%d" ), GetDocument( )->GetMajorRevision( ), GetDocument( )->GetMinorRevision( ) ) ;
	m_revision_edit.SetWindowTextW( version_no ) ;

	CheckUpdateDiskUsageText( ) ;

	SetTimer( MyTimerID, 500, NULL ) ;

	// load up the GDI+ "Image" objects for the disk icon and key image - 
	CheckAndLoadPreviewImage( _T( "DiskIcon.jpg" ), m_disk_icon_image, m_disk_icon_time ) ;
	CheckAndLoadPreviewImage( _T( "KeyImage.jpg" ), m_key_pic_image, m_key_pic_time ) ;

	// might want to update these via the DDX/DDV DoDataExchange method... or might not 
	( (CButton *) GetDlgItem( IDC_LARGE_PRESENTATION ) )->SetCheck( GetDocument( )->GetDoBigPresentation( ) ? BST_CHECKED : BST_UNCHECKED ) ;
	( (CButton *) GetDlgItem( IDC_COMPRESSED ) )->SetCheck( GetDocument( )->GetDoCompressed( ) ? BST_CHECKED : BST_UNCHECKED ) ;
	( (CButton *) GetDlgItem( IDC_FACEBOOK ) )->SetCheck( GetDocument( )->GetDoFacebook( ) ? BST_CHECKED : BST_UNCHECKED ) ;
	( (CButton *) GetDlgItem( IDC_FULL_SIZE ) )->SetCheck( GetDocument( )->GetDoFullSize( ) ? BST_CHECKED : BST_UNCHECKED ) ;
}








/******************************************************************************
CPanelEditUpdateDisk::CheckAndLoadPreviewImage

	This is used for the "key image" and "disk icon" images in the panel. 

	Check for the desired jpeg in the misc directory and create a GDI+ Image object 
	for it if found. 

	What we do in this function may, at first glance, seem very arcane. To wit, 
	if we find the desired jpeg, we create a temp file and copy the jpeg into it, 
	then create a stream on the file, then create an image on that stream, then 
	release the stream then delete the temp file. 

	This method was arrived at after experimenting with and finding massive 
	problems with the simplest and most direct method. If you simply use Image::FromFile 
	directly on the jpeg, it locks up the jpeg until you exit the program. Using 
	SHCreateStreamFromFileEx and then using Image::FromStream was suggested online
	as a way of constructing the Image without tying up the file... but that method 
	still leaves the file in a weird state. Basically, after releasing the stream, 
	you CAN delete the file from ANOTHER program (like explorer or command line) but 
	any call to DeleteFile from THIS program returns success (TRUE) without actually
	deleting the file, and any attempt to delete the file subsequent to the call to 
	DeleteFile will result in a sharing violation. The file WILL disappear only 
	after our program exits. This seems like fairly buggy behavior on the part 
	of Windows, but it is the way it is. 

	So the workaround is to create a temp file, create the stream on that temp file, 
	create the image on that stream, release the stream then call DeleteFile on 
	the temp file. As described above, the temp file will remain around until the 
	program exits... but the original jpeg (KeyImage.jpg, or DiskIcon.jpg) CAN be 
	overwritten when the user selects a new key image or disk icon. The user can 
	repeat this process many times, and accumulate a whole collection of temp 
	jpegs which will all delete once the user exits the program. 

******************************************************************************/
void CPanelEditUpdateDisk::CheckAndLoadPreviewImage( 
	const TCHAR * jpeg_name,				// I - jpeg name w/o path 
	Image * & image_ptr,					// O - gets GDI+ image object 
	FILETIME & write_time					// O - gets the write time of the jpeg 
) 
{
	CString						jpeg_path ;
	WIN32_FILE_ATTRIBUTE_DATA	file_info ;

	jpeg_path.Format( _T( "%s\\misc\\%s" ), GetDocument( )->GetProjectPath( ), jpeg_name ) ;

		// does the desired jpeg file exist in the misc directory? 
	if ( GetFileAttributesEx( jpeg_path, GetFileExInfoStandard, &file_info ) ) 
	{
		CString temp_path ;
		TCHAR	temp_jpeg_path[ MAX_PATH ] ;

		write_time = file_info.ftLastWriteTime ;

		// yes it does - create the ephemeral temp copy which will be used to create the Image... 
		temp_path.Format( _T( "%s\\misc" ), GetDocument( )->GetProjectPath( ) ) ;

		GetTempFileName( temp_path, _T( "tmp" ), 0, temp_jpeg_path ) ;
		CopyFile( jpeg_path, temp_jpeg_path, FALSE ) ;

		{
			IStream *	pStream ;

			if ( S_OK == ::SHCreateStreamOnFileEx( temp_jpeg_path, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &pStream ) ) 
			{
				image_ptr = Image::FromStream( pStream ) ;
				pStream->Release( ) ;
			}
		}

		DeleteFile( temp_jpeg_path ) ;
	}
	else
	{
		image_ptr = NULL ;		// desired jpeg file does NOT exist, so make sure the Image ptr is NULL 
		ZeroMemory( &write_time, sizeof( FILETIME ) ) ;
	}
}






/******************************************************************************
CPanelEditUpdateDisk::CheckUpdateDiskUsageText

	Estimate the disk space needed on the target disk 

******************************************************************************/
void CPanelEditUpdateDisk::CheckUpdateDiskUsageText( )
{
	// if our jpeg's are up-to-date, then calculate the disk space requirement 
	if ( !GetDocument( )->AnyRefreshNeeded( ) )
	{
		UINT64	disk_space ;
		CString	work ;

		disk_space = ( GetDocument( )->GetDiskUsage( ) + 999999 ) / 1000000 ;

		if ( disk_space > 700 )
			work.Format( _T( "Files total %I64d Mb. A DVD is probably needed." ), disk_space ) ;
		else
			work.Format( _T( "Files total %I64d Mb. A CD-ROM should be sufficient." ), disk_space ) ;

		GetDlgItem( IDC_DISK_USAGE )->SetWindowText( work ) ;
	}
}





/******************************************************************************
CPanelEditUpdateDisk::OnBnClickedUdpateFiles

	Message-mapped handler for user-clicks on Update Files button 

******************************************************************************/
void CPanelEditUpdateDisk::OnBnClickedUdpateFiles()
{
	int	refresh_file_ct ;

	BeginWaitCursor( ) ;
	
	GetDocument( )->RealignOrAssignOutputNumbers( ) ;
	GetDocument( )->DeleteUnwantedJpegOutput( ) ;
	GetDocument( )->DeleteOrphanedHTML( ) ;
	GetDocument( )->SaveDocument( ) ;
	GetDocument( )->TallyUpJpegRefreshCount( refresh_file_ct ) ;


	EndWaitCursor( ) ;

	if ( refresh_file_ct ) 
	{
		CRefreshDialog	dlg( GetDocument( ), refresh_file_ct ) ;
		dlg.DoModal( ) ;
	}

	GetDocument( )->ResetPhotoTimes( ) ;
	CheckUpdateDiskUsageText( ) ;

	// update the HTML, now that the jpeg's are generated 
	BeginWaitCursor( ) ;
	UpdateHTML( GetDocument( ) ) ;
	EndWaitCursor( ) ;

	GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::JpegsRefreshed ) ;
}




/******************************************************************************
GetDreamweaverPath

	Fills in the passed CString with the fully qualified pathname of the dreamweaver exe. 
	
	Ideally, I would do some kind of File association magic to determine the user's preferred HTML 
	editor and return it (whether it's Adobe Dreamweaver, or Microsoft Expression, or whatver) but
	this is actually pretty hard to do. AssocQueryString( ), or the underlying iQueryAssociations
	interface, will only return the default browser (firefox, internet explorer, safari, chrome, etc).
	
	AssocQueryString returns an error code (0x80070483 - no application specified for this operation) 
	if you try to get the assocation for "edit" verb. 

	Could root around in the registry for program names associated with .htm.... but then I still 
	have to recognize whether the program name specified is a browser or an html editor. 

******************************************************************************/
bool GetDreamweaverPath( 
	CString & dreamweaver_exe					// O - gets the fully qualified .exe name, suitable for CreateProcess 
)
{
	CString		app_keyname ;
	CRegKey		app_key ;
	bool		isOK = false ;
	LONG		ec ;

	app_keyname.Format( _T( "Applications\\dreamweaver.exe\\shell\\Open\\command" ) ) ;

	if ( ERROR_SUCCESS == ( ec = app_key.Open( HKEY_CLASSES_ROOT, app_keyname, KEY_READ ) ) ) 
	{
		TCHAR	install_path[ 1024 ] ;
		ULONG	install_path_len ;

		install_path_len = sizeof( install_path ) / sizeof( install_path[ 0 ] ) ;
		if ( ERROR_SUCCESS == ( ec = app_key.QueryStringValue( _T( "InstallPath" ), install_path, &install_path_len ) ) )
		{
			isOK = true ;
			// InstallPath includes the trailing slash... 
			dreamweaver_exe.Format( _T( "%sdreamweaver.exe" ), (const TCHAR *) install_path ) ;
		}
	}

	return isOK ;
}





/******************************************************************************
CPanelEditUpdateDisk::UpdateStartPageFields

	Replace some delimited fields in the start page, including revision number, disk title, etc. 

******************************************************************************/
void CPanelEditUpdateDisk::UpdateStartPageFields( 
	const TCHAR * start_page_path			// I - fully qualified name of start page 
)
{
	CString		start_page_html ;

	if ( LoadUTF8File( start_page_path, start_page_html ) )
	{
		CString	revision_str ;
		CString	disk_title_str ;
		CString revision_number_str ;

		m_disk_title_edit.GetWindowText( disk_title_str ) ;
		m_revision_edit.GetWindowText( revision_number_str ) ;

		revision_str.Format( _T( "revision %s" ), (const TCHAR *) revision_number_str ) ;

		// replace the content between the delimiters... but keep the delimiters intact
		ReplaceDelimitedContent( start_page_html, _T( "<!--revnum-->" ), _T( "<!--/revnum-->" ), revision_str ) ;
		ReplaceDelimitedContent( start_page_html, _T( "<!--disktitle-->" ), _T( "<!--/disktitle-->" ), disk_title_str ) ;

		SaveUTF8File( start_page_path, start_page_html ) ;
	}
}





/******************************************************************************
CPanelEditUpdateDisk::OnBnClickedEditStartPage

	Invokes an HTML editor on the start page so the user can enter custom text 

******************************************************************************/
void CPanelEditUpdateDisk::OnBnClickedEditStartPage()
{
	CString		start_page_path ;
	CString		process_arg ;
	CString		cmd_line ;
	TCHAR *		stupid_cmd_line_ptr ;

	start_page_path.Format( _T( "%s\\StartPage.htm" ), GetDocument( )->GetProjectPath( ) ) ;

	UpdateStartPageFields( start_page_path ) ;

	GetDreamweaverPath( process_arg ) ;
	cmd_line.Format( _T( "\"%s\" \"%s\"" ), (const TCHAR *) process_arg, (const TCHAR *) start_page_path ) ;

	STARTUPINFO			startup_info ;
	PROCESS_INFORMATION	process_info ;

	ZeroMemory( &startup_info, sizeof( startup_info ) ) ;
	startup_info.cb = sizeof( startup_info ) ;
	ZeroMemory( &process_info, sizeof( process_info ) ) ;

	stupid_cmd_line_ptr = cmd_line.GetBufferSetLength( 1024 ) ;

	if ( CreateProcess( process_arg, stupid_cmd_line_ptr, NULL, NULL, FALSE, 0, NULL, GetDocument( )->GetProjectPath( ), &startup_info, &process_info ) )
	{
		cmd_line.ReleaseBuffer( ) ;

		CloseHandle( process_info.hProcess ) ;
		CloseHandle( process_info.hThread ) ;
	}
}








/******************************************************************************
CPanelEditUpdateDisk::OnPaint

	Windows handles all the std controls and stuff - all we need to do is paint 
	the GDI+ images for the "key image" and "disk icon" images, if present 

******************************************************************************/
void CPanelEditUpdateDisk::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	Graphics	graphics( dc.GetSafeHdc( ) ) ;
	CRect		frame_rect ;			// MFC rect
	Rect		dest_rect ;				// GDI+ rect 

	if ( m_disk_icon_image )
	{
		GetDlgItem( IDC_DISK_ICON_PIC )->GetWindowRect( &frame_rect ) ;
		ScreenToClient( &frame_rect ) ;

		dest_rect.X = frame_rect.left ;
		dest_rect.Y = frame_rect.top ;
		dest_rect.Height = frame_rect.Height( ) ;
		dest_rect.Width = frame_rect.Width( ) ;

		graphics.DrawImage( m_disk_icon_image, dest_rect ) ;
	}

	if ( m_key_pic_image ) 
	{
		GetDlgItem( IDC_KEY_IMAGE_PIC )->GetWindowRect( &frame_rect ) ;
		ScreenToClient( &frame_rect ) ;

		dest_rect.X = frame_rect.left ;
		dest_rect.Y = frame_rect.top ;
		dest_rect.Height = frame_rect.Height( ) ;
		dest_rect.Width = frame_rect.Width( ) ;

		graphics.DrawImage( m_key_pic_image, dest_rect ) ;
	}
}






/******************************************************************************
CPanelEditUpdateDisk::OnDestroy

	Clean up our window's stuff - just kills the timer at present 

******************************************************************************/
void CPanelEditUpdateDisk::OnDestroy()
{
	KillTimer( MyTimerID ) ;

	CFormView::OnDestroy();
}




/******************************************************************************
CPanelEditUpdateDisk::OnTimer

	Just poll the misc subdirectory for updates to DiskIcon.jpg and/or KeyImage.jpg 

******************************************************************************/
void CPanelEditUpdateDisk::OnTimer(
	UINT_PTR nIDEvent					// I - timer event ID passed by Windows OS
)
{
	if ( nIDEvent == MyTimerID )
	{
		CString						jpeg_path ;
		WIN32_FILE_ATTRIBUTE_DATA	file_info ;
		bool						any_change ;

		any_change = false ;

		jpeg_path.Format( _T( "%s\\misc\\DiskIcon.jpg" ), GetDocument( )->GetProjectPath( ) ) ;
		if ( !GetFileAttributesEx( jpeg_path, GetFileExInfoStandard, &file_info ) ) 
			ZeroMemory( &file_info.ftLastWriteTime, sizeof( FILETIME ) ) ;

		if ( memcmp( &file_info.ftLastWriteTime, &m_disk_icon_time, sizeof( FILETIME ) ) )
		{
			CheckAndLoadPreviewImage( _T( "DiskIcon.jpg" ), m_disk_icon_image, m_disk_icon_time ) ;
			any_change = true ;
		}

		jpeg_path.Format( _T( "%s\\misc\\KeyImage.jpg" ), GetDocument( )->GetProjectPath( ) ) ;
		if ( !GetFileAttributesEx( jpeg_path, GetFileExInfoStandard, &file_info ) ) 
			ZeroMemory( &file_info.ftLastWriteTime, sizeof( FILETIME ) ) ;

		if ( memcmp( &file_info.ftLastWriteTime, &m_key_pic_time, sizeof( FILETIME ) ) ) 
		{
			CheckAndLoadPreviewImage( _T( "KeyImage.jpg" ), m_key_pic_image, m_key_pic_time ) ;
			any_change = true ;
		}

		if ( any_change ) 
			Invalidate( TRUE ) ;
	}

	CFormView::OnTimer(nIDEvent);
}






/******************************************************************************
CPanelEditUpdateDisk::OnBnClickedLargePresentation
CPanelEditUpdateDisk::OnBnClickedCompressed
CPanelEditUpdateDisk::OnBnClickedFacebook
CPanelEditUpdateDisk::OnBnClickedFullSize

	Handlers for the new jpeg output buttons. 
	
******************************************************************************/
void CPanelEditUpdateDisk::OnBnClickedLargePresentation()
{
	bool checked ;

	checked = ( (CButton *) GetDlgItem( IDC_LARGE_PRESENTATION ) )->GetCheck( ) == BST_CHECKED ;
	GetDocument( )->SetDoBigPresentation( checked ) ;

	GetDocument( )->ResetPhotoTimes( ) ;
	GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::JpegsRefreshed ) ;
}

void CPanelEditUpdateDisk::OnBnClickedCompressed()
{
	bool checked ;

	checked = ( (CButton *) GetDlgItem( IDC_COMPRESSED ) )->GetCheck( ) == BST_CHECKED ;
	GetDocument( )->SetDoCompressed( checked ) ;

	GetDocument( )->ResetPhotoTimes( ) ;
	GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::JpegsRefreshed ) ;
}

void CPanelEditUpdateDisk::OnBnClickedFacebook()
{
	bool checked ;

	checked = ( (CButton *) GetDlgItem( IDC_FACEBOOK ) )->GetCheck( ) == BST_CHECKED ;
	GetDocument( )->SetDoFacebook( checked ) ;

	GetDocument( )->ResetPhotoTimes( ) ;
	GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::JpegsRefreshed ) ;
}

void CPanelEditUpdateDisk::OnBnClickedFullSize()
{
	bool checked ;

	checked = ( (CButton *) GetDlgItem( IDC_FULL_SIZE ) )->GetCheck( ) == BST_CHECKED ;
	GetDocument( )->SetDoFullSize( checked ) ;

	GetDocument( )->ResetPhotoTimes( ) ;
	GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::JpegsRefreshed ) ;
}





/******************************************************************************
CPanelEditUpdateDisk::OnBnClickedWatermarkPresentation
CPanelEditUpdateDisk::OnBnClickedWatermarkCompressed
CPanelEditUpdateDisk::OnBnClickedWatermarkFacebook
CPanelEditUpdateDisk::OnBnClickedWatermarkFullSize

	Dummy handlers for watermark buttons. Will implement these for real 
	when and if I decide how I want to actually implement watermarking. 

	I'm half-inclined just to have a single watermark option and if checked
	it means everything but thumbnails get a watermark. 

******************************************************************************/
void CPanelEditUpdateDisk::OnBnClickedWatermarkPresentation()
{
	// TODO: Add your control notification handler code here
}

void CPanelEditUpdateDisk::OnBnClickedWatermarkCompressed()
{
	// TODO: Add your control notification handler code here
}

void CPanelEditUpdateDisk::OnBnClickedWatermarkFacebook()
{
	// TODO: Add your control notification handler code here
}

void CPanelEditUpdateDisk::OnBnClickedWatermarkFullSize()
{
	// TODO: Add your control notification handler code here
}
