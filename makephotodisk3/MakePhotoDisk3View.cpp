
// MakePhotoDisk3View.cpp : implementation of the CMakePhotoDisk3View class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MakePhotoDisk3.h"
#endif

#include "MakePhotoDisk3Doc.h"
#include "MakePhotoDisk3View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMakePhotoDisk3View

IMPLEMENT_DYNCREATE(CMakePhotoDisk3View, CView)

BEGIN_MESSAGE_MAP(CMakePhotoDisk3View, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
//	ON_WM_CREATE()
//	ON_WM_SIZE()
//	ON_WM_SHOWWINDOW()
//	ON_WM_PAINT()

	ON_MESSAGE( WM_NEW_CATALOG, OnNewCatalog )
	ON_MESSAGE( WM_COLLECTION_SELCHANGE, OnCollectionSelchange )
	ON_MESSAGE( WM_IMAGE_SELCHANGE, OnImageSelchange ) 
	ON_MESSAGE( WM_GOTO_DEFINE_PROJECT, OnGotoDefineProject ) 
	ON_MESSAGE( WM_CREATE_PROJECT, OnCreateProject )

//	ON_WM_MOUSEHWHEEL()
//	ON_WM_MOUSEWHEEL()
//	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CMakePhotoDisk3View construction/destruction

CMakePhotoDisk3View::CMakePhotoDisk3View()
{
	m_state = WelcomeScreen ;
}

CMakePhotoDisk3View::~CMakePhotoDisk3View()
{
}

BOOL CMakePhotoDisk3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMakePhotoDisk3View drawing

void CMakePhotoDisk3View::OnDraw(CDC* /*pDC*/)
{
	CMakePhotoDisk3Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMakePhotoDisk3View printing

BOOL CMakePhotoDisk3View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMakePhotoDisk3View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMakePhotoDisk3View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CMakePhotoDisk3View diagnostics

#ifdef _DEBUG
void CMakePhotoDisk3View::AssertValid() const
{
	CView::AssertValid();
}

void CMakePhotoDisk3View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMakePhotoDisk3Doc* CMakePhotoDisk3View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMakePhotoDisk3Doc)));
	return (CMakePhotoDisk3Doc*)m_pDocument;
}
#endif //_DEBUG


// CMakePhotoDisk3View message handlers





/******************************************************************************
******************************************************************************/
//int CMakePhotoDisk3View::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CView::OnCreate(lpCreateStruct) == -1)
//		return -1;
//
//	return 0;
//}







/******************************************************************************
******************************************************************************/
//void CMakePhotoDisk3View::OnSize(UINT nType, int cx, int cy)
//{
//	CView::OnSize(nType, cx, cy);
//
//	CRect		client_rect ;
//	int			vsplit ;
//	int			hsplit ;
//
//	GetClientRect( &client_rect ) ;
//
//	vsplit = client_rect.Width( ) / 4 ;
//	if ( vsplit < 360 )
//		vsplit = 360 ;
//	hsplit = client_rect.Height( ) / 5 ;
//
//	m_control_panel.MoveWindow( 0, 0, vsplit, client_rect.Height( ) ) ;
//	m_light_table.MoveWindow( vsplit, 0, client_rect.Width( ) - vsplit, client_rect.Height( ) - hsplit ) ;
//	m_output_panel.MoveWindow( vsplit, client_rect.Height( ) - hsplit, client_rect.Width( ) - vsplit, hsplit ) ;
//}










/******************************************************************************
void CMakePhotoDisk3View::OnShowWindow




******************************************************************************/
//void CMakePhotoDisk3View::OnShowWindow(BOOL bShow, UINT nStatus)
//{
//	CView::OnShowWindow(bShow, nStatus);
//}










/******************************************************************************
void CMakePhotoDisk3View::OnPaint

Paint the view. The view is covered up by the sub-panels during most operations. 

******************************************************************************/
//void CMakePhotoDisk3View::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//
//	CFont	big_number_font ;
//	CFont	subcaption_font ;
//	CFont	reg_paragraph_font ;
//	CFont *	dc_font ;
//	CRect	num_cell ;
//	CRect	para_cell ;
//	int		i ;
//	CRect	panel_rect ;
//
//	if ( WelcomeScreen == m_state ) 
//	{
//		static struct OUTPUT_TEXT 
//		{
//			const TCHAR * num ;
//			const TCHAR * caption ;
//			const TCHAR * paragraph ;
//		} 
//		start_page_text[ ] =
//		{
//			{	_T( "1" ),		_T( "Choose a lightroom catalog." ),						_T( "Use the combo box and browse button in the panel to the left to choose the Lightroom catalog (.lrcat file) to use" )	},
//			{	_T( "2" ),		_T( "Select a collection within the chosen catalog." ),		_T( "After choosing a catalog, a tree control displays the user-defined collections defined within the catalog." )	},
//			{	_T( "3" ),		_T( "Designate a location to assemble the disk image." ),	_T( "The content for the disk will be assembled in a directory on the hard drive. Do not use a directory with other files in it. You will have the opportunity to create a new directory." )	},
//			{   _T( "4" ),		_T( "Choose the type of disk." ),							_T( "A disk may contain nothing more than low-resolution sample images, or full-size images suitable for printing." ) },
//			{	_T( "5" ),		_T( "Check content and burn the disk." ),					_T( "Use this program to update the image-content of the disk, edit image information and extended comments, and launch the disk-burner." ) }
//		} ;
//
//		// need to shove the output rectangles for the text over to accomodate the control panel 
//		m_control_panel.GetClientRect( &panel_rect ) ;
//		num_cell.top = -10;
//		num_cell.bottom = num_cell.top + 120 ;
//		num_cell.left = panel_rect.Width( ) ;
//		num_cell.right = num_cell.left + 90 ;
//
//		para_cell.top = 20 ;
//		para_cell.bottom = 140 ;
//		para_cell.left = 100 + panel_rect.Width( ) ;
//		para_cell.right = para_cell.left + 600 ;
//
//		// big_number_font.CreateFont( -90, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial Black" ) ) ;
//		big_number_font.CreateFont( -90, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial Black" ) ) ;
//		subcaption_font.CreateFont( -24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial" ) ) ;
//		reg_paragraph_font.CreateFont( -18, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FF_MODERN, _T( "Arial" ) ) ;
//	
//		dc_font = dc.SelectObject( &big_number_font ) ;
//
//		for ( i = 0 ; i < sizeof( start_page_text ) / sizeof( start_page_text[ 0 ] ) ; i++ )
//		{
//			dc.SelectObject( &big_number_font ) ;
//			dc.SetTextColor( RGB( 80, 140, 255 ) ) ;		// light blue 
//			dc.DrawText( start_page_text[ i ].num, -1, &num_cell, DT_RIGHT | DT_TOP | DT_SINGLELINE ) ;
//
//			dc.SetTextColor( RGB( 0, 70, 220 ) ) ;			// dark blue 
//			dc.SelectObject( &subcaption_font ) ;
//			para_cell.top += dc.DrawText( start_page_text[ i ].caption, -1, &para_cell, DT_LEFT | DT_TOP ) ;
//			dc.SelectObject( &reg_paragraph_font ) ;
//			dc.DrawText( start_page_text[ i ].paragraph, -1, &para_cell, DT_LEFT | DT_TOP | DT_WORDBREAK ) ;
//
//			para_cell.top = para_cell.bottom ;
//			para_cell.bottom += num_cell.Height( ) ;
//
//			num_cell.top = num_cell.bottom ;
//			num_cell.bottom = num_cell.top + para_cell.Height( ) ;
//		}
//
//		dc.SelectObject( dc_font ) ;
//	}
//}





/**************************************************************************
void CMakePhotoDisk3View::OnInitialUpdate

Called once by the framework. It's a good place to initialize the view... but
the view is largely unused. Instead, we forward the message to the only active
child panel. 

**************************************************************************/
void CMakePhotoDisk3View::OnInitialUpdate()
{
	CView::OnInitialUpdate();

}





/******************************************************************************
CMakePhotoDisk3View::OnNewCatalog

Handles the WM_NEW_CATALOG message, originated by the control panel when the user
selects a new catalog. 

This message makes a round trip from the control panel and back, b/c it effects 
a change to the document, and the panels are not allowed to update the document
directly, although they are allowed to read the document's data structures (eg, 
the list of collections, the photos in the selected collection, etc). 

******************************************************************************/
LRESULT CMakePhotoDisk3View::OnNewCatalog( WPARAM wp, LPARAM )
{
	const TCHAR * catalog ;

	catalog = (const TCHAR *) wp ;

	// FIXME: In the future, we should implement "view state" so that the guidance text in the view updates to tell the user he's at the next stage.... ??

	GetDocument( )->SetCatalog( catalog ) ;

	/* 
	Tell the panels that a new catalog has been selected. This looks a little weird, since normally it was the control panel which told US that a new catalog was selected, 
	and now we're telling the control panel a new catalog was selected... but any change to the document effected by a child panel must route through the view if that change
	could conceivably affect any other panel
	*/ 
	//m_control_panel.OnNewCatalog( ) ;
	//m_light_table.OnNewCatalog( ) ;
	//m_output_panel.OnNewCatalog( ) ;

	return 0 ;
}







/**************************************************************************
LRESULT CMakePhotoDisk3View::OnCollectionSelchange

	Called to handle the WM_COLLECTION_SELCHANGE message, sent by the collection tree
	in the control panel. 

	The wp parameter tells us whether we're selecting a new collection, specified by the
	lp param, or whether we're de-selecting the current selection. 

**************************************************************************/
LRESULT CMakePhotoDisk3View::OnCollectionSelchange( WPARAM wp, LPARAM lp )
{
	bool change_to ;
	Collection * col ;

	change_to = ( 0 != wp ) ;
	col = (Collection *) lp ;

	if ( change_to && NULL != col )
	{
		m_state = SelectACollection ;

		SetFocus( ) ;				// grab the focus back from the collection tree...
		BeginWaitCursor( ) ;

		// update the document, then the views
		GetDocument( )->SelectCollection( col ) ;

		m_light_table.SelectCollection( col ) ;
		m_output_panel.SelectCollection( col ) ;
		m_control_panel.SelectCollection( col ) ;
		m_light_table.ShowWindow( SW_SHOW ) ;
		m_output_panel.ShowWindow( SW_SHOW ) ;

		EndWaitCursor( ) ;
	}
	else
	{
		m_state = WelcomeScreen ;

		// update the document, then the views
		GetDocument( )->DeselectCollection( ) ;

		m_light_table.DeselectCollection( ) ;
		m_output_panel.DeselectCollection( ) ;
		m_control_panel.DeselectCollection( ) ;
	}

	return 0 ;
}





/******************************************************************************
LRESULT CMakePhotoDisk3View::OnImageSelchange

Handles a WM_IMAGE_SELCHANGE message posted by the light table when the
user selects a new active image. The wp is the number of the selected pic.

Since the light_table originated this message, the only place we need to propogate it to
is going to be the output panel. 
******************************************************************************/
LRESULT CMakePhotoDisk3View::OnImageSelchange( WPARAM wp, LPARAM lp )
{
	int				selected_pic ;
	POSITION		pos ;
	PhotoImage *	photo ;

	selected_pic = (int) wp ;

	photo = GetDocument( )->FindPhoto( selected_pic, pos ) ;
	m_output_panel.SetSelectedPhoto( photo ) ;

	return 0 ;

}






/******************************************************************************
LRESULT CMakePhotoDisk3View::OnGotoDefineProject

Invoked after control panel sends us the WM_GOTO_DEFINE_PROJECT message 


******************************************************************************/
LRESULT CMakePhotoDisk3View::OnGotoDefineProject( WPARAM, LPARAM )
{
	m_state = DefiningProject ;
	// at this point, I really don't need to do anything else...

	return 0 ;
}




	


/******************************************************************************
LRESULT CMakePhotoDisk3View::OnCreateProject


******************************************************************************/
LRESULT CMakePhotoDisk3View::OnCreateProject( WPARAM wp, LPARAM lp )
{
	BeginWaitCursor( ) ;

	const TCHAR * fq_directory = (const TCHAR *) wp ;
	CMakePhotoDisk3Doc::DiskTypes project_type = (CMakePhotoDisk3Doc::DiskTypes ) lp ;

	GetDocument( )->CreateProject( fq_directory, project_type ) ;

	m_state = UpdatingContent ;

	m_light_table.OnCreateProject( ) ;
	m_control_panel.OnCreateProject( ) ;

	EndWaitCursor( ) ;

	return 0 ;
}







/******************************************************************************
BOOL CMakePhotoDisk3View::OnMouseWheel

Windows does NOT dispatch WM_MOUSEWHEEL messages to whichever child window has the focus, so it's 
up to us, the View, to forward them as appropriate. 

******************************************************************************/
//BOOL CMakePhotoDisk3View::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
//{
//	if ( m_state != WelcomeScreen ) 
//	{
//		CRect	child_rect ;
//		CPoint	test_pt ;
//
//		m_light_table.GetClientRect( &child_rect ) ;
//
//		// the CPoint passed in is in screen coordinates... 
//		test_pt = pt ;
//		m_light_table.ScreenToClient( &test_pt ) ;
//
//		if ( child_rect.PtInRect( test_pt ) )
//			return m_light_table.OnMouseWheel( nFlags, zDelta, pt ) ;
//		else
//		{
//			test_pt = pt ;
//			m_output_panel.ScreenToClient( &test_pt ) ;
//			m_output_panel.GetClientRect( &child_rect ) ;
//
//			if ( child_rect.PtInRect( pt ) ) 
//				return m_output_panel.OnMouseWheel( nFlags, zDelta, pt ) ;
//		}
//	}
//
//	return CView::OnMouseWheel(nFlags, zDelta, pt);
//}






/******************************************************************************
void CMakePhotoDisk3View::OnKeyDown

	Needed to support arrow navigation through the light table

******************************************************************************/
//void CMakePhotoDisk3View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//	if ( m_state != WelcomeScreen ) 
//		m_light_table.OnKeyDown( nChar, nRepCnt, nFlags ) ;
//
//	CView::OnKeyDown(nChar, nRepCnt, nFlags);
//}
