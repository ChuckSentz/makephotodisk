/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	"pseudo-static" class used in the output/properties panel. 
	
	When it's in "interactive" mode, hovering the mouse over this class will cause a 
	hyperlink-style underline to appear. When user clicks on it, the owning window 
	(a form or dialog) is notified and can then display an appropriate dialog box 
	to update the contents.

	When not "interactive", it behaves just like a regular static. 

***************************************************************************************/


#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MyStatic.h"


// CMyStatic
 
IMPLEMENT_DYNAMIC(CMyStatic, CStatic)

CMyStatic::CMyStatic()
{
	m_mouse_over = false ;
	m_first_paint = true ;
	m_captured_mouse = false ;
	m_text_edited = false ;
	m_interactive_mode = false ;
	m_use_bold = false ;
}

CMyStatic::~CMyStatic()
{
}


BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CMyStatic message handlers


/******************************************************************************
CMyStatic::SetInteractiveMode

	Set the interactive mode. When interactive is true, will turn the text
	into a hyperlink when mouse hovers over it. When interactive is false, 
	will just display text like any static control 

******************************************************************************/
void CMyStatic::SetInteractiveMode( bool interactive )
{
	if ( interactive ) 
		ModifyStyle( 0, SS_NOTIFY ) ;
	else
		ModifyStyle( SS_NOTIFY, 0 ) ;

	m_interactive_mode = interactive ;
}




/******************************************************************************
CMyStatic::OnPaint

	Message-mapped handler for WM_PAINT message 

	Paint the control. Creates pens and fonts only on the first paint message. 


******************************************************************************/
void CMyStatic::OnPaint()
{
	CPaintDC	dc(this); // device context for painting
	CFont *		default_font ;
	CRect		my_rect ;
	CRect		text_rect ;
	COLORREF	background_color ;
	COLORREF	std_text_color ;
	COLORREF	hover_text_color ;
	COLORREF	edited_highlight ;

	if ( m_first_paint ) 
		FirstPainting( ) ;

	GetClientRect( &my_rect ) ;

	// FIXME - should we use the default background and text colors, or get them from the parent window?? 
	background_color = GetSysColor( COLOR_3DFACE ) ;
	std_text_color = GetSysColor( COLOR_BTNTEXT ) ; 
	hover_text_color = GetSysColor( COLOR_HOTLIGHT ) ;
	edited_highlight = RGB( 255, 255, 0 ) ;

	if ( m_text_edited ) 
		dc.FillSolidRect( &my_rect, edited_highlight ) ;			
	else
		dc.FillSolidRect( &my_rect, background_color ) ;

	// figure out the font and text color based on whether we've got a mouse over us & whether there have been edits....
	if ( m_interactive_mode && m_mouse_over ) 
	{
		dc.SetTextColor( hover_text_color ) ;

		if ( m_use_bold )
			default_font = dc.SelectObject( &m_bold_hover_font ) ;
		else
			default_font = dc.SelectObject( &m_plain_hover_font ) ;
	}
	else
	{
		dc.SetTextColor( std_text_color ) ;

		if ( m_use_bold ) 
			default_font = dc.SelectObject( &m_bold_font ) ;
		else
			default_font = dc.SelectObject(	&m_plain_font ) ;
	}

	dc.DrawText( m_text, text_rect, DT_CALCRECT | DT_LEFT | DT_SINGLELINE ) ;
	ClientToScreen( text_rect ) ;
	GetParent()->ScreenToClient( text_rect ) ;
	SetWindowPos( NULL, text_rect.left, text_rect.top, text_rect.Width( ), text_rect.Height( ), SWP_NOOWNERZORDER ) ;

	GetClientRect( &my_rect ) ;
	dc.DrawText( m_text, my_rect, DT_LEFT | DT_SINGLELINE ) ;

	dc.SelectObject( default_font ) ;
}





/******************************************************************************
CMyStatic::FirstPainting

	Called from OnPaint when processing the very first WM_PAINT message. 

	just create a bunch of fonts handling the first WM_PAINT message. 

******************************************************************************/
void CMyStatic::FirstPainting( ) 
{
	ASSERT( m_first_paint ) ;
	m_first_paint = false ;

	CFont *		my_font ;
	LOGFONT		log_font ;

	my_font = GetFont( ) ;
	my_font->GetLogFont( &log_font ) ;

	m_plain_font.CreateFontIndirect( &log_font ) ;

	log_font.lfUnderline = TRUE ;
	m_plain_hover_font.CreateFontIndirect( &log_font ) ;

	log_font.lfWeight = FW_BOLD ;
	m_bold_hover_font.CreateFontIndirect( &log_font ) ;
	
	log_font.lfUnderline = FALSE ;
	m_bold_font.CreateFontIndirect( &log_font ) ;
}





/******************************************************************************
CMyStatic::EndHover

	Called when mouse has left the control's rectangle 

******************************************************************************/
void CMyStatic::EndHover( )
{
	if ( m_mouse_over ) 
	{
		m_mouse_over = false ;
		Invalidate( ) ;
	}
}


/******************************************************************************
CMyStatic::BeginHover

	Called when mouse has entered the control's rectangle 

******************************************************************************/
void CMyStatic::BeginHover( )
{
	if ( !m_mouse_over )
	{
		m_mouse_over = true ;	
		Invalidate( ) ;
	}
}






/******************************************************************************
CMyStatic::OnMouseMove

	message-mapped handler for WM_MOUSEMOVE notification 

******************************************************************************/
void CMyStatic::OnMouseMove(
	UINT nFlags,						// I - indicates user key-press 
	CPoint point						// I - client window coords of mouse position 
)
{
	CRect	window_rect ;

	GetWindowRect( &window_rect ) ;
	ScreenToClient( &window_rect ) ;

	if ( window_rect.PtInRect( point ) ) 
	{
		ASSERT( m_captured_mouse ) ;
	}
	else if ( m_captured_mouse ) 
	{
		ReleaseCapture( ) ;
		m_captured_mouse = false ;
		EndHover( ) ;
	}

	CStatic::OnMouseMove(nFlags, point);
}






/******************************************************************************
CMyStatic::OnNcHitTest

	Message-mapped handler for WM_NCHITTEST function 

******************************************************************************/
LRESULT CMyStatic::OnNcHitTest(
	CPoint point						// I - screen coords of mouse pointer 
)
{
	CRect	window_rect ;

	GetWindowRect( &window_rect ) ;

	if ( window_rect.PtInRect( point ) )
	{
		if ( !m_captured_mouse && !GetCapture( ) ) 
		{
			BeginHover( ) ;
			SetCapture( ) ;
			m_captured_mouse = true ;
		}
	}
	else if ( m_captured_mouse ) 
	{
		ReleaseCapture( ) ;
		m_captured_mouse = false ;
		EndHover( ) ;
	}

	return CStatic::OnNcHitTest(point);
}






/******************************************************************************
CMyStatic::OnLButtonUp

	message-mapped handler for WM_LBUTTONUP mouse message. 

	The user has released the left mouse button. Notify parent if it happens 
	while over the control, or ignore it if not over the control 

******************************************************************************/
void CMyStatic::OnLButtonUp(
	UINT nFlags,						// I - info on kbd & mouse keys pressed (eg CTRL, right-button, etc) 
	CPoint point						// I - client coordinates of mouse position, ignored - we use m_mouse_over 
)
{
	if ( m_mouse_over ) 
	{
		// send our user-defined notification message to the parent 
		if ( IDOK == GetParent( )->SendMessage( MSNM_EDIT_CONTENT, GetDlgCtrlID( ), (LPARAM) this ) ) 
		{
			Invalidate( ) ;
		}
	}

	// pass it on down to the base class, too 
	CStatic::OnLButtonUp(nFlags, point);
}






/******************************************************************************
CMyStatic::SetText

	Called by parent to set the text of the CMyStatic control. The "overrides" 
	argument should be true if the user has changed the text from the default
	value. If true, this causes the text to be displayed with a highlight 
	indicating that it has been changed by the user 

******************************************************************************/
void CMyStatic::SetText( 
	const TCHAR * text,					// I - the actual text 
	bool overrides						// I - defaults to false. If true, text displayed with a highlight
)
{
	CRect	parent_window_rect ;
	CRect	my_window_rect ;

	GetParent( )->GetWindowRect( &parent_window_rect ) ;
	GetWindowRect( &my_window_rect ) ;

	SetWindowPos( NULL, my_window_rect.left, my_window_rect.top, parent_window_rect.Width( ), my_window_rect.Height( ), SWP_NOZORDER | SWP_NOMOVE ) ;
	 
	m_text = text ;
	m_text_edited = overrides ;
	Invalidate( TRUE ) ;
}
