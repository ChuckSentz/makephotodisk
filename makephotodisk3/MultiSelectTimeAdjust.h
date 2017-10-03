/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
#pragma once



// CMultiSelectTimeAdjust dialog

class CMakePhotoDisk3Doc ;

/*lint -save -e1925		this is a MFC-style dialog class, and has public data members, Scott Meyers notwithstanding */
class CMultiSelectTimeAdjust : public CDialogEx
{
	DECLARE_DYNAMIC(CMultiSelectTimeAdjust)

public:
	CMultiSelectTimeAdjust(CMakePhotoDisk3Doc * pDoc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiSelectTimeAdjust();

// Dialog Data
	enum { IDD = IDD_PHOTO_TIME_MULTI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CMakePhotoDisk3Doc *		m_document ;

public:
	BOOL	m_set_adjust_time;
	CEdit	m_adjust_time_edit;
	CString m_adjust_time_string;
	INT32	m_adjust_seconds ;

	afx_msg void OnBnClickedAdjustTimeCheck();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
/*lint -restore */
