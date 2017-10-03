/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	LightTable is a view class for the upper-right pane of the mainframe, which presents
	a view (modeled after the main Lightroom view) of the images in the collection. 

***************************************************************************************/

#include "stdafx.h"
#include "MakePhotoDisk3.h"
#include "MakePhotoDisk3Doc.h"
#include "LightTable.h"
#include "ImageProperties.h"
#include "MultiSelectPhotoTitleDescr.h"
#include "MultiSelectTimeAdjust.h"
#include "MultiSelectPhotogCamera.h"
#include "MultiSelectComment.h"


// CLightTable

IMPLEMENT_DYNCREATE(CLightTable, CView)

using namespace Gdiplus;


static void TagCell( CRect cell, int tag_position, Graphics & graphics, CGdiPlusBitmapResource & tag ) ;

void LaunchPhotoshopOnFile( const TCHAR * source_file ) ;



BEGIN_MESSAGE_MAP(CLightTable, CView)
//	ON_WM_PAINT()
	ON_WM_PAINT()
	ON_WM_SIZE()

	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEHWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONUP()

	ON_COMMAND( ID__SETTITLEDESCRIPTION, &CLightTable::OnSetTitleDescription )
	ON_COMMAND( ID__SETTIMEADJUST, &CLightTable::OnSetTimeAdjust ) 
	ON_COMMAND( ID__SETPHOTOGANDCAMERA, &CLightTable::OnSetPhotogCamera ) 
	ON_COMMAND( ID__EDITCOMMENT, &CLightTable::OnEditComment ) 
	ON_COMMAND(ID_PROPERTIES, &CLightTable::OnProperties)
	ON_COMMAND(ID_EDIT_SOURCE, &CLightTable::OnEditSource)
	ON_COMMAND(ID_SHOW_IN_EXPLORER, &CLightTable::OnShowInExplorer)
	ON_COMMAND(ID_SELECT_KEY_IMAGE, &CLightTable::OnSelectKeyImage)
	ON_COMMAND(ID_SELECT_DISK_ICON, &CLightTable::OnSelectDiskIcon)
	ON_UPDATE_COMMAND_UI(ID_SELECT_DISK_ICON, &CLightTable::OnUpdateSelectDiskIcon)
	ON_UPDATE_COMMAND_UI(ID_SELECT_KEY_IMAGE, &CLightTable::OnUpdateSelectKeyImage)
END_MESSAGE_MAP()








/*******************************************************************************
CLightTable::CLightTable

	Constructor. Inits member variables, of course, but also calls GdiplusStartup, 
	since we use GDI+ to draw thumbnails of the images in the collection 

*******************************************************************************/
CLightTable::CLightTable( )
{
	GdiplusStartupInput	startup_input ;

	startup_input.GdiplusVersion = 1 ;
	startup_input.DebugEventCallback = NULL ;
	startup_input.SuppressBackgroundThread = FALSE ;
	startup_input.SuppressExternalCodecs = TRUE ;

	GdiplusStartup( &m_gdiplus_token, &startup_input, NULL ) ;

	m_current_start_pic = 0 ;		// position in current collection's image list of the top-left pic - for scroll-bar
	m_cell_size = 0 ;				// size of an individual cell
	m_rows = 0 ;					// number of whole rows visible 
	m_columns = 0 ;					// number of columns visible
}






/*******************************************************************************
CLightTable::CLightTable

	Just shut down GDI+

*******************************************************************************/
CLightTable::~CLightTable()
{
	GdiplusShutdown( m_gdiplus_token ) ;
}






/*******************************************************************************
CLightTable::GetDocument( ) 

	Just return the m_pDocument pointer, cast to our document class. 

*******************************************************************************/
CMakePhotoDisk3Doc* CLightTable::GetDocument( ) const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMakePhotoDisk3Doc)));
	return (CMakePhotoDisk3Doc*)m_pDocument;
}







/******************************************************************************
CLightTable::UpdateScrollBar

	Helper function which makes sure the scalable thumbtack thing on the scroll bar
	is the right size and in the right position 

******************************************************************************/
void CLightTable::UpdateScrollBar( )
{
	SCROLLINFO	scroll_info ;

	scroll_info.cbSize = sizeof( SCROLLINFO ) ;
	scroll_info.fMask = SIF_PAGE | SIF_POS | SIF_RANGE ;
	scroll_info.nMin = 0 ;
	scroll_info.nMax = GetDocument( )->GetPhotoListSize( ) ;
	scroll_info.nPage = static_cast< UINT >( m_rows * m_columns ) ;
	scroll_info.nPos = m_current_start_pic ;

	SetScrollInfo( SB_VERT, &scroll_info, TRUE ) ;
}





#define	MAX_CELL_WIDTH	200
#define	MIN_CELL_WIDTH	120

/******************************************************************************
CLightTable::CalculateColumnsAndRows

	Helper to call whenever view is re-sized to determine how many columns we
	should try to show, then the cell size based on that, then the number of 
	rows based on that and the height of the window. 

*******************************************************************************/
void CLightTable::CalculateColumnsAndRows( int cx, int cy )
{
	m_columns = 5 ;

		// make sure we have enough columns so the cells are small enough.... 
	while ( cx / m_columns > MAX_CELL_WIDTH && cx / ( m_columns + 1 ) >= MIN_CELL_WIDTH ) 
		m_columns++ ;

		// make sure we don't have too many columns, resulting in cells which are too small... 
	while ( cx / m_columns < MIN_CELL_WIDTH && cx / ( m_columns - 1 ) <= MAX_CELL_WIDTH ) 
		m_columns-- ;

		// cell size is the width divided by the number of columns, and the rows = height / cell size 
	if ( m_cell_size = cx / m_columns )
		m_rows = cy / m_cell_size ;
	else
		m_rows = 1 ;
}





/*******************************************************************************
CLightTable::OnVScroll

	Message handler for scroll bar messages 

*******************************************************************************/
void CLightTable::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int	new_start ;

	switch ( nSBCode ) 
	{
		case SB_TOP : 
			break ;

		case SB_BOTTOM :		// not sure when either of these is sent 
			break ;

		case SB_LINEDOWN :		// user clicked the down-arrow at the bottom 
			// add num_columns to m_current_start_pic iff. there's a partial row or hidden rows
			if ( m_current_start_pic + m_rows * m_columns < GetDocument( )->GetPhotoListSize( ) )
			{
				m_current_start_pic += m_columns ;

				UpdateScrollBar( ) ;

				/* FIXME - we should be doing smarter painting, with BitBlt or something... */ 
				Invalidate( ) ;
			}
			break ;

		case SB_LINEUP :		// user clicked the up-arrow at the top
			if ( m_current_start_pic > 0 )
			{
				m_current_start_pic -= m_columns ;

				// m_current_start_pic could conceivably end up less than zero if the user scrolls down, 
				// then changes the window size (which changes the number of columns) then scrolls up to the top...
				if ( m_current_start_pic < 0 ) 
					m_current_start_pic = 0 ;

				UpdateScrollBar( ) ;

				/* FIXME - we should be doing smarter painting, with BitBlt or something... */ 
				Invalidate( ) ;
			}
			break ;

		case SB_PAGEDOWN :		// user clicked the bar between the thumb-tack and the bottom 
			if ( m_current_start_pic + m_rows * m_columns < GetDocument( )->GetPhotoListSize( ) )
			{
				m_current_start_pic += m_rows * m_columns ;
				if ( m_current_start_pic + m_rows * m_columns > GetDocument( )->GetPhotoListSize( ) )
					m_current_start_pic = GetDocument( )->GetPhotoListSize( ) - m_rows * m_columns ;

				UpdateScrollBar( ) ;

				/* FIXME - we should be doing smarter painting, with BitBlt or something... */ 
				Invalidate( ) ;
			}
			break ;
				
		case SB_PAGEUP :		// user clicked the bar betwen the tack and the top
			if ( m_current_start_pic > 0 ) 
			{
				m_current_start_pic -= m_columns * m_rows ;
				if ( m_current_start_pic < 0 ) 
					m_current_start_pic = 0 ;

				UpdateScrollBar( ) ;

				/* FIXME - we should be doing smarter painting, with BitBlt or something... */ 
				Invalidate( ) ;
			}
			break ;

		case SB_THUMBTRACK :	// user clicked the thumb and is dragging it... 
			new_start = static_cast< int >( nPos ) ;
			if ( new_start != m_current_start_pic ) 
			{
				if ( new_start > m_current_start_pic && new_start - m_current_start_pic >= m_columns 
					 || new_start < m_current_start_pic ) 
				{
					m_current_start_pic = new_start ;

					UpdateScrollBar( ) ;
					Invalidate( ) ;
				}
			}
			break ;

		case SB_THUMBPOSITION :	// user has finished dragging the thumb
		case SB_ENDSCROLL : 
		default : 
			break ;
	}

	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}







/*******************************************************************************
CLightTable::OnPaint

	Paint the grid of cells. The user sees a "contact-sheet" like view of the
	collection modeled after the Lightroom grid display 

*******************************************************************************/
void CLightTable::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if ( GetDocument( ) )
	{
		PhotoImage * photo ;
		POSITION	pos ;
		CRect		client_rect ;
		int			row ;
		int			col ;
		CPen *		dc_pen ;
		CPen		dark_pen ;
		Graphics	graphics( dc.GetSafeHdc( ) ) ;
		CFont *		dc_font ;
		CFont		num_caption_font ;
		int			image_no ;
		CGdiPlusBitmapResource	stop_tag ;
		CGdiPlusBitmapResource	caution_tag ;
		CGdiPlusBitmapResource	note_tag ;
		CGdiPlusBitmapResource	no_preview_image ;
		CGdiPlusBitmapResource	outdated_jpeg_tag ;
		CGdiPlusBitmapResource	renumber_needed_tag ;
		Image *		preview_jpeg = NULL ;

		stop_tag.Load( IDB_STOP_TAG, _T( "PNG" ) ) ;
		caution_tag.Load( IDB_CAUTION_TAG, _T( "PNG" ) ) ;
		note_tag.Load( IDB_NOTE_TAG, _T( "PNG" ) ) ;
		no_preview_image.Load( IDB_NO_PREVIEW, _T( "PNG" ) ) ;
		outdated_jpeg_tag.Load( IDB_OUTDATED_JPEG, _T( "PNG" ) ) ;
		renumber_needed_tag.Load( IDB_UPDATE_NUMBER, _T( "PNG" ) ) ;

		dark_pen.CreatePen( PS_SOLID, 1, RGB( 60, 60, 60 ) ) ;

		num_caption_font.CreateFont( 45, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, ANTIALIASED_QUALITY, FF_MODERN, _T( "Arial Black" ) ) ;

		GetClientRect( &client_rect ) ;
		dc.FillSolidRect( &client_rect, RGB( 80, 80, 80 ) ) ;	// fill the background
		dc_pen = dc.SelectObject( &dark_pen ) ;
		dc_font = dc.SelectObject( &num_caption_font ) ;
		dc.SetTextColor( RGB( 148, 148, 148 ) ) ;

		if ( m_current_start_pic > 0 )
			photo = GetDocument( )->FindPhoto( m_current_start_pic, pos ) ;
		else
			photo = GetDocument( )->GetFirstPhoto( pos ) ;

		for ( col = 0, row = 0, image_no = m_current_start_pic ; NULL != photo ; photo = GetDocument( )->GetNextPhoto( pos ), image_no++ )
		{
			CRect	cell ;
			int		img_height ;
			int		img_width ;
			Rect	dest_rect ;
			CRect	dest_cell ;
			CString	caption ;
			bool	no_preview = false ;

			cell.top = row * m_cell_size ; cell.bottom = cell.top + m_cell_size - 1 ;
			cell.left = col * m_cell_size ; cell.right = cell.left + m_cell_size - 1 ;

			// paint the face with 50% grey 
			if ( photo->GetSelectedFlag( ) )
				dc.FillSolidRect( &cell, RGB( 168, 168, 168 ) ) ;
			else
				dc.FillSolidRect(  &cell, RGB( 128, 128, 128 ) ) ;

			// output the text with the image number next
			caption.Format( _T( "%d" ), 1 + image_no ) ;

			dc.DrawText( caption, &cell, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX ) ;

			dc.MoveTo( cell.left, cell.bottom ) ;
			dc.LineTo( cell.left, cell.top ) ;
			dc.LineTo( cell.right, cell.top ) ;

			dc.SelectStockObject( BLACK_PEN ) ;
			dc.LineTo( cell.right, cell.bottom ) ;
			dc.LineTo( cell.left, cell.bottom ) ;

			if ( _tcslen( photo->GetPreviewJpeg( ) ) )
			{
				preview_jpeg = Image::FromFile( photo->GetPreviewJpeg( ) ) ;

				// fix orientation for veritcal raw files - 
				switch ( photo->GetOrientation( ) )
				{
					case 90 :
						// image in jpeg is rotated 90 degrees CW... so rotate 90 degrees CCW
						preview_jpeg->RotateFlip( Rotate270FlipNone ) ;
						break ;

					case -90 :
						// image in jpeg is rotated 90 degrees CCW... so rotate 90 degrees CW
						preview_jpeg->RotateFlip( Rotate90FlipNone ) ;
						break ;

					default :
						// do nothing
						break ;
				}

				img_height = preview_jpeg->GetHeight( ) ;
				img_width = preview_jpeg->GetWidth( ) ;
			}
			else
			{
				no_preview = true ;
				img_height = 100 ;
				img_width = 150 ;
			}

			/* 
				with the ideal image size, the wider dimension of the the 
				image should be just about 75% of the cell size.

			*/
			if ( img_height >= img_width ) 
			{
				dest_rect.Height = (int) ( 0.75 * m_cell_size ) ;
				dest_rect.Y = cell.top + ( m_cell_size - dest_rect.Height ) / 2 ;

				int new_width = (int) ( dest_rect.Height * ( (double) img_width ) / (double) img_height ) ;

				dest_rect.X = cell.left + ( m_cell_size - new_width ) / 2 ;
				dest_rect.Width = new_width ;
			}
			else
			{
				dest_rect.Width = (int) (0.75 * m_cell_size ) ;
				dest_rect.X = cell.left + ( m_cell_size - dest_rect.Width ) / 2 ;

				int new_height = (int) ( dest_rect.Width * ( (double) img_height ) / (double) img_width ) ;

				dest_rect.Y = cell.top + ( m_cell_size - new_height ) / 2 ;
				dest_rect.Height = new_height ;
			}

			if ( no_preview )
				graphics.DrawImage( (Bitmap *) no_preview_image, dest_rect ) ;
			else
				graphics.DrawImage( preview_jpeg, dest_rect ) ;
			// At this point, we are done with preview_jpeg... but evidently, the gdiplus does some kind of smart-pointer cleanup so we don't need to delete it

			dest_cell.top = dest_rect.Y ;
			dest_cell.left = dest_rect.X ;
			dest_cell.bottom = dest_rect.Y + dest_rect.Height ;
			dest_cell.right = dest_rect.X + dest_rect.Width ;

			dest_cell.InflateRect( 1, 1, 1, 1 ) ;
			dc.SelectStockObject( BLACK_PEN ) ;
			dc.MoveTo( dest_cell.left, dest_cell.bottom ) ;
			dc.LineTo( dest_cell.left, dest_cell.top ) ;
			dc.LineTo( dest_cell.right, dest_cell.top ) ;

			dc.SelectStockObject( WHITE_PEN ) ;
			dc.MoveTo( dest_cell.left, dest_cell.bottom ) ;
			dc.LineTo( dest_cell.right, dest_cell.bottom ) ;
			dc.LineTo( dest_cell.right, dest_cell.top ) ;


			// Now draw any tags we need to add 
			if ( photo->UnsupportedFileFormat( ) )
			{
				TagCell( cell, 8, graphics, stop_tag ) ;
			}
			else 
			{
				switch ( GetDocument( )->GetDocState( ) )
				{
					case CMakePhotoDisk3Doc::CatalogCollectionProjectKnown :
						if ( photo->SourceIsMissing( ) )
						{
							TagCell( cell, 8, graphics, stop_tag ) ;
							break ;
						}
						// else keep going... 

						// show the note thing? 
						if ( photo->HasEdits( ) ) 
						{
							TagCell( cell, 6, graphics, note_tag ) ;
							// do NOT break 
						}

						// outdated? 
						if ( photo->JpegRefreshNeeded( GetDocument( ) ) )
						{
							TagCell( cell, 2, graphics, outdated_jpeg_tag ) ;
						}
						else if ( photo->JpegRenumberNeeded( ) )
						{
							TagCell( cell, 2, graphics, renumber_needed_tag ) ;
						}
						// may fall through, because we may still have a problem 

					case CMakePhotoDisk3Doc::CatalogCollectionKnown :
						if ( photo->IsRejected( ) )
						{
							TagCell( cell, 8, graphics, caution_tag ) ;
						}

						// show the note thing? 
						if ( photo->HasEdits( ) ) 
						{
							TagCell( cell, 6, graphics, note_tag ) ;
						}
						break ;

					default :
						ASSERT( FALSE ) ;
						break ;
				}
			}


			// should we go down a row? 
			if ( ( col + 1 ) == m_columns ) 
			{
				col = 0 ; 
				row++ ;
			}
			else
				col++ ;

				// re-select the dark pen before the next go-round 
			dc.SelectObject( &dark_pen ) ;

			if ( row * m_cell_size >= client_rect.bottom ) 
				break ;
		}

		dc.SelectObject( dc_pen ) ;
		dc.SelectObject( dc_font ) ;
	}
}









/*******************************************************************************
TagCell

	Static helper called from inside OnPaint to display a tag on a cell of the 
	light table. The tag is scaled based on the size of the cell. 

	The tag_position designates where in the cell the tag should be placed. 

		-------------------------------
		|         |         |         |
		|    0    |    1    |    2    |
		|         |         |         |
		-------------------------------
		|         |         |         |
		|    3    |    4    |    5    |
		|         |         |         |
		-------------------------------
		|         |         |         |
		|    6    |    7    |    8    |
		|         |         |         |
		-------------------------------

	At present, tags are limited to the upper right corner (2), lower left corner (6) and
	lower right corner (8). 

*******************************************************************************/
static void TagCell( 
	CRect cell,						// I - the screen coordinates of the cell 
	int tag_position,				// I - tag position (per the map in the function comment)
	Graphics & graphics,			// I - GDI+ Graphics class 
	CGdiPlusBitmapResource & tag	// I - GDI+ bitmaps class which lets us use a PNG in a resource 
) 
{
	Rect	dest_rect ;
	int		tag_width ;
	int		tag_height ;
	
	tag_width = ((Bitmap *) tag )->GetWidth( ) ;
	tag_height = ((Bitmap *) tag )->GetHeight( ) ;

	if ( tag_width > cell.Width( ) / 6 || tag_height > cell.Height( ) / 6 ) 
	{
		double wfactor ;
		double hfactor ;
		double factor ;

		wfactor = ( (double) cell.Width( ) / 6 ) / (double) tag_width ;
		hfactor = ( (double) cell.Height( ) / 6 ) / (double) tag_height ;

		if ( hfactor < wfactor ) 
			factor = hfactor ;
		else
			factor = wfactor ;

		tag_height = (int) ( factor * tag_height ) ;
		tag_width = (int) ( factor * tag_width ) ;
	}

	dest_rect.Width = tag_width ;
	dest_rect.Height = tag_height ;

	// set the left and right of the destination cell
	switch ( tag_position )
	{
		case 0 :	// left column
		case 3 :
		case 6 :
			dest_rect.X = cell.left + 2 ;
			break ;

		case 1 :	// center column
		case 4 :
		case 7 :
			dest_rect.X = cell.left + cell.Width( ) / 2 - tag_width / 2 ;
			break ;

		case 2 :	// right column
		case 5 :
		case 8 :
			dest_rect.X = cell.right - tag_width - 2 ;
			break ;
		default :
			ASSERT( FALSE ) ;
			break ;
	}

	// set the top and bottom of the dest cell 
	switch ( tag_position )
	{
		case 0 :	// top row 
		case 1 :
		case 2 :
			dest_rect.Y = cell.top + 2 ;
			break ;

		case 3 :	// center row 
		case 4 :
		case 5 :
			dest_rect.Y = cell.top + cell.Height( ) / 2 - tag_height / 2 ;
			break ;

		case 6 :	// bottom row 
		case 7 :
		case 8 :
			dest_rect.Y = cell.bottom - tag_height - 2 ;
			break ;
		default :
			ASSERT( FALSE ) ;
			break ;
	} 

	graphics.DrawImage( (Bitmap *) tag, dest_rect ) ;
}









/*******************************************************************************
CLightTable::OnSize

	Handles WM_SIZE message to recalculate the cell-size and proportional scroll bar 

*******************************************************************************/
void CLightTable::OnSize(UINT nType, int cx, int cy)
{
	if ( cx && cy ) 
	{
		CalculateColumnsAndRows( cx, cy ) ;
		UpdateScrollBar( ) ;
	}

	CView::OnSize(nType, cx, cy);
}






/*******************************************************************************
CLightTable::OnLButtonDown

	Handles left-button mouse clicks, which typically selects a photo or 
	a range o photos

*******************************************************************************/
void CLightTable::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( GetDocument( )->GetDocState( ) != CMakePhotoDisk3Doc::Virgin ) 
	{
		int	clicked_cell ;

			// determine which cell (if any) was clicked on 
		if ( -1 != ( clicked_cell = ClickSelectCell( point ) ) )
		{
			int		old_last_select = GetDocument( )->GetLastSelectedImageNo( ) ;
			int		old_anchor_select = GetDocument( )->GetAnchorSelectionNo( ) ;
			int		old_select_count = GetDocument( )->GetSelectionCount( ) ;
			bool	control_select = ( ( nFlags & MK_CONTROL ) ? true : false ) ;
			bool	shift_select = ( ( nFlags & MK_SHIFT ) ? true : false ) ;

			if ( shift_select && !control_select ) 
				InvalidateCellRange( old_last_select, clicked_cell ) ;
			else if ( shift_select && control_select ) 
				InvalidateCellRange( old_anchor_select, clicked_cell ) ;
			else if ( control_select && !shift_select )
				InvalidateCell( clicked_cell ) ;
			else if ( !control_select && !shift_select ) 
			{
				// regular click can be very complicated - if there was a multi-select going, need to reset every one of them.... 
				if ( old_select_count > 1 ) 
				{
					PhotoImage * photo ;
					int			photo_number ;
					POSITION	pos ;

					for ( photo_number = 0, photo = GetDocument( )->GetFirstPhoto( pos ) ; photo != NULL ; photo = GetDocument( )->GetNextPhoto( pos ), photo_number++ )
						if ( photo->GetSelectedFlag( ) )
							InvalidateCell( photo_number ) ;
				}
				else
					InvalidateCell( old_last_select ) ;
			}

			InvalidateCell( clicked_cell ) ;

			GetDocument()->UpdateAllViews(this, CMakePhotoDisk3Doc::ImageDeselected);
			GetDocument( )->SetLastSelectedImageNo( clicked_cell, control_select, shift_select ) ;
			GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::ImageSelected ) ;
		}
	}

	CView::OnLButtonDown(nFlags, point);
}







/*******************************************************************************
CLightTable::ClickSelectCell

	Returns 0-based number of cell user has clicked on, or -1 if click was not
	on a cell 

*******************************************************************************/
int CLightTable::ClickSelectCell( 
	CPoint point						// I - click location 
) 
{
	int	r ;
	int	c ;
	int	clicked_cell = -1 ;

	if ( m_cell_size > 0 ) 
	{
		c = point.x / m_cell_size ;
		r = point.y / m_cell_size ;

		clicked_cell = m_current_start_pic + r * m_columns + c ;

		if ( clicked_cell >= GetDocument( )->GetPhotoListSize( ) )
		{
			clicked_cell = -1 ;
		}
	}

	return clicked_cell ;
}












/*******************************************************************************
CLightTable::InvalidateCell

	Protected helper function used to invalidate a particular cell to force
	re-paint. Usually called after editing metadata for a cell, which might 
	cause new tags to be applied 

*******************************************************************************/
void CLightTable::InvalidateCell( 
	int pic_number						// I - 0-based picture number to invalidate
)
{
	// pictures before current_start_pic are not visible
	if ( pic_number >= m_current_start_pic ) 
	{
		int		r ;
		int		c ;
		CRect	client_rect ;

		GetClientRect( &client_rect ) ;

		// calculate the "on screen" row and column of the cell 
		r = ( pic_number - m_current_start_pic ) / m_columns ;
		c = ( pic_number - m_current_start_pic ) - r * m_columns ;

		if ( r * m_cell_size < client_rect.Height( ) )
		{
			// the cell is at least partially visible - 
			CRect cell_rect ;

			cell_rect.top = r * m_cell_size ;
			cell_rect.left = c * m_cell_size ;
			cell_rect.bottom = cell_rect.top + m_cell_size ;
			cell_rect.right = cell_rect.left + m_cell_size ;

			InvalidateRect( &cell_rect ) ;
		}
	}
}





/*******************************************************************************
CLightTable::InvalidateCellRange

	Invalidate a cell to force re-paint. May be called after editing a bunch
	of cells, which may require new tags to be applied

*******************************************************************************/
void CLightTable::InvalidateCellRange( int cell_1, int cell_2 ) 
{
	if ( cell_1 > cell_2 ) 
		InvalidateCellRange( cell_2, cell_1 ) ;

	do
	{
		InvalidateCell( cell_1 ) ;
	}
	while ( cell_1++ < cell_2 ) ;
}



/******************************************************************************
CLightTable::EnsureSelectPicVisible

	Called after selecting any picture. 
	
	Will scroll the page as necessary, or just change the display if both old and 
	new select pics are on screen. Passed the old select pic as an argument, 
	so that it can be invalidated, forcing a repaint.

	Also makes sure the scroll bar is updated as required. 

******************************************************************************/
void CLightTable::EnsureSelectPicVisible( 
	int old_select_pic				// I - old selected pic 
)
{
	if ( GetDocument( )->GetLastSelectedImageNo( ) < m_current_start_pic ) 
	{
		// need to scroll page down 
		while ( m_current_start_pic > GetDocument( )->GetLastSelectedImageNo( ) ) 
			m_current_start_pic -= m_columns ;					// adjust by "columns" so we don't shift pics left/right as we scroll 

		if ( m_current_start_pic < 0 )							// but the above could end up with negative start pic, which is bad
			m_current_start_pic = 0 ;

		// need to re-draw the whole page
		Invalidate( ) ;
		UpdateScrollBar( ) ;
	}
	else if ( GetDocument( )->GetLastSelectedImageNo( ) >= m_current_start_pic + m_rows * m_columns ) 
	{
		// need to scroll page up until select pic is visible 
		while ( GetDocument( )->GetLastSelectedImageNo( ) >= m_current_start_pic + m_columns * m_rows ) 
			m_current_start_pic += m_columns ;

		if ( m_current_start_pic > GetDocument( )->GetLastSelectedImageNo( ) )		// slightly pathological case where window is too short to display an intact row of pics
			m_current_start_pic = GetDocument( )->GetLastSelectedImageNo( ) ;

		// need to re-draw the whole page
		Invalidate( ) ;
		UpdateScrollBar( ) ;
	}
	else
	{
		// select pic is on screen - invalidate both old and new select pic cells 
		InvalidateCell( old_select_pic ) ;
		InvalidateCell( GetDocument( )->GetLastSelectedImageNo( ) ) ;
	}
}





/******************************************************************************
CLightTable::OnKeyDown

	Handles keyboard navigation of the light table 

******************************************************************************/
void CLightTable::OnKeyDown(
	UINT nChar,					// I - virtual key code 
	UINT nRepCnt,				// I - ignored, just passed on to base-class handler 
	UINT nFlags					// I - ignored, just passed on to base-class handler 
)
{
	int	old_select_pic = GetDocument( )->GetLastSelectedImageNo( ) ;

	switch ( nChar ) 
	{
		case VK_UP : 
			if ( GetDocument( )->GetLastSelectedImageNo( ) != -1 && GetDocument( )->GetLastSelectedImageNo( ) >= m_columns ) 
			{
				GetDocument()->UpdateAllViews(this, CMakePhotoDisk3Doc::ImageDeselected);
				GetDocument( )->SetLastSelectedImageNo( GetDocument( )->GetLastSelectedImageNo( ) - m_columns, false, false ) ;
				EnsureSelectPicVisible( old_select_pic ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_DOWN :
			if ( GetDocument( )->GetLastSelectedImageNo( ) != -1 && GetDocument( )->GetLastSelectedImageNo( ) + m_columns <= GetDocument( )->GetPhotoListSize( ) - 1 ) 
			{
				GetDocument()->UpdateAllViews(this, CMakePhotoDisk3Doc::ImageDeselected);
				GetDocument()->SetLastSelectedImageNo(GetDocument()->GetLastSelectedImageNo() + m_columns, false, false);
				EnsureSelectPicVisible( old_select_pic ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_LEFT :
			if ( GetDocument( )->GetLastSelectedImageNo( ) != -1 && GetDocument( )->GetLastSelectedImageNo( ) > 0 )
			{
				GetDocument()->UpdateAllViews(this, CMakePhotoDisk3Doc::ImageDeselected);
				GetDocument()->SetLastSelectedImageNo(GetDocument()->GetLastSelectedImageNo() - 1, false, false);
				EnsureSelectPicVisible( old_select_pic ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_RIGHT :
			if ( GetDocument( )->GetLastSelectedImageNo( ) != -1 && GetDocument( )->GetLastSelectedImageNo( ) < GetDocument( )->GetPhotoListSize( ) - 1 ) 
			{
				GetDocument()->UpdateAllViews(this, CMakePhotoDisk3Doc::ImageDeselected);
				GetDocument()->SetLastSelectedImageNo(GetDocument()->GetLastSelectedImageNo() + 1, false, false);
				EnsureSelectPicVisible( old_select_pic ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_HOME :
			if ( m_current_start_pic != 0 ) 
			{
				m_current_start_pic = 0;
				UpdateScrollBar( ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_END :
			if ( m_current_start_pic + m_rows * m_columns <= GetDocument( )->GetPhotoListSize( ) - 1 ) 
			{
				while ( m_current_start_pic + m_rows * m_columns <= GetDocument( )->GetPhotoListSize( ) - 1 )
					m_current_start_pic += m_columns ;
				UpdateScrollBar( ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_PRIOR :
			if ( m_current_start_pic )
			{ 
				if ( ( m_current_start_pic -= m_rows * m_columns ) < 0 )
					m_current_start_pic = 0 ;
				UpdateScrollBar( ) ;
				Invalidate( ) ;
			}
			break ;

		case VK_NEXT :
			if ( m_current_start_pic + m_columns * m_rows <= GetDocument( )->GetPhotoListSize( ) - 1 ) 
			{
				m_current_start_pic += m_rows * m_columns ;
				UpdateScrollBar( ) ;
				Invalidate( ) ;
			}
			break ;

		default : 
			break ;
	}

	if ( old_select_pic != GetDocument( )->GetLastSelectedImageNo( ) ) 
		GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::ImageSelected ) ;

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}





/*******************************************************************************
CLightTable::OnMouseWheel

	The mouse wheel scrolls the view in the light table up and down. 

*******************************************************************************/
BOOL CLightTable::OnMouseWheel(
	UINT nFlags,			// ignored
	short zDelta,			// I - indicates direction of scroll 
	CPoint pt				// ignored 
)
{
	CRect	client_rect ;

	GetClientRect( &client_rect ) ;

	if ( zDelta > 0 )
	{
		// wheel rotated away from user ==> scroll view DOWN, if there's anything above the view...
		if ( m_current_start_pic ) 
		{
			if ( m_current_start_pic >= m_columns )
			{
				m_current_start_pic -= m_columns ;
			}
			else
				m_current_start_pic = 0 ;

			UpdateScrollBar( ) ;
			InvalidateRect( &client_rect ) ;
		}
	}
	else
	{
		// wheel rotated towards user ==> scroll view UP, if there's anything below the view... 
		if ( m_current_start_pic + m_rows * m_columns < GetDocument( )->GetPhotoListSize( ) )
		{
			m_current_start_pic += m_columns ;
			UpdateScrollBar( ) ;
			InvalidateRect( &client_rect ) ;
		}
	}

	return TRUE ;
}





/*******************************************************************************
CLightTable::OnInitialUpdate

	Called the first time the view is displayed 

*******************************************************************************/
void CLightTable::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_current_start_pic = 0 ;
	m_cell_size = 0 ;
	m_rows = 1 ;
	m_columns = 1 ;
}





/*******************************************************************************
CLightTable::OnUpdate

	Override for MFC standard virtual invoked as a result of calling the document's
	UpdateAllViews member. The ignored pSender argument designates which view called
	for the update. This is almost always one of the control panels, but we don't really
	care. Nor do I do anything with pHint. We handle the update differently based on
	udpate_type. 

*******************************************************************************/
void CLightTable::OnUpdate( 
	CView * /*pSender*/,				// ignored 
	LPARAM update_type,					// I  - reason for update 
	CObject* /*pHint*/					// ignored 
)
{
	switch ( update_type ) 
	{
		case CMakePhotoDisk3Doc::CollectionSelected :
			const Collection * col ;

			if ( ( col = GetDocument( )->GetSelectedCollection( ) ) && !col->GetIsCollectionSet( ) )
			{
				m_current_start_pic = 0 ;
				Invalidate( ) ;
				UpdateScrollBar( ) ;
			}
			break ;

		case CMakePhotoDisk3Doc::CatalogSelected : 
		case CMakePhotoDisk3Doc::CollectionDeselected :
			m_current_start_pic = 0 ;
			Invalidate( ) ;
			break ;

		case CMakePhotoDisk3Doc::MetadataUpdated : 
		case CMakePhotoDisk3Doc::JpegsRefreshed : 
			Invalidate( ) ;
			break ;

		default : 
			break ;
	}
}





/*******************************************************************************
CLightTable::OnRButtonUp

	Handler for right-click, which brings up a popup menu if the user has 
	right-clicked on a cell 

	A different right-click menu is displayed depending on whether a single
	cell is selected or multiple cells. 

*******************************************************************************/
void CLightTable::OnRButtonUp(
	UINT nFlags,					// ignored, but passed through to base class handler 
	CPoint point					// I - point where the user clicked 
)
{
	CMenu	rclick_menu ;
	CMenu *	popup_menu ;
	int		clicked_cell ;

	// take no action unless they've right-clicked on a cell 
	if ( -1 != ( clicked_cell = ClickSelectCell( point ) ) )
	{
		if ( 1 < GetDocument( )->GetSelectionCount( ) ) 
		{
			rclick_menu.LoadMenu( IDR_LIGHT_TABLE_RCLICK_MULTI_SELECT ) ;
			ClientToScreen( &point ) ;
			popup_menu = rclick_menu.GetSubMenu( 0 ) ;
			popup_menu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd( ) ) ;
		}
		else
		{
			if ( -1 != GetDocument( )->GetLastSelectedImageNo( ) )
				InvalidateCell( GetDocument( )->GetLastSelectedImageNo( ) ) ;

			// right click may change the selected slide - or select a slide if none already selected 
			InvalidateCell( clicked_cell ) ;
			GetDocument()->UpdateAllViews(this, CMakePhotoDisk3Doc::ImageDeselected);
			GetDocument( )->SetLastSelectedImageNo( clicked_cell, false, false ) ;
			GetDocument( )->UpdateAllViews( this, CMakePhotoDisk3Doc::ImageSelected ) ;

			// now display the popup context menu 
			rclick_menu.LoadMenu( IDR_LIGHT_TABLE_RCLICK_SINGLE_SELECT ) ;
			ClientToScreen( &point ) ;
			popup_menu = rclick_menu.GetSubMenu( 0 ) ;
			popup_menu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd( ) ) ;
		}
	}

	CView::OnRButtonUp(nFlags, point);
}






/*******************************************************************************
CLightTable::OnProperties

	Display the properties for the currently selected source image 

*******************************************************************************/
void CLightTable::OnProperties()
{
	const PhotoImage * image = GetDocument( )->GetLastSelectedImage( ) ;

	if ( image ) 
	{
		CImageProperties	dlg( image, this ) ;

		dlg.DoModal( ) ;
	}
	else
		ASSERT( FALSE ) ;
}






/*******************************************************************************
CLightTable::OnEditSource

	Let the user edit the source-image using photoshop. 

	FIXME - do I want to support this? 

*******************************************************************************/
void CLightTable::OnEditSource()
{
	const PhotoImage * image = GetDocument( )->GetLastSelectedImage( ) ;

	if ( image ) 
	{
		LaunchPhotoshopOnFile( image->GetSourceFilePath( ) ) ;
	}
	else
		ASSERT( FALSE ) ;
}






/*******************************************************************************
LaunchPhotoshopOnFile

	Used by OnEditSource and ConfirmCopyKeyFile to bring up the source image 
	in Photoshop 

*******************************************************************************/
void LaunchPhotoshopOnFile( 
	const TCHAR * source_file		// I - source file to edit 
)
{
	HKEY	reg_key ;

	if ( ERROR_SUCCESS == ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( "SOFTWARE\\Adobe\\Photoshop" ), 0, KEY_READ | KEY_WOW64_64KEY, &reg_key ) ) 
	{
		int			subkey_index ;
		TCHAR		subkey_name[ 128 ] ;
		DWORD		subkey_name_size ;
		bool		valid_subkey ;
		HKEY		reg_subkey ;

		subkey_index = 0 ;
		subkey_name[ 0 ] = _T( '\0' ) ;

		do
		{
			valid_subkey = false ;
			subkey_name_size = sizeof( subkey_name ) / sizeof( subkey_name[ 0 ] ) ;

			// get the next subkey, if it exists. If it doesn't exist, the name buffer is NOT modified 
			if ( ERROR_SUCCESS == ::RegEnumKeyEx( reg_key, subkey_index, subkey_name, &subkey_name_size, NULL, NULL, NULL, NULL ) )
				valid_subkey = true ;

			subkey_index++ ;
		}
		while ( valid_subkey ) ;

		// last successful call left the subkey_name intact 
		if ( ERROR_SUCCESS == ::RegOpenKeyEx( reg_key, subkey_name, 0, KEY_READ, &reg_subkey ) ) 
		{
			TCHAR				application_path[ MAX_PATH ] ;
			DWORD				application_path_size ;
			DWORD				data_type ;

			application_path_size = sizeof( application_path ) / sizeof( application_path[ 0 ] ) ;

			if ( ERROR_SUCCESS == ::RegQueryValueEx( reg_subkey, _T( "ApplicationPath" ), NULL, &data_type, (LPBYTE) application_path, &application_path_size ) ) 
			{
				if ( data_type == REG_SZ ) 
				{
					CString				cmd_line ;
					STARTUPINFO			startup_info ;
					PROCESS_INFORMATION	process_info ;
					TCHAR *				cmd_line_ptr ;

					cmd_line.Format( _T( "\"%sPhotoshop.exe\" \"%s\"" ), application_path, source_file ) ;

					ZeroMemory( &startup_info, sizeof( STARTUPINFO ) ) ;
					startup_info.cb = sizeof( STARTUPINFO ) ;

					// path's in lightroom are stored with forward slashes, and those don't work when we call CreateProcess( ) 
					cmd_line.Replace( _T( '/' ), _T( '\\' ) ) ;
					cmd_line_ptr = cmd_line.GetBufferSetLength( MAX_PATH ) ;

					CreateProcess( NULL, cmd_line_ptr, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info ) ;

					cmd_line.ReleaseBuffer( ) ;

					CloseHandle( process_info.hProcess ) ;
					CloseHandle( process_info.hThread ) ;
				}
			}

			::RegCloseKey( reg_subkey ) ;
		}

		::RegCloseKey( reg_key ) ;
	}
}








/*******************************************************************************
CLightTable::OnShowInExplorer

	Just bring up explorer with the corresponding source file selected 

*******************************************************************************/
void CLightTable::OnShowInExplorer()
{
	const PhotoImage * image = GetDocument( )->GetLastSelectedImage( ) ;

	if ( image )
	{	
		CString		source_path ;

		CoInitializeEx( NULL, COINIT_MULTITHREADED ) ;

		source_path = image->GetSourceFilePath( ) ;
		source_path.Replace( _T( '/' ), _T( '\\' ) ) ;		// if we don't do this, the forward slashes cause ILCreateFromPath to choke

		ITEMIDLIST * pIDL = ILCreateFromPath( source_path ) ;

		if ( pIDL ) 
		{
			SHOpenFolderAndSelectItems( pIDL, 0, 0, 0 ) ;
			ILFree( pIDL ) ;
		}

		CoUninitialize( ) ;
	}
}




/*******************************************************************************
CLightTable::OnSelectKeyImage

	User has selected a key image. 

*******************************************************************************/
void CLightTable::OnSelectKeyImage()
{
	ConfirmCopyKeyFile( _T( "KeyImage" ) ) ;
	GetDocument( )->UpdateAllViews( this ) ;
}




/*******************************************************************************
CLightTable::OnSelectDiskIcon

	User has selected a source for the disk icon 

*******************************************************************************/
void CLightTable::OnSelectDiskIcon()
{
	ConfirmCopyKeyFile( _T( "DiskIcon" ) ) ;
	GetDocument( )->UpdateAllViews( this ) ;
}







/*******************************************************************************
CLightTable::OnSetTitleDescription

	Handles right-click multi-select menu option to set title / description for
	several photos 

*******************************************************************************/
void CLightTable::OnSetTitleDescription( )
{
	CMultiSelectPhotoTitleDescr	dlg( GetDocument( ), this ) ;

	if ( IDOK == dlg.DoModal( ) )
	{
		if ( dlg.m_set_title || dlg.m_set_descr ) 
		{
			PhotoImage *	photo ;
			POSITION		pos ;

			if ( photo = GetDocument( )->GetFirstSelectedPhoto( pos ) )
			{
				do
				{
					if ( dlg.m_set_title ) 
						photo->SetOverrideTitle( dlg.m_title ) ;

					if ( dlg.m_set_descr ) 
						photo->SetOverridedDescr( dlg.m_descr ) ;

					photo->SetHasDirtyEdits( ) ;
				}
				while ( photo = GetDocument( )->GetNextSelectedPhoto( pos ) ) ;

				Invalidate( ) ;
			}
		}
	}
}





/*******************************************************************************
CLightTable::OnSetTimeAdjust

	Handles multi-select context menu option to set a time adjust

*******************************************************************************/
void CLightTable::OnSetTimeAdjust( ) 
{
	CMultiSelectTimeAdjust	dlg( GetDocument( ), this ) ;

	if ( IDOK == dlg.DoModal( ) )
	{
		PhotoImage *	image ;
		POSITION		pos ;

		for ( image = GetDocument( )->GetFirstSelectedPhoto( pos ) ; NULL != image ; image = GetDocument( )->GetNextSelectedPhoto( pos ) )
		{
			image->SetCreateTimeAdjust( dlg.m_adjust_seconds ) ;
			image->SetHasDirtyEdits( ) ;
		}

		if ( GetDocument( )->GetSortByCaptureTime( ) )
			GetDocument( )->ReOrderPhotosByCaptureTime( ) ;

		Invalidate( ) ;
	}
}




/*******************************************************************************
CLightTable::OnSetPhotogCamera

	Multi-select context menu option to set photographer and/or camera for multiple
	photos 

*******************************************************************************/
void CLightTable::OnSetPhotogCamera( ) 
{
	CMultiSelectPhotogCamera		dlg( GetDocument( ), this ) ;

	if ( IDOK == dlg.DoModal( ) )
	{
		PhotoImage *	image ;
		POSITION		pos ;

		for ( image = GetDocument( )->GetFirstSelectedPhoto( pos ) ; NULL != image ; image = GetDocument( )->GetNextSelectedPhoto( pos ) )
		{
			if ( dlg.m_override_photog ) 
				image->SetOverridePhotographer( dlg.m_photographer ) ;

			if ( dlg.m_override_camera )
				image->SetOverrideCameraName( dlg.m_camera ) ;

			image->SetHasDirtyEdits( ) ;
		}

		Invalidate( ) ;
	}
}







/*******************************************************************************
CLightTable::OnEditComment

	Handle multi-select comment 

*******************************************************************************/
void CLightTable::OnEditComment( ) 
{
	CMultiSelectComment	dlg( GetDocument( ), this ) ;

	if ( IDOK == dlg.DoModal( ) )
	{
		PhotoImage *	image ;
		POSITION		pos ;

		for ( image = GetDocument( )->GetFirstSelectedPhoto( pos ) ; NULL != image ; image = GetDocument( )->GetNextSelectedPhoto( pos ) )
		{
			image->SetExtendedComment( dlg.m_comment ) ;
			image->SetHasDirtyEdits( ) ;
		}

		Invalidate( ) ;
	}
}






















/*******************************************************************************
CLightTable::ConfirmReplaceKeyFile

	Used by ConfirmCopyKeyFile to check for a previous instance of target_file - 
	return false iff. user opts not to replace it need to use wildcard find b/c 
	could have .tif or .psd source and .jpg or .gif image format
	
	FIXME - should probably store the key image selection in the document. 

*******************************************************************************/
bool CLightTable::ConfirmReplaceKeyFile( const TCHAR * target_file ) 
{
	CString			path_template ;
	HANDLE			find_handle ;
	WIN32_FIND_DATA	find_data ;
	bool			isOK = true ;
	bool			first_match = true ;

	path_template.Format( _T( "%s\\misc\\%s.*" ), GetDocument( )->GetProjectPath( ), target_file ) ;
	if ( INVALID_HANDLE_VALUE != ( find_handle = FindFirstFile( path_template, &find_data ) ) ) 
	{
		do
		{
			if ( first_match ) 
			{
				first_match = false ;

				if ( IDNO == AfxMessageBox( _T( "Do you want to replace the current image?" ), MB_YESNO | MB_ICONQUESTION ) ) 
					isOK = false ;
			}

			if ( isOK )
			{
				path_template.Format( _T( "%s\\misc\\%s" ), GetDocument( )->GetProjectPath( ), find_data.cFileName ) ;
				if ( 0 == DeleteFile( path_template ) ) 
				{
					AfxMessageBox( _T( "Unable to delete file!" ), MB_OK | MB_ICONSTOP ) ;
					isOK = false ;
				}
			}
		}
		while ( isOK && FindNextFile( find_handle, &find_data ) ) ;

		FindClose( find_handle ) ;
	}

	return isOK ;
}





/*******************************************************************************
CLightTable::ConfirmCopyKeyFile

	Handles user command to replace the key file. After checking for a previous
	key file (and confirming the user wants to replace it), copies the file and 
	then launches Photoshop so the user can crop it 

*******************************************************************************/
void CLightTable::ConfirmCopyKeyFile( const TCHAR * filename ) 
{
	const PhotoImage * image = GetDocument( )->GetLastSelectedImage( ) ;

	if ( ConfirmReplaceKeyFile( filename ) ) 	
	{
		CString		source_file ;
		CString		dest_file ;
		CString		extension ;

		source_file = image->GetSourceFilePath( ) ;
		extension = source_file.Mid( source_file.ReverseFind( _T( '.' ) ) ) ;

		dest_file.Format( _T( "%s\\misc\\%s%s" ), GetDocument( )->GetProjectPath( ), filename, (const TCHAR *) extension ) ;
		CopyFile( source_file, dest_file, FALSE ) ;

		LaunchPhotoshopOnFile( dest_file ) ;
	}	
}




/*******************************************************************************
CLightTable::OnUpdateSelectDiskIcon

	Command update handler - the command to update the disk icon is only enabled
	after the project type is known

*******************************************************************************/
void CLightTable::OnUpdateSelectDiskIcon(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CMakePhotoDisk3Doc::CatalogCollectionProjectKnown == GetDocument( )->GetDocState( ) ) ;
}




/*******************************************************************************
CLightTable::OnUpdateSelectKeyImage
	
	Command update handler. The command to update the selected key image is enabled 
	only if the project type is known 

*******************************************************************************/
void CLightTable::OnUpdateSelectKeyImage(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CMakePhotoDisk3Doc::CatalogCollectionProjectKnown == GetDocument( )->GetDocState( ) ) ;
}
