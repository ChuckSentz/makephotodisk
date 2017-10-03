/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



The main frame window. 

***************************************************************************************/

// MainFrm.h : interface of the CMainFrame class
//

#pragma once


#include "SplitterWndEx.h"


class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

	CSplitterWndFlex	m_vert_splitter ;
	CSplitterWndFlex	m_horz_splitter ;

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	afx_msg LRESULT OnGotoDefineProject( WPARAM wp, LPARAM lp ) ;
	afx_msg LRESULT OnCreateProject( WPARAM wp, LPARAM lp ) ;
	afx_msg LRESULT OnBackupToSelectCollection ( WPARAM wp, LPARAM lp ) ;
	afx_msg void OnFileNew();
};


