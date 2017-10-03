/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



Helper functions
***************************************************************************************/

#ifndef _importhelpers_h_
#define _importhelpers_h_


#include <stdio.h>

stringt CurrentTimeString( ) ;
stringt GetPortableVolumeName( const TCHAR * portable_path ) ;
bool FileCopy( const TCHAR * source, const TCHAR * dest ) ;
int DaysDifference( const stringt & time1, const stringt & time2 ) ;
void IdentifyThisExecutable( const TCHAR * app_name ) ;
void DisplayExceptionAndTerminate( SQLDatabaseException & sql_exception ) ;


#ifdef __APPLE__

// define ASSERT and VERIFY for APPLE. If building for Windows, these are defined 

#ifndef ASSERT

#ifdef DEBUG

#define	ASSERT( x )		if ( !(x) ) \
						{	\
							fprintf( stderr, "Assertion failed, %s, in %s, line %d\n", #x, __FILE__, __LINE__ ) ;	\
							exit( -1 ) ;		\
						} ;



#define VERIFY( x )		if ( !(x) ) \
						{	\
							fprintf( stderr, "VERIFY failed, %s, in %s, line %d\n", #x, __FILE__, __LINE__ ) ;	\
							exit( -1 ) ;		\
						} ;

#else

#define ASSERT( x )		
#define VERIFY( x )		x


#endif // DEBUG

#endif	// ifndef ASSERT

#endif // ifdef __APPLE__ 


#endif	// ifndef _importhelpers_h_