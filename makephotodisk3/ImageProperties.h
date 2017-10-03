/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




***************************************************************************************/

#pragma once

#include "DocRecords.h"

// CImageProperties dialog

class CImageProperties : public CDialogEx
{
	DECLARE_DYNAMIC(CImageProperties)

public:
	CImageProperties( const PhotoImage * image, CWnd* pParent = NULL);   // standard constructor
	virtual ~CImageProperties();

// Dialog Data
	enum { IDD = IDD_IMAGE_PROPERTIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	const PhotoImage *		m_image ; 

	DECLARE_MESSAGE_MAP()
	CEdit m_file_name;
	CEdit m_path_name;

	void DisplayTimeField( FILETIME & ft, int control_ID ) ;

public:
	virtual BOOL OnInitDialog();
};
