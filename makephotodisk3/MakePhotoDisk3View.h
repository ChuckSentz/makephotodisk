
// MakePhotoDisk3View.h : interface of the CMakePhotoDisk3View class
//

#pragma once


#error Including obsolete file 



#include "OutputPanel.h"
#include "LightTable.h"




class CMakePhotoDisk3View : public CView
{
public:
	enum ViewState
	{
		WelcomeScreen = 1,			// print a welcome message or a start screen 
		SelectACollection,			// light table visible, updates when user selects different collections
		DefiningProject,			// light table visible - shows only contents of selected collection
		UpdatingContent,			// editing comments, updating jpegs, etc. 
	} ;

protected: // create from serialization only
	CMakePhotoDisk3View();
	DECLARE_DYNCREATE(CMakePhotoDisk3View)

	ViewState		m_state ;

// Attributes
public:
	CMakePhotoDisk3Doc* GetDocument() const;
	inline ViewState GetCurrentViewState( ) const
	{
		return m_state ;
	} ;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMakePhotoDisk3View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:

// user-defined messages
	afx_msg LRESULT OnNewCatalog( WPARAM wp, LPARAM lp ) ;
	afx_msg	LRESULT OnCollectionSelchange( WPARAM wp, LPARAM lp ) ;
	afx_msg LRESULT OnImageSelchange( WPARAM wp, LPARAM lp ) ;
	afx_msg LRESULT OnGotoDefineProject( WPARAM, LPARAM ) ;
	afx_msg LRESULT OnCreateProject( WPARAM wp, LPARAM lp ) ;

	DECLARE_MESSAGE_MAP()

public:
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
//	afx_msg void OnPaint();
	virtual void OnInitialUpdate();
//	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
//	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
//	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in MakePhotoDisk3View.cpp
inline CMakePhotoDisk3Doc* CMakePhotoDisk3View::GetDocument() const
   { return reinterpret_cast<CMakePhotoDisk3Doc*>(m_pDocument); }
#endif

