/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



A simple string class emulating most functionality of CString, with the principal 
exception that it does not use reference-counting

***************************************************************************************/

#pragma once

#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef WIN32
	#include <tchar.h>
#else
	// need to supply macro definitions for compiling on MAC

	#define	TCHAR		char

	#define	_tcslen		strlen 
	#define	_tcscpy		strcpy
    #define _vsntprintf vsnprintf
    #define _tprintf    printf
	#define _tcsupr		strupr
	#define	_tcslwr		strlwr
	#define	_tcscmp		strcmp
	#define _tcsncmp	strncmp
    #define _tcsnicmp   strncasecmp
	#define _istgraph	isgraph
    #define _istdigit   isdigit
    #define _tcschr     strchr
	#define _istspace	isspace
    #define _stscanf    sscanf

	#define	_T( x )		x
	#define	_TEXT( x )	x
	#define	TEXT( x )	x

#endif // WIN32






class tstring
{
	TCHAR *	m_data ;
	int		m_length ;

	enum
	{
		INITIAL_ARGUMENT_GUESS	= 64
	} ;

private:
	void copydata( const TCHAR * ccstr ) ;
	void copydata( const tstring & other ) ;	


public:
	tstring( ) ;
	tstring( const TCHAR * ccstr ) ;
	tstring( const TCHAR cchar ) ;
	tstring( const tstring & other ) ;
	~tstring( ) ;

	tstring & operator=( const TCHAR * ccstr ) ;
	tstring & operator=( const tstring & other ) ;
	tstring & operator+=( const TCHAR * appendee ) ;
	tstring & operator+=( const tstring & other ) ;

	void Format( const TCHAR * fmt, ... ) ;
	void FormatV( const TCHAR * fmt, va_list varg ) ;
	operator const TCHAR *( ) const ;

	tstring Left( int len ) const ;
	tstring Mid( int start, int len = INT_MAX ) const ;
	tstring Right( int len ) const ;
	tstring & MakeLower( ) ;
	tstring & MakeUpper( ) ;
	tstring & Trim( ) ;
	tstring & TrimRight( ) ;
	tstring & TrimLeft( ) ;
	tstring Tokenize( const TCHAR * delimiters, int & token_pos ) ;
	int Compare( const TCHAR * other ) const ;
	int CompareNoCase( const TCHAR * other ) const ;
	int Find( TCHAR chr, int start = 0 ) const ;
	int Find( const TCHAR * str, int start = 0 ) const ;
	int ReverseFind( TCHAR chr ) const ;
	int ReverseFind( const TCHAR * str ) const ;
	int Replace( const TCHAR * old_string, const TCHAR * new_string ) ;		// replaces each instance of old_string with new_string and returns # of substitutions made. See comment on function for more info and caveats 

	TCHAR * GetBufferSetLength( int char_ct ) ;
	void ReleaseBuffer( int length = -1 ) ;
	void ReleaseBufferSetLength( int length ) ;

	int GetLength( ) const ;		// CString version prototyped as int GetLength( ) const throw( ) ; .... I have no idea what would make this function throw an exception.... 
	void Empty( ) ;
	bool IsEmpty( ) const ;

	// BEWARE: [] operator is not defined with CString - use GetAt and Insert instead 
	TCHAR operator [] ( int index ) const ;
	TCHAR & operator [] (int index ) ;

	TCHAR GetAt( int index ) const ;
	void SetAt( int index, TCHAR chr ) ;

	int Insert( int index, const TCHAR * str ) ;
	int Insert( int index, TCHAR chr ) ;

	// binary friend operators - concatenation 
	friend const tstring operator+( const tstring & s1, const tstring & s2 )
	{
		tstring concatenation ;

		if ( ( concatenation.m_length = s1.m_length + s2.m_length ) )
		{
			concatenation.m_data = new TCHAR[ concatenation.m_length + 1 ] ;
			memcpy( concatenation.m_data, s1.m_data, sizeof( TCHAR ) * s1.m_length ) ;
			memcpy( concatenation.m_data + s1.m_length, s2.m_data, sizeof( TCHAR ) * s2.m_length ) ;
			concatenation.m_data[ concatenation.m_length ] = _T( '\0' ) ;
		}
		return concatenation ;
	}
	friend const tstring operator+( const tstring & s1, const TCHAR * s2 )
	{
		return s1 + tstring( s2 ) ;
	} ;
	friend const tstring operator+( const TCHAR * s1, const tstring & s2 )
	{
		return tstring( s1 ) + s2 ;
	} ;

	// binary friend operator - != 
	friend bool operator!=( const tstring & s1, const tstring & s2 ) 
	{
		return s1.m_length != s2.m_length || 0 != memcmp( s1.m_data, s2.m_data, s1.m_length * sizeof( TCHAR ) ) ;
	} ;
	friend bool operator!=( const tstring & s1, const TCHAR * s2 )
	{
		return s1 != tstring( s2 ) ;
	} ;
	friend bool operator!=( const TCHAR * s1, const tstring & s2 )
	{
		return tstring( s1 ) != s2 ;
	} ;

	// binary friend operator == 
	friend bool operator==( const tstring & s1, const tstring & s2 )
	{
		return s1.m_length == s2.m_length && 0 == memcmp( s1.m_data, s2.m_data, s1.m_length * sizeof( TCHAR )  ) ;
	} ;
	friend bool operator==( const tstring & s1, const TCHAR * s2 )
	{
		return s1 == tstring( s2 ) ;
	}
	friend bool operator==( const TCHAR * s1, const tstring & s2 )
	{
		return tstring( s1 ) == s2 ;
	}
} ;
