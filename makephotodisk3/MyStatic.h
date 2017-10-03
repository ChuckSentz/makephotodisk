/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	"pseudo-static" class used in the output/properties panel. 
	
	The CMyStatic class displays a line of text which functions like a hover-over hyperlink.

	To use:

	1. Parent window instantiates a CMyStatic member and uses SubclassDlgItem to 
	   associate it with a control id. 
	2. Parent must handle MSNM_EDIT_CONTENT message from the control. The parent then 
	   displays an appropriate dialog so user can edit the content.
	3. When the user is done editing, and the parent wants to change the text for the 
	   control, the parent should call CMyStatic::SetText() to update the text.

	When it's in "interactive" mode, hovering the mouse over this class will cause a 
	hyperlink-style underline to appear. When user clicks on it, the owning window 
	(a form or dialog) is notified and can then display an appropriate dialog box 
	to update the contents.

	When not "interactive", it behaves just like a regular static. 

***************************************************************************************/


#pragma once

/*****************************************************************]


*****************************************************************/

enum
{
	MSNM_EDIT_CONTENT =		WM_USER,		// notification message sent to parent 


} ;

class CMyStatic : public CStatic
{
	DECLARE_DYNAMIC(CMyStatic)

public:
	CMyStatic();
	virtual ~CMyStatic();

protected:
	DECLARE_MESSAGE_MAP()

	CFont		m_plain_font ;
	CFont		m_bold_font ;
	CFont		m_bold_hover_font ;
	CFont		m_plain_hover_font ;
	bool		m_first_paint ;				// used in OnPaint handler to determine if we need to create fonts 
	bool		m_mouse_over ;
	bool		m_captured_mouse ;
	CString		m_text ;
	bool		m_text_edited ;				// text contains user edits. This means we display the text differently 
	bool		m_interactive_mode ;		// if false, the control is just a regular static 
	bool		m_use_bold ;

	void FirstPainting( ) ;
	void BeginHover( ) ;
	void EndHover( ) ;

public:
//	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	void SetText( const TCHAR * text, bool overrides = false ) ;
	void SetInteractiveMode( bool interactive = true ) ;
	void SetBoldFont( bool bold_face = true ) ;
};


