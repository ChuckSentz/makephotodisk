/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	The application object. 

	Only modifications of note support check for an instance of lightroom or another instance 
	of MakePhotoDisk3

***************************************************************************************/

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MakePhotoDisk3.h"
#include "MainFrm.h"
#include "WelcomeView.h"
#include "MakePhotoDisk3Doc.h"
#include <afxpriv.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define MUTEX_NAME				_T( "MakePhotoDisk3_A9E6077B-D9DE-470E-AF81-6EDC4124D2CD" ) 



// CMakePhotoDisk3App

BEGIN_MESSAGE_MAP(CMakePhotoDisk3App, CWinApp)

	ON_COMMAND(ID_APP_ABOUT, &CMakePhotoDisk3App::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
	// ON_COMMAND(ID_FILE_OPEN, &CMakePhotoDisk3App::OnFileOpen)
	// ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, &CMakePhotoDisk3App::OnOpenRecentFile)

	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, &CMakePhotoDisk3App::OnUpdateRecentFileMenu)
END_MESSAGE_MAP()



static bool FindAnInstanceOfLightroom( ) ;
static bool FindAnotherInstance( ) ;




/******************************************************************************
CMakePhotoDisk3App::CMakePhotoDisk3App

	Plain vanilla constructor 

******************************************************************************/
CMakePhotoDisk3App::CMakePhotoDisk3App()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MakePhotoDisk3.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CMakePhotoDisk3App object
CMakePhotoDisk3App theApp;






/******************************************************************************
CMakePhotoDisk3App::InitInstance

	Perform rudimentary check for previous instances of Lightroom and 
	our app. Proceed only if neither is detected. 

******************************************************************************/
BOOL CMakePhotoDisk3App::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	while ( FindAnInstanceOfLightroom( ) )
	{
		if ( IDCANCEL == AfxMessageBox( _T( "An instance of Lightroom appears to be running. You can quit Lighroom and try again or cancel." ), MB_ICONSTOP | MB_RETRYCANCEL ) )
			return FALSE ;
	}

	if ( FindAnotherInstance( ) )
	{
		return FALSE ;		
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("CCS Software" ) ) ;
	LoadStdProfileSettings(6);  // Load standard INI file options (including MRU)
	m_pRecentFileList->m_nMaxDisplayLength = 120 ;

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMakePhotoDisk3Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWelcomeView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

int CMakePhotoDisk3App::ExitInstance()


{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CMakePhotoDisk3App message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CMakePhotoDisk3App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMakePhotoDisk3App message handlers


struct EnumPackage
{
	const TCHAR *	app_name ;
	HWND			window_handle ;
} ;
BOOL CALLBACK EnumWindowsCallback( HWND hWnd, LPARAM lParam ) ;


/******************************************************************************
FindAnotherInstance

	Use a named Mutex to ensure only one instance of this application is allowed
	to run at any time. 

	We don't even need to wait on the Mutex, since we can call GetLastError and
	detect if someone else created it first. The mutex is discarded by the OS 
	after we exit, so we need never close our handle. 

******************************************************************************/
static bool FindAnotherInstance( )
{
	HANDLE	mutex_handle ;

	EnumPackage enum_package = 
	{
		_T( "makephotodisk3" ),	NULL
	} ;

	EnumDesktopWindows( NULL, EnumWindowsCallback, (LPARAM) &enum_package ) ;
	
	if ( NULL != enum_package.window_handle ) 
	{
		if ( mutex_handle = CreateMutex( NULL, TRUE, MUTEX_NAME ) )
		{
			DWORD err = ::GetLastError( ) ;

			if ( ERROR_ALREADY_EXISTS == err )
			{
				AfxMessageBox( _T( "Another instance of MakePhotoDisk is already running." ), MB_ICONSTOP | MB_OK ) ;
				return true ;
			}
		}
	}

	// just return false - and let the handle go into stack oblivion... we own the mutex until we exit
	return false ;
}







/******************************************************************************
EnumWindowsCallback

	Callback function for EnumDesktopWindows called in FindAnInstanceOfLightroom.
	Caller passes a pointer to EnumPackage, which Windows hands back with each 
	invocation of our callback as the 'lParam'. 

******************************************************************************/
BOOL CALLBACK EnumWindowsCallback( 
	HWND hWnd,					// I - next window detected by EnumDesktopWindows
	LPARAM lParam				// I/O - references the original caller's EnumPackage struct
) 
{
	EnumPackage *	ep ;
	TCHAR			window_title[ MAX_PATH ] ;

	ep = (EnumPackage *) lParam ;

	GetWindowText( hWnd, window_title, sizeof( window_title ) ) ;
	_tcslwr_s( window_title, sizeof( window_title ) / sizeof( window_title[ 0 ] ) ) ;

	if ( _tcsstr( window_title, ep->app_name ) )
	{
		ep->window_handle = hWnd ;
		return FALSE ;					// tell Windows to stop enumerating 
	}

	return TRUE ;		// tell Windows to keep going 
}


/******************************************************************************
FindAnInstanceOfLightroom

	Rather unsatisfactory method Windows provides of detecting another application. 
	We just enumerate windows and look for one that has Lightroom in the title. 

******************************************************************************/
static bool FindAnInstanceOfLightroom( )
{
	EnumPackage enum_package = 
	{
		_T( "lightroom" ),	NULL
	} ;

	EnumDesktopWindows( NULL, EnumWindowsCallback, (LPARAM) &enum_package ) ;
	
	if ( NULL != enum_package.window_handle ) 
	{
		return true ;
	}

	return false ;
}










#define FIXBUG

/******************************************************************************
CMakePhotoDisk3App::OpenDocumentFile

	Open a disk project file (.dp)

	This function copes with a rather annoying "feature" of the application 
	framework's handling of MRU filename strings. 


******************************************************************************/
CDocument* CMakePhotoDisk3App::OpenDocumentFile(LPCTSTR lpszFileName)
{
	CMakePhotoDisk3Doc * doc = NULL ;

	if ( PathFileExists( lpszFileName ) )
	{
		CMainFrame * main_frame ;

#ifdef FIXBUG
		CString		 file_name_copy ;
#endif // FIXBUG

		/* 
			UGLY WORKAROUND TO FIX A BUG IN MFC

			Basically, lpszFileName is a constant pointer to TCHAR created deep within MFC internal code handling the MRU file list. It is 
			supposed to be a const pointer to text. But within CString implementation, the cast operator merely returns a pointer to the 
			CString's data pointer. Although the consumer is prevented from altering the text it points to, there's nothing preventing 
			any function call with side-effects from altering the text. 
			
			Under the call below to AddToRecentFileList(), MFC invokes CRecentFileList::Add(), which re-orders the MRU list by copying 
			array elements to move the selected MRU filename to position 0. This copy operation can have the side effect that the data 
			pointer for the selected CString is re-allocated, when a larger string is copied into it. This completely trashes the memory 
			referenced by the const TCHAR pointer (it is filled with MS "magic number" FEEEFEE, which is supposed to indicated freed heap 
			memory. 

			But any subsequent reference to the const char pointer is now referencing free'd memory. 
			
			Immediately afterwards, CRecentFileList::Add attempts to use the trashed pointer and this traps in a call to 
			_AfxSHCreateItemFromParsingName, which is attempting to validate the filename. 
		*/
#ifdef FIXBUG
		file_name_copy = lpszFileName ;
		lpszFileName = file_name_copy ;		// make lpszFileName point to OUR CString, so MFC internals can't screw with it 
#endif 

		if ( main_frame = STATIC_DOWNCAST( CMainFrame, m_pMainWnd ) )
			if ( doc = STATIC_DOWNCAST( CMakePhotoDisk3Doc, main_frame->GetActiveDocument( ) ) )
				if ( doc->LoadDocument( lpszFileName ) )
				{
					AddToRecentFileList( lpszFileName ) ;
					main_frame->OnCreateProject( 0, 0 ) ;
				}
				else
					doc = NULL ;
	}
	else
		AfxMessageBox( _T( "Unable to find the selected file." ), MB_OK ) ;

	return doc ;
}
#undef FIXBUG




/******************************************************************************
CMakePhotoDisk3App::OnUpdateRecentFileMenu

	Default UPDATE_COMMAND_UI handlers are called for a submenu if the ID in question 
	happens to be the first item in the menu.... 

	Don't remember why this is necessary, but believe it works around a problem
	with moving the MRU list to a submenu rather than a section in the File menu

******************************************************************************/
void CMakePhotoDisk3App::OnUpdateRecentFileMenu(CCmdUI *pCmdUI)
{
	if ( pCmdUI->m_pSubMenu == NULL )
		CWinApp::OnUpdateRecentFileMenu( pCmdUI ) ;
	return ;
}
