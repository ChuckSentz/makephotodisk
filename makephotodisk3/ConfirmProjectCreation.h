/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.





CConfirmProjectCreation - confirms all project parameters before creating the 
directories and files.
 
***************************************************************************************/
#pragma once


// CConfirmProjectCreation dialog

class CConfirmProjectCreation : public CDialog
{
	DECLARE_DYNAMIC(CConfirmProjectCreation)

public:
	CConfirmProjectCreation(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfirmProjectCreation();

// Dialog Data
	enum { IDD = IDD_CONFIRM_PROJECT_CREATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedOk();
//	virtual BOOL OnInitDialog();
	CString m_catalog_name;
	CString m_collection_name;
	CString m_directory_name;
	CString m_disk_type;
	CString m_photo_count;
};
