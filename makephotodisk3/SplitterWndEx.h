/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


An extended splitter window class which facilitates swapping of views within a pane,
and locking of panes. Derived from code samples from MSDN journal and code project. 

***************************************************************************************/
#pragma once


// CSplitterWndFlex

class CSplitterWndFlex : public CSplitterWnd
{
protected:
	bool m_locked ;

public:
	CSplitterWndFlex( ) ;
	virtual ~CSplitterWndFlex( ) ;

	void ChangeView( int row, int col, CRuntimeClass * pane_view_class ) ;

	inline CSize GetBorderSize( ) 
	{
		return CSize( m_cxBorder, m_cyBorder ) ;
	} ;

	inline CSize GetSplitterSize( ) 
	{
		return CSize( m_cxSplitter, m_cySplitter ) ; 
	} ;

	inline bool LockSplitBar( bool lock_it = true ) 
	{
		bool rv = m_locked ;

		m_locked = lock_it ;
		return rv ;
	} ;

	inline bool GetLockedBar( ) const 
	{
		return m_locked ;
	} ;

	virtual BOOL OnCmdMsg( UINT nID, int nCode, void * extra, AFX_CMDHANDLERINFO * handler_info ) ;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
} ;


