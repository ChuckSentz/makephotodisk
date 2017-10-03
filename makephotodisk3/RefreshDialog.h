/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Dialog displayed while worker threads are creating new jpegs and html for the project

***************************************************************************************/
#pragma once

#include "afxcmn.h"
#include "MakePhotoDisk3Doc.h"
#include "JpegUpdateWorker.h"

// CRefreshDialog dialog

class CRefreshDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CRefreshDialog)

	enum {	StatusTimerID = 'rdlg' } ;

	JpegUpdateWorker		m_jpeg_update_worker ;
	int						m_refresh_file_ct ;
	CMakePhotoDisk3Doc *	m_doc ;
	UINT_PTR				m_timer_ID ;
	int						m_bar_pos ;
	PhotoImage *			m_current_photo ;
	int						m_status_image_resource_id ;

public:
	CRefreshDialog( CMakePhotoDisk3Doc * doc, int refresh_file_ct, CWnd* pParent = NULL);   // standard constructor
	virtual ~CRefreshDialog();

// Dialog Data
	enum { IDD = IDD_REFRESH_CONTENT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CEdit m_current_file;
	CProgressCtrl m_progress_bar;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCancel();
//	virtual INT_PTR DoModal();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
};
