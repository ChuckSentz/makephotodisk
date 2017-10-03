/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



Worker thread and support functions to update the HTML for a disk project

***************************************************************************************/
#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MakePhotoDisk3.h"
#include "MainFrm.h"
#include "WelcomeView.h"
#include "MakePhotoDisk3Doc.h"
#include "UpdateHTMLWorker.h"

#include "EXIFDigest.h"


void GeneratePresentationHTML( int pic_number, CMakePhotoDisk3Doc * doc, PhotoImage * image ) ;
void CreateContactSheets( CMakePhotoDisk3Doc * doc ) ;

void GuaranteePresentationDimensions( CMakePhotoDisk3Doc *	doc, PhotoImage * image	) ;
void GuaranteeMiniPresentationDimensions( CMakePhotoDisk3Doc *	doc, PhotoImage * image	) ;
void GetDimensionsFromJpeg( const TCHAR * file_path, int &	width, int & height ) ;


static bool ReadResourceLine( CString & resource, CString & line ) ;
void ParseContactTemplate( CString & lead_in, CString & top_page_bar, CString & table_start, CString & row_template, CString & table_end, CString & bottom_page_bar, CString & follow_on ) ;
void OutputContactSheets( CMakePhotoDisk3Doc * doc, const CString & lead_in, const CString & top_page_bar, const CString & table_start, const CString & row_template, const CString & table_end, const CString & bottom_page_bar, const CString & follow_on ) ;
void ReplaceFirstInstance( CString & work, const TCHAR * target, const TCHAR * substitution ) ;
void OutputPageBar( CString & output_file, const CString & page_bar_master, int cur_page, int num_pages ) ;
void TouchStartPage( CMakePhotoDisk3Doc * doc ) ;



/******************************************************************************
UpdateHTML

	Called to refresh the HTML pages for presentation & contact sheet(s). 

******************************************************************************/
void UpdateHTML( 
	CMakePhotoDisk3Doc * doc			// I - document 
) 
{
	int				pic_no ;
	POSITION		pos ;
	PhotoImage *	image ;

	/* 
		create the presentation page HTML for each photo. Any existing page is overwritten.
	*/ 
	for ( image = doc->GetFirstPhoto( pos ), pic_no = 0 ; NULL != image ; image = doc->GetNextPhoto( pos ), pic_no++ )
		GeneratePresentationHTML( pic_no, doc, image ) ;

	// now generate the contact sheets - 
	CreateContactSheets( doc ) ;
}





/******************************************************************************
GeneratePresentationHTML

	Generate the HTML for the presentation page of a specific photo. 

******************************************************************************/
void GeneratePresentationHTML( 
	int pic_number,						// I - picture number in list 
	CMakePhotoDisk3Doc * doc,			// I - the document
	PhotoImage * image					// I - the photo we're working on
) 
{
	// get the html resource for the presentation
	HRSRC	resource_handle ;
	HGLOBAL	global_object ;
	TCHAR *	resource_ptr ;

	if ( resource_handle = FindResource( AfxGetInstanceHandle( ), MAKEINTRESOURCE( IDR_PRESENTATION ), RT_HTML ) ) 
	{
		if ( global_object = LoadResource( AfxGetInstanceHandle( ), resource_handle ) )
		{
			if ( resource_ptr = (TCHAR *) LockResource( global_object ) ) 
			{
				CString	html_string ;

				// the resource is NOT Unicode. So we convert it to unicode in html_string, where we make substitutions 
				html_string.Format( _T( "%.*S" ), SizeofResource( AfxGetInstanceHandle( ), resource_handle ), resource_ptr ) ;

				CString	index_page ;
				CString	prev_page ;
				CString	next_page ;
				CString this_page ;
				CString jpeg_name ;

				CString	full_presentation_jpeg ;
				CString mini_presentation_jpeg ;

				CString	mini_width_string ;
				CString mini_height_string ;
				CString full_width_string ;
				CString full_height_string ;

				CString	facebook_jpeg ;
				CString	compressed_jpeg ;
				CString	fullsize_jpeg ;
				CString	caption ;
				CString	camera_and_time ;
				CString exposure_and_lens ;
				CString page_title ;
				CString	extended_comment ;

				// PhotoImage's number on disk should be up-to-date at this point, after calling RealignOrAssignOutputNumbers
				// ...and yes, argument 'pic_number' is obviously redundant, but it's convenient here and readily available to caller 
				ASSERT( image->GetNumberOnDisk( ) != -1 && image->GetNumberOnDisk( ) == pic_number + 1 ) ;

				jpeg_name.Format( _T( "%d.jpg" ), image->GetNumberOnDisk( ) ) ;
				page_title.Format( _T( "Pic %d" ), image->GetNumberOnDisk( ) ) ;

				// all the html is stored in the 'pages' subdirectory. Just use filenames for index, prev, and next links - since they're in the same directory
				// but need to specify fully qualified path for 'this_page', which is passed at the end to SaveUTF8File() 
				index_page.Format( _T( "index%d.htm" ), ( pic_number / 40 ) + 1 ) ;
				prev_page.Format( _T( "pic%d.htm" ), pic_number ) ;			// pic_number is 0-based, but the user knows the pictures by 1-based numbers
				next_page.Format( _T( "pic%d.htm" ), pic_number + 2 ) ;
				this_page.Format( _T( "%s\\pages\\pic%d.htm" ), doc->GetProjectPath( ), pic_number + 1 ) ;

				html_string.Replace( _T( "$INDEX$" ), index_page ) ;
				html_string.Replace( _T( "$PREVPIC$" ), prev_page ) ;
				html_string.Replace( _T( "$NEXTPIC$" ), next_page ) ;
				html_string.Replace( _T( "$TITLE$" ), page_title ) ;


				// at present, we always do mini presentation
				GuaranteeMiniPresentationDimensions( doc, image ) ;

				if ( doc->GetDoBigPresentation( ) ) 
					GuaranteePresentationDimensions( doc, image ) ;
				else
					DeleteConditionalExpressions( html_string, _T( "PRESENTATION" ) ) ;		// delete any conditional code pulling in a full-size presentation 

				if ( !doc->GetDoCompressed( ) )
					ReplaceDelimitedSubstring( html_string, _T( "<!--compressed_link-->" ), _T( "<!--/compressed_link-->" ), _T( "" ) ) ;

				if ( !doc->GetDoFacebook( ) )
					ReplaceDelimitedSubstring( html_string, _T( "<!--facebook_link-->" ), _T( "<!--/facebook_link-->" ), _T( "" ) ) ;

				if ( !doc->GetDoFullSize( ) )
					ReplaceDelimitedSubstring( html_string, _T( "<!--fullsized_link-->" ), _T( "<!--/fullsized_link-->" ), _T( "" ) ) ;

				// clean up any remaining $IF( PRESENTATION ), and $ENDIF( PRESENTATION )
				CleanupConditionalExpressions( html_string, _T( "PRESENTATION" ) ) ;

				full_width_string.Format( _T( "%d" ), image->GetPresentationWidth( ) ) ;
				full_height_string.Format( _T( "%d" ), image->GetPresentationHeight( ) ) ;
				mini_width_string.Format( _T( "%d" ), image->GetMiniPresentationWidth( ) ) ;
				mini_height_string.Format( _T( "%d" ), image->GetMiniPresentationHeight( ) ) ;

				html_string.Replace( _T( "$PIMAGEWIDTH$" ), full_width_string ) ;
				html_string.Replace( _T( "$PIMAGEHEIGHT$" ), full_height_string ) ;
				html_string.Replace( _T( "$MINIPIMAGEWIDTH$" ), mini_width_string ) ;
				html_string.Replace( _T( "$MINIPIMAGEHEIGHT$" ), mini_height_string ) ;

				/*
					NOTE: since the strings we're formatting here are arguments to document.Write(), just a double 
					backslash here will end up being interpreted as an escape sequence for document.Write to digest.
					That is to say, 4 slashes here become 2 in the string literal, then document.Write sees 2 
					backslashes, which document.Write interprets as a request for a literal, single, backslash
				*/
				full_presentation_jpeg.Format( _T( "..\\\\images\\\\presentation\\\\%s" ), static_cast< const TCHAR *>( jpeg_name ) ) ;
				mini_presentation_jpeg.Format( _T( "..\\\\images\\\\minip\\\\%s" ), static_cast< const TCHAR *>( jpeg_name ) ) ;
				facebook_jpeg.Format( _T( "..\\\\images\\\\facebook\\\\%s" ), static_cast< const TCHAR *>( jpeg_name ) ) ; 
				compressed_jpeg.Format( _T( "..\\\\images\\\\compressed\\\\%s" ), static_cast< const TCHAR *>( jpeg_name ) ) ; 
				fullsize_jpeg.Format( _T( "..\\\\images\\\\fullsize\\\\%s" ), static_cast< const TCHAR *>( jpeg_name ) ) ; 

				html_string.Replace( _T( "$PRESENTATIONJPG$" ), full_presentation_jpeg ) ;
				html_string.Replace( _T( "$MINIPRESENTATIONJPG$" ), mini_presentation_jpeg ) ;

				html_string.Replace( _T( "$COMPRESSEDJPG$" ), compressed_jpeg ) ;
				html_string.Replace( _T( "$FULLJPG$" ), fullsize_jpeg ) ;
				html_string.Replace( _T( "$FACEBOOKJPG$" ), facebook_jpeg ) ;

				caption	= image->GetFormattedCaption( ) ;
				caption.Replace( _T( "\"" ), _T( "\\\"" ) ) ;				// need to escape any embedded double quotes b/c this string will itself be enclosed in quotes as arg to document.Write() 
				camera_and_time = image->GetCreationSubCaption( ) ;
				exposure_and_lens = image->GetExposureSubCaption( ) ;

				html_string.Replace( _T( "$CAPTION$" ), caption ) ;
				html_string.Replace( _T( "$CAMERADATETIME$" ), camera_and_time ) ;
				html_string.Replace( _T( "$EXPOSURE$" ), exposure_and_lens ) ;

				image->GetExtendedComment( extended_comment ) ;
				extended_comment.Replace( _T( "\"" ), _T( "\\\"" ) ) ;		// need to escape any embedded double quotes b/c this string will itself be enclosed in quotes as arg to document.Write() 

				html_string.Replace( _T( "$OPTIONALHTML$" ), extended_comment ) ;

				html_string.Replace( _T( "$INDEX$" ), index_page ) ;
				html_string.Replace( _T( "$PREVPIC$" ), prev_page ) ;
				html_string.Replace( _T( "$NEXTPIC$" ), next_page ) ;

				/* 
					need to re-write some of the javascript so we display (or disable) the prev & next buttons appropriately - 
					<!--firstpic-->, <!--/firstpic--> wraps the script we want for the firstpic (prev disabled, next enabled)
					<!--midpic-->, <!--/midpic--> wraps the script we want for a middle pic (both prev and next enabled)
					<!--lastpic-->, <!--/lastpic--> wraps the script we want for the last pic (next disabled)

					so in each case (first, mid, last), we just zap the two sections which don't apply
				*/
				if ( pic_number == 0 )
				{
					// this is the first pic -
					ReplaceDelimitedSubstring( html_string, _T( "<!--midpic-->" ), _T( "<!--/midpic-->" ), _T( "" ) ) ;
					ReplaceDelimitedSubstring( html_string, _T( "<!--lastpic-->" ), _T( "<!--/lastpic-->" ), _T( "" ) ) ;
				}
				else if ( pic_number < doc->GetPhotoListSize( ) - 1 )
				{
					// this is a middle pic 
					ReplaceDelimitedSubstring( html_string, _T( "<!--firstpic-->" ), _T( "<!--/firstpic-->" ), _T( "" ) ) ;
					ReplaceDelimitedSubstring( html_string, _T( "<!--lastpic-->" ), _T( "<!--/lastpic-->" ), _T( "" ) ) ;
				}
				else
				{
					// this is the last pic - 
					ReplaceDelimitedSubstring( html_string, _T( "<!--firstpic-->" ), _T( "<!--/firstpic-->" ), _T( "" ) ) ;
					ReplaceDelimitedSubstring( html_string, _T( "<!--midpic-->" ), _T( "<!--/midpic-->" ), _T( "" ) ) ;
				}

				// save the unicode html in html_string to the file designated by this_page
				SaveUTF8File( this_page, html_string ) ;

				UnlockResource( global_object ) ;
			}
		}
	}
}









/******************************************************************************
GuaranteePresentationDimensions

	Make sure the PhotoImage has dimensions for the presentation, 
	which usually come from the Photoshop processing, or if we didn't have 
	to update this presentation this pass, get the dimensions from the jpeg. 

******************************************************************************/
void GuaranteePresentationDimensions( 
	CMakePhotoDisk3Doc *	doc,		// I - the document
	PhotoImage * image					// I - the photo we're working on
)
{
	if ( 0 == image->GetPresentationWidth( ) ) 
	{
		CString	file_path ;
		int		jpeg_width ;
		int		jpeg_height ;

		file_path.Format( _T( "%s\\images\\presentation\\%d.jpg" ), doc->GetProjectPath( ), image->GetNumberOnDisk( ) ) ;

		GetDimensionsFromJpeg( file_path, jpeg_width, jpeg_height ) ;

		image->SetPresentationWidth( jpeg_width ) ;
		image->SetPresentationHeight( jpeg_height ) ;
	}
}



/******************************************************************************
GuaranteeMiniPresentationDimensions

	Make sure the PhotoImage has dimensions for the mini presentation, 
	which usually come from the Photoshop processing, or if we didn't have 
	to update this presentation this pass, get the dimensions from the jpeg. 

******************************************************************************/
void GuaranteeMiniPresentationDimensions( 
	CMakePhotoDisk3Doc * doc,			// I - the document
	PhotoImage * image					// I - the photo we're working on
)
{
	if ( 0 == image->GetMiniPresentationWidth( ) ) 
	{
		CString	file_path ;
		int		jpeg_width ;
		int		jpeg_height ;

		file_path.Format( _T( "%s\\images\\minip\\%d.jpg" ), doc->GetProjectPath( ), image->GetNumberOnDisk( ) ) ;

		GetDimensionsFromJpeg( file_path, jpeg_width, jpeg_height ) ;

		image->SetMiniPresentationWidth( jpeg_width ) ;
		image->SetMiniPresentationHeight( jpeg_height ) ;
	}
}



/******************************************************************************
GetDimensionsFromJpeg

	Reads the EXIF data just to get the width and height of the jpeg 

******************************************************************************/
void GetDimensionsFromJpeg( 
	const TCHAR * file_path,				// I - fully qualified path of jpeg file to check 
	int &	width,							// O - gets the width of the jpeg 
	int &	height							// O - gets the height of the jpeg 
)
{
	EXIFDigest	exif_digest( file_path ) ;

	width = exif_digest.GetWidth( ) ;
	height = exif_digest.GetHeight( ) ;
}




/******************************************************************************
SaveUTF8File

	Save a unicode string to an file using UTF8 encoding. 

******************************************************************************/
bool SaveUTF8File( 
	const TCHAR * filename,				// I - filename to write to 
	CString & file_contents				// I - HTML as a UNICODE string 
) 
{
	CFile	page ;
	bool	isOK = false ;

	if ( page.Open( filename, CFile::modeCreate | CFile::modeWrite ) ) 
	{
		char *	ansii_buffer ;
		int		ansii_buffer_size ;

		// first call with zero buffer size just gets the size of the buffer required...
		if ( ansii_buffer_size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, file_contents, file_contents.GetLength( ) + 1, NULL, 0, NULL, NULL ) )
		{
			ansii_buffer = new char[ ansii_buffer_size ] ;

			if ( WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, file_contents, file_contents.GetLength( ) + 1, ansii_buffer, ansii_buffer_size, NULL, NULL ) )
			{
#pragma warning( disable: 4996 ) 
				page.Write( ansii_buffer, ansii_buffer_size ) ;

				isOK = true ;
			}

			delete [] ansii_buffer ;
		}

		page.Close( ) ;
	}

	return isOK ;
}






/******************************************************************************
LoadUTF8File

	Load UTF8-encoded HTML page, translating the UTF8 into a unicode CString

******************************************************************************/
bool LoadUTF8File( 
	const TCHAR * filename,				// I - filename to read 
	CString & file_contents				// O - gets a CString with the UNICODE version of file's contents
) 
{
	CFile	page ;
	bool	isOK = false ;

	if ( page.Open( filename, CFile::modeRead ) )
	{
		char *	ansii_buffer ;

		if ( ansii_buffer = new char[ (int) page.GetLength( ) ] )
		{
			if ( page.GetLength( ) == page.Read( ansii_buffer, (int) page.GetLength( ) ) )
			{
				TCHAR * fc_buffer ;
				int		wc_count ;
				int		wc_actual_count ;

				// the number of unicode characters should be just about the same as the ANSI file length, but add a hefty margin anyway....
				wc_count = (int) page.GetLength( ) + 1024 ;
				fc_buffer = file_contents.GetBufferSetLength( wc_count ) ;

				// get the actual number of characters after conversion 
				wc_actual_count = MultiByteToWideChar( CP_UTF8, 0, ansii_buffer, (int) page.GetLength( ), fc_buffer, wc_count ) ;
				ASSERT( wc_actual_count < wc_count ) ;
				fc_buffer[ wc_actual_count ] = _T( '\0' ) ;

				isOK = true ;

				file_contents.ReleaseBufferSetLength( wc_actual_count ) ;
			}

			delete [] ansii_buffer ;
		}

		page.Close( ) ;
	}

	return isOK ;
}



 


/******************************************************************************
ReplaceDelimitedSubstring

	replaces everything between delimiters, overwriting the delimiters as well 
	in the process

	If right delimiter is NULL, will replace EVERYTHING from left delimiter to
	the end of the string 

	Note: this function replaces EVERY matching instance of the delimiters 
	with the replacement content

******************************************************************************/
bool ReplaceDelimitedSubstring( 
	CString & actual,					// I/O - string to work on 
	const TCHAR * l_delim,				// I - left delimiter
	const TCHAR * r_delim,				// I - right delimiter, which may be NULL 
	const TCHAR * repl					// I - replacement 
) 
{
	int		pos ;
	int		len ;
	bool	found_one = false ;

	while ( -1 != ( pos = actual.Find( l_delim ) ) )
	{
		if ( r_delim != NULL )
		{
			int	pos2 ;
			
			if ( -1 != ( pos2 = actual.Find( r_delim, pos + 1 ) ) )
			{
				len = int( ( pos2 + _tcslen( r_delim ) ) - pos ) ;
				found_one = true ;
			}
			else
				break ;		// found l_delim, but not r_delim ==> no match but need to break to prevent infinite looping 
		}
		else
		{
			len = int( _tcslen( l_delim ) ) ;
			found_one = true ;
		}

		if ( found_one )
		{
			actual = actual.Left( pos ) + CString( repl ) + actual.Mid( pos + len ) ;
		}		
	}
	
	return found_one ;
}





/******************************************************************************
ReplaceDelimitedCondition

	Some of the HTML resources may have conditionally included code similar to 
	the C preprocessor #if( ) / #endif. 

	Our syntax is: $IF(symbol)<conditional text>$ENDIF(symbol). The symbol
	must appear with both the $IF and $ENDIF, since no nesting is supported, 
	and there must be no space between 

	At present, this is only used for the conditional compilation of HTML to 
	display a full-sized presentation image. 

******************************************************************************/
bool ReplaceDelimitedCondition( 
	CString & actual,					// I/O - the string to work on 
	const TCHAR * conditional,			// I - identifier to look for with $IF( ) and $ENDIF( ) 
	const TCHAR * new_content			// I - new text to replace the delimited content
)
{
	bool	found_match = false ;
	int		begin_if ;
	int		begin_endif ;
	int		start_pos ;
	CString	if_sequence ;
	CString endif_sequence ;

	if_sequence.Format( _T( "$IF(%s)" ), conditional ) ;
	endif_sequence.Format( _T( "$ENDIF(%s)" ), conditional ) ;

	start_pos = 0 ;
	while ( -1 != ( begin_if = actual.Find( if_sequence, start_pos ) ) && -1 != ( begin_endif = actual.Find( endif_sequence, begin_if ) ) )
	{
		actual = actual.Left( begin_if + if_sequence.GetLength( ) ) + new_content + actual.Mid( begin_endif ) ;
		start_pos += if_sequence.GetLength( ) + _tcslen( new_content ) + endif_sequence.GetLength( ) ;
		found_match = true ;
	}

	return found_match ;
}




/******************************************************************************
MatchTokenSequence

	search for a sequence of 4 tokens, which must 

	the contents of referenced begin_sequence and follow_squence are undefined if
	this function returns false 

******************************************************************************/
bool MatchTokenSequence( 
	CString & actual,				// I - the string to search 
	int			start_pos,			// I - the start position for the search 
	int	&		begin_sequence,		// O - gets offset of the start of the first token 
	int &		follow_sequence,	// O - gets offset of the first character AFTER the last token 
	const TCHAR *	token1,			// I - the first token, probably "$IF" or "$ENDIF"
	const TCHAR *	token2,			// I - the 2nd token, probably "(" 
	const TCHAR *	token3,			// I - the 3rd token, probably the conditional string - eg, "PRESENTATION"
	const TCHAR *	token4			// I - the 4th token, probably ")" 
)
{
	int		match_pos ;
	bool	got_match = false ;
	CString	match_token ;

	while ( !got_match && -1 != ( match_pos = actual.Find( token1, start_pos ) ) )
	{
		CString	twofer ;

		twofer = CString( token1 ) + token2 ;

		if ( actual.Mid( match_pos, twofer.GetLength( ) ) == twofer || actual.Tokenize( _T( " \n\t\r" ), match_pos ) == token1 ) 
		{
			begin_sequence = match_pos ;



		}
	}

	return got_match ;
}









/******************************************************************************
ReturnDelimitedContent

	Searches for the specified delimiters and attempts to extract the string content between them. 
	Returns true if a match for the delimiters was found. The content extracted between them may
	still be zero length.... Unlike ReplaceDelimitedSubstring, this function returns ONLY the first 
	qualifying substring between delimiters, and r_delim may not be NULL 

******************************************************************************/
bool ReturnDelimitedContent( 
	const CString & text,				// I - text to search 
	const TCHAR * l_delim,				// I - left delimiter
	const TCHAR * r_delim,				// I - right delimiter 
	CString & content					// O - the extracted content 
)
{
	int		lpos ;
	int		rpos ;
	bool	found_one = false ;

	ASSERT( l_delim != NULL && r_delim != NULL ) ;
	ASSERT( _tcslen( l_delim ) > 0 && _tcslen( r_delim ) > 0 ) ;

	content.Empty( ) ;

	if ( -1 != ( lpos = text.Find( l_delim ) ) )
	{
		if ( -1 != ( rpos = text.Find( r_delim, lpos + 1 ) ) )
		{
			content = text.Mid( lpos + _tcslen( l_delim ), rpos - lpos - _tcslen( l_delim ) ) ;
			found_one = true ;
		}
	}

	return found_one ;
}




/******************************************************************************
ReplaceDelimitedContent

	Similar to ReplaceDelimitedString, but only replaces the content between the 
	delimiters, leaving the delimiters in place, and only matches the first
	delimited instance 

******************************************************************************/
bool ReplaceDelimitedContent( 
	CString & text,						// I/O - string to work on 
	const TCHAR * l_delim,				// I - left delimiter
	const TCHAR * r_delim,				// I - right delimiter 
	const TCHAR * repl,					// I - replacement content 
	int start_pos,						// I - start position to search (default 0)
	int * follow_pos					// O - optionally get 1st character following closing delimiter (default NULL) 
)
{
	int		lpos ;
	int		rpos ;
	bool	found_one = false ;

	ASSERT( l_delim != NULL && r_delim != NULL && repl != NULL ) ;
	ASSERT( _tcslen( l_delim ) > 0 && _tcslen( r_delim ) > 0 ) ;

	if ( -1 != ( lpos = text.Find( l_delim, start_pos ) ) )
	{
		if ( -1 != ( rpos = text.Find( r_delim, lpos + 1 ) ) )
		{
			text = text.Left( lpos + _tcslen( l_delim ) ) + CString( repl ) + text.Mid( rpos ) ;

			// does caller want the offset of the 1st character AFTER the delimited pair? 
			if ( follow_pos )
				*follow_pos = lpos + _tcslen( l_delim ) + _tcslen( repl ) + _tcslen( r_delim ) ;

			found_one = true ;
		}
	}

	return found_one ;
}


/******************************************************************************
CleanupConditionalExpressions

	Delete every instance of $IF(<conditional_id>) and $ENDIF(<conditional_id>),
	but leave the enclosed content, whether it's there or not. 

	This is NOT the same as DeleteConditionalExpressions( ). This function 
	deletes the $IF( ) and $ENDIF( ) pair, but LEAVES anything in-between intact. 

******************************************************************************/
bool CleanupConditionalExpressions( 
	CString & actual,							// I/O - string with the HTML resource 
	const TCHAR * conditional_id				// I - identifier used in conditional 
) 
{
	bool	found_match = false ;
	CString	if_sequence ;
	CString endif_sequence ;

	if_sequence.Format( _T( "$IF(%s)" ), conditional_id ) ;
	endif_sequence.Format( _T( "$ENDIF(%s)" ), conditional_id ) ;

	if ( actual.Replace( if_sequence, _T( "" ) ) )
		found_match = true ;

	if ( actual.Replace( endif_sequence, _T( "" ) ) )
		found_match = true ;

	return found_match ;
}




/******************************************************************************
DeleteConditionalExpressions

	Delete all conditionally compiled HTML based on the identifier specified
	by conditional. 

	Not the same as CleanupConditionExpressions( ), sine this deletes 
	the $IF()/$ENDIF() pair and EVERYTHING in between... 

******************************************************************************/
bool DeleteConditionalExpressions( 
	CString & actual,						// I/O - the big-ass string with the HTML resource 
	const TCHAR * conditional				// I - the conditional's identifier 
)
{
	bool	found_match = false ;
	int		begin_if ;
	int		begin_endif ;
	int		start_pos ;
	CString	if_sequence ;
	CString endif_sequence ;

	if_sequence.Format( _T( "$IF(%s)" ), conditional ) ;
	endif_sequence.Format( _T( "$ENDIF(%s)" ), conditional ) ;

	start_pos = 0 ;
	while ( -1 != ( begin_if = actual.Find( if_sequence, start_pos ) ) && -1 != ( begin_endif = actual.Find( endif_sequence, begin_if ) ) )
	{
		actual = actual.Left( begin_if ) + actual.Mid( begin_endif + endif_sequence.GetLength( ) ) ;
		start_pos = begin_if ;
		found_match = true ;
	}

	return found_match ;
}





/******************************************************************************
CreateContactSheets

	Parses the contact sheet template in the resource and generate the contact 
	sheet(s) for the disk 

******************************************************************************/
void CreateContactSheets( 
	CMakePhotoDisk3Doc * doc			// I - document object 
)
{
	CString	lead_in ;
	CString	top_page_bar ;
	CString	table_start ;
	CString	row_template ;
	CString	table_end ;
	CString	bottom_page_bar ;
	CString follow_on ;

	ParseContactTemplate( lead_in, top_page_bar, table_start, row_template, table_end, bottom_page_bar, follow_on ) ;
	OutputContactSheets( doc, lead_in, top_page_bar, table_start, row_template, table_end, bottom_page_bar, follow_on ) ;
	TouchStartPage( doc ) ;
}









/******************************************************************************
ParseContactTemplate

	Load the resource with the template for a contact sheet and break it into
	its constituent pieces-parts, based on embedded delimiters - 

	lead_in			HTML stuff at the start of the page
	top_page_bar	HTML where we optionally output links for other pages by number
	table_start		HTML following page bar but before a page row
	row_template	HTML to contain a row of pics, which can be repeated to fill out the contact sheet
	table_end		HTML to finish up the table
	bottom_page_bar	HTML for the optional links for other pages by number 
	follow_on		HTML needed to round out the HTML for the page after the page bar

******************************************************************************/
void ParseContactTemplate( 
	CString & lead_in,					// O - gets portion of page preceding <!--pagebar> 
	CString & top_page_bar,				// O - gets portion after <!--pagebar> up to <!--/pagebar>
	CString & table_start,				// O - gets next chunk, up to line with first <!--picrow> 
	CString & row_template,				// O - gets portion of table corresponding to a row (will be repeated as required in output file)
	CString & table_end,				// O - gets end of table 
	CString & bottom_page_bar,			// O - gets bottom page bar 
	CString & follow_on					// O - gets everything after page bar 
) 
{
	HRSRC	prototype_resource_hdl ;
	bool	isOK = false ;	
	
	if ( prototype_resource_hdl = FindResource( AfxGetInstanceHandle( ), MAKEINTRESOURCE( IDR_CONTACT_SHEET ), RT_HTML ) ) 
	{
		HGLOBAL	res_hdl ;
		
		if ( res_hdl = LoadResource( AfxGetInstanceHandle( ), prototype_resource_hdl ) )
		{
			char * ptr ;
			
			if ( ptr = (char *) LockResource( res_hdl ) )
			{
				CString		resource ;
				CString		in_line ;

				resource.Format( _T( "%S" ), ptr ) ;		// convert to UNICODE 

				// get the header before the page listing 
				while ( ReadResourceLine( resource, in_line ) )
				{
					in_line += TEXT( "\n" ) ;
					
					if ( -1 != in_line.Find( TEXT( "<!--pagebar-->" ) ) )
					{
						isOK = true ;
						break ;
					}
					lead_in += in_line ;
				}

				// now get the end of the "pagebar" section - 
				do 
				{
					top_page_bar += in_line ;
					top_page_bar += TEXT( "\n" ) ;
					
					if ( -1 != in_line.Find( TEXT( "<!--/pagebar-->" ) ) )
					{
						break ;
					}
				}
				while ( ReadResourceLine( resource, in_line ) ) ;


				// the start of the table consists of everything after the pagebar up to <!--picrow-->
				while ( ReadResourceLine( resource, in_line ) )
				{
					if ( -1 != in_line.Find( TEXT( "<!--picrow-->" ) ) )
					{
						break ;		// we're done - this is the start of the row 
					}
					table_start += in_line ;
					table_start += TEXT( "\n" ) ;
				}

				// row_template starts witht he row with <!--picrow--> 
				row_template = in_line ;

				if ( isOK )
				{
					isOK = false ;
					
					while ( ReadResourceLine( resource, in_line ) )
					{
						in_line += TEXT( "\n" ) ;
						
						row_template += in_line ;
						if ( -1 != in_line.Find( TEXT( "<!--/picrow-->" ) ) )
						{
							isOK = true ;
							break ;
						}
					}
				}

				// now get the bottom of the table 
				while ( ReadResourceLine( resource, in_line ) )
				{
					if ( -1 != in_line.Find( TEXT( "<!--pagebar-->" ) ) )
					{
						break ;
					}					
					
					table_end += in_line ;
					table_end += TEXT( "\n" ) ;
				}

				// now get the optional bottom page bar template 
				do 
				{
					bottom_page_bar += in_line ;
					bottom_page_bar += TEXT( "\n" ) ;
					
					if ( -1 != in_line.Find( TEXT( "<!--/pagebar-->" ) ) )
					{
						break ;
					}
				}
				while ( ReadResourceLine( resource, in_line ) ) ;

				// now get the follow-on 
				while ( ReadResourceLine( resource, in_line ) )
				{
					in_line += TEXT( "\n" ) ;
					follow_on += in_line ;
				}

				UnlockResource( res_hdl ) ;
			}
		}
	}
}





/******************************************************************************
ReadResourceLine

	Given a CString containing the entire resource file, this peels off the 
	first full line up to the newline

******************************************************************************/
static bool ReadResourceLine( 
	CString & resource,					// I/O - CString containing entire resource file 
	CString & line						// O - gets next line of resource 
) 
{
	int		newline_pos ;

	if ( -1 != ( newline_pos = resource.Find( _T( '\n' ) ) ) )
	{
		line = resource.Left( newline_pos ) ;
		resource = resource.Mid( newline_pos + 1 ) ;

		return true ;
	}
	return false ;
}







/******************************************************************************
OutputContactSheets

	Generate the HTML for the contact-sheet pages using the parts parsed from
	the resource by ParseContactTemplate( ). 

	Each contact page consists of invariant "lead_in" followed by the by a 
	"page bar" which is formatted as a list of page numbers which are HTML 
	links directly to the corresponding contact page. 

	Then an invariant section "table_start" which gets us to the first row.

	Then "row_template" is output a number of times updated to reference the
	thumbnail for each photo and a link to its presentation page. No page 
	contains more than 40 photographs. 

	Following the row content, we have "table_end" to fill out the table. 

	Then "bottom_page_bar" is output with page numbers and HTML links directly
	to the other contact pages. 

	Then the "follow_on" to close out the page. 

******************************************************************************/
void OutputContactSheets( 
	CMakePhotoDisk3Doc * doc,			// I - the document object
	const CString & lead_in,			// I - the lead-in portion of the template
	const CString & top_page_bar,		// I - the top "page bar" portion of the template
	const CString & table_start,		// I - the stuff between the page bar and the 1st row 
	const CString & row_template,		// I - template for a row
	const CString & table_end,			// I - stuff after the last row but before the bottom "page bar" 
	const CString & bottom_page_bar,	// I - the bottom "page bar" 
	const CString & follow_on			// I - everything after the "page bar" 
)
{
	PhotoImage *	photo ;
	POSITION		pos ;
	bool			isOK = false ;
	int				cur_page ;
	int				num_pages ;

	// NOTE: if you change any logic dealing with the max number of photos per page, 
	// you also need to check DetermineIndexFileName( ) 
	if ( doc->GetPhotoListSize( ) > 40 ) 
	{
		num_pages = ( doc->GetPhotoListSize( ) + 39 ) / 40 ;
	}
	else
		num_pages = 1 ;

	photo = doc->GetFirstPhoto( pos ) ;


	for ( cur_page = 1 ; cur_page <= num_pages ; cur_page++ )
	{
		CString			output_text ;		// gets the entire page's HTML before writing it to a file at end of the page loop 
		CString			page_name ;

		page_name.Format( TEXT( "%s\\pages\\index%d.htm" ), doc->GetProjectPath( ), cur_page ) ;

		output_text = lead_in ;

		CString	subdir ;

		// output the cur_page bar - if necessary
		OutputPageBar( output_text, top_page_bar, cur_page, num_pages ) ;

		// now output the table start - 
		output_text += table_start ;
		
		int			i ;
		int			col_ct ;
		CString		output_row ;
		int			start_picture ;
		int			end_picture ;
		CString		picnumb ;

		// determine what pictures go on this page... 
		start_picture = ( cur_page - 1 ) * 40 ;
		if ( cur_page == num_pages )
			end_picture = doc->GetPhotoListSize( ) ;
		else
			end_picture = start_picture + 40 ;

		// format rows until we've filled out the contents of this page 
		for ( i = start_picture, col_ct = 0 ; i < end_picture ; i++, photo = doc->GetNextPhoto( pos ) )
		{
			CString tmp ;
			CString	relative_path_to_presentation ;
			CString	relative_path_to_thumbnail ;

			relative_path_to_presentation.Format( _T( "pic%d.htm" ), i + 1 ) ;
			relative_path_to_thumbnail.Format( _T( "thumbnail\\%d.jpg" ), photo->GetNumberOnDisk( ) ) ;
			tmp.Format( _T( "%d" ), i + 1 ) ;

			if ( 0 == col_ct )
				output_row = row_template ;

			ReplaceFirstInstance( output_row, _T( "<!--picfield-->" ), _T( "" ) ) ;
			ReplaceFirstInstance( output_row, _T( "<!--/picfield-->" ), _T( "" ) ) ;
			ReplaceFirstInstance( output_row, _T( "target.jpg" ), relative_path_to_presentation ) ;
			ReplaceFirstInstance( output_row, _T( "thumbfile.jpg" ), relative_path_to_thumbnail ) ;
			ReplaceFirstInstance( output_row, _T( ";thumbnumber;" ), tmp ) ;

			col_ct++ ;
		
			if ( 4 == col_ct )
			{
				output_text += output_row ;
				col_ct = 0 ;
			}
		}

		if ( col_ct )
		{
			ReplaceDelimitedSubstring( output_row, TEXT( "<!--picfield-->" ), TEXT( "<!--/picfield-->" ), TEXT( "" ) ) ;
			output_text += output_row ;
		}

		output_text += table_end ;

		// output the cur_page bar - if necessary
		OutputPageBar( output_text, bottom_page_bar, cur_page, num_pages ) ;

		// write out the file for this page 
		SaveUTF8File( page_name, output_text ) ;
	}
}






/******************************************************************************
ReplaceFirstInstance

	Replace only the first instance of a target with a substition. The 
	CString Replace method replaces all instances of the target with the 
	substitution text 

******************************************************************************/
void ReplaceFirstInstance( 
	CString & work,						// I/O - string to modify 
	const TCHAR * target,				// I - target to search for 
	const TCHAR * substitution			// I - substitute text 
) 
{
	int	match ;

	if ( -1 != ( match = work.Find( target ) ) )
	{
		if ( NULL == substitution )
			work = work.Left( match ) + work.Mid( match + _tcslen( target ) ) ;
		else
			work = work.Left( match ) + CString( substitution ) + work.Mid( match + _tcslen( target ) ) ;
	}
}





/******************************************************************************
OutputPageBar

	Output the little row of numbers which link to the other contact sheets in the disk.

******************************************************************************/
void OutputPageBar( 
	CString & output_text,					// O - gets the formatted text 
	const CString & page_bar_master,		// I - provides the template for the list 
	int cur_page,							// I - currently active page 
	int num_pages							// I - number of pages 
)
{
	if ( num_pages > 1 )
	{
		CString	page_bar = page_bar_master ;
		CString	entry ;
		CString	pagefields ;				// gets HTML for the list of numbers and links 
		
		int		link_page ;
		
		for ( link_page = 1 ; link_page <= num_pages ; link_page++ )
		{
			if ( link_page != cur_page )
				entry.Format( TEXT( "<a href=\"index%d.htm\">%d</a>" ), link_page, link_page ) ;
			else
				entry.Format( TEXT( "<b>%d</b>" ), link_page ) ;
		
			pagefields += entry ;
			pagefields += TEXT( "  " ) ;
		}

		// insert the list of page links into page_bar, a copy of the page bar master 
		ReplaceDelimitedSubstring( page_bar, TEXT( "<!--pagefields-->" ), NULL, pagefields ) ;
		output_text += page_bar ;
	}
}





/******************************************************************************
TouchStartPage

	Update some fields in the start page, including revision number, disk title
	current year, create date, and MakePhotoDisk build number 

******************************************************************************/
void TouchStartPage( 
	CMakePhotoDisk3Doc * doc				// I - the document object 
)
{
	CString		start_page_filename ;
	CString		start_page_backup ;
	CString		start_page_contents ;
	CFile		start_page ;

	start_page_filename.Format( _T( "%s\\StartPage.htm" ), doc->GetProjectPath( ) ) ;

	if ( LoadUTF8File( start_page_filename, start_page_contents ) )
	{
		SYSTEMTIME		current_time ;
		CString			work ;
		int				major_ver ;
		int				minor_ver ;

		GetLocalTime( &current_time ) ;

	// I'm perfectly happy with the "unsafe" version of stscanf... 
#pragma warning( disable: 4996 ) 

		// update the revision number. We're looking for something like this: <!--revnum-->revision 0.01<!--/revnum-->
		ReturnDelimitedContent( start_page_contents, _T( "<!--revnum-->" ), _T( "<!--/revnum-->" ), work ) ;
		work.MakeLower( ) ;
		_stscanf( work, _T( "revision %d.%d" ), &major_ver, &minor_ver ) ;
		minor_ver++ ;
		work.Format( _T( "revision %d.%d" ), major_ver, minor_ver ) ;
		ReplaceDelimitedContent( start_page_contents, _T( "<!--revnum-->" ), _T( "<!--/revnum-->" ), work ) ;

		// update the disk title. <!--disktitle-->disk title<!--/disktitle-->
		work = doc->GetDiskTitle( ) ;
		ReplaceDelimitedContent( start_page_contents, _T( "<!--disktitle-->" ), _T( "<!--/disktitle-->" ), work ) ;

		// update the copyright year	<!--currentyear-->2011<!--/currentyear-->
		work.Format( _T( "%d" ), current_time.wYear ) ;
		ReplaceDelimitedContent( start_page_contents, _T( "<!--currentyear-->" ), _T( "<!--/currentyear-->" ), work ) ;

		// update the create date:   <!--createdate-->04-16-2011<!--/createdate--> 
		work.Format( _T( "%02.2d-%02.2d-%d" ), current_time.wMonth, current_time.wDay, current_time.wYear ) ;
		ReplaceDelimitedContent( start_page_contents, _T( "<!--createdate-->" ), _T( "<!--/createdate-->" ), work ) ;

		/*
		* update the MakePhotoDisk3 buildno -		<!--buildno-->09-22-2010 12:39:10<!--/buildno-->
		*
		*	Since the days of MakePhotoDisk 1, we've actually used the modified time of our .exe file, rather than
		*	the predefined __DATE__ and __TIME__ macros, since I don't routinely do clean builds, but I could switch
		*	to those macros and switch to clean rebuilds
		*/
		WIN32_FILE_ATTRIBUTE_DATA	attr_data ;
		TCHAR						exe_name[ 1024 ] ;
		
		// determine the build date by checking the file modify time on our .exe 
		if ( GetModuleFileName( NULL, exe_name, sizeof( exe_name ) / sizeof( exe_name[ 0 ] ) ) )
		{
			if ( GetFileAttributesEx( exe_name, GetFileExInfoStandard, &attr_data ) )
			{
				FILETIME	build_time ;
				SYSTEMTIME	build_systime ;
				
				FileTimeToLocalFileTime( &attr_data.ftLastWriteTime, &build_time ) ;
				FileTimeToSystemTime( &build_time, &build_systime ) ;

				work.Format( _T( "%02.2d-%02.2d-%02.2d %02.2d:%02.2d:%02.2d" ), build_systime.wMonth, build_systime.wDay, build_systime.wYear, build_systime.wHour, build_systime.wMinute, build_systime.wSecond ) ;
				ReplaceDelimitedContent( start_page_contents, _T( "<!--buildno-->" ), _T( "<!--/buildno-->" ), work ) ;
			}
		}

		// now - copy the current .htm file to a backup file and save the new content 
		start_page_backup.Format( _T( "%s\\StartPage.bak" ), doc->GetProjectPath( ) ) ;

		if ( MoveFileEx( start_page_filename, start_page_backup, MOVEFILE_REPLACE_EXISTING ) ) 
			SaveUTF8File( start_page_filename, start_page_contents ) ;
	}
}

