/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Dialog box for creation time, photog credit, camera
***************************************************************************************/

#pragma once
#include "afxdtctl.h"

#include "MakePhotoDisk3Doc.h"
#include "atlcomtime.h"


// CPhotoCaptureTimeEditDlg dialog

class CPhotoCaptureTimeEditDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPhotoCaptureTimeEditDlg)

public:
	CPhotoCaptureTimeEditDlg(PhotoImage * original, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPhotoCaptureTimeEditDlg();

// Dialog Data
	enum { IDD = IDD_PHOTO_TIME_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

protected:
	PhotoImage									m_image ;
	PhotoImage *								m_original_image ;
	CDateTimeCtrl								m_date_time_picker ;
	CEdit										m_camera_name_edit ;
	CEdit										m_photo_credit_edit ;

public:
	virtual BOOL OnInitDialog();

protected:
	BOOL			m_override_create_time ;
	BOOL			m_adjust_time ;
	BOOL			m_override_camera_ID ;
	BOOL			m_override_photog ;
	COleDateTime	m_create_time ;
	CString			m_camera_ID ;
	CString			m_photog ;
	CEdit			m_adjust_time_edit ;
	CString			m_adjust_time_string ;

	void UpdateOverrideTimes( ) ;
	void UpdateImageCopy( ) ;

public:
	afx_msg void OnBnClickedOverrideCreateTime();
	afx_msg void OnBnClickedAdjustTimeCheck();
	afx_msg void OnBnClickedEditCameraName();
	afx_msg void OnBnClickedPhotographerCreditCheck();
	virtual void OnOK();
	CString m_preview_string;
	afx_msg void OnNMKillfocusCreateDateTime(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnKillfocusAdjustTime();
	afx_msg void OnEnKillfocusPhotographerEdit();
	afx_msg void OnEnKillfocusCameraNameEdit();
} ;
