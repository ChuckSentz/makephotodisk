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
#include "PanelSelectCollection.h"
#include "SplitterWndEx.h" 

static bool PlausibleCatalogFile( const TCHAR * catalog_path ) ;


#define		MAX_MRU_CATALOG_LIST_SIZE		8



// CPanelSelectCollection

IMPLEMENT_DYNCREATE(CPanelSelectCollection, CFormView)

CPanelSelectCollection::CPanelSelectCollection()
	: CFormView(CPanelSelectCollection::IDD)
{

}

CPanelSelectCollection::~CPanelSelectCollection()
{
}




void CPanelSelectCollection::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CATALOG_COMBO, m_catalog_combo);
	DDX_Control(pDX, IDC_COLLECTION_TREE, m_collection_tree);
	DDX_Control(pDX, IDC_NEXT_STEP, m_next_button);
}


/******************************************************************************
******************************************************************************/
BEGIN_MESSAGE_MAP(CPanelSelectCollection, CFormView)
	ON_BN_CLICKED(IDC_CATALOG_BROWSE, &CPanelSelectCollection::OnBnClickedCatalogBrowse)
	ON_CBN_SELENDOK(IDC_CATALOG_COMBO, &CPanelSelectCollection::OnCbnSelendokCatalogCombo)
	ON_BN_CLICKED(IDC_NEXT_STEP, &CPanelSelectCollection::OnBnClickedNextStep)
//	ON_CBN_SELCHANGE(IDC_CATALOG_COMBO, &CPanelSelectCollection::OnCbnSelchangeCatalogCombo)
	ON_NOTIFY(TVN_SELCHANGING, IDC_COLLECTION_TREE, &CPanelSelectCollection::OnTvnSelchangingCollectionTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_COLLECTION_TREE, &CPanelSelectCollection::OnTvnSelchangedCollectionTree)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CPanelSelectCollection diagnostics

#ifdef _DEBUG
void CPanelSelectCollection::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPanelSelectCollection::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif


CMakePhotoDisk3Doc* CPanelSelectCollection::GetDocument( ) const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMakePhotoDisk3Doc)));
	return (CMakePhotoDisk3Doc*)m_pDocument;
}

#endif //_DEBUG


// CPanelSelectCollection message handlers



/******************************************************************************
CPanelSelectCollection::OnBnClickedCatalogBrowse

	AFX message-mapped handler for click on Browse button next to the 
	catalog-select combo

******************************************************************************/
void CPanelSelectCollection::OnBnClickedCatalogBrowse()
{
	CFileDialog	file_dlg( TRUE, _T( ".lrcat" ), NULL, OFN_FILEMUSTEXIST, NULL, this, 0, TRUE ) ;
	CString		cat_path ;

	// set the initial file selection for the dialog based on the currently select catalog in the combo box (always the top of the list - element 0)
	m_catalog_combo.GetLBText( 0, cat_path ) ;	// curious fact, the version which takes a LPTSTR returns int, but the overload that takes a CString is void. 

	if ( cat_path.GetLength( ) )
	{
		int	last_slash_pos ;

		// extract just the directory part - 
		cat_path.Replace( _T( '/' ), _T( '\\' ) ) ;

		if ( -1 != ( last_slash_pos = cat_path.ReverseFind( _T( '\\' ) ) ) ) 
		{
			cat_path = cat_path.Left( last_slash_pos ) ;
			file_dlg.m_ofn.lpstrInitialDir = cat_path ;
		}
	}
	// else - nothing in the combo box - 

	// OK - now display the standard file-picker dialog 
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

			// i is the index of the string in the combo box. If 0, it's already at the top of the list
			// if > 0, move it to the top, and if -1, need to add it...
			if ( 0 < ( i = m_catalog_combo.FindString( -1, fq_path ) ) )
			{
				m_catalog_combo.DeleteString( i ) ;
				m_catalog_combo.InsertString( 0, fq_path ) ;
			}
			else if ( i == -1 ) 
			{
				m_catalog_combo.InsertString( 0, fq_path ) ;
				if ( MAX_MRU_CATALOG_LIST_SIZE < m_catalog_combo.GetCount( ) )
					m_catalog_combo.DeleteString( MAX_MRU_CATALOG_LIST_SIZE ) ;
			}

			// select the user's choice in the combo box, and send the notification for selection-change 
			m_catalog_combo.SetCurSel( 0 ) ;
			OnCbnSelendokCatalogCombo( ) ;
		}
	}
}





/******************************************************************************
CPanelSelectCollection::OnCbnSelendokCatalogCombo

	Message-mapped handler for CBN_SELENDOK combo-box notification 
	
	A new catalog in the combo box has been selected 

******************************************************************************/
void CPanelSelectCollection::OnCbnSelendokCatalogCombo()
{
	CString		fq_catalog_path ;

	m_catalog_combo.GetWindowText( fq_catalog_path ) ;

	if ( GetDocument( )->GetSelectionCount( ) ) 
		GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::ImageDeselected ) ;

	GetDocument( )->SetCatalog( fq_catalog_path ) ;
	
	GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::CatalogSelected ) ;

	// make sure the selected catalog is at the top of the list of drop-downs...
	if ( 0 != m_catalog_combo.GetCurSel( ) ) 
	{
		m_catalog_combo.DeleteString( m_catalog_combo.GetCurSel( ) ) ;
		m_catalog_combo.InsertString( 0, fq_catalog_path ) ;
		m_catalog_combo.SetCurSel( 0 ) ;
	}

	HTREEITEM	hRoot ;

	m_collection_tree.DeleteAllItems( ) ;
	hRoot = m_collection_tree.InsertItem( _T( "Collections" ), 0, 0, 0, TVI_LAST ) ;
	m_collection_tree.SetItemState( hRoot, TVIS_BOLD, TVIS_BOLD ) ;

	RecursiveTreeBuild( GetDocument( )->GetCollectionList( ), hRoot ) ;
	m_collection_tree.Expand( hRoot, TVE_EXPAND ) ;
}






/******************************************************************************
CPanelSelectCollection::RecursiveTreeBuild

	Build the tree of collections. Since some collections can be collections of 
	collections (aka a "collection set"), we need to be able to go recursive

******************************************************************************/
void CPanelSelectCollection::RecursiveTreeBuild( 
	CTypedPtrList< CPtrList, Collection *> & col_list,		// I - list of collections which are children of the parent 
	HTREEITEM parent_handle									// I - handle of the parent's entry in the tree 
) 
{
	Collection *	col ;
	POSITION		pos ;
	TVINSERTSTRUCT	tv_insert ;
	HTREEITEM		col_handle ;

	// walk through the list of collections 
	if ( pos = col_list.GetHeadPosition( ) )
	{
		do
		{
			// .GetNext returns the item @ pos, then increments pos to next one before returning 
			col = col_list.GetNext( pos ) ;

			// set up a TVINSERTSTRUCT to insert an item into the tree control for this new child
			tv_insert.hParent = parent_handle ;
			tv_insert.hInsertAfter = TVI_LAST ;
			tv_insert.item.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_CHILDREN | TVIF_SELECTEDIMAGE | TVIF_TEXT ;
			tv_insert.item.iImage = col->GetIsCollectionSet( ) ? 1 : 2 ;
			tv_insert.item.iSelectedImage = col->GetIsCollectionSet( ) ? 1 : 2 ;
			tv_insert.item.cChildren = col->GetIsCollectionSet( ) ? col->GetChildQueue( ).GetCount( ) : 0 ;
			tv_insert.item.lParam = (LPARAM) col ;
			tv_insert.item.pszText = (TCHAR *) col->GetName( ) ;

			col_handle = m_collection_tree.InsertItem( &tv_insert ) ;

			// if we just inserted a collection set, we gotta go recursive to insert its children, if any 
			if ( col->GetIsCollectionSet( ) && col->GetChildQueue( ).GetCount( ) != 0 ) 
				RecursiveTreeBuild( col->GetChildQueue( ), col_handle ) ;
		}
		while ( NULL != pos ) ;
	}
}






/******************************************************************************
CPanelSelectCollection::OnTvnSelchangingCollectionTree

	Message-mapped handler for TVN_SELCHANGING notification. 

	If there's a selection in the tree view, a selection change generates 
	two notifications: TVN_SELCHANGING followed by TVN_SELCHANGED 

	Want to pass on notification to the lightroom and properties views, 
	then disable the Next button. 

******************************************************************************/
void CPanelSelectCollection::OnTvnSelchangingCollectionTree(
	NMHDR *pNMHDR,							// I - pointer to NMTREEVIEW structure 
	LRESULT *pResult						// O - just gets 0 
)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0 ;

	if ( pNMTreeView->itemOld.hItem != m_collection_tree.GetRootItem( ) )
	{
		m_collection_tree.SetItemState( pNMTreeView->itemOld.hItem, 0, TVIS_BOLD ) ;		// un-bold the old item 
		if ( GetDocument( )->GetSelectionCount( ) ) 
			GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::ImageDeselected ) ;
		GetDocument( )->DeselectCollection( ) ;
		GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::CollectionDeselected ) ;
	}
	m_next_button.EnableWindow( FALSE ) ;
}




/******************************************************************************
CPanelSelectCollection::OnTvnSelchangedCollectionTree

	Message-mapped notification handler for TVN_SELCHANGED. 

	The selected item in the tree view has changed, pNMHDR->itemNew is it. 

******************************************************************************/
void CPanelSelectCollection::OnTvnSelchangedCollectionTree(
	NMHDR *pNMHDR,						// I - pointer to NMTREEVIEW structure 
	LRESULT *pResult					// O - just set to 0 for now 
)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	* pResult = 0 ;

	m_collection_tree.SetItemState( pNMTreeView->itemNew.hItem, TVIS_BOLD, TVIS_BOLD ) ;

	if ( pNMTreeView->itemNew.hItem != m_collection_tree.GetRootItem( ) )
	{
		Collection *	col ;

		// tests my theory that the lParam is valid and is just a way of passing back the item data. MS help is, as usual, ambiguous 
		ASSERT( pNMTreeView->itemNew.lParam == m_collection_tree.GetItemData( pNMTreeView->itemNew.hItem ) ) ;
		
		col = (Collection *) pNMTreeView->itemNew.lParam ;

		BeginWaitCursor( ) ;
		GetDocument( )->SelectCollection( col ) ;
		GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::CollectionSelected ) ;
		EndWaitCursor( ) ;

		if ( NULL != col && !col->GetIsCollectionSet( ) )
			m_next_button.EnableWindow( TRUE ) ;
	}
}





/******************************************************************************
CPanelSelectCollection::OnBnClickedNextStep

	Message-mapped notification handler for user-click on Next button 

	Tell parent to swap out this panel for the next one - the define project one 

******************************************************************************/
void CPanelSelectCollection::OnBnClickedNextStep()
{
	HTREEITEM	sel_item ;

	sel_item = m_collection_tree.GetSelectedItem( ) ;
	ASSERT( sel_item != NULL && sel_item != m_collection_tree.GetRootItem( ) ) ;
	GetParentFrame( )->PostMessage( WM_GOTO_DEFINE_PROJECT ) ;
}





/******************************************************************************
CPanelSelectCollection::OnInitialUpdate

	Virtual override for OnInitialUpdate member. Our splitter window class 
	will send WM_INITIALUPDATE, which invokes this member, every time this view
	class is successfully swapped into the pane 

******************************************************************************/
void CPanelSelectCollection::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_collection_image_list.Create( IDB_COLLECTION_TREE_IMAGES, 16, 0, RGB( 255, 0, 255 ) ) ;
	m_collection_tree.SetImageList( &m_collection_image_list, TVSIL_NORMAL ) ;

	CRect dlg_rect( 0, 0, 206, 380 ) ;
	CSplitterWndFlex * parent_splitter ;

	// need to re-size the dialog rectangle to fit the pane 
	MapDialogRect( m_hWnd, &dlg_rect ) ;
	parent_splitter = dynamic_cast< CSplitterWndFlex *>( GetParent( ) ) ;
	parent_splitter->SetColumnInfo( 0, dlg_rect.Width( ), dlg_rect.Width( ) ) ;
	parent_splitter->RecalcLayout( ) ;

	LoadMRUCatalogList( ) ;

	m_catalog_combo.SetCurSel( 0 ) ;
	OnCbnSelendokCatalogCombo( ) ;

	// next button is disabled until user has selected a valid collection 
	m_next_button.EnableWindow( FALSE ) ;
}





/******************************************************************************
CPanelSelectCollection::LoadMRUCatalogList

	Load the MRU list of catalogs used recently. 

******************************************************************************/
void CPanelSelectCollection::LoadMRUCatalogList( ) 
{
	CString	long_keyname ;
	HKEY	key ;

	// FIXME - can use Lightroom agprefs file to load a list of additional catalogs.... 

	// load the MRU list of catalgos from our registry key - 
	long_keyname.Format( _T( "SOFTWARE\\%s\\%s" ), MY_COMPANY_REGISTRY_KEY, MY_PRODUCT_REGISTRY_KEY ) ;


	if ( ERROR_SUCCESS == ::RegCreateKeyEx( HKEY_CURRENT_USER, long_keyname, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL ) ) 
	{
		DWORD	value_type ;
		TCHAR	data[ MAX_MRU_CATALOG_LIST_SIZE * MAX_PATH ] ;
		DWORD	data_size ;

		data_size = sizeof( data ) ;

		if ( ERROR_SUCCESS == ::RegQueryValueEx( key, _T( "MRU Catalog" ), NULL, &value_type, (BYTE *) data, &data_size ) )
		{
			// data now contains a unicode array of strings. Strings are separated by a unicode nul
			TCHAR *		catalog = data ;

			do
			{
				m_catalog_combo.AddString( catalog ) ;
				catalog += _tcslen( catalog ) + 1 ;

				// MULTI_SZ registry data is a list of strings delimited by nuls, and terminated by a double nul
			}
			while ( *catalog && catalog < data + data_size / sizeof( TCHAR ) ) ;	// data_size is bytes, but the pointer math is in Unicode TCHAR's
		}
		::RegCloseKey( key ) ;
	}
}




/******************************************************************************
CPanelSelectCollection::SaveMRUCatalogList

	Save our MRU list of catalogs 

******************************************************************************/
void CPanelSelectCollection::SaveMRUCatalogList( ) 
{
	CString	long_keyname ;
	HKEY	key ;

	// load the MRU list of catalgos from our registry key - 
	long_keyname.Format( _T( "SOFTWARE\\%s\\%s" ), MY_COMPANY_REGISTRY_KEY, MY_PRODUCT_REGISTRY_KEY ) ;

	if ( ERROR_SUCCESS == ::RegOpenKeyEx( HKEY_CURRENT_USER, long_keyname, 0, KEY_READ | KEY_WRITE, &key ) )
	{
		TCHAR	data[ MAX_MRU_CATALOG_LIST_SIZE * MAX_PATH ] ;
		TCHAR *	data_pos ;
		int		i ;

		// convert string array into a buffer full of \0-terminated strings, terminated by a double \0
		for ( i = 0, data_pos = data ; i < m_catalog_combo.GetCount( ) ; i++ )
		{
			CString	cat_path ;

			m_catalog_combo.GetLBText( i, cat_path ) ;

			_tcscpy_s( data_pos, sizeof( data ) / sizeof( TCHAR ) - ( data_pos - data ), cat_path ) ;
			data_pos += _tcslen( data_pos ) + 1 ;
		}
		*data_pos++ = _T( '\0' ) ;

		// need to cast data_post and data as BYTE * so the pointer math gives us the number of BYTES, not the number of TCHAR's 
		if ( ERROR_SUCCESS != ::RegSetValueEx( key, _T( "MRU Catalog" ), NULL, REG_MULTI_SZ, (BYTE *) data, (BYTE *) data_pos - (BYTE *) data ) ) 
		{
			ASSERT( FALSE ) ;
			AfxMessageBox( _T( "Unable to save registry value!" ), MB_OK | MB_ICONEXCLAMATION ) ;
		}
		::RegCloseKey( key ) ;
	}
}





/******************************************************************************
bool PlausibleCatalogFile

	Perform a rudimentary validation after a user has selected a catalog
	file via the browse button

******************************************************************************/
static bool PlausibleCatalogFile( 
	const TCHAR * catalog_path				// I - filename to check 
) 
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
CPanelSelectCollection::OnDestroy

	Message-mapped WM_DESTROY handler 

******************************************************************************/
void CPanelSelectCollection::OnDestroy()
{
	CFormView::OnDestroy();

	// save any changes to the MRU catalog list - do it here instead of OnNext handler b/c we want to update even if the
	// user just selects catalogs then exits the program
	SaveMRUCatalogList( ) ; 
}
