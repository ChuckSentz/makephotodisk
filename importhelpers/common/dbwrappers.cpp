/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




databaase wrapper classes. These just ensure consistent location and table definitions 
for all databases 
***************************************************************************************/

#ifdef WIN32
	#include "stdafx.h"
#else 
	#include <dirent.h>
	#include <unistd.h>
	#include <uuid/uuid.h>
	#include <time.h>
	#include <sys/statvfs.h>
#endif

#include "dbschema.h"
#include "tstring.h"
#include "sqlitedb.h"
#include "importhelpers.h"
#include "dbwrappers.h"
#include <errno.h>


#ifndef WIN32
	bool TranslatePlist( const TCHAR * filename ) ;
#endif






/******************************************************************************
LaptopDatabase::Open

	Called to open the laptop db. If NULL is specified for path, will open the 
	db in the default location.

	The db is created if it does not already exist. 

	Guarantees all the tables we need exist and are defined exactly how we 
	need them, too. 

******************************************************************************/
bool LaptopDatabase::Open( 
	const TCHAR * location				// I - path to laptop db, or NULL 
) 
{
	stringt db_path ;
	bool	isOK = false ;

	if ( location )
    {
		db_path = location ;			// if they specified a location, which may just be a directory, use it. If it is just a directory, we'll fix it below 

        if ( 0 == db_path.Right( 1 ).Compare( _T( "/" ) ) )		// do we just have a directory? If so, append the filename
            db_path += LAPTOP_DB_NAME ;
	}
    else
		LaptopDatabase::GetLaptopDBPath( db_path ) ;	// otherwise, get the default path 

	try
	{
		if ( SQLDatabase::Open( db_path ) )
		{
			if ( CreateTable( LAPTOP_DB_FILE_TABLE, LAPTOP_DB_FILE_TABLE_DEFINITION ) )
				if ( CreateTable( LAPTOP_DB_PORTABLE_DRIVES_TABLE, LAPTOP_DB_PORTABLE_DRIVES_TABLE_DEFINITION ) )
					if ( CreateTable( LAPTOP_DB_X_PORTABLE_MAP_TABLE, LAPTOP_DB_X_PORTABLE_MAP_DEFINITION ) )
						isOK = true ;
		}
	}
	catch ( SQLDatabaseException & sql_exception )
	{
		DisplayExceptionAndTerminate( sql_exception ) ;
	}

	return isOK ;
}






/******************************************************************************
LaptopDatabase::GetLaptopDBPath

	Class static. 
	
	Returns default path where we want the laptop's database 

******************************************************************************/
bool LaptopDatabase::GetLaptopDBPath( 
	stringt & db_path					// O - gets default path to laptop DB 
)
{
	stringt	work ;

#ifdef WIN32

	TCHAR * path ;
	
	path = work.GetBufferSetLength( MAX_PATH ) ;
	SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, 0, path ) ;
	work.ReleaseBuffer( ) ;
	work += _T( "/Pictures/Imported/" ) ;

#else

	work.Format( _T( "%s/Pictures/Imported/" ), getenv( _T( "HOME" ) ) ) ;    

#endif
	
    db_path.Format( _T( "%s%s" ), (const TCHAR *) work, LAPTOP_DB_NAME ) ;
	return true ;
}






/******************************************************************************
PortableDatabase::Open

	Used to open the portable database (on an external hard drive). 

	Typically, just pass the path to the root of the portable hard drive 

	Creates or verifies all the tables we expect 

******************************************************************************/
bool PortableDatabase::Open( 
	const TCHAR * portable_path			// I - path to portable, optionally including the db filename 
)
{
	bool isOK = false ;

	// portable_path may be just the drive name, the drive name plus a subdirectory, or the full file name - but NULL is not allowed 
	ASSERT( portable_path != NULL ) ;

	stringt db_path = portable_path ;
	if ( 0 == db_path.Right( 1 ).Compare( _T( "/" ) ) )
	{
		// it's a directory. Doesn't matter if it's the drive name or a subdirectory on the drive. Concatenate the filename 
		db_path += PORTABLE_DB_NAME ;
	}
	// else - it's a fully qualified path. Proceed.... 

	try
	{
		if ( SQLDatabase::Open( db_path ) )
		{
			if ( CreateTable( PORTABLE_DB_FILE_TABLE, PORTABLE_DB_FILE_TABLE_DEFINITION ) ) 
				if ( CreateTable( PORTABLE_DB_STACKING_TABLE, PORTABLE_DB_STACKING_TABLE_DEFINITION ) )
					if ( CreateTable( PORTABLE_DB_KEYWORDS_TABLE, PORTABLE_DB_KEYWORDS_TABLE_DEFINITION ) )
						if ( CreateTable( PORTABLE_DB_KEYWORDS_X_IMAGES_TABLE, PORTABLE_DB_KEYWORDS_X_IMAGES_TABLE_DEF ) ) 
							isOK = true ;
		}
	}
	catch ( SQLDatabaseException & sql_exception )
	{
		DisplayExceptionAndTerminate( sql_exception ) ;
	}

	return isOK ;
}






/******************************************************************************
LightroomDatabase::GetLightroomDBPath

	Accessor which returns the lightroom db path 

******************************************************************************/
void LightroomDatabase::GetLightroomDBPath( stringt & lightroom_path )
{
	lightroom_path = m_db_path ;
}




/******************************************************************************
LightroomDatabase::Open

	Open the lightroom database.

	Passing NULL for lightroom_path causes this function to lookup Adobe's 
	configuration data to determine the correct path of the last used 
	lightroom database 

******************************************************************************/
bool LightroomDatabase::Open( 
	const TCHAR * lightroom_path		// I - fully qualified path to lightroom db (or NULL)
) 
{
	m_db_path.Empty( ) ;

	if ( lightroom_path ) 
	{
		m_db_path = lightroom_path ;

		try
		{
			return SQLDatabase::Open( lightroom_path ) ;
		}
		catch ( SQLDatabaseException & sql_exception )
		{
			DisplayExceptionAndTerminate( sql_exception ) ;
		}
	}
	else
	{

#ifdef WIN32
		stringt path ;

		::SHGetSpecialFolderPath( NULL, path.GetBufferSetLength( MAX_PATH + 1 ), CSIDL_PROFILE, FALSE ) ;
		path.ReleaseBuffer( ) ;
		path += _T( "/AppData/Roaming/Adobe/Lightroom/Preferences/Lightroom 6 preferences.agprefs" ) ;
		    
		FILE * fp ;
		bool got_path = false ;

		// Need to replace slashes with stupid Windows backslashes....
		path.Replace( _T( "/" ), _T( "\\" ) ) ;

		if ( fp = _tfopen( path, _T( "r" ) ) ) 
		{
			stringt line ;

			while ( _fgetts( line.GetBufferSetLength( 512 ), 512, fp ) )
			{
				line.ReleaseBuffer( ) ;
				line.TrimLeft( ) ;

				if ( 0 == line.Left( 17 ).CompareNoCase( _T( "libraryToLoad20 =" ) ) ) 
				{
					// right of the equals sign, there's something like the following (including quotes): "C:\\Users\\CSentz\\Documents\\Hoagy\\lightroom\\Hoagy Catalog 4\\Hoagy Catalog-2.lrcat", 
					line = line.Mid( 1 + line.Find( _T( '"' ) ) ) ;
					line = line.Left( line.Find( '"' ) ) ;

					while ( line.Replace( _T( "/" ), _T( "\\" ) ) )
						;

					while ( line.Replace( _T( "\\\\" ), _T( "\\" ) ) )
						;

					got_path = true ;
					break ;
				}
			}

			fclose( fp ) ;

			if ( got_path ) 
			{
				m_db_path = line ;

				try
				{
					return SQLDatabase::Open( line ) ;
				}
				catch ( SQLDatabaseException & sql_exception )
				{
					DisplayExceptionAndTerminate( sql_exception ) ;
				}
			}
			else
				return false ;
		}

#else
		stringt source ;
		stringt dest ;

		// lightroom MRU files & preferences stored in a binary-format plist file.... 
		source.Format( _T( "%s/Library/Preferences/com.adobe.Lightroom6.plist" ), getenv( _T( "HOME" ) ) ) ;
		dest.Format( _T( "%s/temp.plist" ), getenv( _T( "HOME" ) ) ) ;

		// make a copy of the plist file before invoking TranslatePlist
		if ( FileCopy( source, dest ) )
		{
			// spawn shell command to translate binary-format plist file into text-readable one
			if ( TranslatePlist( dest ) )
			{
				FILE *	plist_file ;

                if ( ( plist_file = fopen( dest, "r" ) ) )
                {
                    stringt line ;
                    TCHAR * ptr ;
                    bool	expecting_lib_name = false ;

                    ptr = line.GetBufferSetLength( 1024 ) ;

                    while ( fgets( ptr, 1024, plist_file ) )
                    {
                        line.ReleaseBuffer( ) ;

                        if ( -1 != line.Find( _T( "	<key>libraryToLoad20</key>" ) ) )
                        {
                            expecting_lib_name = true ;
                        }
                        else if ( expecting_lib_name ) 
                        {
                            // <string>/Users/CSentz/Documents/Hoagy/lightroom/Hoagy Catalog-2.lrcat</string>
                            if ( -1 != line.Find( _T( "<string>" ) ) && -1 != line.Find( _T( "</string>" ) ) )
                            {
                                ASSERT( line.Find( _T( "<string>" ) ) < line.Find( _T( "</string>" ) ) ) ;

                                line.ReleaseBuffer( ) ;
                                line = line.Left( line.Find( _T( "</string>" ) ) ).Mid( line.Find( _T( "<string>" ) ) + 8 ) ;

                                fclose( plist_file ) ;
                                unlink( dest ) ;
                                line.ReleaseBuffer( ) ;
								m_db_path = line ;


								try
								{
	                                return SQLDatabase::Open( line ) ;
								}
								catch ( SQLDatabaseException & sql_exception )
								{
									DisplayExceptionAndTerminate( sql_exception ) ;
								}
                            }
                        }
                        ptr = line.GetBufferSetLength( 1024 ) ;
                    }

                    // cleanup on error 
                    line.ReleaseBuffer( ) ;
                    fclose( plist_file ) ;
                    unlink( dest ) ;
                    printf( _T( "Unable to find LibraryToLoad in plist file\n" ) ) ;
                    exit( -1  ) ;
                }
                else
                {
                    printf( _T( "Unable to open file \"%s\", errno=%d\n" ), (const TCHAR *) dest, errno ) ;
                    exit( -1 ) ;
                }
			}
		}
		else
		{
			printf( _T( "Unable to copy plist file, errno %d. Exiting" ), errno ) ;
			exit( -1 ) ;
		}
#endif
	}

	// Shouldn't be possible to get here....
    ASSERT( FALSE ) ;
	return false ;
}




#ifndef WIN32

/******************************************************************************
TranslatePlist

	spawns plutil shell command, which translates a binary plist file into 
	the text-readible xml format

******************************************************************************/
bool TranslatePlist( 
	const TCHAR * filename				// I - fully qualified name of plist file to xlat 
) 
{
	bool	isOK = false ;
	pid_t	pid ;
	int		child_status ;

	pid = fork( ) ;

	if ( 0 == pid ) 
	{
		execl( _T( "/usr/bin/plutil" ), _T( "/usr/bin/plutil" ), _T( "-convert" ), _T( "xml1" ), filename, NULL ) ;

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

	return isOK ;
}

#endif



