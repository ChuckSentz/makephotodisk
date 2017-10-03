/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Worker thread to generate HTML for presentations and contact sheets. 
***************************************************************************************/
#pragma once 

void UpdateHTML( CMakePhotoDisk3Doc * doc ) ;
bool SaveUTF8File( const TCHAR * filename, CString & file_contents ) ;
bool LoadUTF8File( const TCHAR * filename, CString & file_contents ) ;

/*
*	these are 3 very similar functions...
*
*		ReplaceDelimitedSubstring	-	replaces the text between delimiters, as well as the delimiters, with a replacement text 
*
*		ReturnDelimitedContent		-	return the text between delimiters, but makes no alteration to the text 
*
*		ReplaceDelimitedContent		-	replaces the text between delimiters, but leaves the delimiters in place 
*
*/
bool ReplaceDelimitedSubstring( CString & actual, const TCHAR * l_delim, const TCHAR * r_delim, const TCHAR * repl ) ;
bool ReturnDelimitedContent( const CString & text, const TCHAR * l_delim, const TCHAR * r_delim, CString & content ) ;
bool ReplaceDelimitedContent( CString & text, const TCHAR * l_delim, const TCHAR * r_delim, const TCHAR * repl, int start_pos = 0, int * follow_pos = NULL ) ;
bool ReplaceDelimitedCondition( CString & actual, const TCHAR * conditional, const TCHAR * new_content ) ;
bool DeleteConditionalExpressions( CString & actual, const TCHAR * conditional ) ;
bool CleanupConditionalExpressions( CString & actual, const TCHAR * conditional_id ) ;



