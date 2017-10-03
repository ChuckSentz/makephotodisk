/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


***************************************************************************************/
#pragma once



// CMultiSelectPhotogCamera dialog

class CMakePhotoDisk3Doc ;


/*lint -save -e1925		this is a MFC-style dialog class, and has public data members, Scott Meyers notwithstanding */
class CMultiSelectPhotogCamera : public CDialogEx
{
	DECLARE_DYNAMIC(CMultiSelectPhotogCamera)

	CMakePhotoDisk3Doc *	m_document ;

public:
	CMultiSelectPhotogCamera(CMakePhotoDisk3Doc * pDoc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiSelectPhotogCamera();

// Dialog Data
	enum { IDD = IDD_PHOTOCAM_MULTI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_override_photog;
	CEdit m_photog_edit;
	CString m_photographer;
	BOOL m_override_camera;
	CEdit m_camera_edit;
	CString m_camera;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCustomPhotogCheck();
	afx_msg void OnBnClickedCustomCameraCheck();
	afx_msg void OnBnClickedOk();
};
/*lint -restore */
