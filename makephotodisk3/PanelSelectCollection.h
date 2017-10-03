/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


The first panel seen after user has opted to create a new project. User selects catalog
and collection. 

***************************************************************************************/
#pragma once




// CPanelSelectCollection form view

class Collection ;
class CMakePhotoDisk3Doc ;

class CPanelSelectCollection : public CFormView
{
	DECLARE_DYNCREATE(CPanelSelectCollection)

protected:
	CPanelSelectCollection();           // protected constructor used by dynamic creation
	virtual ~CPanelSelectCollection();

	void LoadMRUCatalogList( ) ;
	void SaveMRUCatalogList( ) ;
	void RecursiveTreeBuild( CTypedPtrList< CPtrList, Collection *> & col_list, HTREEITEM parent_handle ) ;

	CImageList		m_collection_image_list ;	// image list for collection tree - 

public:
	enum { IDD = IDD_SELECT_CATALOG_COLLECTION };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNextStep();
	afx_msg void OnBnClickedCatalogBrowse();
//	afx_msg void OnCbnSelchangeCatalogCombo();
	afx_msg void OnCbnSelendokCatalogCombo();
	afx_msg void OnTvnSelchangingCollectionTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedCollectionTree(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void OnInitialUpdate();


	CMakePhotoDisk3Doc * GetDocument( ) const ;	// non-debug version is inline

protected:
	// controls I need to reference 
	CComboBox	m_catalog_combo;
	CTreeCtrl	m_collection_tree;
	CButton		m_next_button;
public:
	afx_msg void OnDestroy();
};


#ifndef _DEBUG  // debug version in MakePhotoDisk3View.cpp
inline CMakePhotoDisk3Doc* CPanelSelectCollection::GetDocument() const
   { return reinterpret_cast<CMakePhotoDisk3Doc*>(m_pDocument); }
#endif


