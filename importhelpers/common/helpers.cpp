/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



Helper functions 
***************************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>




#ifdef WIN32
	#include "stdafx.h"

#else 
	#include <dirent.h>
	#include <unistd.h>
	#include <uuid/uuid.h>
	#include <sys/statvfs.h>
	#include <mach-o/dyld.h>

#define _stscanf    sscanf
#define _tgetenv    getenv
#define _tsetenv    setenv

#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#include "tstring.h"
#include "sqlitedb.h"
#include "dbschema.h"
#include "importhelpers.h"
#include "dbwrappers.h"



bool EnsureDestinationPath( const TCHAR * dest ) ;

static bool RecursiveEnsurePath( const stringt & pathname ) ;
bool VerifyOrCreateDirectory( const TCHAR * pathname ) ;
stringt GetPortableVolumeName( const TCHAR * portable_path ) ;




/******************************************************************************
CurrentTimeString

	Return a stringt with the current time formated like so:

		YYYY-MM-DD HH:MM:SS

		Where 
			YYYY	is the 4-digit year
			MM		is the month, encoded as [01, 02, ..., 12]
			DD		is the day, [01,...,31]
			HH		is the 24-hour time of day formatted as [00, 01, ..., 23]
			MM		is the minute [00, ..., 59]
			SS		is the second [00, ..., 59]

	A simple string comparison may be used to compare two times encoded in this way.

******************************************************************************/
stringt CurrentTimeString( ) 
{
	stringt work ;

#ifdef WIN32
	SYSTEMTIME	utc_time ;

	GetSystemTime( &utc_time ) ;
	work.Format( _T( "%04.4d-%02.2d-%02.2d %02.2d:%02.2d:%02.2d" ), utc_time.wYear, utc_time.wMonth, utc_time.wDay, utc_time.wHour, utc_time.wMinute, utc_time.wSecond ) ;
#else
	time_t	ttime ;
	tm *	utc_cur_time ;

	ttime = time( NULL ) ;
	utc_cur_time = gmtime( &ttime ) ;
	work.Format( _T( "%04.4d-%02.2d-%02.2d %02.2d:%02.2d:%02.2d" ), utc_cur_time->tm_year + 1900, utc_cur_time->tm_mon + 1, utc_cur_time->tm_mday, utc_cur_time->tm_hour, utc_cur_time->tm_min, utc_cur_time->tm_sec ) ;
#endif

	return work ;
}





#ifdef WIN32


/******************************************************************************
TimeStringToFileTime

	WIN32 only. Converts a time string encoded ala CurrentTimeString into a FILETIME	

******************************************************************************/
FILETIME TimeStringToFileTime( 
	const stringt & time_str			// I - time string to convert 
) 
{
	int year ;
	int	month ;
	int	day ;
	int	hour ;
	int	minute ;
	int	second ;
	SYSTEMTIME	sys_time ;
	FILETIME	ft ;

	_stscanf( time_str, _T( "%d-%d-%d %d:%d:%d" ), &year, &month, &day, &hour, &minute, &second ) ;

	sys_time.wYear = year ;
	sys_time.wMonth = month ;
	sys_time.wDay = day ;
	sys_time.wHour = hour ;
	sys_time.wMinute = minute ;
	sys_time.wSecond = second ;
	sys_time.wMilliseconds = 0 ;

	SystemTimeToFileTime( &sys_time, &ft ) ;

	return ft ;
}


#else

/******************************************************************************
TimeStringToTime_T

	for Apple/UNIX - convert a time string encoded ala CurrentTimeString into
	a time_t 

	It appears that time_t is 64 bits on Apple, hence time_t is not limited to
	years 1900-2037 as it was on 32-bit UNIX. 

******************************************************************************/
time_t TimeStringToTime_T( 
	const TCHAR * time_str				// I - time string to convert 
)
{
	int year ;
	int month ;
	int day ;
	int hour ;
	int minute ;
	int second ;
	struct tm	temp_tm ;

	_stscanf( time_str, _T( "%d-%d-%d %d:%d:%d" ), &year, &month, &day, &hour, &minute, &second ) ;

	temp_tm.tm_year = year-1900 ;
	temp_tm.tm_mon = month-1 ;
	temp_tm.tm_mday = day ;
	temp_tm.tm_hour = hour ;
	temp_tm.tm_min = minute ;
	temp_tm.tm_sec = second ;

    // need to use BSD extenstion - mktime() assumes we want temp_tm interpretted as a local time
	return timegm( &temp_tm ) ;
}

#endif





/******************************************************************************
DaysDifference

	return the days difference between two strings encoded by CurrentTimeString.
	The number of days is rounded DOWN 

******************************************************************************/
int DaysDifference( 
	const stringt & time1,		// I - first time 
	const stringt & time2		// I - other time 
) 
{
#ifdef WIN32

	FILETIME	ft1 ;
	FILETIME	ft2 ;
	LONGLONG	diff ;

	ft1 = TimeStringToFileTime( time1 ) ;
	ft2 = TimeStringToFileTime( time2 ) ;

	// calculate difference in seconds... then return difference in days 
	diff = ( ( ( LARGE_INTEGER *) &ft1 )->QuadPart - ( ( LARGE_INTEGER *) &ft2 )->QuadPart ) / 10000000 ;
	return (int) ( diff / ( 3600 * 24 ) ) ;

#else

	time_t	t1 ;
	time_t	t2 ;
	double	time_delta ;

	t1 = TimeStringToTime_T( time1 ) ;
	t2 = TimeStringToTime_T( time2 ) ;

	time_delta = difftime( t1, t2 ) ;
	time_delta /= ( 24 * 3600 ) ;		// convert seconds to days 

	return (int) time_delta ;

#endif
}








/******************************************************************************
GetPortableVolumeName

	Returns the volume name, given an OS-specific path to the root of a portable
	drive 

******************************************************************************/
stringt GetPortableVolumeName( const TCHAR * portable_path ) 
{

#ifdef WIN32

	TCHAR volume_name_buffer[ MAX_PATH + 1 ] ;
	DWORD volume_name_buffer_size = MAX_PATH + 1 ;

	GetVolumeInformation( portable_path, volume_name_buffer, volume_name_buffer_size, NULL, NULL, NULL, NULL, 0 ) ;
	return stringt( volume_name_buffer ) ;

#else
	stringt	work ;

	work = portable_path ;

	ASSERT( -1 != work.Find( _T( "/Volumes/" ) ) ) ;

	work = work.Left( work.ReverseFind( _T( '/' ) ) ) ;
	work = work.Mid( 1 + work.ReverseFind( _T( '/' ) ) ) ;

	return work ;

#endif

}












/******************************************************************************
FileCopy

	Apple doesn't provide a ready-made API to copy a file, like WIN32 does. So
	you either open the file and read/write the data yourself, or fork and 
	launch the shell's cp command

******************************************************************************/
bool FileCopy( 
	const TCHAR * source,				// I - source filename 
	const TCHAR * dest					// I - dest filename 
)
{
	bool    isOK = false ;
    stringt dest_folder ;
    int     last_slash ;
    
    // dest is a filename. Make sure the folder it is supposed to go to exists
    dest_folder = dest ;
    last_slash = dest_folder.ReverseFind( _T( '/' ) ) ;
    if ( last_slash > 0 )
        dest_folder = dest_folder.Left( last_slash ) ;

    EnsureDestinationPath( dest_folder ) ;

#ifdef WIN32

	if ( CopyFile( source, dest, FALSE ) )
		isOK = true ;
	else
		_tprintf( _T( "FileCopy( \"%s\", \"%s\" ) failed, GetLastError( )=%d\n" ), source, dest, GetLastError( ) ) ;

#else

	pid_t	pid ;
	int		child_status ;

	pid = fork( ) ;

	if ( 0 == pid ) 
	{
		execl( _T( "/bin/cp" ), _T( "/bin/cp" ), source, dest, NULL ) ;

		_tprintf( _T( "execl( ) returned! errno %d\n" ), errno ) ;
		exit( -1 ) ;
	}
	else
	{
		// retry as many times as necessary due to EINTR error 
		while ( -1 == waitpid( pid, &child_status, 0 ) && errno == EINTR )
			;

		isOK = ( 0 != WIFEXITED( child_status ) ) ;
	}

#endif

	return isOK ;
}






/******************************************************************************
EnsureDestinationPath

	guarantee the designated path exists 

******************************************************************************/
bool EnsureDestinationPath( 
	const TCHAR * dest					// I - the path to guarantee exists 
) 
{
	stringt		dest_path ;

	dest_path = dest ;

	ASSERT( dest_path.GetLength( ) > 1 ) ;
	ASSERT( -1 == dest_path.Find( _T( '\\' ) ) ) ;	// we assume that we're not going to see backslashes. If this is not the case, use Replace() to translate backslashes into forward slashes 

	// RecursiveEnsurePath can't handle paths which have a trailing '/' 
	if ( dest_path.GetAt( dest_path.GetLength( ) - 1 ) == _T( '/' ) )
		return RecursiveEnsurePath( dest_path.Left( dest_path.GetLength( ) - 1 ) ) ;
	else
		return RecursiveEnsurePath( dest_path ) ;
}







/******************************************************************************
RecursiveEnsurePath

	If unable to verify (or create) the path as is, lop off the last directory 
	name and go recursive. If that succeeds, try again 1 more time with the
	last directory name restored... 

******************************************************************************/
static bool RecursiveEnsurePath( 
	const stringt & pathname			// I - pathname to guarantee 
)
{
    bool    isOK = false ;
    int     pass_no = 0 ;
    bool	retry = false ;

	ASSERT( pathname.GetAt( pathname.GetLength( ) - 1 ) != _T( '/' ) ) ;

    do
    {
		pass_no++ ;
		retry = false ;

		// try to create the full path - 		        
        isOK = VerifyOrCreateDirectory( pathname ) ;

		// if we fail after the first attempt - try creating the directory above it, and retry 
        if ( pass_no == 1 && !isOK )
        {
			int	last_slash ;

			if ( -1 != ( last_slash = pathname.ReverseFind( _T( '/' ) ) ) )
				if ( RecursiveEnsurePath( pathname.Left( last_slash ) ) )
					retry = true ;
		}
	}
	while ( !isOK && pass_no == 1 && retry ) ;

    return isOK ;
}





/******************************************************************************
VerifyOrCreateDirectory

	Call an API which attempts to create the specified directory 

******************************************************************************/
bool VerifyOrCreateDirectory( 
	const TCHAR * pathname				// I - path to check 
) 
{

#ifdef WIN32

	if ( CreateDirectory( pathname, NULL ) )
		return true ;
	else
	{
		// we're happy if the directory already exists, too 
		return ERROR_ALREADY_EXISTS == GetLastError( ) ;
	}

#else

	if ( 0 == mkdir( pathname, 0777 ) )
		return true ;
	else
	{
		return EEXIST == errno ;
	}

#endif 
}





/******************************************************************************
IdentifyThisExecutable

	Provides unambiguous identification of exactly which version of the
	application is being executed. 

	printfs the app name, followed on the next line by the build-time
	
******************************************************************************/
void IdentifyThisExecutable( 
	const TCHAR * application_name		// I - this application's name 
) 
{
	TCHAR		exe_name[ 1024 ] ;
	TCHAR *		am_pm ;
	int			hour ;
	stringt		work ;

	_tprintf( _T( "%s\n" ), application_name ) ;

#ifdef WIN32
	WIN32_FILE_ATTRIBUTE_DATA	attr_data ;
			
	// determine the build date by checking the file modify time on our .exe 
	if ( GetModuleFileName( NULL, exe_name, sizeof( exe_name ) / sizeof( exe_name[ 0 ] ) ) )
	{
		if ( GetFileAttributesEx( exe_name, GetFileExInfoStandard, &attr_data ) )
		{
			FILETIME	build_time ;
			SYSTEMTIME	build_systime ;
				
			FileTimeToLocalFileTime( &attr_data.ftLastWriteTime, &build_time ) ;
			FileTimeToSystemTime( &build_time, &build_systime ) ;

			hour = build_systime.wHour ;
			if ( hour < 12 ) 
			{
				am_pm = _T( "am" ) ;
				if ( hour == 0 ) 
					hour = 12 ;
			}
			else
			{
				am_pm = _T( "pm" ) ;
				if ( hour > 12 )
					hour -= 12 ;
			}

			work.Format( _T( "Built on %02.2d-%02.2d-%04.4d, at %02.2d:%02.2d:%02.2d%s" ), build_systime.wMonth, build_systime.wDay, build_systime.wYear, hour, build_systime.wMinute, build_systime.wSecond, am_pm ) ;
		}
	}
#else
	uint32_t	buffer_size ;

	buffer_size = sizeof( exe_name ) ;

	if ( 0 == _NSGetExecutablePath( exe_name, &buffer_size ) ) 
	{
		struct stat file_st ;
		
		if ( 0 == stat( exe_name, &file_st ) ) 
		{ 
			struct tm * tm_time ;

			if ( ( tm_time = localtime( &file_st.st_mtime ) ) )
			{
				hour = tm_time->tm_hour ;

				if ( hour < 12 ) 
				{
					am_pm = _T( "am" ) ;

					if ( hour == 0 ) 
						hour = 12 ;
				}
				else
				{
					am_pm = _T( "pm" ) ;

					if ( hour > 12 )
						hour -= 12 ;
				}

				work.Format( _T( "Built on %02.2d-%02.2d-%04.4d, at %02.2d:%02.2d:%02.2d%s" ), tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_year + 1900, hour, tm_time->tm_min, tm_time->tm_sec, am_pm ) ;
			}
			else
			{
				ASSERT( FALSE ) ;
				work = _T( "localtime( ) failed. Is it 2038 already?!" ) ;
			}
		}
	}
	else
	{
		ASSERT( FALSE ) ;		// we've got an executable path over 1024 characters long?!?!?!
		work = _T( "executable path unavailable" ) ;
	}

#endif

	_tprintf( _T( "%s\n\n" ), (const TCHAR *) work ) ;
}




/******************************************************************************
DisplayExceptionAndTerminate

	A database operation has thrown an exception. Exceptions are unrecoverable
	errors and not anticipated - we just print out what the exception tells us
	about the problem and terminate. 
	
******************************************************************************/
void DisplayExceptionAndTerminate( 
	SQLDatabaseException & sql_exception		// I - the exception 
)
{
	_tprintf( _T( "SQL database exception\n" ) ) ;
    
#ifdef _UNICODE
	_tprintf( _T( "         Origin: %S (line %d)\n" ), sql_exception.GetFileName( ), sql_exception.GetLineNo( ) ) ;
#else
	_tprintf( _T( "         Origin: %s (line %d)\n" ), sql_exception.GetFileName( ), sql_exception.GetLineNo( ) ) ;
#endif

	_tprintf( _T( "          Error: %d (sub-code %d)\n" ), sql_exception.GetNativeError( ), sql_exception.GetSubError( ) ) ;
	_tprintf( _T( "    Description: %s\n" ), sql_exception.GetErrorText( ) ) ;
	_tprintf( _T( "       SQL Text: \"%s\"\n" ), sql_exception.GetSQLText( ) ) ;
	_tprintf( _T( "\n" ) ) ;
	_tprintf( _T( "program terminated\n" ) ) ;

	exit( -1 ) ;
}

