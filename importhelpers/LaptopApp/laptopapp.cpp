/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




LaptopApp.cpp: entry point and key functions of the laptop app. 

***************************************************************************************/

#ifdef UNICODE

#ifdef WIN32
#include "stdafx.h"
#endif


#else
#define TCHAR	char
#define	_tmain	main
#endif


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>


#ifdef WIN32
	#include <objbase.h>
	#include <Shlobj.h>
	#include <winioctl.h>
#else 
	#include <dirent.h>
	#include <unistd.h>
	#include <uuid/uuid.h>
	#include <time.h>
	#include <sys/statvfs.h>
    #include <grp.h>

	#define _tgetenv        getenv
    #define _ttoi           atoi
	bool RemoveFile( const TCHAR * filename ) ;
	bool RemoveDirectory( const TCHAR * pathname ) ;


	struct PortableDrive
	{
		char volume_name [ 256 ] ;
		char mount_point [ 256 ] ;
		bool exfat_format ;
	};

	bool ValidDisk( const char * device_name, bool * bad_portable, char * volume_name, char * mount_point ) ;
	int FindPortables( PortableDrive * drive, int array_size ) ;

#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "tstring.h"
#include "sqlitedb.h"
#include "dbschema.h"
#include "importhelpers.h"
#include "dbwrappers.h"



using namespace std ;



bool PortableDriveFound( stringt & portable_path ) ;
bool PromptForAnotherPortable( stringt & portable_path ) ;
void PropogateFinalDispositionFlags( const stringt & portable_path ) ;
bool PreExportCheckForNewFiles( int & file_count, uint64 & cummulative_lengths, int redundancy ) ;
bool SufficientRoomOnPortable( const stringt & portable_path, int file_ct, uint64 byte_count ) ;
bool ExportFilesToPortable( const stringt & portable_path, bool & additional_bu_needed, int file_count, uint64 cummulative_lengths, int redundancy  ) ;
bool RecursiveEnsurePath( const stringt & pathname ) ;
bool VerifyOrCreateDirectory( const TCHAR * pathname ) ;
bool RemoveFileAndDirectory( stringt file_name ) ;
void DeleteEligableFiles(  int minimum_age ) ;
void DeleteMatchingFiles( const TCHAR * path, const TCHAR * target ) ;
bool MatchTarget( const TCHAR * actual, const TCHAR * target ) ;
void CheckAndFixTetheredNames( SQLDatabase & lightroom_db ) ;

#ifdef WIN32

	// The one and only application object
	CWinApp theApp;
	using namespace std;

#endif



#ifdef __APPLE__

#define MAX_PATH    256

#endif



/******************************************************************************
UsageHelp

	Just prints up an explanation of switches 

******************************************************************************/
void UsageHelp( ) 
{
	_tprintf( _T( "LaptopApp { -rn }{ -cn }\n" ) ) ;
	_tprintf( _T( "\n" ) ) ;
	_tprintf( _T( "     -cn       Clean eligable files. Files are only eligable if they have\n" ) ) ;
	_tprintf( _T( "               been imported to the desktop. The 'n' argument specifies\n" ) ) ;
	_tprintf( _T( "               the minimum number of days since the file was imported.\n" ) ) ;
	_tprintf( _T( "\n" ) ) ;
	_tprintf( _T( "     -rn	      Where n > 0, sets the number of portables we try to backup to.\n" ) ) ;
	_tprintf( _T( "\n" ) ) ;
	_tprintf( _T( "LaptopApp -c       cleans all files which have been imported to the desktop\n" ) ) ;
	_tprintf( _T( "LaptopApp -c30     cleans all files which were imported to the desktop at least 30 days ago\n" ) ) ;
	_tprintf( _T( "LaptopApp -c -r4   cleans all imported files, and tells app to backup to 4 portable drives.\n\n" ) ) ;
}






/******************************************************************************
main

	Digests command-line switches. Looks for a portable drive. If a portable is
	found, we read the portable's database and update the laptop's database to
	reflect which files have finished the migration to the desktop, and 
	update the portable database to reflect what was backed up from other 
	portable drives. 

	Determines what's eligible for backup, then drives the process of backing 
	them up with sufficient redundancy. 

******************************************************************************/
int _tmain(	int argc, TCHAR* argv[], TCHAR* envp[] )
{
	int nRetCode = 0 ;

	try		// global try/catch to get any SQL exceptions not caught at a lower level 
	{

	#ifdef WIN32

		HMODULE hModule = ::GetModuleHandle(NULL);

		if (hModule != NULL)
		{
			// initialize MFC and print and error on failure
			if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
			{
				// TODO: change error code to suit your needs
				_tprintf(_T("Fatal Error: MFC initialization failed\n"));
				nRetCode = 1;
			}
			else
			{

	#endif
				stringt portable_path ;
				int		redundancy = 2 ;

				IdentifyThisExecutable( _T( "Laptop Import Helper" ) ) ;

				if ( argc > 1 )
				{
					int		i ;
					int		days_age = 0 ;

					for ( i = 1 ; i < argc ; i++ )
					{
						if ( argv[ i ][ 0 ] == _T( '-' ) )
							switch ( argv[ i ][ 1 ] ) 
							{
								case _T( 'c' ) : 
								case _T( 'C' ) :
									days_age = _ttoi( argv[ i ] + 2 ) ;
									DeleteEligableFiles( days_age ) ;
									break ;						

								case _T( 'r' ) :
								case _T( 'R' ) :
									redundancy = _ttoi( argv[ i ] + 2 ) ;
									if ( redundancy < 1 )
										redundancy = 1 ;
									if ( redundancy == 1 ) 
									{
										TCHAR	answer ;

										_tprintf( _T( "WARNING: best practice normally requires at least 2 portable backups of all\n" ) ) ;
										_tprintf( _T( "images. You have specified only 1 backup. This is recommended only if you\n" ) ) ;
										_tprintf( _T( "are sure that the portable will be imported at the desktop promptly.\n" ) ) ;
										_tprintf( _T( "\n\n" ) ) ;
										_tprintf( _T( "Are you sure you want only 1 backup copy (Y/N)? " ) ) ;

										answer = getchar( ) ;

										if ( answer != _T( 'y' ) && answer != _T( 'Y' ) ) 
										{
											_tprintf( _T( "Determine the options you want and try again.\n\n" ) ) ;
											UsageHelp( ) ;
											exit( -1 ) ;
										}
									}
									break ;

								default :
									_tprintf( _T( "Invalid switch \"%s\"\n\n" ), argv[ i ] ) ;
									// fall through to help....

								case _T( '?' ) :
								case _T( 'h' ) :
								case _T( 'H' ) :
									UsageHelp( ) ;
									exit( -1 ) ;
							}
					}
				}
    

				if ( PortableDriveFound( portable_path ) )
				{
			/*
					1. check the portable db and migrate information about files which have made it home to the desktop
					2. check for images in local lr db that need backing up - calculate the space required
						2a. if there's insufficient space on the portable, prompt the user to delete expendable files from the portable to make room 
					3. copy the images to the portable
					4. prompt the user to delete old files from the portable (perhaps use a guideline like 30 days after making it home)
					5. prompt the user to delete old files from the laptop 
					6. if the first pass, prompt the user to insert the other portable drive then go to step 1
			*/
					PropogateFinalDispositionFlags( portable_path ) ;

					int file_ct ;
					uint64  byte_total ;
        
					PreExportCheckForNewFiles( file_ct, byte_total, redundancy ) ;

					if ( file_ct )
					{
						bool additional_bu_required = false ;

						do
						{
							if ( SufficientRoomOnPortable( portable_path, file_ct, byte_total ) )
							{
								additional_bu_required = false ;
								ExportFilesToPortable( portable_path, additional_bu_required, file_ct, byte_total, redundancy ) ;
							}

							if ( additional_bu_required )
								if ( !PromptForAnotherPortable( portable_path ) )
									break ;
						}
						while ( additional_bu_required ) ;
					}
				}

	#ifdef WIN32

			}
		}
		else
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
			nRetCode = 1;
		}

	#endif
	}
	catch ( SQLDatabaseException & sql_exception ) 
	{
		DisplayExceptionAndTerminate( sql_exception ) ;
	}

	return nRetCode;
}






/******************************************************************************
FileLength

	Return a file length in a 64-bit integer. 

******************************************************************************/
uint64 FileLength( 
	const TCHAR * file_path				// I - fully qualified path of file 
)
{

#ifdef WIN32

	WIN32_FILE_ATTRIBUTE_DATA	file_attr ;

	if ( ::GetFileAttributesEx( file_path, GetFileExInfoStandard, &file_attr ) )
	{
		 ULARGE_INTEGER	jumbo_int ;

		 jumbo_int.LowPart = file_attr.nFileSizeLow ;
		 jumbo_int.HighPart = file_attr.nFileSizeHigh ;
		 return jumbo_int.QuadPart ;
	}

#else 

    struct stat file_st ;
    
    if ( 0 == stat( file_path, &file_st ) )
        return file_st.st_size ;

#endif

    return 0 ;
}






/******************************************************************************
DeleteEligableFiles

	Currently only invoked via user command-line option. Checks the database for 
	files which are	flagged as 'imported-to-desktop' and were imported to the 
	desktop at least 'minimum_days' ago.

	Any qualifying files are then deleted from the disk and from the lightroom 
	database, then flagged as 'deleted' in the MasterFileTable. 

	NOTE: 
	The records cannot be removed from the MasterFileTable until the 'deleted' 
	status has been propogated to all relevant portables (at which point the 
	file is also removed from the portable). Once that has been completed and 
	portable_count ==> 0 on a deleted file, then the record in the 
	MasterFileTable can actually be deleted. 

******************************************************************************/
void DeleteEligableFiles( 
	int minimum_days					// I - minimum file age 
)
{
	LaptopDatabase		laptop_db ;
	LightroomDatabase	lightroom_db ;
	stringt				current_time ;
	
	current_time = CurrentTimeString( ) ;

	if ( laptop_db.Open( ) )
	{
		SQLCursor eligable_file ;

		if ( lightroom_db.Open( ) )
		{
			if ( laptop_db.ExecuteQuery( eligable_file, _T( "SELECT * FROM MasterFileTable WHERE imported_to_desktop = 1 AND deleted_from_laptop = 0" ) ) ) 
			{
				do
				{
					stringt file_name ;
					stringt file_path ;
					stringt time_imported_to_desktop ;

					eligable_file.GetFieldValue( _T( "file_name" ), file_name ) ;
					eligable_file.GetFieldValue( _T( "source_path" ), file_path ) ;
					eligable_file.GetFieldValue( _T( "time_imported_to_desktop" ), time_imported_to_desktop ) ;
					
					if ( DaysDifference( current_time, time_imported_to_desktop ) >= minimum_days )
                    {
						_tprintf( _T( "Deleting \"%s\"\n" ), (const TCHAR *) ( file_path + file_name ) ) ;
                        fflush( stdout ) ;
                        
						// remove images for this file from any collections - 
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM AgLibraryCollectionImage " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile .lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove metadata..
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM Adobe_imageProperties " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove more metadata..
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM AgMetadataSearchIndex " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove even more metadata..
						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgLibraryIPTC " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove even more metadata..
						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgLibraryImportImage " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove records related to development 
						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgSourceColorProfileConstants " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove records related to development 
						lightroom_db.ExecuteSQL( _T( "DELETE FROM Adobe_imageDevelopSettings " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove even more metadata 
						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgHarvestedExifMetadata " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// and even more metadata 
						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgHarvestedIptcMetadata " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// and just a little bit more metadata 
						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgHarvestedDNGMetadata " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						lightroom_db.ExecuteSQL( _T( "DELETE FROM AgLibraryImageChangeCounter " )
                                                    _T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// remove some more..... metadata 
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM Adobe_AdditionalMetadata " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// delete the development history records  for any development done on any images derived from the root file 
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM Adobe_libraryImageDevelopHistoryStep " )
													_T( "WHERE " )
														_T( "image IN ( SELECT Adobe_Images.id_local FROM Adobe_Images, AgLibraryFile " )
														_T( "WHERE Adobe_Images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// don't forget to zap the lrdata files with all those little previews.... 
						SQLCursor image ;

						if ( lightroom_db.ExecuteQuery( image, _T( "SELECT AgLibraryFile.id_global, Adobe_Images.id_global FROM Adobe_Images, AgLibraryFile WHERE Adobe_images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s'" ), (const TCHAR *) file_name ) )
						{
							do
							{
								stringt guid ;
								stringt	preview_filename ;
								stringt preview_path ;

								lightroom_db.GetLightroomDBPath( preview_path ) ;
								preview_path.MakeLower( ) ;
								preview_path.Replace( _T( ".lrcat" ), _T( " previews.lrdata/" ) ) ;

								image.GetFieldValue( _T( "AgLibraryFile.id_global" ), guid ) ;
								preview_filename.Format( _T( "%s%c/%4.4s" ), (const TCHAR *) preview_path, guid[ 0 ], (const TCHAR *) guid ) ;		// use lrdata path + 1st character of guid + 1st 4 characters
								preview_path = preview_filename ;

								preview_filename.Format( _T( "%s-*.lrprev" ), (const TCHAR *) guid ) ;

								DeleteMatchingFiles( preview_path, preview_filename ) ;
							}
							while ( image.StepNext( ) ) ;
						}

						// delete the images for the file 
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM Adobe_images " )
													_T( "WHERE " )
														_T( "rootFile IN ( SELECT AgLibraryFile.id_local FROM AgLibraryFile " )
														_T( "WHERE lc_idx_filename = '%s' )" ), (const TCHAR *) file_name ) ;

						// delete the actual file record... 
						lightroom_db.ExecuteSQL(	_T( "DELETE FROM AgLibraryFile " )
													_T( "WHERE " )
														_T( "AgLibraryFile.lc_idx_filename = '%s' " ), (const TCHAR *) file_name ) ;


						RemoveFileAndDirectory( file_path + file_name ) ;
						laptop_db.ExecuteSQL( _T( "UPDATE MasterFileTable SET deleted_from_laptop = 1 WHERE file_name = '%s'" ), (const TCHAR *) file_name ) ;
					}
                    else
                    {
						_tprintf( _T( "NOT deleting \"%s\"\n" ), (const TCHAR *) ( file_path + file_name ) ) ;
                        fflush( stdout ) ;
                    }
				}
				while ( eligable_file.StepNext( ) ) ;
                
				_tprintf( _T( "\n\nDone!\n\n" ) ) ;
                fflush( stdout ) ;
			}
		}
	}
}







/******************************************************************************
DeleteMatchingFiles

	Used to delete all files matching a target filename containing wildcards. 
	This is required to delete the .lrprev files containing preview jpegs of 
	deleted files.

	lrprev files are named with the GUID from the Adobe_Images record followed 
	by another GUID of unknown origin then the .jpeg

******************************************************************************/
void DeleteMatchingFiles( 
	const TCHAR * path,					// I - root directory to search under 
	const TCHAR * target				// I - filename pattern to match, optionally w/ wildcards 
) 
{
	stringt				work_path ;

#ifdef WIN32

	HANDLE				find_handle ;
	WIN32_FIND_DATA		find_data ;

	work_path.Format( _T( "%s%s" ), path, target ) ;
	if ( INVALID_HANDLE_VALUE != ( find_handle = FindFirstFile( work_path, &find_data ) ) )
	{
		do
		{
			if ( !( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				work_path.Format( _T( "%s%s" ), path, find_data.cFileName ) ;
				DeleteFile( work_path ) ;
			}
		}
		while ( FindNextFile( find_handle, &find_data ) ) ;

		FindClose( find_handle ) ;
	}

#else

	DIR *	dir ;

	if ( ( dir = opendir( path ) ) )
	{
		struct dirent * dir_entry ;

		while ( NULL != ( dir_entry = readdir( dir ) ) )
		{
			if ( MatchTarget( dir_entry->d_name, target ) ) 
			{
				work_path.Format( _T( "%s/%s" ), path, dir_entry->d_name ) ;
				RemoveFile( work_path ) ;
			}
		}

		closedir( dir ) ;
	}

#endif
}




#ifndef WIN32 



/******************************************************************************
MatchTarget

	UNIX API's available on Mac OS X don't support matching wildcards, so we
	have to do it manually. 

******************************************************************************/
bool MatchTarget( 
	const TCHAR * actual,				// I - actual filename to test
	const TCHAR * target				// I - template with optional wildcards 
) 
{
	for ( ; ; )
	{
		switch ( *target ) 
		{
			case _T( '*' ) :
				if ( MatchTarget( actual, target + 1 ) )
					return true ;
				else if ( *actual && MatchTarget( actual + 1, target ) )
					return true ;
				else if ( *actual && MatchTarget( actual + 1, target + 1 ) )
					return true ;

				return false ;

			case _T( '\0' ) :
				return *actual == _T( '\0' ) ;

			default :
				if ( *actual != *target ) 
				{
					return false ;
				}
				else
				{
					actual++ ;
					target++ ;
				}
				break ;
		}
	}

	// should never get here 
	ASSERT( FALSE ) ;
	return false ;
}
#endif








/******************************************************************************
PropogateFinalDispositionFlags

	Check the db on the portable drive to see if there were files imported from 
	this drive to the desktop. If the file was imported to the desktop from the 
	OTHER hard drive, then the import fields in the portable's db will not be set, 
	but there's a possibility that the laptop db's flags ARE. 

******************************************************************************/
void PropogateFinalDispositionFlags( 
	const stringt & portable_path		// I - path to root directory of portable 
)
{
	LaptopDatabase		laptop_db ;

		// open the laptop db and silently initialize tables, if required
    if ( laptop_db.Open( ) )
    {
        SQLCursor 	drive_query ;
        stringt		volume_name ;
        int			drive_id = -1 ;

		// Extract the volume name from the portable_path, which is "/volumes/" followed by the volume name
        volume_name = GetPortableVolumeName( portable_path ) ;

        // determine if this portable is known to us...
        if ( laptop_db.ExecuteQuery( drive_query, _T( "SELECT ROWID FROM PortableDriveTable WHERE volume_name='%s'" ), (const TCHAR *) volume_name ) )
        {
            PortableDatabase	portable_db ;
            
            VERIFY( drive_query.GetFieldValue( 0, drive_id ) ) ;
            
            if ( portable_db.Open( portable_path ) )
            {
                SQLCursor	new_import ;
				SQLCursor	deleted_file ;
                int         import_ct = 0 ;
                
                // query the portable db for files which have been newly imported at the desktop, and record the import on the laptop db.
                if ( portable_db.ExecuteQuery( new_import, _T( "SELECT ROWID, * FROM PortableFileTable WHERE new_import_to_desktop != 0" ) ) )
                {
                    _tprintf( _T( "Recording new imports in MasterFileTable... " ) ) ;
                    fflush( stdout ) ;
                    
                    do
                    {
                        stringt		file_name ;
                        stringt		time_imported_to_desktop ;

                        new_import.GetFieldValue( _T( "file_name" ), file_name ) ;
                        new_import.GetFieldValue( _T( "time_imported_to_desktop" ), time_imported_to_desktop ) ;

                        // record the import in the MasterFileTable 
                        laptop_db.ExecuteSQL( _T( "UPDATE MasterFileTable SET imported_to_desktop=1, time_imported_to_desktop='%s' WHERE file_name = '%s'" ),
                                                (const TCHAR *) time_imported_to_desktop,
                                                (const TCHAR *) file_name
                                            ) ;

						// need to set the flag that tells us the portable knows about this particular import, too 
                        laptop_db.ExecuteSQL( _T( "UPDATE MasterFileXPortableMapping SET import_known_to_portable = 1 WHERE portable_drive_record = %d AND master_file_record IN ( SELECT ROWID FROM MasterFileTable WHERE file_name = '%s' )" ), 
												drive_id,
												(const TCHAR *) file_name 
											) ;
                        
                        if ( !( ++import_ct % 100 ) )
                        {
                            _tprintf( _T( "." ) ) ;
                            fflush( stdout ) ;
                        }
                    }
                    while ( new_import.StepNext( ) ) ;

                    _tprintf( _T( "\n\nDONE!\n\n" ) ) ;
                    
                    // reset the 'new import' flag in all records...
                    portable_db.ExecuteSQL( _T( "UPDATE PortableFileTable SET new_import_to_desktop = 0" ) ) ;
                }

				/* 
				* Do NOT need to reflect import flags from MasterXPortable mapping table to the MasterFileTable b/c they must already be known to the MasterFileTable 
				* but we need to reflect import flags to the portable drive's portable table, and when we do so, update the corresponding record in MasterFileXPortableMapping table.
				*/
				SQLCursor reflect_imports ;

				if ( laptop_db.ExecuteQuery( reflect_imports,		_T( "SELECT MasterFileTable.ROWID, * FROM MasterFileTable, MasterFileXPortableMapping " ) 
																	_T( "WHERE " )
																	_T( "	MasterFileTable.imported_to_desktop = 1 " )
																	_T( "AND " )
																	_T( "	MasterFileXPortableMapping.import_known_to_portable = 0 " )
																	_T( "AND " )
																	_T( "	MasterFileXPortableMapping.portable_drive_record = %d " )
																	_T( "AND " )
																	_T( "	MasterFileXPortableMapping.master_file_record = MasterFileTable.ROWID" ) , 
																	drive_id 
																) ) 
				{
                    _tprintf( _T( "Updating portable to reflect imports from another drive...." ) ) ;
                    fflush( stdout ) ;
                    int import_ct = 0 ;
                
					do
					{
						stringt	import_time ;
						stringt file_name ;
						int		file_recno ;

						reflect_imports.GetFieldValue( _T( "MasterFileTable.time_imported_to_desktop" ), import_time ) ;
						reflect_imports.GetFieldValue( _T( "MasterFileTable.file_name" ), file_name ) ;
						reflect_imports.GetFieldValue( 0, file_recno ) ;

						portable_db.ExecuteSQL( _T( "UPDATE PortableFileTable SET imported_to_desktop = 1, time_imported_to_desktop = '%s' WHERE file_name = '%s'" ), (const TCHAR *) import_time, (const TCHAR *) file_name ) ;
						laptop_db.ExecuteSQL( _T( "UPDATE MasterFileXPortableMapping SET import_known_to_portable = 1 WHERE master_file_record = %d AND portable_drive_record = %d" ), file_recno, drive_id ) ;
					
                        if ( !( ++ import_ct % 100 ) )
                        {
                            _tprintf( _T( "." ) ) ;
                            fflush( stdout ) ;
                        }
                    }
					while ( reflect_imports.StepNext( ) ) ;

                    _tprintf( _T( "\n\nDONE!\n\n" ) ) ;
                    fflush( stdout ) ;
				}


				// now - what about deleted files which haven't been deleted from this portable? 
				if ( laptop_db.ExecuteQuery( deleted_file, _T( "SELECT ROWID, * FROM MasterFileTable WHERE deleted_from_laptop = 1" ) ) )
				{
					stringt			file_name ;
					SQLCursor 		portable_match ;
					int				file_id ;
					int				portable_count ;

					do
					{
						deleted_file.GetFieldValue( 0, file_id ) ;
						deleted_file.GetFieldValue( _T( "file_name" ), file_name ) ;
						deleted_file.GetFieldValue( _T( "portable_count" ), portable_count ) ;

                        _tprintf( _T( "Deleting '%s' from portable\n" ), (const TCHAR *) file_name ) ;
                        fflush( stdout ) ;

						if ( portable_db.ExecuteQuery( portable_match, _T( "SELECT * FROM PortableFileTable WHERE file_name='%s'" ), (const TCHAR *) file_name ) ) 
						{
							stringt		path_on_portable ;

							portable_match.GetFieldValue( _T( "file_path" ), path_on_portable ) ;
							RemoveFileAndDirectory( portable_path + path_on_portable + file_name ) ;

							portable_db.ExecuteSQL( _T( "DELETE FROM PortableFileTable WHERE file_name = '%s' " ), (const TCHAR *) file_name ) ;

							laptop_db.ExecuteSQL( _T( "DELETE FROM MasterFileXPortableMapping WHERE master_file_record = %d AND portable_drive_record = %d" ), file_id, drive_id ) ;
						
							if ( --portable_count == 0 ) 
								laptop_db.ExecuteSQL( _T( "DELETE FROM MasterFileTable WHERE file_name = '%s' " ), (const TCHAR *) file_name ) ;
							else
								laptop_db.ExecuteSQL( _T( "UPDATE MasterFileTable SET portable_count = %d WHERE file_name = '%s' " ), portable_count, (const TCHAR *) file_name ) ;
						}
					}
					while ( deleted_file.StepNext( ) ) ;

                    _tprintf( _T( "\n\nDONE!\n\n" ) ) ;
                    fflush( stdout ) ;
				}
            }
        }
        else
        {
            // we've never seen this portable drive before???
            ASSERT( FALSE ) ;
        }
    }
}






#define EXTRA_BYTES_PER_FILE	4096			// SWAG of disk overhead per file above and beyond file length - (for on-disk database record, plus avg 1/2 sector wasted storage per file, plus directory entry ) 

/******************************************************************************
SufficientRoomOnPortable

	Estimate whether there's enough free space on the target portable for a 
	given number of files with a given cummulative length. 

******************************************************************************/
bool SufficientRoomOnPortable( 
	const stringt & portable_path,		// I - path to root of portable drive
	int file_ct,						// I - number of files to back up
	uint64 byte_count					// I - cummulative length of the files
)
{
    bool isOK = false ;

#ifdef WIN32

	ULARGE_INTEGER	bytes_avail ;

	if ( ::GetDiskFreeSpaceEx( portable_path, &bytes_avail, NULL, NULL ) )
	{
		if ( bytes_avail.QuadPart > ( byte_count + file_ct * EXTRA_BYTES_PER_FILE ) )
			isOK = true ;
	}

#else

    struct statvfs stat_vfs ;
    
    if ( 0 == statvfs( portable_path, &stat_vfs ) )
    {
        uint64 free_space ;
        
        free_space = (uint64)stat_vfs.f_bfree * (uint64)stat_vfs.f_frsize ;
		isOK = ( free_space > ( byte_count + file_ct * EXTRA_BYTES_PER_FILE ) ) ;
    }

#endif
    
    return isOK ;
}





/******************************************************************************
IdentifyFilesToBackup

	This function is used by both PreExportCheckForNewFiles() and 
	ExportFilesToPortable( ) to create the query we use against the lightroom 
	database to identify files which need to be backed up. Since both functions 
	require the identical criteria, we may as well identify the files in 
	a single function. 

	You MUST Attach the lightroom db to the laptop db BEFORE calling this

	The query is a little tricky.... 

	We retrieve from the lightroom catalog the three fields required to form 
	the fully qualified pathname of each and every image file. 

	We identify the files of interest using a subquery constructed using a
	AgLibraryFile LEFT OUTER JOINED on MasterFileTable. 
	
	 
	LEFT OUTER JOIN of the AgLIbraryFile x (LaptopDB).MasterFileTable ON 
	AgLibraryFile.lc_iex_filename = MasterFileTable.file_name. 

	We're interested in all cases where either file_name in the MasterFileTable 
	is NULL (meaning - there's a record in the lightroom db but not in our db, 
	in which case we've never seen this file before) or where it is found but 
	portable_count is less than 'redundancy' (meaning we haven't made enough 
	external copies of the file) AND imported_to_desktop is NOT true (were it 
	true, we'd be completely done with the file). 

******************************************************************************/
bool IdentifyFilesToBackup(
	SQLCursor & sql_cursor,				// O - gets cursor 
	SQLDatabase & lightroom_db			// I - lightroom db with laptop db already attached 
) 
{
	return lightroom_db.ExecuteQuery(	sql_cursor,
										_T(  "SELECT Adobe_images.id_local, Adobe_images.colorLabels, Adobe_images.pick, Adobe_images.rating, AgLibraryFile.lc_idx_filename, AgLibraryFolder.pathFromRoot, AgLibraryRootFolder.absolutePath " )
										_T( "FROM Adobe_images, AgLibraryFile, AgLibraryFolder, AgLibraryRootFolder " ) 
										_T( "WHERE Adobe_images.rootFile = AgLibraryFile.id_local " )
										_T( "   AND AgLibraryFolder.id_local = AgLibraryFile.folder " )
										_T( "	AND AgLibraryRootFolder.id_local = AgLibraryFolder.rootFolder " )
										_T( "	AND lc_idx_filename IN " )
										_T( "	( SELECT AgLibraryFile.lc_idx_filename " )
										_T( "		FROM AgLibraryFile LEFT OUTER JOIN MasterFileTable " )
										_T( "		ON AgLibraryFile.lc_idx_filename=MasterFileTable.file_name " )
										_T( "		WHERE MasterFileTable.file_name is NULL OR ( MasterFileTable.imported_to_desktop = 0 AND MasterFileTable.portable_count < MasterFileTable.required_redundancy ) ) " ) ) ;
}





/******************************************************************************
CheckAndFixTetheredFilenames

    We now use the name format:
 
    <year>.<month>.<day>-<24hour>.<minute>.<second>-<session #>.<sequence #>
    
    2015.07.22-11.50.24-02.1904

    Lightroom uses the image's capture time to create the filename when importing
    from media, but uses the UTC version of the computer's time to create the 
    filename when shooting tethered, so with the above example, created 
    7/22/2015 at 11:50:24 is incorrectly be named:
    
    2015.07.22-15.50.24-02.1904
 
 *****************************************************************************/
void CheckAndFixTetheredNames( SQLDatabase & lightroom_db )
{
	SQLCursor	cursor ;

	if ( lightroom_db.ExecuteQuery( cursor, _T( "SELECT * FROM AgLibraryFile, Adobe_images WHERE Adobe_images.rootFile = AgLibraryFile.id_local" ) ) )
	{
		do
		{
			stringt	create_time ;
			int		id_local ;
			int		folder_id ;
			stringt original_filename ;

			cursor.GetFieldValue( _T( "Adobe_images.captureTime" ), create_time ) ;
			cursor.GetFieldValue( _T( "AgLibraryFile.id_local" ), id_local ) ;
			cursor.GetFieldValue( _T( "AgLibraryFile.originalFilename" ), original_filename ) ;
			cursor.GetFieldValue( _T( "AgLibraryFile.folder" ), folder_id ) ;

			int create_time_year ;
			int create_time_month ;
			int create_time_day ;
			int create_time_hour ;
			int create_time_minute ;
			int create_time_second ;

			int filename_year ;
			int filename_month ;
			int filename_day ;
			int filename_hour ;
			int filename_minute ;
			int filename_second ;

			stringt filename_follow_on ;
			TCHAR * follow_on_ptr ;
			stringt	old_path_and_filename ;
			stringt new_path_and_filename ;

			follow_on_ptr = filename_follow_on.GetBufferSetLength( MAX_PATH ) ;

			if ( 7 == _stscanf( (const TCHAR *) original_filename, _T( "%d.%d.%d-%d.%d.%d%s" ), &filename_year, &filename_month, &filename_day, &filename_hour, &filename_minute, &filename_second, follow_on_ptr ) )
			{
				filename_follow_on.ReleaseBuffer( ) ;

				if ( 6 == _stscanf( (const TCHAR *) create_time, _T( "%d-%d-%dT%d:%d:%d" ), &create_time_year, &create_time_month, &create_time_day, &create_time_hour, &create_time_minute, &create_time_second ) )
				{
					// the day, month or year may also be off... but the hours are always unequal if suffering from the tethered shooting time anomaly 
					if ( filename_hour != create_time_hour ) 
					{
						stringt new_original_filename ;

						new_original_filename.Format( _T( "%04.4d.%02.2d.%02.2d-%02.2d.%02.2d.%02.2d%s" ), 
													create_time_year,
													create_time_month,
													create_time_day,
													create_time_hour,
													create_time_minute,
													create_time_second,
													(const TCHAR *) filename_follow_on ) ;

						/***
						 *
						 *	Fortunately, since the naming problem only occurs with tethered shoots, and 
						 *	since all photos from a tethered session are placed in the same session directory
						 *	rather than broken up into a directory tree reflecting the year and day of
						 *	shooting, we DO NOT need to worry about moving the file to another directory
						 *	because of a date change in the file name. Moving the file to another directory
						 *	could entail updates to the AgLibraryFolder and AgLibraryRootFolder tables 
						 *
						****/
					
						// to rename the file we need to update several fields in AgLibraryFile: 
						SQLCursor	path_cursor ;

						if ( lightroom_db.ExecuteQuery(	path_cursor,
													_T(  "SELECT AgLibraryFolder.pathFromRoot, AgLibraryRootFolder.absolutePath FROM AgLibraryFolder, AgLibraryRootFolder WHERE AgLibraryFolder.id_local = %d AND AgLibraryRootFolder.id_local = AgLibraryFolder.rootFolder " ), 
													folder_id ) )
						{
							stringt path_from_root ;
							stringt root_path ;
							stringt	file_path ;

							path_cursor.GetFieldValue( _T( "AgLibraryFolder.pathFromRoot" ), path_from_root ) ;
							path_cursor.GetFieldValue( _T( "AgLibraryRootFolder.absolutePath" ), root_path ) ;

							old_path_and_filename = root_path ;
							if ( path_from_root.GetLength( ) )
								old_path_and_filename += path_from_root ;
							new_path_and_filename = old_path_and_filename ;

							old_path_and_filename += original_filename ;
							new_path_and_filename += new_original_filename ;
						}

						stringt lc_idx_filename ;
						stringt base_name ;
						stringt idx_file_name ;

						idx_file_name = new_original_filename ;
						lc_idx_filename = idx_file_name ;
						lc_idx_filename.MakeLower( ) ;
						base_name = idx_file_name ;
						base_name = base_name.Left( base_name.ReverseFind( _T( '.' ) ) ) ;
                        
#ifdef WIN32
						if ( MoveFile( old_path_and_filename, new_path_and_filename ) ) 
#else
                        if ( 0 == rename( old_path_and_filename, new_path_and_filename ) )
#endif 
                        {
                            if ( lightroom_db.ExecuteSQL( _T( "UPDATE AgLibraryFile SET originalFilename='%s', lc_idx_filename='%s', idx_filename='%s', baseName='%s' WHERE id_local=%d" ),
                                                                (const TCHAR *) new_original_filename,
                                                                (const TCHAR *) lc_idx_filename,
                                                                (const TCHAR *) idx_file_name,
                                                                (const TCHAR *) base_name,
                                                                id_local
                                                            ) )
                            {
                                _tprintf( _T( "%s ==> %s\n" ), (const TCHAR *) original_filename, (const TCHAR *) new_original_filename ) ;
                            }
                            else
							{
#ifdef WIN32
								MoveFile( new_path_and_filename, old_path_and_filename ) ;
#else                                
								rename( new_path_and_filename, old_path_and_filename ) ;
#endif
							}
                        }
                        else
                            _tprintf( _T( "Unable to rename file \"%s\", errno == %d\n" ), (const TCHAR *) old_path_and_filename, errno ) ;
                    }
				}
			}
		}
		while ( cursor.StepNext( ) ) ;
	}
}






/******************************************************************************
PreExportCheckForNewFiles

	Called before exporting files to determine the count of files we need to 
	backup, the minimum	total storage space needed (the cummulative file 
	lengths). 

	Uses IdentifyFilesToBackkup to setup the complex query of the lightroom db
	joined with the laptop db. 

******************************************************************************/
bool PreExportCheckForNewFiles( 
	int & file_count,					// O - gets count of qualifying files 
	uint64 & cummulative_lengths,		// O - gets cummulative lengths of qualifying files 
	int redundancy						// I - tells us if we need more backups of a given file 
)
{
	LightroomDatabase	augmented_lr_db ;
	stringt				laptop_db_path ;

    file_count = 0 ;
    cummulative_lengths = 0 ;

	if ( LaptopDatabase::GetLaptopDBPath( laptop_db_path ) )
	{
		if ( augmented_lr_db.Open( ) )
		{
			if ( augmented_lr_db.Attach( laptop_db_path, _T( "lapdb" ) ) )
			{
				SQLCursor image_cursor ;

				if ( IdentifyFilesToBackup( image_cursor, augmented_lr_db ) )
				{
                
					do
					{
						stringt file_name ;	
						stringt	path_from_root ;
						stringt	root_path ;
						stringt	file_path ;

						image_cursor.GetFieldValue( _T( "AgLibraryFile.lc_idx_filename" ), file_name ) ;
						image_cursor.GetFieldValue( _T( "AgLibraryFolder.pathFromRoot" ), path_from_root ) ;
						image_cursor.GetFieldValue( _T( "AgLibraryRootFolder.absolutePath" ), root_path ) ;

						file_path = root_path ;

						// path from root may well be null. If it is NOT null, it has a terminal slash, same as root_path, so we can just concatenate the paths... 
						if ( path_from_root.GetLength( ) )
							file_path += path_from_root ;
						file_path += file_name ;

						file_count++ ;
						cummulative_lengths += FileLength( file_path ) ;
					}
					while ( image_cursor.StepNext( ) ) ;
                    
                    CheckAndFixTetheredNames( augmented_lr_db ) ;
				}
				// if IdentifyFilesToBackup( ) returns false, it just means there are no files to work on 
			}
			else
				ASSERT( FALSE ) ;	// failure here is a catastrophic programming boo-boo.... 
		}
		else
			ASSERT( FALSE ) ;	// catastrophic programming boo-boo.... 
	}
	else
		ASSERT( FALSE ) ;	// catastrophic programming boo-boo.... 
	    
    return cummulative_lengths != 0 ;
}






/******************************************************************************
FormDestinationPath

	Figure out a good path to use for the portable (destination), based on the 
	path on the laptop (source). Normally, the path on the laptop is formed 
	based on the date the photo was created, and we take everything from the 
	year onward. 

	For example, 

		/Users/Csentz/Pictures/2014/2014-03-20/20140320-123459-09003.nef

	becomes

		2014/2014-03-20/20140320-123459-09003.nef

	But thethered sessions are different. The path on the laptop will be something
	like: 

		/users/csentz/pictures/tethered/<session-name>/20140320-123459-09003.nef

	or

		/users/csentz/pictures/studio session/<session-name>/20140320-123459-09003.nef

	In these cases we just lop off everything in front of 


******************************************************************************/
stringt FormDestinationPath( 
	const stringt & source_path			// I - full path of source file on laptop 
) 
{
	stringt work ;
	int		next_slash ;

    work = source_path.Mid( 1 ) ;

	while ( -1 != ( next_slash = work.Find( _T( '/' ) ) ) )
	{
        if ( 0 == work.Left( next_slash ).CompareNoCase( _T( "Studio Session" ) )
            ||
             0 == work.Left( next_slash ).CompareNoCase( _T( "tethered" ) ) )
        {
            // we've got a tethered session
            return work ;
        }
        else if ( next_slash == 4 && _istdigit( work[ 0 ] ) && _istdigit( work[ 1 ] ) && _istdigit( work[ 2 ] ) && _istdigit( work[ 3 ] ) )
        {
            // the leading directory appears to be a year - eg, /2013/...
            return work ;
        }
        
        work = work.Mid( next_slash + 1 ) ;
    }

    // unable to find the year, or the tethered directory - just return the whole path minus the leading '/'
    return source_path.Mid( 1 ) ;
}





// structure and prototypes for keyword support 
struct KeywordRec
{
	stringt	name ;
	stringt last_applied ;
	stringt	date_created ;
	int		old_recno ;
	int		old_parent ;
	int		new_recno ;
	int		new_parent ;
	list< int >		file_recno ;

	KeywordRec( ) 
	{
		old_recno = 0 ;
		old_parent = 0;
		new_recno = 0 ;
		new_parent = 0 ;
	} ;
} ;

bool FindKeywordByLightroomRecNo( list< KeywordRec *> & kw_list, int parent_ID ) ;
bool TranslateRecordNo( list< KeywordRec * > & kw_list, int old_id, int & new_id ) ;
KeywordRec * FindKeywordByNameAndParentID( list< KeywordRec *> & kw_list, const TCHAR * kw_name, int parent_ID ) ;



/******************************************************************************
ExportFilesToPortable

	Query the lightroom db to find files, and export them to the portable, 
	if required. 

	We set additional_bu_needed iff we have any file(s) which require 
	additional backups.

	NOTE: This function appears to be updating the lightroom database, but 
	is actually updating the laptop db through the lightroom db connection, 
	since that database has attached the laptop db for the cross-db join. 

******************************************************************************/
bool ExportFilesToPortable( 
	const stringt & portable_path,		// I - path to root of portable drive
	bool & additional_bu_needed,		// O - set to true if more bakups needed
	int file_count,						// I - total file count to backup 
	uint64 cummulative_lengths,			// I - total file length to backup 
	int redundancy						// I - number of redundant copies 
)
{
	LightroomDatabase	augmented_lr_db ;
    PortableDatabase	portable_db ;
    bool				isOK = false ;
	int					drive_id ;
	int					file_id ;

    if ( portable_db.Open( portable_path ) )
    {
		{	
			// perform a quick check to see if this portable is in the laptop db. 
			// we use braces to limit the scope of the laptop_db variable 
			LaptopDatabase	laptop_db ;	

			if ( laptop_db.Open( ) )
			{
				stringt		volume_name ;
				SQLCursor	drive_query ;

				// first - get the device id for the portable
				volume_name = GetPortableVolumeName( portable_path ) ;
			
				if ( laptop_db.ExecuteQuery( drive_query, _T( "SELECT ROWID FROM PortableDriveTable WHERE volume_name='%s'"), (const TCHAR *) volume_name ) ) 
				{
					VERIFY( drive_query.GetFieldValue( 0, drive_id ) ) ;
				}
				else
				{
					// something is terribly wrong. The record should have been added when the portable was detected 
					_tprintf( _T( "Critical error - drive record not found in ExportFilesToPortable()\n" ) ) ;
					exit( -1 ) ;
				}
			}
		}
		// the laptop db is closed by the SQLDatabase destructor... 


		if ( augmented_lr_db.Open( ) )
		{
			// need to attach the laptop_db for cross-database joins 
			stringt			laptop_db_path ;

			if ( LaptopDatabase::GetLaptopDBPath( laptop_db_path ) ) 
			{	
				if ( augmented_lr_db.Attach( laptop_db_path, _T( "lapdb" ) ) ) 
				{
					SQLCursor 	lr_file ;
					int			file_no ;
					list< KeywordRec * >	kw_list ;
					list< KeywordRec * >::iterator	kw_iterator ;

					// assume we're going to finish with this pass 
					additional_bu_needed = false ;

					if ( IdentifyFilesToBackup( lr_file, augmented_lr_db ) )
					{
						file_no = 1 ;
						isOK = true ;   // assume success, and set it false iff a serious error occurs

						do  // ... while lr_file->StepNext( ) 
						{
							stringt		path_from_root ;
							stringt		root_path ;
							stringt		lc_idx_filename ;
							stringt		master_file_name ;
							stringt		source_path ;
							stringt		dest_path ;
							stringt		relative_destination_path ;
							uint64		file_size ;
							bool		backup_this_file ;
							stringt		color_label ;
							int			image_recno ;
							int			pick ;
							int			rating ;

							backup_this_file = false ;
							SQLCursor 	match ;

							lr_file.GetFieldValue( _T( "AgLibraryFolder.pathFromRoot" ), path_from_root ) ;
							lr_file.GetFieldValue( _T( "AgLibraryRootFolder.absolutePath" ), root_path ) ;
							lr_file.GetFieldValue( _T( "AgLibraryFile.lc_idx_filename" ), lc_idx_filename ) ;
							lr_file.GetFieldValue( _T( "MasterFileTable.file_name" ), master_file_name ) ;				// will be "" if MasterFileTable.file_name is NULL 
							lr_file.GetFieldValue( _T( "Adobe_images.id_local" ), image_recno ) ;
							lr_file.GetFieldValue( _T( "Adobe_images.colorLabels" ), color_label ) ;
							lr_file.GetFieldValue( _T( "Adobe_images.pick" ), pick ) ;
							lr_file.GetFieldValue( _T( "Adobe_images.rating" ), rating ) ;

							// form the fully qualified path for the file 
							source_path = root_path ;
							// path from root may well be null. If it is NOT null, it has a terminal slash, same as root_path, so we can just concatenate the paths... 
							if ( path_from_root.GetLength( ) )
								source_path += path_from_root ;

							relative_destination_path = FormDestinationPath( source_path ) ;
							dest_path.Format( _T( "%s%s" ), (const TCHAR *) portable_path, (const TCHAR *) relative_destination_path ) ;

								// master_file_name will be non-empty if it has already been backed up, but we need more copies 
							if ( !master_file_name.GetLength( ) ) 
							{
								// Get a lock error doing inserts directly on an attached database, so we modify the laptop tables through the lightroom db 
								VERIFY( augmented_lr_db.ExecuteSQL( _T( "INSERT INTO MasterFileTable ( file_name, file_size, source_path, portable_count, imported_to_desktop, time_imported_to_desktop, deleted_from_laptop, required_redundancy ) VALUES( '%s', %lld, '%s', 0, 0, '', 0, %d )" ), 
																	(const TCHAR *) lc_idx_filename, 
																	file_size,
																	(const TCHAR *) source_path,
																	// portable count starts out as 0
																	// imported to desktop starts as 0 (false)
																	// time imported to desktop is empty string
																	// deleted from laptop is 0 (false)
																	redundancy
																	) ) ;
								backup_this_file = true ;
								master_file_name = lc_idx_filename ;
							}
							else 
							{
								// file already known to db - make sure it's not already backed up to this portable! 
								SQLCursor bu_check ;

								if ( augmented_lr_db.ExecuteQuery( bu_check, _T( "SELECT ROWID FROM MasterFileTable, MasterFileXPortableMapping WHERE file_name = '%s' AND MasterFileTable.ROWID == MasterFileXPortableMapping.master_file_record AND MasterFileXPortableMapping.portablea_drive_record = %d " ), (const TCHAR *) master_file_name, drive_id ) )
								{ 
									/* This particular file is ALREADY on the attached portable. This may happen if the user
									*  has accidently attached a portable which was already used to b/u this file - in any 
									*  event, we need to backup this file to ANOTHER portable so we leave 'backup_this_file' as
									*  false and set additional_bu_needed to true 
									*/								

									additional_bu_needed = true ;		// need at least 1 more drive for at least this one file 
									backup_this_file = false ;			// ... and we leave 'backup_this_file' as false 
								}
								else
									backup_this_file = true ;
							}

							if ( backup_this_file ) 
							{
								int		portable_count ;

								ASSERT( 0 != master_file_name.GetLength( ) ) ;

								_tprintf( _T( "Copying %d of %d: \"%s\"\n" ), file_no, file_count, (const TCHAR *) lc_idx_filename ) ;
								FileCopy( source_path + lc_idx_filename, dest_path + lc_idx_filename ) ;
								file_size = FileLength( source_path + lc_idx_filename ) ;

								// update laptop db to reflect the file copy 
								if ( augmented_lr_db.ExecuteQuery( match, _T( "SELECT ROWID, * FROM MasterFileTable WHERE file_name = '%s'" ), (const TCHAR *) master_file_name ) )
								{
									int	file_rec_no_in_portable_db ;

									match.GetFieldValue( 0, file_id ) ;
									match.GetFieldValue( _T( "portable_count" ), portable_count ) ;
							
									augmented_lr_db.ExecuteSQL( _T( "INSERT INTO MasterFileXPortableMapping ( master_file_record, portable_drive_record, import_known_to_portable ) VALUES ( %d, %d, 0 )" ), file_id, drive_id ) ;
									portable_count++ ;
									augmented_lr_db.ExecuteSQL( _T( "UPDATE MasterFileTable SET portable_count = %d WHERE file_name = '%s'" ), portable_count, (const TCHAR *) master_file_name ) ;

									// set additional_bu_needed true if we're short on portables for any file 
									if ( portable_count < redundancy ) 
										additional_bu_needed = true ;

									portable_db.ExecuteSQL( _T( "INSERT INTO PortableFileTable ( file_name, file_size, file_path, rating, color_label, pick, imported_to_desktop, new_import_to_desktop, time_imported_to_desktop, time_copied_to_portable ) VALUES ( '%s', %lld, '%s', %d, '%s', %d, 0, 0, '', '%s' )" ),
																(const TCHAR *) master_file_name,
																file_size,
																(const TCHAR *) relative_destination_path,
																rating,
																color_label.IsEmpty( ) ? _T( "" ) : (const TCHAR *) color_label,
																pick,
																(const TCHAR *) CurrentTimeString( )
																) ;

									file_rec_no_in_portable_db = portable_db.GetLastInsertRowID( ) ;

									// store the stacking information for this "image/file" 
									SQLCursor stacking_cursor ;

									if ( augmented_lr_db.ExecuteQuery( stacking_cursor, _T( "SELECT position, stack, collapsed FROM AgLibraryFolderStackImage WHERE image = %d" ), image_recno ) )
									{
										int	stack_position ;
										int	stack_id ;
										int collapsed ;

										stacking_cursor.GetFieldValue( _T( "position" ), stack_position ) ;
										stacking_cursor.GetFieldValue( _T( "stack" ), stack_id ) ;
										stacking_cursor.GetFieldValue( _T( "collapsed" ), collapsed ) ;

										portable_db.ExecuteSQL( _T( "INSERT INTO StackingTable ( file_id, stack_position, collapsed, stack_ID ) VALUES( %d, %d, %d, %d )" ), 
																		file_rec_no_in_portable_db, 
																		stack_position,
																		collapsed,
																		stack_id ) ;
									}

									// find any and all keywords applied to this image and buld the in-memory graph of keyword data 
									SQLCursor kw_query ;

									if ( augmented_lr_db.ExecuteQuery( kw_query, _T( "SELECT name, lastApplied, dateCreated, AgLibraryKeyword.id_local, parent FROM AgLibraryKeyword, AgLibraryKeywordImage WHERE AgLibraryKeyword.id_local = AgLibraryKeywordImage.tag AND AgLibraryKeywordImage.image = %d" ), image_recno ) )
									{
										do
										{
											stringt	kw_name ;
											int		parent ;
											KeywordRec *	keyword_rec ;

											kw_query.GetFieldValue( _T( "name" ), kw_name ) ;
											kw_query.GetFieldValue( _T( "parent" ), parent ) ;

											if ( !( keyword_rec = FindKeywordByNameAndParentID( kw_list, kw_name, parent ) ) )
											{
												if ( keyword_rec = new KeywordRec )
												{
													kw_query.GetFieldValue( _T( "name" ), keyword_rec->name ) ;
													kw_query.GetFieldValue( _T( "lastApplied" ), keyword_rec->last_applied ) ;
													kw_query.GetFieldValue( _T( "dateCreated" ), keyword_rec->date_created ) ;
													kw_query.GetFieldValue( _T( "AgLibraryKeyword.id_local" ), keyword_rec->old_recno ) ;
													kw_query.GetFieldValue( _T( "parent" ), keyword_rec->old_parent ) ;
											
													kw_list.push_back( keyword_rec ) ;
												}
											}

											ASSERT( NULL != keyword_rec ) ;
											keyword_rec->file_recno.push_back( file_rec_no_in_portable_db ) ;
										}
										while ( kw_query.StepNext( ) ) ;
									}
								}
								else
									ASSERT( FALSE ) ;
                            
								file_no++ ;
							}
						}
						while ( lr_file.StepNext( ) ) ;												
					}


					/* Now, we work on the keywords 
					**
					**		We start with just a list of keywords actually applied to the images we need to backup.  
					**		But we need to collect parent keywords which might be in the dictionary, but not 
					**		applied to any of our images this go-round.
					**
					**		Then, we need to build the new keyword dictionary, with translated parent references 
					*/
				
					// 1st - make sure we have all the ancestor keywords we need - 
					for ( kw_iterator = kw_list.begin( ) ; kw_iterator != kw_list.end( ) ; kw_iterator++ )
					{
						int	parent_id ;
					
						parent_id = (*kw_iterator)->old_parent ;

						if ( parent_id )
						{
							if ( !FindKeywordByLightroomRecNo( kw_list, parent_id ) )
							{
								SQLCursor 		kw_query ;

								if ( augmented_lr_db.ExecuteQuery( kw_query, _T( "SELECT name, lastApplied, dateCreated, id_local, parent FROM AgLibraryKeyword WHERE id_local = %d" ), parent_id ) )
								{
									KeywordRec *	keyword_rec ;

									if ( ( keyword_rec = new KeywordRec ) )
									{
										kw_query.GetFieldValue( _T( "name" ), keyword_rec->name ) ;
										kw_query.GetFieldValue( _T( "lastApplied" ), keyword_rec->last_applied ) ;
										kw_query.GetFieldValue( _T( "dateCreated" ), keyword_rec->date_created ) ;
										kw_query.GetFieldValue( _T( "id_local" ), keyword_rec->old_recno ) ;
										kw_query.GetFieldValue( _T( "parent" ), keyword_rec->old_parent ) ;

										if ( 0 == keyword_rec->old_parent ) 
											kw_list.push_front( keyword_rec ) ;		// put the root record at the start of the list
										else
											kw_list.push_back( keyword_rec ) ;
									}
								}
							}
						}
					}

					/**
						2 - construct the new keyword dictionary with the correct child/parent relationships. We may need
						to make several passes to process the list, b/c on a given pass there's no guarantee we've already
						added a necessary parent keyword to the new dictionary 
					**/
					bool go_again ;

					do
					{
						go_again = false ;

						// now - start building our new table
						for ( kw_iterator = kw_list.begin( ) ; kw_iterator != kw_list.end( ) ; kw_iterator++ )
						{
							// keywords which have already been processed will have a non-zero new_recno - 
							if ( 0 == ( *kw_iterator )->new_recno )
							{
								// if old_parent is zero, this is the root keyword 
								if ( !( ( *kw_iterator )->old_parent ) )
								{
									portable_db.ExecuteSQL( _T( "INSERT INTO PortableKeywordsTable ( keyword, parent, date_created, last_applied ) VALUES ( '%s', 0, '%s', '%s' )" ),
																			(const TCHAR *) ( *kw_iterator )->name, 
																			// parent forced to 0
																			(const TCHAR *) ( *kw_iterator )->date_created,
																			(const TCHAR *) ( *kw_iterator )->last_applied ) ;

									( *kw_iterator )->new_recno = portable_db.GetLastInsertRowID( ) ;
								}
								else if ( TranslateRecordNo( kw_list, ( *kw_iterator )->old_parent, ( *kw_iterator )->new_parent ) )
								{
									// old_parent is non-zero, but we know the new parent record number, so can insert this name 
									portable_db.ExecuteSQL( _T( "INSERT INTO PortableKeywordsTable ( keyword, parent, date_created, last_applied ) VALUES ( '%s', %d, '%s', '%s' ) " ),
																			(const TCHAR *) ( *kw_iterator )->name, 
																			( *kw_iterator ) ->new_parent,  
																			(const TCHAR *) ( *kw_iterator )->date_created, 
																			(const TCHAR *) ( *kw_iterator )->last_applied ) ;

									( *kw_iterator )->new_recno = portable_db.GetLastInsertRowID( ) ;
								}
								else
									// need to do another pass to get the parent's rec no translated 
									go_again = true ;		// need to do another pass b/c this keyword's parent is somewhere after this keyword
							}
						}
					}
					while ( go_again ) ;

					// 3 - now build the keyword cross file table - need an entry for every keyword which has a reference to a known image (ie, file_recno is not zero) 
					for ( kw_iterator = kw_list.begin( ) ; kw_iterator != kw_list.end( ) ; kw_iterator++ )
					{
						list< int >::iterator	recno_iterator ;					

						for ( recno_iterator = ( *kw_iterator )->file_recno.begin( ) ; recno_iterator != ( *kw_iterator )->file_recno.end( ) ; recno_iterator++ )
						{
							portable_db.ExecuteSQL( _T( "INSERT INTO KeywordsXImagesTable ( keyword_rec, image_rec ) VALUES ( %d, %d )" ),  
															( *kw_iterator )->new_recno,
															( *recno_iterator )
													) ;
						}
					}

					// cleanup the list - 
					while ( !kw_list.empty( ) ) 
					{
						while ( !kw_list.front( )->file_recno.empty( ) ) 
							kw_list.front( )->file_recno.pop_front( ) ;

						delete kw_list.front( ) ;
						kw_list.pop_front( ) ;
					}
				}
				else
					ASSERT( FALSE ) ;	// Attach( ) should never fail 
			}
			else
				ASSERT( FALSE ) ;		// GetLaptopDBPath( ) should never fail 
		}
	}
		    
    return isOK ;
}





/******************************************************************************
FindKeywordByLightroomRecNo

	Find a KeywordRec in the list by its record number in lightroom 

******************************************************************************/
bool FindKeywordByLightroomRecNo( 
	list< KeywordRec *> & kw_list,		// I - list of keywords to search 
	int old_parent						// I - lr record number of parent 
) 
{
	list< KeywordRec * >::iterator	kw_iterator ;
	bool found_it = false ;

	for ( kw_iterator = kw_list.begin( ) ; !found_it && kw_iterator != kw_list.end( ) ; kw_iterator++ ) 
	{
		if ( ( *kw_iterator )->old_recno == old_parent )
		{
			found_it = true ;
		}
	}

	return found_it ;
}





/******************************************************************************
FindKeywordByNameAndParentID

	Search the keyword list for record matching given name and parent 

******************************************************************************/
KeywordRec * FindKeywordByNameAndParentID( 
	list< KeywordRec *> & kw_list,		// I - keyword list 
	const TCHAR * kw_name,				// I - keyword name
	int old_parent						// I - keyword's parent 
)
{
	list< KeywordRec * >::iterator	kw_iterator ;
	KeywordRec *	keyword_rec = NULL ;

	for ( kw_iterator = kw_list.begin( ) ; NULL == keyword_rec && kw_iterator != kw_list.end( ) ; kw_iterator++ ) 
	{
		if ( ( *kw_iterator )->old_parent == old_parent && 0 == _tcscmp( (*kw_iterator)->name, kw_name ) )
			keyword_rec = *kw_iterator ;
	}

	return keyword_rec ;
}




/******************************************************************************
TranslateRecordNo

	Search the list for a matching lr record number. Return true iff. the new
	record number is known 

******************************************************************************/
bool TranslateRecordNo( 
	list< KeywordRec * > & kw_list,			// I - keyword list 
	int old_id,								// I - db from lightroom db 
	int & new_id							// O - gets record number in portable db 
) 
{
	list< KeywordRec * >::iterator	kw_iterator ;
	bool	found_it = false ;
	bool	got_xlat = false ;

	for ( kw_iterator = kw_list.begin( ) ; !found_it && kw_iterator != kw_list.end( ) ; kw_iterator++ )
	{
		if ( old_id == ( *kw_iterator )->old_recno )
		{
			found_it = true ;
			if ( new_id = ( *kw_iterator )->new_recno )
				got_xlat = true ;
		}  
	} 

	return got_xlat ;
}


#ifndef WIN32

/******************************************************************************
FindPortables

Scan /dev/ directory and check out any "disk*" devices found. Returns the number
of portable drives found. NOTE: this number may include some inelligible drives. 
A portable drive is inelligible if it is not formatted exFAT. 

******************************************************************************/
int FindPortables( 
	PortableDrive * drive,			// O - array filled with portable drives found 
	int array_size					// I - how many entries in array 
)
{
	DIR *   dir ;
	int     i = 0 ;

	if ( NULL != ( dir = opendir( "/dev/" ) ) )
	{
		struct dirent * dir_entry ;

		while ( NULL != ( dir_entry = readdir( dir ) ) && i < array_size )
		{
			if ( 0 == strncmp( dir_entry->d_name, "disk", 4 ) )
			{
				bool bad_portable = false ;

				// returns true iff have usable portable drive.
				if ( ValidDisk( dir_entry->d_name, &bad_portable, drive [ i ].volume_name, drive [ i ].mount_point ) || bad_portable )
				{
                    drive[ i ].exfat_format = !bad_portable ;
					i++ ;
				}
			}
		}

		closedir( dir ) ;
	}

	return i ;
}






/******************************************************************************
ValidDisk

Determine if a disk device (found under /dev/) corresponds to a valid, usable volume. 
To be usable, a volume must be accessed via USB and must be formated ExFAT. 

******************************************************************************/
bool ValidDisk( 
	const char * device_name,	// I - device name found under /dev/, eg "disk1s1" 
	bool * bad_portable,		// O - true if the device is portable but is not ExFAT 
	char * volume_name,			// O - gets volume name, eg "SkinnyBlack" 
	char * mount_point			// O - gets mount point, eg "/Volumes/SkinnyBlack/" 
)
{
	int pid ;
	int pipefd [ 2 ] ;
	bool is_usb = false ;
	bool is_exfat = false ;
	bool ruled_out = false ;
	bool mounted = false ;

	if ( 0 != pipe( pipefd ) )
	{
		exit( -1 ) ;
	}

	pid = fork( ) ;
	if ( pid < 0 )
	{
		exit( -1 ) ;
	}
	else if ( pid == 0 )
	{
		char fq_devname [ 256 ] ;

		sprintf( fq_devname, "/dev/%s", device_name ) ;

		dup2( pipefd [ 1 ], STDOUT_FILENO ) ;

		close( pipefd [ 0 ] ) ;
		close( pipefd [ 1 ] ) ;

		execl( "/usr/sbin/diskutil", "diskutil", "info", fq_devname, NULL ) ;
		exit( 0 ) ;
	}
	else
	{
		// retry as many times as necessary due to EINTR error
		int child_status ;

		while ( -1 == waitpid( pid, &child_status, 0 ) && errno == EINTR )
			;

		close( pipefd [ 1 ] ) ;
		FILE * child_output ;
		char buffer [ 1024 ] ;
		char * newline ;

		*volume_name = '\0' ;
		*mount_point = '\0' ;
		is_usb = false ;
		is_exfat = false ;

		child_output = fdopen( pipefd [ 0 ], "r" ) ;
		while ( fgets( buffer, sizeof buffer, child_output ) )
		{
			/**
			We're looking for...

			"Volume Name:" followed by the user-friendly volume name, eg "SkinnyBlack"
			"Mount Point:" followed by actual mountpoint (without trailing slash, eg, /Volumes/SkinnyBlack
			"File System Personality:" followed by "ExFAT" - required
			"Protocol:" followed by "USB" - required

			First column is 0, all headers begin at column 3, all data is at column 29.
			There can be lines with zero length

			**/

			if ( NULL != ( newline = strchr( buffer, '\n' ) ) )
			{
				*newline = '\0' ;
			}

			if ( NULL != strstr( buffer, "Volume Name:" ) )
			{
				strcpy( volume_name, buffer + 29 ) ;
			}

			if ( NULL != strstr( buffer, "Mount Point:" ) )
			{
				strcpy( mount_point, buffer + 29 ) ;
				strcat( mount_point, "/" ) ;
			}

			if ( NULL != strstr( buffer, "File System Personality:" ) )
			{
				if ( 0 != strcasecmp( buffer + 29, "ExFAT" ) )
				{
					ruled_out = true ;
				}
				else
				{
					is_exfat = true ;
				}
			}

			if ( NULL != strstr( buffer, "Protocol:" ) )
			{
				if ( 0 != strcmp( buffer + 29, "USB" ) )
				{
					ruled_out = true ;
				}
				else
				{
					is_usb = true ;
				}
			}

			if ( NULL != strstr( buffer, "Mounted:" ) )
			{
				mounted = ( 0 == strcmp( buffer + 29, "Yes" ) ) ;
			}
		}

		fclose( child_output ) ;
		close( pipefd [ 0 ] ) ;      // necessary?!?!?!?
	}

	if ( mounted && is_usb && ruled_out )
	{
		*bad_portable = true ;
	}

	return mounted && !ruled_out && is_usb && is_exfat ;
}



#endif






/******************************************************************************
PortableDriveFound

	Check for a usable external hard drive. 

******************************************************************************/
bool PortableDriveFound( 
	stringt & path						// O - gets path to root of portable drive 
)
{
    bool		isOK = false ;

#ifdef WIN32

	TCHAR		drives_buffer[ 26 * 8 ] ;
	TCHAR *		next_drive ;
	stringt		test_path ;
	int			char_ct ;
    int			try_no = 0 ;

	do
	{
		if ( char_ct = GetLogicalDriveStrings( sizeof( drives_buffer ) / sizeof( drives_buffer[ 0 ] ), drives_buffer ) )
		{
			next_drive = drives_buffer ;

			do
			{
				HANDLE	drive_handle ;

				// next_drive points to a drive designation formated like: "C:\", which we want to re-format as "\\.\C:"
				test_path.Format( _T( "\\\\.\\%c:" ), *next_drive ) ;

				if ( INVALID_HANDLE_VALUE != ( drive_handle = CreateFile( test_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) ) )
				{
					TCHAR	output_buffer[ 256 ] ;
					DWORD	bytes_returned ;

					// this just weeds out CD-roms 
					if ( DeviceIoControl( drive_handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, output_buffer, sizeof( output_buffer ), &bytes_returned, NULL ) ) 
					{
						if ( bytes_returned == sizeof( STORAGE_DEVICE_NUMBER ) ) 
						{
							STORAGE_DEVICE_NUMBER * sdn_struct ;

							sdn_struct = (STORAGE_DEVICE_NUMBER *) output_buffer ;
							if ( FILE_DEVICE_DISK == sdn_struct->DeviceType )
							{
								test_path.Format( _T( "%sportable.db" ), next_drive ) ;
								if ( INVALID_FILE_ATTRIBUTES != GetFileAttributes( test_path ) )
								{
									path.Format( _T( "%s" ), next_drive ) ;
									isOK = true ;
								}
							}
						}
					}

					CloseHandle( drive_handle ) ;
				}
				next_drive += _tcslen( next_drive ) + 1 ;
			}
			while ( !isOK && ( next_drive - drives_buffer < char_ct ) ) ;
		}

		if ( !isOK && try_no < 10 )
		{
            _tprintf( _T( "Waiting for portable drive... " ) ) ;
			Sleep( 5 * 1000 ) ;
            _tprintf( _T( "checking again\n" ) ) ;
		}
	}
	while ( !isOK && try_no++ < 5 ) ;

#else

	PortableDrive	portable_drive [ 4 ] ;
	int				try_no = 0 ;
	LaptopDatabase	laptop_db ;
	int				drive_ct ;

	if ( laptop_db.Open(  ) )
	{
		do
		{
			if ( 0 < ( drive_ct = FindPortables( portable_drive, sizeof( portable_drive ) / sizeof( portable_drive [ 0 ] ) ) ) )
			{
				int i ;

				for ( i = 0 ; !isOK && i < drive_ct ; i++ )
				{
					if ( portable_drive [ i ].exfat_format )
					{
						SQLCursor 	drive_query ;
						stringt     current_time_str ;

						current_time_str = CurrentTimeString( ) ;
                        path = portable_drive[ i ].mount_point ;
                    
						// we've found a qualified drive.
						isOK = true ;

						// Now, see if it's already in the database
						if ( laptop_db.ExecuteQuery( drive_query, _T( "SELECT * FROM %s WHERE volume_name='%s'" ), LAPTOP_DB_PORTABLE_DRIVES_TABLE, portable_drive[ i ].volume_name ) )
						{
							laptop_db.ExecuteSQL( _T( "UPDATE PortableDriveTable SET last_seen_date='%s' WHERE volume_name='%s' " ), (const TCHAR *) current_time_str, portable_drive[ i ].volume_name ) ;
						}
						else
						{
							laptop_db.ExecuteSQL( _T( "INSERT INTO PortableDriveTable ( volume_name, last_seen_date, first_seen_date, file_write_count, disk_use_count ) VALUES ( '%s', '%s', '%s', 0, 0 )" ),
								portable_drive[ i ].volume_name,
								(const TCHAR *) current_time_str,
								(const TCHAR *) current_time_str ) ;
						}
					}
					else
					{
						printf( _T( "Portable drive '%s' is not ExFAT format.\n" ), portable_drive [ i ].volume_name ) ;
					}
				}
			}

			if ( !isOK && try_no < 10 )
			{
                printf( _T( "Waiting for a portable drive... " ) ) ;
                fflush( stdout ) ;
				sleep( 5 ) ;
                printf( _T( "checking again\n" ) ) ;
			}
		}
		while ( !isOK && try_no++ < 10 ) ;
	}

#endif

    return isOK ;
}





#ifdef WIN32

/******************************************************************************
RemoveFile

	Remove a file

******************************************************************************/
bool RemoveFile( 
	const TCHAR * filename				// I - file to remove 
)
{
	return 0 != DeleteFile( filename ) ;
}

#else
/******************************************************************************
RemoveFile
	
	Remove a file - UNIX style 

******************************************************************************/
bool RemoveFile( 
	const TCHAR * filename				// I - filename to remove 
)
{
	return 0 == remove( filename ) ;
}

/******************************************************************************
RemoveDirectory

	Remove a directory UNIX style. The WIN32 version is part of the WIN32 API

******************************************************************************/
bool RemoveDirectory( 
	const TCHAR * pathname				// I - directory to remove 
) 
{
	return 0 == remove( pathname ) ;
}

#endif

/******************************************************************************
RemoveFileAndDirectory

	Delete a file, then delete the directory which contains it, continuing up
	the path until RemoveDirectory fails - 

******************************************************************************/
bool RemoveFileAndDirectory( 
	stringt file_name					// I - filename to delete 
) 
{
	bool isOK = false ;

	if ( ( isOK = RemoveFile( file_name ) ) )
	{
		int	last_slash ;

		// peel off the last component of the pathname, then try to delete the directory 
		while ( 0 < ( last_slash = file_name.ReverseFind( _T( '/' ) ) ) )
		{
			file_name = file_name.Left( last_slash ) ;
			if ( !RemoveDirectory( file_name ) )		// will succeed only if the directory is empty
				return isOK ;
		}
	}

	return isOK ;
}









/******************************************************************************
PromptForAnotherPortable

	We're done with the current portable drive, prompt the user to connect another
	and we'll continue whenhe hits return key 

******************************************************************************/
bool PromptForAnotherPortable( 
	stringt & portable_path				// O - gets path of new portable 
)
{
	stringt	old_portable_name ;
	stringt new_portable_path ;
	stringt new_portable_name ;

	old_portable_name = GetPortableVolumeName( portable_path ) ;

	for ( ; ; ) 
	{
		_tprintf( _T( "Some images still need to be backed up to another portable.\n\n" ) ) ;
		_tprintf( _T( "Eject drive \"%s\" and attach any other portable drive.\n\n" ), (const TCHAR *) old_portable_name ) ;
		_tprintf( _T( "Hit the return key to continue... " ) ) ;
        fflush( stdout ) ;
	
		// require a return key 
        while ( _T( '\n' ) != getchar( ) )
            ;

		if ( PortableDriveFound( new_portable_path ) )
		{
			new_portable_name = GetPortableVolumeName( new_portable_path ) ;

			if ( 0 == new_portable_name.CompareNoCase( old_portable_name ) ) 
				_tprintf( _T( "\n\nYou must eject drive \"%s\".\n\n" ), (const TCHAR *) old_portable_name ) ;
			else
			{
				portable_path = new_portable_path ;
				return true ;
			}
		}
		else
			_tprintf( _T( "\n\nNo drive found\n\n" ) ) ;
	}

	return false ;
}
