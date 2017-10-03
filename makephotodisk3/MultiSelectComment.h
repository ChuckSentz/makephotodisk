/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/

#pragma once


// CMultiSelectComment dialog

class CMakePhotoDisk3Doc ;

/*lint -save -e1925		this is a MFC-style dialog class, and has public data members, Scott Meyers notwithstanding */
class CMultiSelectComment : public CDialogEx
{
	DECLARE_DYNAMIC(CMultiSelectComment)

	CMakePhotoDisk3Doc *	m_document ;

public:
	CMultiSelectComment(CMakePhotoDisk3Doc * pDoc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiSelectComment();

// Dialog Data
	enum { IDD = IDD_PHOTO_COMMENT_MULTI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_add_comment;
	CEdit m_comment_edit;
	CString m_comment;
	afx_msg void OnBnClickedAddComment();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
/*lint -restore */
