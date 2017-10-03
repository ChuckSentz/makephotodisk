#pragma once




#error including obsolete file!!!!!!





// CControlPanel
class CMakePhotoDisk3View  ;

class CControlPanel : public CWnd
{
	DECLARE_DYNAMIC(CControlPanel)

	CString					m_fq_catalog_path ;			// mostly b/c we need to have an object which will stick around while we post message to the view 
	CString					m_fq_project_path ;			// for the same reason above 
	CMakePhotoDisk3View *	m_view ;
	CComboBox				m_catalog_combo ;
	CButton					m_browse_button ;
	CButton					m_next_button ;
	CTreeCtrl				m_collection_tree ;
	CImageList				m_collection_image_list ;

	CComboBox				m_location_combo ;
	CButton					m_browse_dir ;
	CEdit					m_dir_name ;
	CComboBox				m_disk_type_combo ;
	CStatic					m_disk_type_text ;
	CButton					m_create_project ;

	LOGFONT					m_std_font_lf ;
	CFont					m_std_font ;
	CStringArray			m_catalogs ;
	CStringArray			m_directories ;

	void LoadCatalogList( ) ;
	void RecursiveTreeBuild( CTypedPtrList< CPtrList, Collection *> & col_list, HTREEITEM parent_handle ) ;
	void PredictDiskRootDirectory( ) ;
	
public:
	CControlPanel( CMakePhotoDisk3View * view ) ;
	virtual ~CControlPanel();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedCatalogBrowse();
	afx_msg void OnBnClickedDirectoryBrowse( ) ;
	afx_msg void OnBnClickedNext( ) ;
	afx_msg void OnBnClickedCreateProject( ) ;

	void OnInitialUpdate( ) ;			// synthetic MFC function. Called from the view's OnInitialUpdate 
	void OnCbnSelendokCatalogCombo( ) ;
	void OnCBnSelendokDiskTypeCombo( ) ;
	void OnCBnSelendokProjectLocation( ) ;

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);


	// these parallel the view's message 
	void OnNewCatalog( ) ;
	void SelectCollection( Collection * col ) ;
	void DeselectCollection( ) ;
	void OnCreateProject( ) ;


//	void OnCollectionSelchange( WPARAM wp, LPARAM lp ) ;
//	void OnImageSelchange( WPARAM wp, LPARAM lp ) ;

};


