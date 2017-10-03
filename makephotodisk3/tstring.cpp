/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



A simple string class emulating most functionality of CString, with the principal 
exception that it does not use reference-counting

***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>


#include "tstring.h"


// we don't get the standard AFX/MFC ASSERT macro...
#ifdef DEBUG

#define	ASSERT( x )		if ( !(x) ) \
						{	\
							fprintf( stderr, "Assertion failed, %s, in %s, line %d\n", #x, __FILE__, __LINE__ ) ;	\
							exit( -1 ) ;		\
						} ;

#else

#define ASSERT( x )		

#endif // DEBUG


#ifndef __min

#define __min( x, y )		( (x) < (y) ? (x) : (y) )

#endif



/******************************************************************************
tstring constructors - 

******************************************************************************/


tstring::tstring( ) 
{
	m_data = NULL ;
	m_length = 0 ;
}



tstring::tstring( const TCHAR * ccstr ) 
{
	copydata( ccstr ) ;
}



tstring::tstring( const TCHAR cchar )
{
	m_length = 1 ;
	m_data = new TCHAR [ 2 ] ;
	m_data[ 0 ] = cchar ;
	m_data[ 1 ] = _T( '\0' ) ;
}



tstring::tstring( const tstring & other )
{
	copydata( other ) ;
}



/******************************************************************************
PRIVATE MEMBER

******************************************************************************/
void tstring::copydata( const TCHAR * ccstr ) 
{
	int	len ;

	if ( ccstr && ( len = (int) _tcslen( ccstr ) ) )
	{
		m_length = len ;
		m_data = new TCHAR[ m_length + 1 ] ;
		_tcscpy( m_data, ccstr ) ;
	}
	else
	{
		m_length = 0 ;
		m_data = NULL ;
	}
}



/******************************************************************************
PRIVATE MEMBER

******************************************************************************/
void tstring::copydata( const tstring & other ) 
{
	if ( ( m_length = other.m_length ) )
	{
		m_data = new TCHAR[ m_length + 1 ] ;
		memcpy( m_data, other.m_data, sizeof( TCHAR ) * ( m_length + 1 ) ) ;
	}
	else
	{
		m_data = NULL ;
	}
}






tstring::~tstring( )
{
	delete [] m_data ;
}











/******************************************************************************
operator=

	Assignment operator for const TCHAR * 

******************************************************************************/
tstring & tstring::operator=( const TCHAR * ccstr ) 
{
	if ( m_length ) 
		delete [] m_data ;

	copydata( ccstr ) ;		// copydata will handle NULL 

	return *this ;
}




/******************************************************************************
operator=

	Assignment operator when passed another tstring. Could rely on const TCHAR *
	cast + the other assign operator, but this is marginally quicker with a 
	tstring source 

******************************************************************************/
tstring & tstring::operator=( const tstring & other )
{
	if ( this != &other ) 
	{
		if ( m_length )
			delete [] m_data ;

		copydata( other ) ;
	}

	return *this ;
}




/******************************************************************************
operator const TCHAR * 

	If tstring is empty, returns a pointer to a zero-length string, rather than
	a NULL 

******************************************************************************/
tstring::operator const TCHAR * ( ) const 
{
	if ( m_data ) 
		return (const TCHAR *) m_data ;
	else
		return _T( "" ) ;
}




/******************************************************************************
tstring::GetLength

	returns length, in characters 

******************************************************************************/
int tstring::GetLength( ) const
{
	return m_length ;
}



/******************************************************************************
tstring::Empty

	Named for MFC CString compatibility. Not to be confused with IsEmpty 

******************************************************************************/
void tstring::Empty( )
{
	if ( m_length ) 
		delete [] m_data ;
	m_data = NULL ;
	m_length = 0 ;
}



/******************************************************************************
******************************************************************************/
bool tstring::IsEmpty( ) const
{
	return 0 == m_length ;
}






/******************************************************************************
tstring::Left

	Return a tstring containing the first 'len' characters of the string, or
	the entire string if 'len' is greater than the string's length. 

******************************************************************************/
tstring tstring::Left( int len ) const
{
	tstring	work ;

	if ( len >= m_length ) 
		work = *this ;
	else if ( len > 0 )
	{
		work.m_length = len ;
		work.m_data = new TCHAR[ len + 1 ] ;
		memcpy( work.m_data, m_data, len * sizeof( TCHAR ) ) ;
		work.m_data[ len ] = _T( '\0' ) ;
	}
	// else - work alrady constructed as an empty tstring 
		
	return work ;
}






/******************************************************************************
tstring::Mid

	'start' is 0-based index into the string. 

******************************************************************************/
tstring tstring::Mid( int start, int len /* =INT_MAX */ ) const 
{
	tstring work ;

	if ( start < 0 ) 
		start = 0 ;		// verified - this replicates CString's behavior. 

	if ( len < 0 )
		len = 0 ;		// verified - this replicates CString's behavior. 

	if ( len > m_length ) 
		len = m_length ;		// avoid out-of-bounds results if default INT_MAX or other huge values is passed in 

	if ( len > 0 && start < m_length ) 
	{
		// if start is the index of the first character of the new string, start + len is the index of the new '\0', UNLESS it's off the end of the string.... 
		if ( start + len > m_length ) 
			len = m_length - start ;

		work.m_length = len ;
		work.m_data = new TCHAR [ work.m_length + 1 ] ;
		// pointer math in 2nd argument automatically accounts for sizeof TCHAR, but need to manually scale 3rd argument up, and need +1 to copy the terminating '\0' 
		memcpy( work.m_data, m_data + start, sizeof( TCHAR) * len ) ;
		work.m_data[ len ] = _T( '\0' ) ;
	}
	// else - just return the empty string the default constructor set up in 'work'

	return work ;
}









/******************************************************************************
tstring::Right

	return the 'len' last characters of the string 

******************************************************************************/
tstring tstring::Right( int len ) const 
{
	tstring work ;

	if ( len >= m_length ) 
		work = *this ;
	else if ( len > 0 )
	{
		work.m_length = len ;
		work.m_data = new TCHAR[ len + 1 ] ;
		// DO NOT multiply anything in 2nd argument by sizeof( TCHAR ) b/c the compiler automatically does the necessary pointer math, but DO need to multiply len by sizeof( TCHAR )
		memcpy( work.m_data, m_data + m_length - len, sizeof( TCHAR ) * ( len + 1 ) ) ;
	}

	return work ;
}













/******************************************************************************
tstring::FormatV

	Like CString::FormatV. A public member, but also used by tstring::Format 

******************************************************************************/
void tstring::FormatV( const TCHAR * fmt, va_list arg_ptr ) 
{
	TCHAR *		buffer ;
	int			print_result ;
	va_list		arg_ptr_backup ;		// vsnprintf destroys the va_list arg, so we need to backup and restore a copy of the va_list to call vsnprintf again 

	delete [] m_data ;
	m_length = 0 ;

	// make a "backup" copy of the original arg_ptr, which we will use to reset arg_ptr for a 2nd+ call to vsnprintf 
	va_copy( arg_ptr_backup, arg_ptr ) ;

	/*
	*	There is some major suckage involving using variations of the vsnprintf function
	*	on Windows, because Microsoft is inconsistent with UNIX implementation of the 
	*	function. 
	*
	*	With MS, the function returns -1 if the supplied buffer is too small, and you just have
	*	to guess at how large a buffer is needed, and retry.... but on the Mac, the return value 
	*   is the number of characters that would be required for the full result. 
	*/

#ifdef WIN32
	size_t		initial_buffer_size ;
	size_t		buffer_size ;

	initial_buffer_size = _tcslen( fmt ) + INITIAL_ARGUMENT_GUESS ;
	buffer_size = initial_buffer_size ;
	buffer = new TCHAR[ buffer_size + 1 ] ;

	do
	{
		print_result = _vsntprintf( buffer, buffer_size, fmt, arg_ptr ) ;

		if ( -1 == print_result ) 
		{
			va_copy( arg_ptr, arg_ptr_backup ) ;

			buffer_size += initial_buffer_size ;
			delete [] buffer ;
			buffer = new TCHAR[ buffer_size + 1 ] ;
		}
		else 
		{
			ASSERT( print_result < buffer_size ) ;
			buffer[ print_result ] = _T( '\0' ) ;	// MS documentation: "the return value does NOT include the terminating null if one is written", hence it's the string length 
		}
	}
	while ( -1 == print_result ) ;

	m_length = print_result ;
	m_data = new TCHAR[ print_result + 1 ] ;
	memcpy( m_data, buffer, sizeof( TCHAR ) * print_result ) ;	// it's possible there is no nul termination, if print_result exactly equals buffer_size
	m_data[ print_result + 1 ] = _T( '\0' ) ;

	delete [] buffer ;

#else

	// try with a zero buffer just to get the buffer size required -  
	print_result = _vsntprintf( NULL, 0, fmt, arg_ptr ) ;
	va_copy( arg_ptr, arg_ptr_backup ) ;

    buffer = new TCHAR[ print_result + 1 ] ;

	if ( print_result != _vsntprintf( buffer, print_result + 1, fmt, arg_ptr ) )
		ASSERT( FALSE ) ;

	buffer[ print_result ] = _T( '\0' ) ;
	m_data = buffer ;
	m_length = print_result ;

#endif
}


/******************************************************************************
tstring::Format

	Format string using printf-style format specifications and variable args 

******************************************************************************/
void tstring::Format( const TCHAR * fmt, ... ) 
{
	va_list		arg_ptr ;

	va_start( arg_ptr, fmt ) ;
	FormatV( fmt, arg_ptr ) ;
}





/******************************************************************************
tstring::Find

	find the first instance of chr in string, starting with zero-relative start_pos.
	Returns -1 if no instance of chr is found 

******************************************************************************/
int tstring::Find( TCHAR chr, int start_pos /* =0 */ ) const 
{
	int i ;

	for ( i = start_pos ; i < m_length ; i++ )
		if ( m_data[ i ] == chr ) 
			return i ;

	return -1 ;
}



/******************************************************************************
tstring::Find

	find the first instance of substring str in string, starting with zero-relative 
	start_pos. Returns -1 if no instance of the substr is found 

******************************************************************************/
int tstring::Find( const TCHAR * str, int start_pos /* =0 */ ) const 
{
	int i ;
	int mlen = (int) _tcslen( str ) ;

	for ( i = start_pos ; i <= m_length - mlen ; i++ )
		if ( m_data[ i ] == str[ 0 ] ) 
			if ( 0 == memcmp( m_data + i, str, sizeof( TCHAR ) * mlen ) ) 
				return i ;

	return -1 ;
}







/******************************************************************************
tstring::ReverseFind

	Search for the last instance of character 'chr' in string 

******************************************************************************/
int tstring::ReverseFind( TCHAR chr ) const
{
	int i ;

	for ( i = m_length - 1 ; i >= 0 ; i-- )
		if ( m_data[ i ] == chr ) 
			return i ;

	return -1 ;
}




/******************************************************************************
tstring::ReverseFind
	
	CString functionality does not include ReverseFind taking a substring 

******************************************************************************/
int tstring::ReverseFind( const TCHAR * str ) const
{
	int i ;
	int mlen = (int) _tcslen( str ) ;

	for ( i = m_length - mlen ; i >= 0 ; i-- ) 
	{
		if ( 0 == memcmp( m_data + i, str, sizeof( TCHAR ) * mlen ) )
			return i ;
	}
	return -1 ;
}







/******************************************************************************
tstring::Replace

	This function DOES make multiple substitutions but does NOT make substitutions recursively, for example, 
	given calling tstring::Replace( _T( "//" ), _T( "/" ) ) will change "////" to "//", not "/"

******************************************************************************/
int tstring::Replace( const TCHAR * old_string, const TCHAR * new_string ) 
{
	int sub_count = 0 ;
	int cur_pos = 0 ;
	int sub_pos ;
	int sub_len = (int) _tcslen( old_string ) ;

	while ( -1 != ( sub_pos = Find( old_string, cur_pos ) ) )
	{
		*this = Left( sub_pos ) + new_string + Mid( sub_pos + sub_len ) ;
		sub_count++ ;
	}

	return sub_count ;
}









/******************************************************************************
GetBufferSetLength( ) 

	The CString implementation of this seems a little ill-conceived. If you ask for a buffer 
	larger than the existing string content, you get the buffer with the smaller 0-terminated
	string at the beginning, and whatever memory-garbage alloc returns filling out the 
	balance of the buffer. But a call to CString::GetLength returns the newly allocated 
	length, as does CString::GetAllocLength.

	If you use ANY CString member functions before releasing the returned TCHAR ptr, the 
	memory referenced by ptr is released. Appending a new string to the CString will append
	the new material following the garbage memory... leaving the original contents intact, 
	including any embeded nuls. 

	for example, representing \0 with '.' and ? representing uninitialized memory: 

	CString test ;
	char * ptr ;

	test = "this is a string!" ;
	ptr = test.GetBufferSetLength( 80 ) ;
	
	// results in ptr referencing 
	"this is a string!.????????????????????????????????????????????????????????????."

	test.GetLength( ) returns 80
	test.GetAllocLength( ) returns 80 

	test += "this too!" ;

	// ptr now references freed memory and must not be used, meanwhile test contains

	"this is a string!.????????????????????????????????????????????????????????????this too!."




******************************************************************************/
TCHAR * tstring::GetBufferSetLength( int new_length )
{
	if ( new_length != m_length ) 
	{
		TCHAR * buffer ;
		int		copy_ct ;

		copy_ct = __min( m_length, new_length ) ;

		// need to set up tstring's buffer to accomodate up to 'new_length' characters, but whether that is bigger or smaller than the current buffer, make sure it's '\0' terminated 
		buffer = new TCHAR[ new_length + 1 ] ;
		memcpy( buffer, m_data, sizeof( TCHAR ) * copy_ct ) ;
		buffer[ copy_ct ] = _T( '\0' ) ;

		if ( new_length > copy_ct ) 
			buffer[ new_length ] = _T( '\0' ) ;

		delete [] m_data ;
		m_data = buffer ;
		m_length = new_length ;
	}

	return m_data ;
}






/******************************************************************************
tstring::ReleaseBuffer

	Should be called after you're finished with the pointer returned by 
	GetBufferSetLength( ). 

******************************************************************************/
void tstring::ReleaseBuffer( int length /* =-1 */ )
{
	if ( length < 0 )
	{
		ASSERT( length == -1 ) ;
		m_data[ m_length ] = _T( '\0' ) ;	// make sure the string is 0-terminated.... 

		for ( length = 0 ; m_data[ length ] != _T( '\0' ) ; length++ )
			;
		// ends with length equal to the actual length of the string up to the first \0 
	}

	if ( length > 0 ) 
	{
		if ( length != m_length )
		{
			TCHAR * buffer ;
	
			buffer = new TCHAR[ length + 1 ] ;

			if ( length > m_length ) 
			{
				// they've asked us to make the buffer BIGGER than the existing string - guaranteeing an embedded nul. 
				memcpy( buffer, m_data, sizeof( TCHAR ) * m_length ) ;
				buffer[ m_length ] = _T( '\0' ) ;
			}			
			else
				memcpy( buffer, m_data, sizeof( TCHAR ) * length ) ;

			buffer[ length ] = _T( '\0' ) ;

			delete [] m_data ;
			m_data = buffer ;
			m_length = length ;
		}
	}
	else
	{
		delete [] m_data ;
		m_data = NULL ;
		m_length = 0 ;
	}
}


/******************************************************************************
tstring::ReleaseBufferSetLength

	Exists only to duplicate CString functionality

******************************************************************************/
void tstring::ReleaseBufferSetLength( int length )
{
	ASSERT( length >= 0 ) ;
	ReleaseBuffer( length ) ;
}




/******************************************************************************
tstring::MakeUpper

	Uppercases all alphabetic characters in tstring. Since strings can have 
	embedded nuls after using GetBufferSetLength / ReleaseBuffer, we do not 
	simply stop at the first nul. This is slightly different behavior than 
	CString, which will stop at the first nul

******************************************************************************/
tstring & tstring::MakeUpper( ) 
{
	int i ;

	for ( i = 0 ; i < m_length ; i++ )
	{
		switch ( m_data[ i ] ) 
		{
			case _T( 'a' ) :
			case _T( 'b' ) :
			case _T( 'c' ) :
			case _T( 'd' ) :
			case _T( 'e' ) :
			case _T( 'f' ) :
			case _T( 'g' ) :
			case _T( 'h' ) :
			case _T( 'i' ) :
			case _T( 'j' ) :
			case _T( 'k' ) :
			case _T( 'l' ) :
			case _T( 'm' ) :
			case _T( 'n' ) :
			case _T( 'o' ) :
			case _T( 'p' ) :
			case _T( 'q' ) :
			case _T( 'r' ) :
			case _T( 's' ) :
			case _T( 't' ) :
			case _T( 'u' ) :
			case _T( 'v' ) :
			case _T( 'w' ) :
			case _T( 'x' ) :
			case _T( 'y' ) :
			case _T( 'z' ) :
				m_data[ i ] += ( _T( 'A' ) - _T( 'a' ) ) ;
				break ;
		}
	}

	return *this ;
}


/******************************************************************************
tstring::MakeLower

	Lowercases all alphabetic characters in string. See comment on MakeUpper. 

******************************************************************************/
tstring & tstring::MakeLower( ) 
{
	int	i ;

	for ( i = 0 ; i < m_length ; i++ )
	{
		switch ( m_data[ i ] ) 
		{
			case _T( 'A' ) :
			case _T( 'B' ) :
			case _T( 'C' ) :
			case _T( 'D' ) :
			case _T( 'E' ) :
			case _T( 'F' ) :
			case _T( 'G' ) :
			case _T( 'H' ) :
			case _T( 'I' ) :
			case _T( 'J' ) :
			case _T( 'K' ) :
			case _T( 'L' ) :
			case _T( 'M' ) :
			case _T( 'N' ) :
			case _T( 'O' ) :
			case _T( 'P' ) :
			case _T( 'Q' ) :
			case _T( 'R' ) :
			case _T( 'S' ) :
			case _T( 'T' ) :
			case _T( 'U' ) :
			case _T( 'V' ) :
			case _T( 'W' ) :
			case _T( 'X' ) :
			case _T( 'Y' ) :
			case _T( 'Z' ) :
				m_data[ i ] += ( _T( 'a' ) - _T( 'A' ) ) ;
				break ;
		}
	}

	return *this ;
}






/******************************************************************************
tstring::operator

	Concatenation operator 

******************************************************************************/
tstring & tstring::operator+=( const tstring & other ) 
{
	if ( other.m_length ) 
	{
		TCHAR *		buffer ;

		buffer = new TCHAR[ m_length + other.m_length + 1 ] ;
		memcpy( buffer, m_data, sizeof( TCHAR) * m_length ) ;
		memcpy( buffer + m_length, other.m_data, sizeof( TCHAR ) * other.m_length ) ;
		buffer[ m_length + other.m_length ] = _T( '\0' ) ;

		delete [] m_data ;

		m_data = buffer ;
		m_length += other.m_length ;
	}

	return *this ;
}



/******************************************************************************
tstring::operator+=
	
	concatenation operator 

******************************************************************************/
tstring & tstring::operator+=( const TCHAR * appendee ) 
{
	int	slen ;

	if ( appendee != NULL && 0 != ( slen = (int) _tcslen( appendee ) ) )
	{
		TCHAR * buffer ;

		buffer = new TCHAR[ m_length + slen + 1 ] ;
		memcpy( buffer, m_data, sizeof( TCHAR ) * m_length ) ;
		memcpy( buffer + m_length, appendee, slen * sizeof( TCHAR ) ) ;
		buffer[ m_length + slen ] = _T( '\0' ) ;
		delete [] m_data ;
		m_data = buffer ;
		m_length += slen ;
	}

	return *this ;
}








/******************************************************************************
tstring::Compare

	comparison function. Return value is:

		-1	this string lexically precedes other string 

		0	this string is lexially equivalent to other string 

		1	this string lexically follows the other string 

******************************************************************************/
int tstring::Compare( const TCHAR * other ) const
{
	int	stringlen = (int) _tcslen( other ) ;
	int	worklen = __min( stringlen, m_length ) ;
	int	cmp ;

	if ( 0 == ( cmp = _tcsncmp( m_data, other, worklen ) ) )
    {
		if ( m_length < stringlen )
        {
			cmp = -1 ;
        }
		else if ( m_length > stringlen )
        {
			cmp = 1 ;
        }
    }
    
	return cmp ;
}




/******************************************************************************
tstring::CompareNoCase

	case-insensitive compare. 

		-1	this string lexically precedes other string 

		0	this string is lexially equivalent to other string 

		1	this string lexically follows the other string 


******************************************************************************/
int tstring::CompareNoCase( const TCHAR * other ) const 
{
	int	stringlen = (int) _tcslen( other ) ;
	int	worklen = __min( stringlen, m_length ) ;
	int	cmp ;

	if ( 0 == ( cmp = _tcsnicmp( m_data, other, worklen ) ) )
    {
		if ( m_length < stringlen )
        {
			cmp = -1 ;
        }
		else if ( m_length > stringlen )
        {
			cmp = 1 ;
        }
    }
    
	return cmp ;
}





/******************************************************************************
tstring::Tokenize

	returns the first substring of tstring, following token_pos, delimited by
	any character in string specified by delimiters

******************************************************************************/
tstring tstring::Tokenize( const TCHAR * delimiters, int & token_pos ) 
{
	tstring	token ;

	if ( token_pos < 0 ) 
		token_pos = 0 ;

		// find the first character at token_pos or later which is neither a delimiter nor a \0 
	while ( token_pos < m_length && _T( '\0' ) != m_data[ token_pos ] && NULL != _tcschr( delimiters, m_data[ token_pos ] ) )
		token_pos++ ;

		// we're either at the beginning of a token, or the end of the string 
	if ( token_pos < m_length && _T( '\0' ) != m_data[ token_pos ] ) 
	{
		int	token_start = token_pos ;

			// scan now for the first delimter or the end of string 
		while ( token_pos < m_length && _T( '\0' ) != m_data[ token_pos ] && NULL == _tcschr( delimiters, m_data[ token_pos ] ) )
			token_pos++ ;

		token = Mid( token_start, token_pos - token_start ) ;
	}

	return token ;
}



/******************************************************************************
tstring::Trim

	trim leading and trailing whitespace 

******************************************************************************/
tstring & tstring::Trim( )
{
	TrimLeft( ) ;
	TrimRight( ) ;
	return *this ;
}





/******************************************************************************
tstring::TrimRight( )

	trim trailing whitepsace 

******************************************************************************/
tstring & tstring::TrimRight( )
{
	if ( m_length ) 
	{
		int i ;

		for ( i = m_length - 1 ; i >= 0 && _istspace( m_data[ i ] ) ; i-- )
			;

			// after the above loop, i is the index of the rightmost non-whitespace character 
		if ( i < m_length - 1 && i >= 0 ) 
		{
			TCHAR * buffer ;

			/*
				The ( i >= 0 ) condition proves that a non-whitespace character is found 
				...and ( i < m_length - 1 ) proves that this non-whitespace character is 
				not the last character of the string 

				At this point, i is the zero-based INDEX of the last non-whitespace 
				character, so the new LENGTH is i + 1 characters, and hence the allocation
				for m_data should be i + 2 TCHAR's 
			*/
			buffer = new TCHAR[ i + 2 ] ;
			memcpy( buffer, m_data, ( i + 1 ) * sizeof( TCHAR ) ) ;	
			buffer[ i + 1 ] = _T( '\0' ) ;
			delete [] m_data ;
			m_length = i + 1 ; 
			m_data = buffer ;
		}
		else if ( -1 == i ) 
		{
			// string is nothing but whitepsace 
			delete [] m_data ;
			m_length = 0 ;
		}
		// else, string has no trailing whitepsace 
	}

	return *this ;
}




/******************************************************************************
tstring::TrimLeft

	Trim leading whitespace

******************************************************************************/
tstring & tstring::TrimLeft( )
{
	int	i ;

	for ( i = 0 ; i < m_length && _istspace( m_data[ i ] ) ; i++ )
		;

	// i is the index of the leftmost non-whitespace character 
	if ( i > 0 )
	{
		if ( i < m_length ) 
		{
			TCHAR * buffer ;

			buffer = new TCHAR[ m_length - i + 1 ] ;
			memcpy( buffer, &m_data[ i ], sizeof( TCHAR ) * ( m_length - i + 1 ) ) ;
			// the above memcpy gets the terminating \0 

			m_length = m_length - i ;
			delete [] m_data ;
			m_data = buffer ;
		}
		else
		{
			// first non-whitespace character is \0
			delete [] m_data ;
			m_length = 0 ;
			m_data = NULL ;
		}
	}

	return *this ;
}





/******************************************************************************
tstring::operator[] 

	CString has NO [] operator. Use GetAt/SetAt instead 

******************************************************************************/
TCHAR tstring::operator[] ( int index ) const
{
	if ( index >= 0 && index < m_length ) 
		return m_data[ index ] ;
	else
		return _T( '\0' ) ;
}

	
/******************************************************************************
tstring::operator[] 

	This version returns a reference to an element of the character array, so 
	it can be used to set a value 

	CString has NO [] operator. Use GetAt/SetAt instead 

******************************************************************************/
TCHAR & tstring::operator[] (int index ) 
{
	ASSERT( index >= 0 && index < m_length ) ;

	return m_data[ index ] ;		// must return a reference - let 'em crash and burn in release builds if they call with invalid index 
} 




/******************************************************************************
tstring::GetAt

	For CString compatibility - 

******************************************************************************/
TCHAR tstring::GetAt( int index ) const 
{
	if ( index < 0 || index >= m_length )
		return _T( '\0' ) ;
	else
		return m_data[ index ] ;
}



/******************************************************************************
tstring::SetAt

	For CString compatibility. Like CString::GetAt, will not alter the string
	if index is out of bounds 

******************************************************************************/
void tstring::SetAt( int index, TCHAR chr ) 
{
	if ( index >= 0 && index < m_length ) 
	{
		m_data[ index ] = chr ;
	}
}






/******************************************************************************
tstring::Insert

	insert a string at position index. If index is < 0 or greater than m_length, 
	the debug version assert-fails, but returns in any case with tstring unchanged 

	There is no CString equivalent to this. 

******************************************************************************/
int tstring::Insert( int index, const TCHAR * str ) 
{
	if ( index >= 0 && index <= m_length ) 
	{
		int	insert_len ;

		insert_len = _tcslen( str ) ;

		if ( insert_len > 0 ) 
		{
			if ( m_length > 0 )
			{
				TCHAR *	buffer ;
			
				buffer = new TCHAR[ m_length + insert_len + 1 ] ;
				memcpy( buffer, m_data, sizeof( TCHAR ) * index ) ;
				memcpy( buffer + index, str, insert_len * sizeof( TCHAR ) ) ;			
				memcpy( buffer + index + insert_len, m_data + index, sizeof( TCHAR ) * ( m_length - index + 1 ) ) ;		// +1 gets the terminating \0 

				delete [] m_data ;
				m_data = buffer ;
				m_length = m_length + insert_len ;
			}
			else 
				*this = str ;	// just use the assignment operator 
		}
		// else - no change 
	}
	else
		ASSERT( FALSE ) ;

	return m_length ;
}



/******************************************************************************
tstring::Insert

	since this single-character version requuires as much work as the string 
	version, just use the string version 

******************************************************************************/
int tstring::Insert( int index, TCHAR chr ) 
{
	TCHAR temp_str[ 2 ] ;

	temp_str[ 0 ] = chr ;
	temp_str[ 1 ] = _T( '\0' ) ;

	return Insert( index, temp_str ) ;
}



