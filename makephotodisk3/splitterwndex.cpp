/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	Implementation of a customized splitter class which supports dynamically replacing
	the panes of a splitter window. Based on an MSDN article which I can no longer locate. 

***************************************************************************************/

#include "stdafx.h"
#include "SplitterWndEx.h"
#include <afxpriv.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CSplitterWndFlex::CSplitterWndFlex()
{
	m_locked = false ;
}

CSplitterWndFlex::~CSplitterWndFlex()
{
}



/******************************************************************************
CSplitterWndFlex::ChangeView

	Replace the attached view in the designated pane

******************************************************************************/
void CSplitterWndFlex::ChangeView( 
	int row,							// I - 0-based row of the pane 
	int col,							// I - 0-based column of the pane 
	CRuntimeClass* new_view_class		// I - the new view class to attach 
)
{
	ASSERT( new_view_class->IsDerivedFrom( RUNTIME_CLASS( CView ) ) ) ;

	CView * current_view = STATIC_DOWNCAST( CView, GetPane( row, col ) ) ;
	bool	activate_new_view ;

	// now set up a creation context. First, we need a pointer to the enclosing CFrameWnd 
	CFrameWnd * parent_frame_window = current_view->GetParentFrame( ) ;

	ASSERT( NULL != parent_frame_window ) ;

	activate_new_view = ( current_view == parent_frame_window->GetActiveView( ) ) ;

	CCreateContext cc ;

	cc.m_pNewViewClass = new_view_class ;
	cc.m_pCurrentDoc = current_view->GetDocument( ) ;
	cc.m_pNewDocTemplate = cc.m_pCurrentDoc ? cc.m_pCurrentDoc->GetDocTemplate( ) : NULL ;
	cc.m_pCurrentFrame = parent_frame_window ;

	// delete the old view, and replace it with the new one...
	DeleteView( row, col ) ;
	VERIFY( CreateView( row, col, new_view_class, CSize( 0, 0 ), &cc ) ) ;
	RecalcLayout( ) ;

	if ( activate_new_view )
		parent_frame_window->SetActiveView( STATIC_DOWNCAST( CView, GetPane( row, col ) ) ) ;

	// send the newly installed view its WM_INITIALUPDATE message 
	CWnd * pWnd = GetPane( row, col ) ;
	if ( pWnd )
		pWnd->SendMessage( WM_INITIALUPDATE ) ;
}





/******************************************************************************
CSplitterWndFlex::OnCmdMsg

	Function which routes commands to all splitter panes / views 

******************************************************************************/
BOOL CSplitterWndFlex::OnCmdMsg( 
	UINT nID,							// I - command id 
	int nCode,							// I - command notification code ( ON_COMMAND, CN_EVENT, CN_UPDATE_COMMAND_UI, etc) 
	void* pExtra,						// I - optional extra context 
	AFX_CMDHANDLERINFO* pHandlerInfo	// I - usually NULL
)
{
	int	row ;
	int	col ;

	for ( row = 0 ; row < GetRowCount( ) ; row++ )
	{
		for ( col = 0 ; col < GetColumnCount( ) ; col++ )
		{
			if ( GetPane( row, col )->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
				return TRUE ;
		}
	}

	return CSplitterWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ;
}




/******************************************************************************
	Message-map and Message handlers 

******************************************************************************/
BEGIN_MESSAGE_MAP(CSplitterWndFlex, CSplitterWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void CSplitterWndFlex::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( !m_locked ) 
		CSplitterWnd::OnLButtonDown(nFlags, point);
	else
		CWnd::OnLButtonDown(nFlags, point);
}

void CSplitterWndFlex::OnMouseMove(UINT nFlags, CPoint point)
{

	if ( !m_locked ) 
		CSplitterWnd::OnMouseMove(nFlags, point);
	else
		CWnd::OnMouseMove( nFlags, point ) ;
}

BOOL CSplitterWndFlex::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( !m_locked ) 
		return CSplitterWnd::OnSetCursor(pWnd, nHitTest, message);
	else
		return CWnd::OnSetCursor( pWnd, nHitTest, message ) ;
}

void CSplitterWndFlex::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
{
	CSplitterWnd::OnDrawSplitter(pDC, nType, rect);
}
