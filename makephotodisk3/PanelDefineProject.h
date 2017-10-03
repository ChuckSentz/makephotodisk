/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



One of the views used in the left-hand "control panel" when defining a new project. After
the user has selected a catalog and a collection, this panel allows him to select a 
location where the disk files will be assembled, the disk type, etc.  

***************************************************************************************/
#pragma once



#define		MAX_MRU_LOCATION_LIST_SIZE		 8


// CPanelDefineProject form view
class CMakePhotoDisk3Doc ;

class CPanelDefineProject : public CFormView
{
	DECLARE_DYNCREATE(CPanelDefineProject)

protected:
	CPanelDefineProject();           // protected constructor used by dynamic creation
	virtual ~CPanelDefineProject();

public:
	enum { IDD = IDD_DEFINE_PROJECT };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void SaveMRULocationList( ) ;
	void LoadMRULocationList( ) ;
	bool PassSourceFilesCheck( ) ;

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

	CMakePhotoDisk3Doc * GetDocument( ) const ;	// non-debug version is inline


protected:
	CComboBox	m_location_combo;
	CEdit		m_dir_name_edit;
	CComboBox	m_disk_type_combo;
	CButton		m_create_project_button;

	void InitializeDiskTypeCombo( ) ;
	void PredictDiskRootDirectory( ) ;

public:
	afx_msg void OnBnClickedBrowseDirectory();
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedCreateProject();
	afx_msg void OnCbnSelendokProjectTypeCombo();
	afx_msg void OnCbnSelendokProjectLocation();
//	afx_msg void OnEnChangeDirectoryName();
	CStatic m_disk_type_text;
//	afx_msg void OnEnKillfocusDirectoryName();
};


#ifndef _DEBUG  // debug version in MakePhotoDisk3View.cpp
inline CMakePhotoDisk3Doc* CPanelDefineProject::GetDocument() const
   { return static_cast<CMakePhotoDisk3Doc*>(m_pDocument); }
#endif
