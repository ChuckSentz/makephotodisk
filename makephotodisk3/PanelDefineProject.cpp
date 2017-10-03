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
#include "PanelDefineProject.h"
#include "SplitterWndEx.h"
#include "ConfirmProjectCreation.h"

// CPanelDefineProject

IMPLEMENT_DYNCREATE(CPanelDefineProject, CFormView)

CPanelDefineProject::CPanelDefineProject()
	: CFormView(CPanelDefineProject::IDD)
{

}

CPanelDefineProject::~CPanelDefineProject()
{
}





void CPanelDefineProject::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROJECT_LOCATION, m_location_combo);
	DDX_Control(pDX, IDC_DIRECTORY_NAME, m_dir_name_edit);
	DDX_Control(pDX, IDC_PROJECT_TYPE_COMBO, m_disk_type_combo);
	DDX_Control(pDX, IDC_CREATE_PROJECT, m_create_project_button);
	DDX_Control(pDX, IDC_DISK_TYPE_DESCRIPTION, m_disk_type_text);
}


/******************************************************************************
	
	message-map for define project panel 

******************************************************************************/
BEGIN_MESSAGE_MAP(CPanelDefineProject, CFormView)
	ON_BN_CLICKED(IDC_BROWSE_DIRECTORY, &CPanelDefineProject::OnBnClickedBrowseDirectory)
	ON_BN_CLICKED(IDC_BACK, &CPanelDefineProject::OnBnClickedBack)
	ON_BN_CLICKED(IDC_CREATE_PROJECT, &CPanelDefineProject::OnBnClickedCreateProject)
	ON_CBN_SELENDOK(IDC_PROJECT_TYPE_COMBO, &CPanelDefineProject::OnCbnSelendokProjectTypeCombo)
	ON_CBN_SELENDOK(IDC_PROJECT_LOCATION, &CPanelDefineProject::OnCbnSelendokProjectLocation)
//	ON_EN_CHANGE(IDC_DIRECTORY_NAME, &CPanelDefineProject::OnEnChangeDirectoryName)
//	ON_EN_KILLFOCUS(IDC_DIRECTORY_NAME, &CPanelDefineProject::OnEnKillfocusDirectoryName)
END_MESSAGE_MAP()


// CPanelDefineProject diagnostics

#ifdef _DEBUG
void CPanelDefineProject::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPanelDefineProject::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif

CMakePhotoDisk3Doc* CPanelDefineProject::GetDocument( ) const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMakePhotoDisk3Doc)));
	return (CMakePhotoDisk3Doc*)m_pDocument;
}


#endif //_DEBUG


// CPanelDefineProject message handlers





/******************************************************************************
CPanelDefineProject::OnInitialUpdate
	
	First order of business is to make sure the pane is large enough to display 
	the contents of the panel 

	Then fill in the collection name, the location combo, and the edit box with 
	the directory name based off the collection name 

******************************************************************************/
void CPanelDefineProject::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	CRect dlg_rect( 0, 0, 206, 380 ) ;
	CSplitterWndFlex * parent_splitter ;

	MapDialogRect( m_hWnd, &dlg_rect ) ;
	
	if ( NULL != ( parent_splitter = dynamic_cast< CSplitterWndFlex *>( GetParent( ) ) ) )
	{
		parent_splitter->SetColumnInfo( 0, dlg_rect.Width( ), dlg_rect.Width( ) ) ;
		parent_splitter->RecalcLayout( ) ;
	}

	ASSERT( NULL != GetDocument( )->GetSelectedCollection( ) ) ;

	CString	work ;

	GetDlgItem( IDC_COLLECTION_NAME )->SetWindowText( GetDocument( )->GetSelectedCollection( )->GetName( ) ) ;
	work.Format( _T( "A collection of %d images" ), GetDocument( )->GetPhotoListSize( ) ) ;
	GetDlgItem( IDC_COLLECTION_DESCRIPTION )->SetWindowText( work ) ;

	m_dir_name_edit.SetWindowText( GetDocument( )->GetSelectedCollection( )->GetName( ) ) ;

	InitializeDiskTypeCombo( ) ;
	LoadMRULocationList( ) ;
	PredictDiskRootDirectory( ) ;
}







/******************************************************************************
CPanelDefineProject::InitializeDiskTypeCombo

	Just fill the disk type combo box

******************************************************************************/
void CPanelDefineProject::InitializeDiskTypeCombo( )
{
	int	n ;
	int	selected ;

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
CPanelDefineProject::PredictDiskRootDirectory

	Try to predict the desired location based on the location of the 
	catalog used... 

******************************************************************************/
void CPanelDefineProject::PredictDiskRootDirectory( )
{
	CString	root_path ;
	int		match_index ;
	int		lr_dir ;

	root_path = GetDocument( )->GetCatalog( ) ;
	root_path.MakeLower( ) ;
	lr_dir = root_path.Find( _T( "lightroom" ) ) ;

	if ( -1 != lr_dir ) 
	{
		root_path = GetDocument( )->GetCatalog( ) ;
		root_path = root_path.Left( lr_dir - 1 ) ;

		if ( -1 != ( match_index = m_location_combo.FindString( -1, root_path ) ) )
		{
			m_location_combo.SetCurSel( match_index ) ;
		}
		else
		{
			if ( MAX_MRU_LOCATION_LIST_SIZE == m_location_combo.GetCount( ) )
				m_location_combo.DeleteString( m_location_combo.GetCount( ) - 1 ) ;

			m_location_combo.InsertString( 0, root_path ) ;
			m_location_combo.SetCurSel( 0 ) ;
		}
	}
}









/******************************************************************************
BrowseCallbackProc

	Callback for SHBrowseForFolder 


******************************************************************************/
static int CALLBACK BrowseCallbackProc(
	HWND hwnd,							// I - hWnd of browse dialog 
	UINT uMsg,							// I - message - only interested in one 
	LPARAM lParam,						// I - lParam of message - 
	LPARAM lpData						// I - lpData of message 
)
{    
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path. 
	switch ( uMsg ) 
	{        
		case BFFM_INITIALIZED:
		{            
			if ( NULL != lpData )
			{                
				SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData ) ;
			}
		}
	}
	return 0 ; // The function should always return 0.
} 





/******************************************************************************
BrowseForFolder

	Cut-and-paste function from Codeproject or some other website which gives us 
	ready-made browse-for-directory funcitonality 

	FIXME - would really prefer if the display would scroll to ensure the currently selected directory 
	is visible. 

******************************************************************************/
BOOL BrowseForFolder(
	HWND hwnd,							// I - hWnd for owner, the define project panel
	LPCTSTR szCurrent,					// I - gets path of current folder (will start here)
	LPTSTR szPath						// O - gets path of selected folder 
)
{    
	BROWSEINFO   bi = { 0  } ;   
	LPITEMIDLIST pidl ;    
	TCHAR        szDisplay[ MAX_PATH ] ;    
	BOOL         retval ;     
	
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ;
	
	bi.hwndOwner      = hwnd ;
	
	bi.pszDisplayName = szDisplay ;
	bi.lpszTitle      = TEXT( "Select a location for the disk directory" ) ;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE ;
	bi.lpfn           = BrowseCallbackProc ;
	bi.lParam         = (LPARAM) szCurrent ;

	pidl = SHBrowseForFolder( &bi ) ;

	if ( NULL != pidl )    
	{        
		retval = SHGetPathFromIDList( pidl, szPath ) ;        
		CoTaskMemFree( pidl ) ;    
	}    
	else    
	{        
		retval = FALSE ;    
	}     
	
	if ( !retval )    
	{        
		szPath[ 0 ] = TEXT( '\0' ) ;    
	}     
	
	CoUninitialize( ) ;    
	return retval ;
}




/******************************************************************************
CPanelDefineProject::OnBnClickedBrowseDirectory

	Message-mapped handler for click on Browse Directory button - 

	Uses a canned select-folder dialog to let user pick a directory 

******************************************************************************/
void CPanelDefineProject::OnBnClickedBrowseDirectory( )
{
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
}





/******************************************************************************
CPanelDefineProject::OnCbnSelendokProjectTypeCombo

	Message-mapped handler for combo box notification SELENDOK 

	Get the user's choice for disk type and update the descriptive text appropriately 

******************************************************************************/
void CPanelDefineProject::OnCbnSelendokProjectTypeCombo( )
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
	GetDocument( )->SetDiskType( (CMakePhotoDisk3Doc::DiskTypes) disk_type ) ;
	m_disk_type_text.Invalidate( ) ;
}




/******************************************************************************
CPanelDefineProject::OnBnClickedBack

	Message-mapped handler for user-click on Back button 

	Change the control panel back to the previous pane, which allows user to 
	select catalog & collection 

******************************************************************************/
void CPanelDefineProject::OnBnClickedBack()
{
	GetParentFrame( )->PostMessage( WM_BACKUP_TO_SELECT_COLLECTION ) ;
}



/******************************************************************************
CPanelDefineProject::PassSourceFilesCheck

	Checks contents of selected collection to make sure that all source files are 
	present and all source file formats are supported (eg, they're .psd or .jpg)

******************************************************************************/
bool CPanelDefineProject::PassSourceFilesCheck( )
{
	bool isOK = true ;
	PhotoImage *	image ;
	POSITION		pos ;

	if ( image = GetDocument( )->GetFirstPhoto( pos ) ) 
	{
		do
		{
			if ( image->SourceIsMissing( ) )
			{
				isOK = false ;
				AfxMessageBox( _T( "The source file for one or more images in this collection is missing." ), MB_OK | MB_ICONSTOP ) ;
			}
			else if ( image->UnsupportedFileFormat( ) )
			{
				isOK = false ;
				AfxMessageBox( _T( "One or more images in this collection are in an unsupported file format." ), MB_OK | MB_ICONSTOP ) ;
			}
		}
		while ( isOK && ( image = GetDocument( )->GetNextPhoto( pos ) ) ) ;
	}

	return isOK ;
}







/******************************************************************************
CPanelDefineProject::OnBnClickedCreateProject

	Message-mapped handler for user click on the "create project" button at the
	bottom of the panel. 

	If successful, this also advances the panel to the next step - 

******************************************************************************/
void CPanelDefineProject::OnBnClickedCreateProject()
{
	CString					directory ;
	CString					location ;

	m_location_combo.GetLBText( m_location_combo.GetCurSel( ), location ) ;
	m_dir_name_edit.GetWindowText( directory ) ;

	directory.Trim( ) ;
	if ( directory.GetLength( ) )
	{
		// validate the characters in directory name - 
		if ( -1 == directory.FindOneOf( _T( "<>:\"/\\|?*" ) ) )
		{
			CString	fq_path ;

			fq_path.Format( _T( "%s\\%s" ), (const TCHAR *) location, (const TCHAR *) directory ) ;

			// the directory must not exist
			if ( INVALID_FILE_ATTRIBUTES == ::GetFileAttributes( fq_path ) && ERROR_FILE_NOT_FOUND == ::GetLastError( ) ) 
			{
				// the colleciton must not contain unsupported or missing source files 
				if ( PassSourceFilesCheck( ) )
				{
					CConfirmProjectCreation	dlg ;

					dlg.m_catalog_name = GetDocument( )->GetCatalog( ) ;
					dlg.m_collection_name = GetDocument( )->GetSelectedCollection( )->GetName( ) ;
					dlg.m_directory_name = fq_path ;
					dlg.m_photo_count.Format( _T( "%d images in collection" ), GetDocument( )->GetPhotoListSize( ) ) ;
					m_disk_type_combo.GetLBText( m_disk_type_combo.GetCurSel( ), dlg.m_disk_type ) ;

					// the user must confirm all parameters...
					if ( IDOK == dlg.DoModal( ) )
					{
						// set up the options in the directory 
						GetDocument( )->SetLocation( location ) ;
						GetDocument( )->SetDirectory( directory ) ;
						GetDocument( )->SetDiskType( (CMakePhotoDisk3Doc::DiskTypes) m_disk_type_combo.GetItemData( m_disk_type_combo.GetCurSel( ) ) ) ;

						BeginWaitCursor( ) ;

						// copy the template files, then tell the mainframe to advance to the next control panel
						if ( GetDocument( )->CreateProject( ) ) 
							GetParentFrame( )->PostMessage( WM_CREATE_PROJECT ) ;
						else
						{
							m_dir_name_edit.SetSel( 0, -1 ) ;
							m_dir_name_edit.SetFocus( ) ;
						}

						SaveMRULocationList( ) ;
						EndWaitCursor( ) ;
					}
				}
			}
			else
			{
				AfxMessageBox( _T( "The directory already exists." ), MB_OK | MB_ICONSTOP ) ;
				m_dir_name_edit.SetSel( 0, -1 ) ;
				m_dir_name_edit.SetFocus( ) ;
			}
		}
		else
		{
			AfxMessageBox( _T( "The directory name contains illegal characters." ), MB_OK | MB_ICONSTOP ) ;
			m_dir_name_edit.SetSel( 0, -1 ) ;
			m_dir_name_edit.SetFocus( ) ;
		}
	}
	else
	{
		AfxMessageBox( _T( "The directory name may not be blank." ), MB_OK | MB_ICONSTOP ) ;
		m_dir_name_edit.SetSel( 0, -1 ) ;
		m_dir_name_edit.SetFocus( ) ;
	}
}








/******************************************************************************
CPanelDefineProject::OnCbnSelendokProjectLocation

	message-mapped handler for combo box CBN_SELENDOK notification. 

******************************************************************************/
void CPanelDefineProject::OnCbnSelendokProjectLocation()
{
	CString		location ;

	if ( 0 != m_location_combo.GetCurSel( ) )
	{
		m_location_combo.GetLBText( m_location_combo.GetCurSel( ), location ) ;
		m_location_combo.DeleteString( m_location_combo.GetCurSel( ) ) ;
		m_location_combo.InsertString( 0, location ) ;
		m_location_combo.SetCurSel( 0 ) ;
	}
}





/******************************************************************************
CPanelDefineProject::LoadMRULocationList

	Initialize the location combo box with the MRU list of locations loaded
	from the registry key 

******************************************************************************/
void CPanelDefineProject::LoadMRULocationList( ) 
{
	CString	long_keyname ;
	HKEY	key ;

	long_keyname.Format( _T( "SOFTWARE\\%s\\%s" ), MY_COMPANY_REGISTRY_KEY, MY_PRODUCT_REGISTRY_KEY ) ;

	if ( ERROR_SUCCESS == ::RegCreateKeyEx( HKEY_CURRENT_USER, long_keyname, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL ) ) 
	{
		DWORD	value_type ;
		TCHAR	data[ MAX_MRU_LOCATION_LIST_SIZE * MAX_PATH + 1 ] ;
		DWORD	data_size ;

		data_size = sizeof( data ) ;

		if ( ERROR_SUCCESS == ::RegQueryValueEx( key, _T( "MRU Root Directory" ), NULL, &value_type, (BYTE *) data, &data_size ) ) 
		{
			TCHAR *	directory = data ;

			do
			{
				m_location_combo.AddString( directory ) ;
				directory += _tcslen( directory ) + 1 ;
			}
			while ( *directory && directory < data + data_size / sizeof( TCHAR ) ) ;
		}

		::RegCloseKey( key ) ;
	}
}





/******************************************************************************
CPanelDefineProject::SaveMRULocationList

	Save the MRU location list, including the user selection in the combo box 

******************************************************************************/
void CPanelDefineProject::SaveMRULocationList( ) 
{
	CString	long_keyname ;
	HKEY	key ;

	long_keyname.Format( _T( "SOFTWARE\\%s\\%s" ), MY_COMPANY_REGISTRY_KEY, MY_PRODUCT_REGISTRY_KEY ) ;

	if ( ERROR_SUCCESS == ::RegCreateKeyEx( HKEY_CURRENT_USER, long_keyname, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL ) ) 
	{
		TCHAR	data[ MAX_MRU_LOCATION_LIST_SIZE * MAX_PATH + 1 ] ;
		TCHAR *	data_pos ;
		int		i ;

		for ( data_pos = data, i = 0 ; i < m_location_combo.GetCount( ) ; i++ )
		{
			CString location ;

			m_location_combo.GetLBText( i, location ) ;

			_tcscpy_s( data_pos, sizeof( data ) / sizeof( TCHAR ) - ( data_pos - data ), (const TCHAR *) location ) ;
			data_pos += _tcslen( data_pos ) + 1 ;
		}
		*data_pos++ = _T( '\0' ) ;

		if ( ERROR_SUCCESS != ::RegSetValueEx( key, _T( "MRU Root Directory" ), NULL, REG_MULTI_SZ, (BYTE *) data, (BYTE *) data_pos - (BYTE *) data  ) )
		{
			ASSERT( FALSE ) ;
			AfxMessageBox( _T( "Unable to save registry value!" ), MB_OK | MB_ICONEXCLAMATION ) ;
		}
		::RegCloseKey( key ) ;
	}
} 


