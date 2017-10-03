// ControlPanel.cpp : implementation file
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MakePhotoDisk3Doc.h"
#include "MakePhotoDisk3View.h"
#include "MainFrm.h"
#include "ControlPanel.h"
#include "ConfirmProjectCreation.h"


BOOL BrowseForFolder(HWND hwnd, LPCTSTR szCurrent, LPTSTR szPath) ;



// CControlPanel

IMPLEMENT_DYNAMIC(CControlPanel, CWnd)

CControlPanel::CControlPanel( CMakePhotoDisk3View * view )
	: 	m_view ( view )
{
}

CControlPanel::~CControlPanel()
{
}


BEGIN_MESSAGE_MAP(CControlPanel, CWnd)
	ON_BN_CLICKED( IDC_CATALOG_BROWSE, &CControlPanel::OnBnClickedCatalogBrowse)
	ON_BN_CLICKED( IDC_NEXT_STEP, &CControlPanel::OnBnClickedNext )
	ON_BN_CLICKED( IDC_BROWSE_DIRECTORY, &CControlPanel::OnBnClickedDirectoryBrowse )
	ON_BN_CLICKED( IDC_CREATE_PROJECT, &CControlPanel::OnBnClickedCreateProject ) 
	ON_CBN_SELENDOK( IDC_CATALOG_COMBO, &CControlPanel::OnCbnSelendokCatalogCombo)
	ON_CBN_SELENDOK( IDC_PROJECT_TYPE_COMBO, &CControlPanel::OnCBnSelendokDiskTypeCombo )
	ON_CBN_SELENDOK( IDC_PROJECT_LOCATION, &CControlPanel::OnCBnSelendokProjectLocation ) 
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
END_MESSAGE_MAP()





// CControlPanel message handlers










/******************************************************************************
void CControlPanel::OnPaint

Handler for paint message. Since the control panel's controls change depending on
what mode we're in, need to switch/case off the current state (stored in the view)

Operating modes are as follows, with the first two modes only available after the
user has opted to create a new project
	
	WelcomeScreen
			Control panel the same as below, but in this state when no collection 
			is selected. Prints a introductory message in the light table's area

	SelectACollection 
			User determines which catalog file he wants to use, then browses the
			collections available in that catalog. 

			When the user clicks "Next", he goes to DefineProject phase 

	DefiningProject
			After hitting "next" on the page for the first mode, the user gets to 
			choose a location wherein the disk image will be assembled, and the 
			type of disk to create. 

			When the user clicks "Define Project", the selections are finalized, the
			directory structure and template files are copied over 

	UpdatingContent 
			This is the mode we start in if the user has opened a project, or after
			the user has clicked "create" on the define project panel. The user can
			peruse the contents of the collection, edit meta-data on images in the 
			collection (these edits only affect the disk output - we DO NOT modify
			anything in the catalog or the source image files). 

			When the user is satisfied with these edits, he clicks "update files" and
			the image files and html is updated as required to reflect the current
			choises

For the time being - that's enough to have a working program. The user still has
to do manual HTML edits to update the commentary on the disk and to insert the 
"disk image" and the like. The user must also go outside this program to create the
label file, then burn the disk. Ultimately, it would be great to have a program which
would also supervise this phase of disk creation. 

******************************************************************************/
void CControlPanel::OnPaint()
{
	CRect		client_rect ;
	CFont *		dc_font ;
	int			caption_height ;

	caption_height = ( m_std_font_lf.lfHeight < 0 ? - m_std_font_lf.lfHeight : m_std_font_lf.lfHeight ) ;

	GetClientRect( &client_rect ) ;

	CPaintDC dc(this); // device context for painting

	dc_font = dc.SelectObject( &m_std_font ) ;
	dc.FillSolidRect( &client_rect, GetSysColor( COLOR_3DFACE ) ) ;

	switch ( m_view->GetCurrentViewState( ) ) 
	{
		case CMakePhotoDisk3View::WelcomeScreen :
		case CMakePhotoDisk3View::SelectACollection : 
			{
				client_rect.left += 15 ;
				client_rect.bottom = 15 + caption_height ;
				dc.DrawText( _T( "Select Catalog" ), -1, &client_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE ) ;
			}
			break ;

		case CMakePhotoDisk3View::DefiningProject : 
			{
				CString	caption ;

				client_rect.left += 15 ;
				client_rect.bottom = 15 + caption_height ;

				caption.Format( _T( "Collection \"%s\"." ), m_view->GetDocument( )->GetSelectedCollection( )->GetName( ) ) ;
				dc.DrawText( caption, -1, &client_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE ) ;

				client_rect.top = client_rect.bottom ;
				client_rect.bottom += caption_height ;
				caption.Format( _T( "Consisting of %d images" ), m_view->GetDocument( )->GetPhotoListSize( ) ) ;
				dc.DrawText( caption, -1, &client_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE ) ;

				// have a full blank line between captions and next line - 
				client_rect.top = client_rect.bottom + caption_height ;
				client_rect.bottom = client_rect.top + caption_height ;
				dc.DrawText( _T( "Hard drive location to contain disk directory"  ), -1, &client_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE ) ;

				client_rect.top = client_rect.bottom + 21 + caption_height ;
				client_rect.bottom = client_rect.top + caption_height ;
				dc.DrawText( _T( "Directory name for disk image" ), -1, &client_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE ) ;

				client_rect.top = client_rect.bottom + 21 + caption_height ;
				client_rect.bottom = client_rect.top + caption_height ;
				dc.DrawText( _T( "Select project type" ), -1, &client_rect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE ) ;
			}
			break ;

		case CMakePhotoDisk3View::UpdatingContent : 



			break ;
	}
		
	dc.SelectObject( dc_font ) ;
}







/******************************************************************************
int CControlPanel::OnCreate

MFC standard WM_CREATE handler. 

We create the controls and set their fonts to the Windows standard. WM_CREATE is called
before the first WM_SIZE message, so our client rect is always 0 x 0, but we just use
that for everything and rely on our handler for WM_SIZE to re-size and re-position everything
correctly.

******************************************************************************/
int CControlPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// get and save the LOGFONT to create the "dialog" type face 
	NONCLIENTMETRICS	non_client_metrics ;

	non_client_metrics.cbSize = sizeof( NONCLIENTMETRICS ) ;
	SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof( NONCLIENTMETRICS ), &non_client_metrics, 0 ) ;
	m_std_font_lf = non_client_metrics.lfMessageFont ;
	m_std_font.CreateFontIndirect( &m_std_font_lf ) ;

	/* 
	Create the controls, but just use client_rect for all rectangles. In WM_CREATE handler, our 
	client_rect is all zero, but the RECT argument is required in all the Create calls. So we 
	still rely on logic in WM_SIZE handler to move the controls wehre they belong
	*/ 
	CRect	client_rect ;

	GetClientRect( &client_rect ) ;

	// Create the controls for selecting a catalog and collection.... 
	m_collection_image_list.Create( IDB_COLLECTION_TREE_IMAGES, 16, 0, RGB( 255, 0, 255 ) ) ;
	m_catalog_combo.Create( CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, client_rect, this, IDC_CATALOG_COMBO ) ;
	m_browse_button.Create(_T( "&Browse" ), BS_PUSHBUTTON, client_rect, this, IDC_CATALOG_BROWSE ) ;
	m_next_button.Create( _T( "&Next" ), BS_PUSHBUTTON, client_rect, this, IDC_NEXT_STEP ) ;
	m_collection_tree.Create( TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_HASBUTTONS, client_rect, this, IDC_COLLECTION_TREE ) ;
	m_collection_tree.SetImageList( &m_collection_image_list, TVSIL_NORMAL ) ;

	// create the controls for phase 2 - 
	m_location_combo.Create( CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, client_rect, this, IDC_PROJECT_LOCATION ) ;
	m_browse_dir.Create(_T( "&Browse" ), BS_PUSHBUTTON, client_rect, this, IDC_BROWSE_DIRECTORY ) ;
	m_dir_name.Create( ES_LEFT, client_rect, this, IDC_DIRECTORY_NAME ) ;
	m_disk_type_combo.Create( CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, client_rect, this, IDC_PROJECT_TYPE_COMBO ) ;
	m_disk_type_text.Create( _T( "" ), SS_LEFT, client_rect, this, IDC_DISK_TYPE_DESCRIPTION ) ;
	m_create_project.Create( _T( "&Create Project" ), BS_PUSHBUTTON, client_rect, this, IDC_CREATE_PROJECT ) ;

	// set the font for each control so it looks normal 
	m_browse_button.SetFont( &m_std_font ) ;
	m_catalog_combo.SetFont( &m_std_font ) ;
	m_collection_tree.SetFont( &m_std_font ) ;
	m_next_button.SetFont( &m_std_font ) ;

	m_location_combo.SetFont( &m_std_font ) ;
	m_browse_dir.SetFont( &m_std_font ) ;
	m_dir_name.SetFont( &m_std_font ) ;
	m_disk_type_combo.SetFont( &m_std_font ) ;
	m_disk_type_text.SetFont( &m_std_font ) ;
	m_create_project.SetFont( &m_std_font ) ;

	LoadCatalogList( ) ;

	return 0;
}




/******************************************************************************
void CControlPanel::OnInitialUpdate( )

Called from within the View's OnInitialUpdate as a convenient time/place to do
one-time initialization just before the windows are displayed

******************************************************************************/
void CControlPanel::OnInitialUpdate( )
{
	// populate the combo box 
	int	i ;
	int	n ;
	int	selected ;

	for ( i = 0 ; i < m_catalogs.GetSize( ) ; i++ )
	{
		m_catalog_combo.AddString( m_catalogs.GetAt( i ) ) ;
	}

	for ( i = 0 ; i < m_directories.GetSize( ) ; i++ )
	{
		m_location_combo.AddString( m_directories.GetAt( i ) ) ;
	}

	/*
	* Types of disks we can create are: 
	*
	*	family		includes presentation, full-size, compressed, and facebook images and general release
	*	hired		includes presentation, full-size, compressed images and a specific release
	*	client		includes small presentation, facebook, compressed, restrictive release and instructions for ordering prints
	*	sampler		includes small presentation, and very limited release, contact info and instructions for ordering prints
	*	
	*/
	selected = n = m_disk_type_combo.AddString( _T( "Family & Friends" ) ) ;
	m_disk_type_combo.SetItemData( n, CMakePhotoDisk3Doc::FamilyAndFriendsDisk ) ;

	n = m_disk_type_combo.AddString( _T( "On Assignment" ) ) ;
	m_disk_type_combo.SetItemData( n, CMakePhotoDisk3Doc::AssignmentProduct ) ;

	n = m_disk_type_combo.AddString( _T( "Client disk" ) ) ;
	m_disk_type_combo.SetItemData( n, CMakePhotoDisk3Doc::ClientDisk ) ;

	n = m_disk_type_combo.AddString( _T( "Sampler disk" ) ) ;
	m_disk_type_combo.SetItemData( n, CMakePhotoDisk3Doc::SamplerDisk ) ;

	m_disk_type_combo.SetCurSel( selected ) ;
}




/******************************************************************************
void CControlPanel::LoadCatalogList

Load from the registry the list of catalogs we've used and store them in a string array

FIXME: Think about it. Is this the place for this? It's not really a 
document-specific thing... but... 

******************************************************************************/
void CControlPanel::LoadCatalogList( )
{
	CString	long_keyname ;
	HKEY	key ;

	long_keyname.Format( _T( "SOFTWARE\\%s\\%s" ), MY_COMPANY_REGISTRY_KEY, MY_PRODUCT_REGISTRY_KEY ) ;

	if ( ERROR_SUCCESS == ::RegOpenKeyEx( HKEY_CURRENT_USER, long_keyname, 0, KEY_READ | KEY_WRITE, &key ) )
	{
		DWORD	value_type ;
		TCHAR	data[ 4 * MAX_PATH ] ;
		DWORD	data_size ;

		data_size = sizeof( data ) ;

		if ( ERROR_SUCCESS == ::RegQueryValueEx( key, _T( "MRU Catalog" ), NULL, &value_type, (BYTE *) data, &data_size ) )
		{
			// data now contains a unicode array of strings. Strings are separated by a unicode nul
			TCHAR *		catalog = data ;

			do
			{
				m_catalogs.Add( catalog ) ;
				catalog += _tcslen( catalog ) + 1 ;

				// MULTI_SZ registry data is a list of strings delimited by nuls, and terminated by a double nul
			}
			while ( *catalog && catalog < data + data_size / sizeof( TCHAR ) ) ;	// data_size is bytes, but the pointer math is in Unicode TCHAR's
		}

		data_size = sizeof( data ) ;

		if ( ERROR_SUCCESS == ::RegQueryValueEx( key, _T( "MRU Root Directory" ), NULL, &value_type, (BYTE *) data, &data_size ) ) 
		{
			TCHAR *		directory = data ;

			do
			{
				m_directories.Add( directory ) ;
				directory += _tcslen( directory ) + 1 ;
			}
			while ( *directory && directory < data + data_size / sizeof( TCHAR ) ) ;
		}

		::RegCloseKey( key ) ;
	}
}






/******************************************************************************
void CControlPanel::OnShowWindow

Process WM_SHOW message. 

******************************************************************************/
void CControlPanel::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);


	switch ( m_view->GetCurrentViewState( ) ) 
	{
		case CMakePhotoDisk3View::WelcomeScreen	:
		case CMakePhotoDisk3View::SelectACollection :
			{
				m_browse_button.ShowWindow( SW_SHOW ) ;
				m_catalog_combo.ShowWindow( SW_SHOW ) ;
				m_collection_tree.ShowWindow( SW_SHOW ) ;
				m_next_button.ShowWindow( SW_SHOW ) ;
				m_next_button.EnableWindow( FALSE ) ;		// next button is initially disabled 
			}
			break ;

		case CMakePhotoDisk3View::DefiningProject : 
			{





			}
			break ;

		case CMakePhotoDisk3View::UpdatingContent : 
			break ;
	}
}








/******************************************************************************
void CControlPanel::OnSize

re-size all the controls. This cannot be mode-dependent. We want to re-size and 
re-arrange controls for all versions of the panel, regardless of whether those 
controls are visible or not, and we need to do this when the window is re-sized,
which may only happen during startup. 

******************************************************************************/
void CControlPanel::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect	client_rect ;
	int		caption_height ;

	GetClientRect( &client_rect ) ;

		// don't make assumptions about whether lfHeight is >0 or <0... 
	caption_height = ( m_std_font_lf.lfHeight < 0 ? - m_std_font_lf.lfHeight : m_std_font_lf.lfHeight ) ;

		// size the controls for phase 1 
	m_browse_button.MoveWindow( client_rect.right - 15 - 72, 15 + caption_height, 72, 21 ) ;
	m_catalog_combo.MoveWindow( 15, 15 + caption_height, client_rect.Width( ) - 30 - 72 - 12, 8 * 21 ) ;
	m_collection_tree.MoveWindow( 15, 15 + caption_height + 21 + 15, client_rect.Width( ) - 30, client_rect.Height( ) - (15 + caption_height + 21 + 15)	- ( 21 + 15 * 2 ) ) ;
	m_next_button.MoveWindow( 15, client_rect.bottom - ( 15 + 21 ), client_rect.Width( ) - 30, 21 ) ;

		// size the controls for phaase 2:
	m_location_combo.MoveWindow( 15, 15 + 4 * caption_height, client_rect.Width( ) - 30 - 72 - 12, 8 * 21 ) ;
	m_browse_dir.MoveWindow( client_rect.right - 15 - 72, 15 + 4 * caption_height, 72, 21 ) ;
	m_dir_name.MoveWindow( 15, 15 + 6 * caption_height + 21, client_rect.Width( ) - 30, 21 ) ;
	m_disk_type_combo.MoveWindow( 15, 15 + 8 * caption_height + 2 * 21, client_rect.Width( ) - 30, 8 * 21 ) ;
	m_disk_type_text.MoveWindow( 15, 15 + 9 * caption_height + 3 * 21, client_rect.Width( ) - 30, 14 * caption_height ) ;
	m_create_project.MoveWindow( 15, client_rect.bottom - ( 15 + 21 ), client_rect.Width( ) - 30, 21 ) ;
}






/******************************************************************************
****
****	Section 1 - 
****
****		The following functions are only invoked/involved in the first 
****		phase of the control panel, when the user is selecting a catalog
****		and collection 
****
****
******************************************************************************/



/******************************************************************************
bool PlausibleCatalogFile

Perform a rudimentary validation after a user has selected a catalog file via the browse button

******************************************************************************/
bool PlausibleCatalogFile( const TCHAR * catalog_path ) 
{
	CFile	test_file ;
	bool	isOK = false ;

	if ( test_file.Open( catalog_path, CFile::modeRead ) )
	{
		char	buffer[ sizeof( DB_SIGNATURE ) ] ;

		if ( sizeof( DB_SIGNATURE ) == test_file.Read( buffer, sizeof( DB_SIGNATURE ) ) )
			if ( 0 == memcmp( buffer, DB_SIGNATURE, sizeof( DB_SIGNATURE ) ) )
				isOK = true ;

		test_file.Close( ) ;
	}

	return isOK ;
}


/******************************************************************************
void CControlPanel::OnBnClickedCatalogBrowse

Handle the browse button - put up the standard Windows Open File dialog 

******************************************************************************/
void CControlPanel::OnBnClickedCatalogBrowse()
{
	CFileDialog	file_dlg( TRUE, _T( ".lrcat" ), NULL, OFN_FILEMUSTEXIST, NULL, this, 0, TRUE ) ;

	if ( IDOK == file_dlg.DoModal( ) )
	{
		CString	fq_path ;

		fq_path.Format( _T( "%s\\%s" ), file_dlg.GetFolderPath( ), file_dlg.GetFileName( ) ) ;

		if ( !PathFileExists( fq_path ) )
			AfxMessageBox( _T( "The designated catalog is not a valid file location." ), MB_OK ) ;
		else if ( !PlausibleCatalogFile( fq_path ) )
			AfxMessageBox( _T( "The designated file is not a valid SQLite database." ), MB_OK ) ;
		else
		{
			int		i ;
			bool	already_selected ;

			// if it's already in our collection of catalog strings, delete it (we'll re-insert it later) 
			for ( i = 0 ; i < m_catalogs.GetSize( ) ; i++ )
				if ( 0 == m_catalogs.GetAt( i ).Compare( fq_path ) ) 
				{
					already_selected = ( 0 == i ) ;

					m_catalogs.RemoveAt( i ) ;
					m_catalog_combo.DeleteString( i ) ;
					break ;
				}

			// insert it at the head of the list (if this causes the last one to roll off the bottom... tough.
			if ( m_catalogs.GetSize( ) == 4 )
			{
				m_catalogs.RemoveAt( 3, 1 ) ;
				m_catalog_combo.DeleteString( 3 ) ;
			}
			m_catalogs.InsertAt( 0, fq_path ) ;

			// then make sure it's at the top of the combo box list and selected
			m_catalog_combo.InsertString( 0, fq_path ) ;
			m_catalog_combo.SelectString( -1, fq_path ) ;
			m_catalog_combo.SetEditSel( 0, -1 ) ;
			OnCbnSelendokCatalogCombo( ) ;
		}
	}
}






/******************************************************************************
void CControlPanel::OnCbnSelendokCatalogCombo

The user selects a catalog file from the combo box. We tell the View b/c this selection 
will affect the document and all the other views as well as us. The view then calls us back,
see OnNewCatalog(), and then we update the collection tree with the data collected from
the database by the document 

******************************************************************************/
void CControlPanel::OnCbnSelendokCatalogCombo()
{
	m_catalog_combo.GetWindowText( m_fq_catalog_path ) ;

	// cast-o-rama b/c we gotta tell CString to cast it to a (const TCHAR *) to make sure we're passing the right argument, then cast to a (WPARAM) to keep the compiler happy
	m_view->PostMessage( WM_NEW_CATALOG, (WPARAM) ( (const TCHAR *) m_fq_catalog_path ) ) ;
	m_view->SetFocus( ) ;
}





/******************************************************************************
void CControlPanel::OnNewCatalog

Called by View after the document has finished updating its contents to reflect the new
catalog selection. 

Delete the current contents of the tree, then populate it to reflect the new catalog

******************************************************************************/
void CControlPanel::OnNewCatalog( ) 
{
	m_collection_tree.DeleteAllItems( ) ;

	HTREEITEM hRoot = m_collection_tree.InsertItem(_T("Collections"), 0, 0, 0, TVI_LAST );
	m_collection_tree.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	RecursiveTreeBuild( m_view->GetDocument( )->GetCollectionList( ), hRoot ) ;

	m_collection_tree.Expand(hRoot, TVE_EXPAND);
}






/******************************************************************************
void CControlPanel::RecursiveTreeBuild

Just walk the document's tree of collection-sets and collections and populate the collection tree

******************************************************************************/
void CControlPanel::RecursiveTreeBuild( CTypedPtrList< CPtrList, Collection *> & col_list, HTREEITEM parent_handle ) 
{
	Collection *	col ;
	POSITION		pos ;
	TVINSERTSTRUCT	tv_insert ;
	HTREEITEM		col_handle ;

	if ( pos = col_list.GetHeadPosition( ) )
	{
		do
		{
			col = col_list.GetNext( pos ) ;

			tv_insert.hParent = parent_handle ;
			tv_insert.hInsertAfter = TVI_LAST ;
			tv_insert.item.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_CHILDREN | TVIF_SELECTEDIMAGE | TVIF_TEXT ;
			tv_insert.item.iImage = col->GetIsCollectionSet( ) ? 1 : 2 ;
			tv_insert.item.iSelectedImage = col->GetIsCollectionSet( ) ? 1 : 2 ;
			tv_insert.item.cChildren = col->GetIsCollectionSet( ) ? col->GetChildQueue( ).GetCount( ) : 0 ;
			tv_insert.item.lParam = (LPARAM) col ;
			tv_insert.item.pszText = (TCHAR *) col->GetName( ) ;

			col_handle = m_collection_tree.InsertItem( &tv_insert ) ;

			if ( col->GetIsCollectionSet( ) && col->GetChildQueue( ).GetCount( ) != 0 ) 
				RecursiveTreeBuild( col->GetChildQueue( ), col_handle ) ;
		}
		while ( NULL != pos ) ;
	}
}





/******************************************************************************
BOOL CControlPanel::OnNotify

Handles notifications from the tree control. The selchange messages are handled by
the view, which then calls us back after the document is done.

We wouldn't need to the view to call us back, of course, if we used SendMessage rather
than PostMessage... and SendMessage would probably work fine in most cases, but there's
still the possibility that the document may get hung up processing one of these 
messages (say, the collection is absolutely huge and the disk drive is slow enough that 
processing and extracting all the jpegs takes a long, lon time... or the database has been 
locked by another user, or a db error occurs...), so I prefer to use non-blocking PostMessage 
and let the program component (whether document or another view) handle its own problems and 
leave us free to still process window messages like WM_PAINT and WM_SIZE... 

******************************************************************************/
BOOL CControlPanel::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *	nm_header ;

	nm_header = (NMHDR *) lParam ;
	HTREEITEM	sel_item ;

	switch ( nm_header->code )
	{
		case TVN_SELCHANGING :
			if ( NULL != ( sel_item = m_collection_tree.GetSelectedItem( ) ) )
			{
				if ( sel_item != m_collection_tree.GetRootItem( ) ) 
					m_collection_tree.SetItemState( sel_item, 0, TVIS_BOLD);

				m_next_button.EnableWindow( FALSE ) ;
				m_view->PostMessage( WM_COLLECTION_SELCHANGE, 0, NULL ) ;
			}
			break ;

		case TVN_ITEMEXPANDING :
			{
				NMTREEVIEW *	nm_treeview = (NMTREEVIEW *) nm_header ;

				Collection * col = (Collection *) nm_treeview->itemNew.lParam ;
			}
			break ;

		case TVN_SELCHANGED :
			if ( NULL != ( sel_item = m_collection_tree.GetSelectedItem( ) ) )
			{
				Collection * col ;

				m_collection_tree.SetItemState( sel_item, TVIS_BOLD, TVIS_BOLD ) ;

				col = (Collection *) m_collection_tree.GetItemData( sel_item ) ;

				m_view->PostMessage( WM_COLLECTION_SELCHANGE, 1, (LPARAM) col ) ;
			}
			break ;
	}

	return CWnd::OnNotify(wParam, lParam, pResult);
}






/******************************************************************************
void CControlPanel::SelectCollection

Called from view after it and the document have handled the WM_COLLECTION_SELCHANGE 
message with a non-null collection 

We "round-trip" the message b/c we use PostMessage rather than SendMessage. See
extended discussion in OnNotify comment for an explanation of why.

******************************************************************************/
void CControlPanel::SelectCollection( Collection * col )
{
	// enable the next button if the user has selected a non-empty collection
	m_next_button.EnableWindow( m_view->GetDocument( )->GetPhotoListSize( ) != 0 ) ;
}





/******************************************************************************
void CControlPanel::DeselectCollection

Called from view after it and the document have handled the WM_COLLECTION_SELCHANGE 
message with a null collection 

We "round-trip" the message b/c we use PostMessage rather than SendMessage. See
extended discussion in OnNotify comment for an explanation of why.

******************************************************************************/
void CControlPanel::DeselectCollection( )
{
	m_next_button.EnableWindow( FALSE ) ;
}













/******************************************************************************
void CControlPanel::OnBnClickedNext

The user has selected a catalog, and a collection within that catalog. Now the
user wants to go on to define the project directory and type... 

******************************************************************************/
void CControlPanel::OnBnClickedNext( )
{
	m_catalog_combo.ShowWindow( SW_HIDE ) ;
	m_browse_button.ShowWindow( SW_HIDE ) ;
	m_collection_tree.ShowWindow( SW_HIDE ) ;
	m_next_button.ShowWindow( SW_HIDE ) ;

	m_view->PostMessageW( WM_GOTO_DEFINE_PROJECT ) ;

	m_location_combo.ShowWindow( SW_SHOW ) ;
	m_browse_dir.ShowWindow( SW_SHOW ) ;
	m_dir_name.ShowWindow( SW_SHOW ) ;
	m_disk_type_combo.ShowWindow( SW_SHOW ) ;
	m_disk_type_text.ShowWindow( SW_SHOW ) ;
	m_create_project.ShowWindow( SW_SHOW ) ;

	// now - initialize the m_dir_name member 
	m_dir_name.SetWindowText( m_view->GetDocument( )->GetSelectedCollection( )->GetName( ) ) ;

	// a little wierd, but it sets the static text per the selection in the project type combo...
	OnCBnSelendokDiskTypeCombo( ) ;
	PredictDiskRootDirectory( ) ;

	Invalidate( FALSE ) ;			// make sure the whole panel gets re-painted 
}








/******************************************************************************
****
****	Section 2 - 
****
****		The following functions are only invoked/involved in the second
****		phase of the control panel, when the user is defining where the 
****		project files should be assembled and what type of project it 
****		should be 
****
****
******************************************************************************/






/******************************************************************************
void CControlPanel::PredictDiskRootDirectory

Try to predict the root directory for the project based on the catalog path. 
We assume the disk is probably going to go at the same level as the "lightroom"
directory which contains the lightroom catalog. 

******************************************************************************/
void CControlPanel::PredictDiskRootDirectory( )
{
	CString	root_path ;
	int		match_index ;
	int		lr_dir ;

	root_path = m_fq_catalog_path ;
	root_path.MakeLower( ) ;
	lr_dir = root_path.Find( _T( "lightroom" ) ) ;

	if ( -1 != lr_dir ) 
	{
		root_path = m_fq_catalog_path.Left( lr_dir - 1 ) ;
		if ( -1 != ( match_index = m_location_combo.FindString( -1, root_path ) ) )
		{
			m_location_combo.SetCurSel( match_index ) ;
		}
	}
}






/******************************************************************************
void CControlPanel::OnBnClickedCatalogBrowse

Handle the browse directory button, in case the user doesn't like the "predicted"
root directory 

******************************************************************************/
void CControlPanel::OnBnClickedDirectoryBrowse( ) 
{
	m_view->SetFocus( ) ;
	CString	start_path ;
	CString selected_path ;
	TCHAR *	current_path_str ;
	TCHAR * selected_path_str ;

	// try to initialize start_path with the selected contents of the combo box (if any)
	if ( -1 != m_location_combo.GetCurSel( ) )
		m_location_combo.GetLBText( m_location_combo.GetCurSel( ), start_path ) ;

	// if we have a start path, get a TCHAR * to it so BrowseForFolder knows to start there....
	if ( start_path.GetLength( ) )
		current_path_str = start_path.GetBufferSetLength( MAX_PATH + 1 ) ;
	else
		current_path_str = NULL ;

	selected_path_str = selected_path.GetBufferSetLength( MAX_PATH + 1 ) ;

	// let user browse for the folder 
	if ( BrowseForFolder( m_hWnd, current_path_str, selected_path_str ) ) 
	{
		int match ;
		
		if ( -1 != ( match = m_location_combo.FindString( -1, selected_path_str ) ) )
			m_location_combo.DeleteString( match ) ;

		m_location_combo.InsertString( 0, selected_path_str ) ;
		m_location_combo.SetCurSel( 0 ) ;
	}

	// unlock the string buffers.... 
	if ( current_path_str ) 
		start_path.ReleaseBuffer( ) ;
	selected_path.ReleaseBuffer( ) ;

	m_view->SetFocus( ) ;
}






/******************************************************************************
void CControlPanel::OnCBnSelendokProjectLocation

The user selects a project location. 

******************************************************************************/
void CControlPanel::OnCBnSelendokProjectLocation( ) 
{
	m_view->SetFocus( ) ;
}




/******************************************************************************
void CControlPanel::OnCBnSelendokDiskTypeCombo

The user selects a project type from the combo box. 

******************************************************************************/
void CControlPanel::OnCBnSelendokDiskTypeCombo( )
{
	int	disk_type = m_disk_type_combo.GetItemData( m_disk_type_combo.GetCurSel( ) ) ;

	switch ( disk_type )
	{
		case CMakePhotoDisk3Doc::FamilyAndFriendsDisk : 
			m_disk_type_text.SetWindowText( _T( "Family && Friends disk\n\nFor each image, this disk will include a\n    high-quality presentation\n    full-size image\n    facebook-sized image\n    compressed image\n\nThe disk comes with a full release for non-commercial use." ) ) ;
			break ;

		case CMakePhotoDisk3Doc::AssignmentProduct : 
			m_disk_type_text.SetWindowText( _T( "Assignment disk\n\nFor each image, this disk will include a\n    high-quality presentation\n    full-size image\n    compressed image\n\nThe disk comes with a release per assignment contract." ) ) ;
			break ;

		case CMakePhotoDisk3Doc::ClientDisk :
			m_disk_type_text.SetWindowTextW( _T( "Client disk\n\nFor each image, this disk will include a\n    small presentation\n    facebook sized image\n    compressed image\n\nThe disk comes with a restrictive release and contact information." ) ) ;
			break ;

		case CMakePhotoDisk3Doc::SamplerDisk : 
			m_disk_type_text.SetWindowTextW( _T( "Sample disk\n\nFor each image, this disk will include a\n    small presentation\n\nExplicit non-release and contact information." ) ) ;
			break ;
	} 
	m_disk_type_text.Invalidate( ) ;
	m_view->SetFocus( ) ;
}




/**********************************************************************************
static int CALLBACK BrowseCallbackProc

Callback for SHBrowseForFolder 

**********************************************************************************/
static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{    
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.    

	switch (uMsg)    
	{        
		case BFFM_INITIALIZED:
		{            
			if ( NULL != lpData )
			{                
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);            
			}
		}
	}
	return 0 ; // The function should always return 0.
} 



/******************************************************************************
BOOL BrowseForFolder

Cut-and-paste function from Codeproject or somet other website which gives us 
ready-made browse-for-directory funcitonality 

******************************************************************************/
BOOL BrowseForFolder(HWND hwnd, LPCTSTR szCurrent, LPTSTR szPath)
{    
	BROWSEINFO   bi = { 0 };    
	LPITEMIDLIST pidl;    
	TCHAR        szDisplay[MAX_PATH];    
	BOOL         retval;     
	
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );     
	
	bi.hwndOwner      = hwnd;    
	
	bi.pszDisplayName = szDisplay;    
	bi.lpszTitle      = TEXT( "Select a location for the disk directory" ) ;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;    
	bi.lpfn           = BrowseCallbackProc;    
	bi.lParam         = (LPARAM) szCurrent;     
	pidl = SHBrowseForFolder(&bi);     

	if (NULL != pidl)    
	{        
		retval = SHGetPathFromIDList(pidl, szPath);        
		CoTaskMemFree(pidl);    
	}    
	else    
	{        
		retval = FALSE;    
	}     
	
	if (!retval)    
	{        
		szPath[0] = TEXT('\0');    
	}     
	
	CoUninitialize();    
	return retval;
}





/******************************************************************************
void CControlPanel::OnBnClickedCreateProject

We now have a catlog, a collection, a location, a directory and a disk type.
We are finally ready to actually create the document on disk, copy the 
template files, and gear up to marshall the files for the disk image. 

******************************************************************************/
void CControlPanel::OnBnClickedCreateProject( )
{
	CConfirmProjectCreation	dlg ;
	CString					directory ;

	m_location_combo.GetLBText( m_location_combo.GetCurSel( ), m_fq_project_path ) ;
	m_dir_name.GetWindowText( directory ) ;

	directory.Trim( ) ;
	if ( directory.GetLength( ) )
	{
		m_fq_project_path += _T( "\\" ) ;
		m_fq_project_path += directory ;

		dlg.m_catalog_name = m_view->GetDocument( )->GetCatalog( ) ;
		dlg.m_collection_name = m_view->GetDocument( )->GetSelectedCollection( )->GetName( ) ;
		dlg.m_directory_name = m_fq_project_path ;
		dlg.m_photo_count.Format( _T( "%d images in collection" ), m_view->GetDocument( )->GetPhotoListSize( ) ) ;
		m_disk_type_combo.GetLBText( m_disk_type_combo.GetCurSel( ), dlg.m_disk_type ) ;

		if ( IDOK == dlg.DoModal( ) )
		{
			m_view->PostMessage( WM_CREATE_PROJECT, (WPARAM) ( (const TCHAR *) m_fq_project_path ), (LPARAM) m_disk_type_combo.GetItemData( m_disk_type_combo.GetCurSel( ) ) ) ;
		}
	}
	else
	{
		AfxMessageBox( _T( "The directory name may not be blank." ), MB_OK | MB_ICONSTOP ) ;
		GetDlgItem( IDC_DIRECTORY_NAME )->SetFocus( ) ;
		( (CEdit *) GetDlgItem( IDC_DIRECTORY_NAME ) )->SetSel( 0, -1 ) ;
	}
}




/******************************************************************************
void CControlPanel::OnCreateProject

Called from the view's handler for the WM_CREATE_PROJECT message, after the
document has successfully saved. 

******************************************************************************/
void CControlPanel::OnCreateProject( )
{
	// hide the create project controls 
	m_location_combo.ShowWindow( SW_HIDE ) ;
	m_browse_dir.ShowWindow( SW_HIDE ) ;
	m_dir_name.ShowWindow( SW_HIDE ) ;
	m_disk_type_combo.ShowWindow( SW_HIDE ) ;
	m_disk_type_text.ShowWindow( SW_HIDE ) ;
	m_create_project.ShowWindow( SW_HIDE ) ;


	// update control panel controls for phase 3 - 
	Invalidate( ) ;
}
