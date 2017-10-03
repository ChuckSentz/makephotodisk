/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



The "light table" simulation, our main view into the contents of a collection. 

Also contains wrapper class so we can load PNG's from our compiled resources.

***************************************************************************************/
#pragma once

#include "GdiPlusBitmap.h"

// CLightTable
class CMakePhotoDisk3View ;
class Collection ;

class CLightTable : public CView
{
	DECLARE_DYNCREATE(CLightTable)

public: 

protected:
	ULONG			m_gdiplus_token ;

	int				m_current_start_pic ;		// position in current collection's image list of the top-left pic - for scroll-bar
	int				m_cell_size ;				// size of an individual cell
	int				m_rows ;					// number of whole rows visible 
	int				m_columns ;					// number of columns visible

protected:
	void UpdateScrollBar( ) ;
	void CalculateColumnsAndRows( int cx, int cy ) ;
	void InvalidateCell( int pic_number ) ;
	void InvalidateCellRange( int cell_1, int cell_2 ) ;

	void EnsureSelectPicVisible( int old_select_pic ) ;
	inline virtual void OnDraw(CDC* ) 
	{
		// required of a View - but we only do WM_PAINT processing 
	} ;
	int ClickSelectCell( CPoint point ) ;
	void ConfirmCopyKeyFile( const TCHAR * filename ) ;
	bool ConfirmReplaceKeyFile( const TCHAR * target_file ) ;

public:
	CLightTable( );
	virtual ~CLightTable();

	CMakePhotoDisk3Doc * GetDocument( ) const ;	// non-debug version is inline (see below)

protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnPaint();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

//	void OnNewCatalog( ) ;
//	void OnCreateProject( ) ;
//	void CollectionSelchange( ) ;
//	void SelectCollection( const Collection * col ) ;
//	void DeselectCollection( ) ;

	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnEditSource();
	afx_msg void OnShowInExplorer();
	afx_msg void OnSelectKeyImage();
	afx_msg void OnSelectDiskIcon();
	afx_msg void OnSetTitleDescription( ) ;
	afx_msg void OnSetTimeAdjust( ) ;
	afx_msg void OnSetPhotogCamera( ) ;
	afx_msg void OnEditComment( ) ;

	afx_msg void OnUpdateSelectDiskIcon(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectKeyImage(CCmdUI *pCmdUI);
};


#ifndef _DEBUG  // debug version in MakePhotoDisk3View.cpp
inline CMakePhotoDisk3Doc* CLightTable::GetDocument() const
   { return static_cast<CMakePhotoDisk3Doc*>(m_pDocument); }
#endif





