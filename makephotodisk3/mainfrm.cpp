/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Typical MFC CMainFrame class with a handful of modifications to create splitters, handle
re-sizing, and process messages which result in changing the contents of the window 
panes. 
***************************************************************************************/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MainFrm.h"
#include "PanelSelectCollection.h"
#include "MakePhotoDisk3Doc.h"
#include "PanelDefineProject.h"
#include "PanelEditUpdateDisk.h"
#include "LightTable.h"
#include "OutputPanel.h"
#include "WelcomeView.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()

	ON_MESSAGE( WM_GOTO_DEFINE_PROJECT, OnGotoDefineProject ) 
	ON_MESSAGE( WM_CREATE_PROJECT, OnCreateProject )
	ON_MESSAGE( WM_BACKUP_TO_SELECT_COLLECTION, OnBackupToSelectCollection ) 
	ON_COMMAND(ID_FILE_NEW, &CMainFrame::OnFileNew)

END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}



int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}






/******************************************************************************
CMainFrame::OnCreateClient

	Create the vertical and horizontal splitters. 

******************************************************************************/
BOOL CMainFrame::OnCreateClient(
	LPCREATESTRUCT lpcs,				// I - windows create structure thing
	CCreateContext* pContext			// I - MFC create context thing 
)
{
	m_vert_splitter.CreateStatic( this, 1, 2 ) ;
	m_vert_splitter.CreateView( 0, 0, RUNTIME_CLASS( CPanelSelectCollection ), CSize( 0, 0 ), pContext ) ;
	m_vert_splitter.LockSplitBar( ) ;

	m_horz_splitter.CreateStatic( &m_vert_splitter, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_vert_splitter.IdFromRowCol( 0, 1 ) ) ;

	m_horz_splitter.CreateView( 0, 0, RUNTIME_CLASS( CLightTable ), CSize( 0, 0 ), pContext ) ;
	m_horz_splitter.CreateView( 1, 0, RUNTIME_CLASS( COutputPanel ), CSize( 0, 0 ), pContext ) ;
	m_horz_splitter.LockSplitBar( ) ;

	return TRUE ;
}





/******************************************************************************
CMainFrame::OnSize

	Just guarantee that the output pane is a modest size below the light table 

******************************************************************************/
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	if ( cy > 0 && m_horz_splitter.m_hWnd ) 
	{
		m_horz_splitter.SetRowInfo( 0, cy - 200, cy - 200 ) ;
		m_horz_splitter.RecalcLayout( ) ;
	}
}






/******************************************************************************
CMainFrame::OnGetMinMaxInfo

	just prevent the user from scaling the window down too small

******************************************************************************/
void CMainFrame::OnGetMinMaxInfo(
	MINMAXINFO* lpMMI					// O - gets min x & y sizes 
)
{
	lpMMI->ptMinTrackSize.x = 1200 ;
	lpMMI->ptMinTrackSize.y = 820 ;

	CFrameWnd::OnGetMinMaxInfo(lpMMI);
}





/******************************************************************************
CMainFrame::OnGotoDefineProject

	Message-mapped handler for WM_GOTO_DEFINE_PROJECT user message, which means 
	we swap the CPanelDefineProject view into the control-panel pane 

******************************************************************************/
LRESULT CMainFrame::OnGotoDefineProject( WPARAM, LPARAM ) 
{
	m_vert_splitter.ChangeView( 0, 0, RUNTIME_CLASS( CPanelDefineProject ) ) ;

	return 0 ;
}





/******************************************************************************
CMainFrame::OnCreateProject

	Message-mapped handler for WM_CREATE_PROJECT user message, which means 
	we swap in the CPanelEditUpdateDisk view into the control pane 

******************************************************************************/
LRESULT CMainFrame::OnCreateProject( WPARAM, LPARAM )
{
	m_vert_splitter.ChangeView( 0, 0, RUNTIME_CLASS( CPanelEditUpdateDisk ) ) ;

	return 0 ;
}





/******************************************************************************
CMainFrame::OnBackupToSelectCollection

	Message-mapped handler for WM_BACKUP_TO_SELECT_COLLECTION user-message, which
	means we swap in the CPanelSelectCollection view 

******************************************************************************/
LRESULT CMainFrame::OnBackupToSelectCollection ( WPARAM, LPARAM )
{
	m_vert_splitter.ChangeView( 0, 0, RUNTIME_CLASS( CPanelSelectCollection ) ) ;
	return 0 ;
}




/**************************************************************************************************
CMainFrame::OnFileNew

	All we do is tell the vertical splitter to change its view, since that will cause a new
	document to be created. 

**************************************************************************************************/
void CMainFrame::OnFileNew()
{
	m_vert_splitter.ChangeView( 0, 0, RUNTIME_CLASS( CPanelSelectCollection ) ) ;
}


