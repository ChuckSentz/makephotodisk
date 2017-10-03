/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



***************************************************************************************/
// WelcomeView.cpp : implementation file
//

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "WelcomeView.h"


// CWelcomeView

IMPLEMENT_DYNCREATE(CWelcomeView, CView)

CWelcomeView::CWelcomeView()
{

}

CWelcomeView::~CWelcomeView()
{
}

BEGIN_MESSAGE_MAP(CWelcomeView, CView)
END_MESSAGE_MAP()


// CWelcomeView drawing

void CWelcomeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CWelcomeView diagnostics

#ifdef _DEBUG
void CWelcomeView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CWelcomeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CWelcomeView message handlers
