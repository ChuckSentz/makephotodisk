/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Dialog box for editing title and description
***************************************************************************************/

#pragma once

#include "MakePhotoDisk3Doc.h"


// CPhotoTitleDescrEditDlg dialog

class CPhotoTitleDescrEditDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPhotoTitleDescrEditDlg)

public:
	CPhotoTitleDescrEditDlg(PhotoImage * image, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPhotoTitleDescrEditDlg();

// Dialog Data
	enum { IDD = IDD_PHOTO_TITLE_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	PhotoImage		m_image ;
	PhotoImage *	m_original_image ;

	void UpdateImageCopy( ) ;

public:
	BOOL m_override_title;
	BOOL m_override_description;
	CEdit m_title_edit_ctrl;
	CString m_title_string;
	CEdit m_description_edit_ctrl;
	CString m_description_string;
	afx_msg void OnBnClickedCustomTitleCheck();
	afx_msg void OnBnClickedCustomDescrCheck();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnKillfocusTitleEdit();
	afx_msg void OnEnKillfocusDescrEdit();
	virtual BOOL OnInitDialog();
	CString m_preview_string;
};
