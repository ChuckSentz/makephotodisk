/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



The application object. 

***************************************************************************************/

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


#define	MY_COMPANY_REGISTRY_KEY		_T( "CCS Software" ) 
#define MY_PRODUCT_REGISTRY_KEY		_T( "MakePhotoDisk3" ) 

									// this is ASCII. Do NOT enclose in _T()
#define	DB_SIGNATURE				"SQLite format 3"





// CMakePhotoDisk3App:
// See MakePhotoDisk3.cpp for the implementation of this class
//
class CMakePhotoDisk3App : public CWinApp
{
public:
	CMakePhotoDisk3App();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

/*lint -save -e1925 */
	UINT  m_nAppLook;
/*lint -restore */

	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnAppAbout();
	afx_msg void OnUpdateRecentFileMenu(CCmdUI *pCmdUI);
};

extern CMakePhotoDisk3App theApp;


/*
 user defined messages... 
 
	These are for jumping to a different stage in the process of creating a project, and
	will usually result in changing the "control panel" contents.

	Any ordinary event notifications should typically be routed through the document. 
	Add a handler in the document, then use UpdateAllViews

*/
enum 
{
	WM_GOTO_DEFINE_PROJECT = WM_USER,
	WM_CREATE_PROJECT,
	WM_BACKUP_TO_SELECT_COLLECTION,
};


