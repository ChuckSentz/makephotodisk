/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
#pragma once



// CMultiSelectPhotoTitleDescr dialog
class CMakePhotoDisk3Doc ;

/*lint -save -e1925		this is a MFC-style dialog class, and has public data members, Scott Meyers notwithstanding */
class CMultiSelectPhotoTitleDescr : public CDialogEx
{
	DECLARE_DYNAMIC(CMultiSelectPhotoTitleDescr)

	CMakePhotoDisk3Doc *	m_document ;

public:
	CMultiSelectPhotoTitleDescr(CMakePhotoDisk3Doc * pDoc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiSelectPhotoTitleDescr();

// Dialog Data
	enum { IDD = IDD_PHOTO_TITLE_MULTI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CEdit m_title_edit_control;
	CString m_title;
	CEdit m_descr_edit_control;
	CString m_descr;
	BOOL m_set_title;
	BOOL m_set_descr;
	afx_msg void OnBnClickedCustomTitleCheck();
	afx_msg void OnBnClickedCustomDescrCheck();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
/*lint -restore */
