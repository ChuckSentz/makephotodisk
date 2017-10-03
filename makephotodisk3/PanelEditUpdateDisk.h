/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



Control panel view for the bulk of editing a project. The user can update the project's
jpegs, edit html, etc. 

This panel is the first one a user will see after loading an existing project, and the 
3rd one after the user has opted to create a new project. 

***************************************************************************************/
#pragma once




using namespace Gdiplus;



// CPanelEditUpdateDisk form view

class CPanelEditUpdateDisk : public CFormView
{
	DECLARE_DYNCREATE(CPanelEditUpdateDisk)

	enum {	MyTimerID = 'euti' } ;


protected:
	CPanelEditUpdateDisk();           // protected constructor used by dynamic creation
	virtual ~CPanelEditUpdateDisk();

	CEdit						m_disk_title_edit;
	CEdit						m_revision_edit;
	CStatic						m_disk_type_text;
	Image *						m_disk_icon_image ;
	Image *						m_key_pic_image ;
	FILETIME					m_disk_icon_time ;
	FILETIME					m_key_pic_time ;

public:
	enum { IDD = IDD_UPDATE_PROJECT };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void CheckUpdateDiskUsageText( ) ;
	void CheckAndLoadPreviewImage( const TCHAR * jpeg_name, Image * & image_ptr, FILETIME & write_time ) ;
	void UpdateStartPageFields( const TCHAR * start_page_path ) ;

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedUdpateFiles();

	CMakePhotoDisk3Doc * GetDocument( ) const ;	// non-debug version is inline

	afx_msg void OnBnClickedEditStartPage();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedLargePresentation();
	afx_msg void OnBnClickedCompressed();
	afx_msg void OnBnClickedFacebook();
	afx_msg void OnBnClickedFullSize();
	afx_msg void OnBnClickedWatermarkPresentation();
	afx_msg void OnBnClickedWatermarkCompressed();
	afx_msg void OnBnClickedWatermarkFacebook();
	afx_msg void OnBnClickedWatermarkFullSize();
};


#ifndef _DEBUG  // debug version in MakePhotoDisk3View.cpp
inline CMakePhotoDisk3Doc* CPanelEditUpdateDisk::GetDocument() const
   { return static_cast<CMakePhotoDisk3Doc*>(m_pDocument); }
#endif
