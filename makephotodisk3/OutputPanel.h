/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



The output panel, which displays either information on the selected collection, the 
selected image, or the project. 

***************************************************************************************/
#pragma once

#include "MyStatic.h"


// COutputPanel
class PhotoImage ;
class Collection ;

class COutputPanel : public CFormView
{
	DECLARE_DYNCREATE(COutputPanel)

public:
	enum { IDD = IDD_EDIT_OUTPUT_FORM } ;

	COutputPanel( );
	virtual ~COutputPanel();

	CMakePhotoDisk3Doc * GetDocument( ) const ;	// non-debug version is inline (see below)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void ScrollToTopOfForm( ) ;

public:
	afx_msg void OnPaint();

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) ;
	virtual void OnInitialUpdate();
	afx_msg void OnStnClickedTitleAndDescription();
	afx_msg void OnStnClickedCreatedWithCameraOnDate();
	afx_msg void OnStnClickedExposureData();
	afx_msg void OnBnClickedEnableExtendedComment();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

protected:
	CMyStatic m_title_subtitle ;
	CMyStatic m_creation_static ;
	CStatic m_exposure_static;

	BOOL m_enable_extended_comment;
	CEdit m_comment_edit_box;
	CButton m_enable_extended_checkbox;
	CString m_comment;
};


#ifndef _DEBUG  // debug version in MakePhotoDisk3View.cpp
inline CMakePhotoDisk3Doc* COutputPanel::GetDocument() const
   { return static_cast<CMakePhotoDisk3Doc*>(m_pDocument); }
#endif

