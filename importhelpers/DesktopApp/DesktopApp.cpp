/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



main() and key functions of desktop app

***************************************************************************************/

#include "stdafx.h"
#include "winioctl.h"
#include "DesktopApp.h"
#include "dbschema.h"
#include "tstring.h"
#include "sqlitedb.h"
#include "dbwrappers.h"
#include "importhelpers.h"
#include <list>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

bool PortableDetected( stringt & portable_path ) ;
bool IdentifyImportedFiles( const stringt & portable_path ) ;
bool FindOrphanedFiles( const stringt & portable_path ) ;
bool ChangeLightroomDB( ) ;	






/******************************************************************************
_tmain

	If we detect a portable drive, we then process imported files, then check
	for orphaned files. "orphaned files" are image files found on the portable
	drive which are neither marked as "imported to desktop" nor found in the 
	current lightroom database. 

	If orphaned files are found, we give the user a chance to pick another
	lightroom database to run against. 

	At present, the only mechanism a user has for picking another lightroom 
	catalog is to load lightroom and then open the desired catalog, then 
	exit lightroom. 

******************************************************************************/
int _tmain(	int argc, TCHAR* argv[], TCHAR* envp[] )
{
	int nRetCode = 0;

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
			try
			{
				stringt portable_path ;
				stringt lightroom_path ;
				LightroomDatabase lr_db ;

				IdentifyThisExecutable( _T( "Desktop Import Helper" ) ) ;

				if ( lr_db.Open( ) )
				{
					lr_db.GetLightroomDBPath( lightroom_path ) ;
					lr_db.Close( ) ;

					if ( PortableDetected( portable_path ) ) 
					{
						_tprintf( _T( "Lightroom database: \"%s\"\n" ), (const TCHAR *) lightroom_path ) ;
						_tprintf( _T( "          Portable: \"%s\"\n" ), (const TCHAR *) GetPortableVolumeName( portable_path ) ) ;

						do
						{
							IdentifyImportedFiles( portable_path ) ;
						}
						while ( FindOrphanedFiles( portable_path ) && ChangeLightroomDB( ) ) ;
					}
					else
						_tprintf( _T( "No usable portable drive found.\n" ) ) ;
				}
				else
					_tprintf( _T( "Unable to identify Lightroom database.\n" ) ) ;
			} 
			catch ( SQLDatabaseException & sql_exception ) 
			{
				DisplayExceptionAndTerminate( sql_exception ) ;
			}
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}






/******************************************************************************
PortableDetected

	Looks for portable drives and returns true if one is found. 

******************************************************************************/
bool PortableDetected( 
	stringt & portable_path				// O - receives path of root directory on portable 
)
{
	stringt	test_path ;
	bool	isOK = false ;
	TCHAR	drives_buffer[ 26 * 8 ] ;
	int		char_ct ;
	TCHAR *	next_drive ;

	// fills a buffer with something like "C:\.D:\.E:\.F:\.G:\.M;\.Z:\." - where '.' represents the nul character, \0 
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
				TCHAR	output_buffer[ 4 * 1024 ] ;
				DWORD	bytes_returned ;

				// this just weeds out the local hard drives and CD-ROMS. The first FILE_DEVICE_DISK we see with portable.db in the root is the winner
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
								portable_path.Format( _T( "%s" ), next_drive ) ;
								portable_path.Replace( _T( "\\" ), _T( "/" ) ) ;
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

	return isOK ;
}





/******************************************************************************
CreateGuidString

	Some items in the lightroom db require a GUID for the id_global field. So we
	use this function to create a GUID and format it the way Lightroom likes 

******************************************************************************/
void CreateGuidString( 
	stringt & guid_str					// O - gets a formatted string representation of a GUID 
) 
{
	GUID	guid ;

	CoCreateGuid( &guid ) ;
	guid_str.Format( _T( "%08.8lX-%04.4X-%04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X" ), guid.Data1, guid.Data2, guid.Data3, guid.Data4[ 0 ], guid.Data4[ 1 ], guid.Data4[ 2 ], guid.Data4[ 3 ], guid.Data4[ 4 ], guid.Data4[ 5 ], guid.Data4[ 6 ], guid.Data4[ 7 ] )  ;
}









struct Keyword
{
	struct Keyword *	parent ;			// parent keyword, or NULL if root keyword  - do NOT delete this pointer, it is a reference into the list 
	int					keyword_no ;		// keyword rec_no from portable db 
	int					parent_no ;			// recno of parent in the portable_db 
	int					lr_recno ;			// recno of this keyword in the destkop's lightroom db 
	stringt				keyword_text ;		// the actual keyword
	stringt				date_created ;
	stringt				last_applied ;
	stringt				lr_genealogy ;		// genealogy from the desktop's lightroom db 

	Keyword( )
	{
		parent = NULL ;
		keyword_no = 0 ;
		parent_no = 0 ;
		lr_recno = 0 ;
	}
} ;
struct KeywordByImage
{
	int					portable_recno ;
	list< Keyword * >	applied_keyword ;	// list of pointers to Keyword - do NOT delete these pointers, they are references to memory allocated building the Keyword list 

	KeywordByImage( )
	{
		portable_recno = 0 ;
	} 
} ;





/******************************************************************************
FindKeywordsAppliedToImage

	If any keywords are applied to a particular image, identified by its rec# from
	the portable db, this function returns a pointer to the KeywordByImage 
	structure created under LoadKeywords(). This structure contains a list 
	of keywords applied to the image 

	If no keywords are applied, the return is NULL 

******************************************************************************/
KeywordByImage * FindKeywordsAppliedToImage( 
	list< KeywordByImage * > & kw_by_image_list,		// I - the list of keywords by image 
	int portable_file_recno								// I - portable db rec# of image file 
)
{
	list< KeywordByImage * >::iterator list_iterator ;

	for ( list_iterator = kw_by_image_list.begin( ) ; list_iterator != kw_by_image_list.end( ) ; list_iterator++ )
		if ( ( *list_iterator )->portable_recno == portable_file_recno ) 
			return *list_iterator ;

	return NULL ;	// no keywords applied to this file 
}







/******************************************************************************
FindKeywordByRecno

	Search the list of Keyword pointers for the Keyword pointer for a given
	keyword number 

	keyword number is the keyword's record number from the portable db 

******************************************************************************/
Keyword * FindKeywordByRecno( 
	list< Keyword * > & kw_list,		// I - list of Keyword pointers 
	int keyword_no						// I - keyword # to look for 
) 
{
	list< Keyword * >::iterator	kw_iterator ;

	for ( kw_iterator = kw_list.begin( ) ; kw_iterator != kw_list.end( ) ; kw_iterator++ )
	{
		if ( keyword_no == ( *kw_iterator )->keyword_no ) 
			return *kw_iterator ;		// found it!
	}
	
	return NULL ;		// not found 
}







/******************************************************************************
LoadKeywords

	Read the portable_db's PortableKeywordsTable and KeywordsXImagesTabl and
	build in-memory structures representing the relationships found therein 

	kw_list: 
		a list of Keywords, each corresponds to a key word found in the 
		portable db, and each Keyword structure contains a pointer to 
		its parent Keyword, which is guaranteed to also be in the list

	kw_by_imge_list: 
		a list of structures, each corresponds to an image with keywords 
		applied to it, and each contains a list of pointers to 
		all the Keywords applied to this image 

******************************************************************************/
void LoadKeywords( 
	SQLDatabase & portable_db,						// I - the portable db
	list< Keyword * > & kw_list,					// O - list of keywords 
	list< KeywordByImage * > & kw_by_image_list		// O - see comment above fct. 
)
{
	SQLCursor	kw_query ;

	// read keywords from the portable db 
	if ( portable_db.ExecuteQuery( kw_query, _T( "SELECT * FROM PortableKeywordsTable" ) ) )
	{
		Keyword *	cur_keyword = NULL ;

		do
		{
			cur_keyword = new Keyword ;

			kw_query.GetFieldValue( _T( "rec_no" ), cur_keyword->keyword_no ) ;
			kw_query.GetFieldValue( _T( "parent" ), cur_keyword->parent_no ) ;
			kw_query.GetFieldValue( _T( "keyword" ), cur_keyword->keyword_text ) ;
			kw_query.GetFieldValue( _T( "date_created" ), cur_keyword->date_created ) ;
			kw_query.GetFieldValue( _T( "last_applied" ), cur_keyword->last_applied ) ;

			/**
				When date_created and last_applied strings are inserted into the lightroom db, 
				they are unquoted. This causes SQLite to take them as floating point values rather
				than strings. 
				
				But we can't insert empty unquoted strings, since this results in a SQL error, so
				we convert an empty string to "NULL" and SQL is happy 
			**/
			if ( cur_keyword->date_created.IsEmpty( ) )
				cur_keyword->date_created = _T( "NULL" ) ;

			if ( cur_keyword->last_applied.IsEmpty( ) )
				cur_keyword->last_applied = _T( "NULL" ) ;

			kw_list.push_back( cur_keyword ) ;
		}
		while ( kw_query.StepNext( ) ) ;
	}


	// kw_list now has a list of all keywords we found. Now we process the list, setting 
	// the 'parent' member to point to the Keyword for each entry's parent, except for 
	// the 'root' entry, which has no parent 
	list< Keyword * >::iterator	kw_iterator ;

	for ( kw_iterator = kw_list.begin( ) ; kw_iterator != kw_list.end( ) ; kw_iterator++ )
	{
		if ( ( *kw_iterator )->parent_no ) 
		{
			list< Keyword * >::iterator	parent_iterator ;

			for ( parent_iterator = kw_list.begin( ) ; parent_iterator != kw_list.end( ) ; parent_iterator++ )
			{
				if ( ( *parent_iterator )->keyword_no == ( *kw_iterator )->parent_no )
				{
					( *kw_iterator )->parent = *parent_iterator ;
					break ;
				}
			}
			ASSERT( NULL != ( *kw_iterator )->parent ) ;
		}
		// else - parent_no == 0 ==> this Keyword is the root 
	}


	// now build a list of image #'s with a list of keywords applied 
	SQLCursor 			kw_x_image_cursor ;
	KeywordByImage *	kw_by_image = NULL ;

	int					last_portable_recno = -1 ;
	int					portable_recno ;
	int					kw_recno ;
	Keyword *			cur_keyword = NULL ;

	// read the keywords x image table. We need to get records ordered by image_rec #
	if ( portable_db.ExecuteQuery( kw_x_image_cursor, _T( "SELECT image_rec, keyword_rec FROM KeywordsXImagesTable ORDER BY image_rec ASC" ) ) )
	{
		do
		{
			kw_x_image_cursor.GetFieldValue( _T( "image_rec" ), portable_recno ) ;

			if ( portable_recno != last_portable_recno ) 
			{
				kw_by_image = new KeywordByImage ;
				kw_by_image->portable_recno = portable_recno ;
				last_portable_recno = portable_recno ;
				kw_by_image_list.push_front( kw_by_image ) ;
			}

			// get the Keyword pointer and queue it up 
			kw_x_image_cursor.GetFieldValue( _T( "keyword_rec" ), kw_recno ) ;
			cur_keyword = FindKeywordByRecno( kw_list, kw_recno ) ;
			ASSERT( cur_keyword != NULL ) ;
			kw_by_image->applied_keyword.push_back( cur_keyword ) ;
		}
		while ( kw_x_image_cursor.StepNext( ) ) ;
	}

	return ;
}










/******************************************************************************
GuaranteeKeywordAndLineage

	When we have a Keyword applied to an image, we need to ensure that it appears
	in the AgLibraryKeyword table, as well as ALL its ancestors right back up
	to the 'root' keyword 

******************************************************************************/
bool GuaranteeKeywordAndLineage( 
	LightroomDatabase & lightroom_db,	// I/O - lightroom db 
	Keyword * kw						// I/O - Keyword pointer for the keyword 
) 
{
	bool		isOK = false ;
	SQLCursor	kw_dictionary ;

	// if the new record number for this keyword is unknown.... 
	if ( 0 == kw->lr_recno ) 
	{
		// is this keyword the 'root' ? 
		if ( kw->parent_no == 0 )
		{
			// keyword kw is the "root". We're guaranteed this lightroom db already has an entry for the "root" 
			if ( lightroom_db.ExecuteQuery( kw_dictionary, _T( "SELECT * FROM AgLibraryKeyword WHERE parent IS NULL" ) ) )
			{
				kw_dictionary.GetFieldValue( _T( "genealogy" ), kw->lr_genealogy ) ;
				kw_dictionary.GetFieldValue( _T( "id_local" ), kw->lr_recno ) ;

				isOK = true ;
			}
			else
			{
				ASSERT( FALSE ) ;
				// fall out to return false 
			}
		}
		else 
		{
			// if it's not the root, it MUST have an in-memory parent keyword 
			ASSERT( kw->parent != NULL ) ;

			// and first, we must guarantee that its ancestry is in the table
			if ( isOK = GuaranteeKeywordAndLineage( lightroom_db, kw->parent ) )
			{
				// check and see if this keyword is already in the table. This happens if we're adding keywords which are identical to ones already in the database 
				if ( lightroom_db.ExecuteQuery( kw_dictionary, _T( "SELECT * FROM AgLibraryKeyword WHERE parent = %d AND name = '%s'" ), kw->parent->lr_recno, (const TCHAR *) kw->keyword_text ) )
				{
					kw_dictionary.GetFieldValue( _T( "genealogy" ), kw->lr_genealogy ) ;
					kw_dictionary.GetFieldValue( _T( "id_local" ), kw->lr_recno ) ;

					isOK = true ;
				}
				else
				{
					// this keyword is not in the lightroom db - so we must add it 
					stringt	guid ;
					stringt	lc_name ;

					CreateGuidString( guid ) ;
					lc_name = kw->keyword_text ;
					lc_name.MakeLower( ) ;

					// we can't know the genealogy until we know our record number - so we insert with a dummy genealogy and then update 
					if ( isOK = lightroom_db.ExecuteSQL( _T( "INSERT INTO AgLibraryKeyword ( id_global, dateCreated, imageCountCache, lastApplied, lc_name, name, parent ) VALUES ( '%s', %s, NULL, %s, '%s', '%s', %d ) " ), 
												(const TCHAR *) guid, 
												(const TCHAR *) kw->date_created,			// notice the corresponding %s format specifier must be UNQUOTED
												// imageCountCache hard-coded as NULL
												(const TCHAR *) kw->last_applied,			// notice the corresponding %s format specifier must be UNQUOTED 
												(const TCHAR *) lc_name,
												(const TCHAR *) kw->keyword_text,	
												kw->parent->lr_recno ) )
					{
						// need to know the record number to format the genealogy field 
						kw->lr_recno = lightroom_db.GetLastInsertRowID( ) ;
						kw->lr_genealogy.Format( _T( "%s/3%d" ), (const TCHAR *) kw->parent->lr_genealogy, kw->lr_recno ) ;

						isOK = lightroom_db.ExecuteSQL( _T( "UPDATE AgLibraryKeyword SET genealogy='%s' WHERE id_local = %d" ), (const TCHAR *) kw->lr_genealogy, kw->lr_recno ) ;
					}
				}
			}
			else 
				ASSERT( FALSE ) ;		// GuaranteeKeywordAndLineage for parent FAILED!?!?!?
		}
	}
	else
		isOK = true ;	// else - nonzero lr_recno proves this keyword is already in the AgLibraryKeyword table
	

	return isOK ;
}







/******************************************************************************
RecordAppliedKeyword

	We know that a keyword is applied to an image corresponding to lr_file_recno, 
	so apply the keyword to all the image records for the image file 

	Even though we will only import FILES form the laptop, the lightroom db 
	associates keywords with IMAGES. 

******************************************************************************/
bool RecordAppliedKeyword( 
	LightroomDatabase & lightroom_db,		// I/O - the lightroom db we're updating 
	int lr_file_recno,						// I - the record number of the image file in AgLibraryFile 
	Keyword * kw							// I/O - pointer to Keyword to apply to this image 
) 
{
	bool			isOK = false ;
	SQLCursor 		images ;
	int				image_no ;

	ASSERT( kw->lr_recno != 0 ) ;

	if ( lightroom_db.ExecuteQuery( images, _T( "SELECT id_local FROM Adobe_images WHERE rootFile = %d" ), lr_file_recno ) )
	{
		do
		{
			images.GetFieldValue( _T( "id_local" ), image_no ) ;

			isOK = lightroom_db.ExecuteSQL( _T( "INSERT INTO AgLibraryKeywordImage (image, tag ) VALUES ( %d, %d )" ), image_no, kw->lr_recno ) ;
		}
		while ( isOK && images.StepNext( ) ) ;
	}
	else
		ASSERT( FALSE ) ;

	return isOK ;
}




/******************************************************************************
IdentifyImportedFiles

	This is the place where we do the bulk of our work. 

	We scam through the portable_db to identify files which might have been freshly
	added to lightroom. 

	If one of these candidates is found in the lightroom database, we know it has
	been imported. 

	We check for and migrate any color labeling, stacking, or pick status into the
	desktop database. 

	We then apply any keywords 

******************************************************************************/
bool IdentifyImportedFiles( 
	const stringt & portable_path		// I - the root directory of the portable drive 
)
{
	bool isOK = true ;
	PortableDatabase	portable_db ;
	LightroomDatabase	lightroom_db ;

	if ( portable_db.Open( portable_path ) ) 
	{
		if ( lightroom_db.Open( ) )
		{
			SQLCursor 	files_of_interest ;
			SQLCursor 	imported_file ;
			SQLCursor  rename_files ;

			// First, check the lightroom configuration. 
			// we CANNOT deal with a lightroom db where imports from the portable will be renamed on us.... 
			if ( lightroom_db.ExecuteQuery( rename_files, _T( "SELECT * FROM Adobe_variablesTable WHERE name = 'AgImportDialog_renamingTokensOn' AND value <> 0" ) ) )
			{
				_tprintf( _T( "WARNING: the selected Lightroom Catalog is configured to rename files on import.\n" ) ) ;
				_tprintf( _T( "\n" ) ) ;
				_tprintf( _T( "Therefore, filenames on the desktop will never match filenames on the portable\n" ) ) ;
				_tprintf( _T( "drive and the laptop. The 'rename files' option on the import page MUST be\n" ) ) ;
				_tprintf( _T( "unchecked.\n" ) ) ;
				_tprintf( _T( "\n" ) ) ;
				_tprintf( _T( "Please delete the imported files, and re-run the import.\n" ) ) ;
				_tprintf( _T( "\n" ) ) ;

				exit( -1 ) ;
			}


			// Scan the portable DB for files which have NOT yet been recorded as imported (but which actually turn up in the lightroom db) 
			if ( portable_db.ExecuteQuery( files_of_interest, _T( "SELECT * FROM PortableFileTable WHERE imported_to_desktop = 0" ) ) )
			{
				list< Keyword * >			kw_list ;
				list< KeywordByImage * >	kw_by_image_list ;

				// read in all the keyword dictionary from portable_db 
				LoadKeywords( portable_db, kw_list, kw_by_image_list ) ;

				do
				{
					stringt		file_name ;
					stringt		current_time ;

					files_of_interest.GetFieldValue( _T( "file_name" ), file_name ) ;

					// ok - here's where we check if it's in the lr database 
					if ( lightroom_db.ExecuteQuery( imported_file, _T( "SELECT * FROM AgLibraryFile WHERE lc_idx_filename = '%s'" ), (const TCHAR *) file_name ) )
					{
						_tprintf( _T( "%s was imported" ), (const TCHAR *) file_name ) ;

						// at this point, we've identified a newly imported file 
						int			portable_file_recno ;
						int			lr_file_recno ;
						SQLCursor 	stack_by_file_id ;

						stringt		color_label ;
						int			pick ;
						int			rating ;

						// get the interesting fields from the PortableFileTable record 
						files_of_interest.GetFieldValue( _T( "rating" ), rating ) ;
						files_of_interest.GetFieldValue( _T( "color_label" ), color_label ) ;
						files_of_interest.GetFieldValue( _T( "pick" ), pick ) ;
						files_of_interest.GetFieldValue( _T( "rec_no" ), portable_file_recno ) ;

						imported_file.GetFieldValue( _T( "id_local" ), lr_file_recno ) ;

						/**
						 We now have the lightroom_db record # corresponding to portable_db record # for the file being processed - check for any applied keywords 
						 and update the lightroom_db keyword tables accordingly 
						**/
						KeywordByImage *	kw_by_image ;

						if ( kw_by_image = FindKeywordsAppliedToImage( kw_by_image_list, portable_file_recno ) ) 
						{
							list< Keyword * >::iterator kw_iterator ;

							_tprintf( _T( "  ...applying keywords " ) ) ;

							for ( kw_iterator = kw_by_image->applied_keyword.begin( ) ; kw_iterator != kw_by_image->applied_keyword.end( ) ; kw_iterator++ )
							{
								_tprintf( _T( "." ) ) ;
								GuaranteeKeywordAndLineage( lightroom_db, ( *kw_iterator ) ) ;
								RecordAppliedKeyword( lightroom_db, lr_file_recno, ( *kw_iterator ) ) ;
							}

							_tprintf( _T( " (done)" ) ) ;
						}

						// now do color label, pick, rating.... 
						lightroom_db.ExecuteSQL( _T( "UPDATE Adobe_images SET colorLabels='%s', pick=%d, rating=%d WHERE rootFile=%d" ), (const TCHAR *) color_label, pick, rating, lr_file_recno ) ;

						// stacking is a bit complicated... 
						if ( portable_db.ExecuteQuery( stack_by_file_id, _T( "SELECT * FROM StackingTable WHERE file_id = %d" ), portable_file_recno ) )
						{
							// a given image may be in only one stack_by_file_id at a time. 
							int		stack_position ;
							int		portable_stack_id ;
							int		collapsed ;

							stack_by_file_id.GetFieldValue( _T( "stack_position" ), stack_position ) ;
							stack_by_file_id.GetFieldValue( _T( "stack_ID" ), portable_stack_id ) ;
							stack_by_file_id.GetFieldValue( _T( "collapsed" ), collapsed ) ;

							if ( stack_position == 1 ) 
							{
								/*
									if this file is the top image in the stack, then propogate the stack definition to the desktop database.
									On first re-reading of this code, my initial thought was "Oh, this is wrong! We haven't necessarily processed
									the remaining files in the stack!!" 

									But keep in mind that the lightroom database has already got the new files in it. So the files are on the desktop
									and in the lightroom database. 
								*/
								stringt	guid_str ;
								int		lr_stack_record ;

								CreateGuidString( guid_str ) ;

								// create the stack record in AgLibraryFolderStack - 
								lightroom_db.ExecuteSQL( _T( "INSERT INTO AgLibraryFolderStack ( id_global, collapsed, text ) VALUES ( '%s', %d, '' )" ), (const TCHAR *) guid_str, collapsed ) ;
								lr_stack_record = lightroom_db.GetLastInsertRowID( ) ;

								SQLCursor 	portable_stack ;
								SQLCursor	lr_image ;
								SQLCursor 	portable_file_in_stack ;

								if ( portable_db.ExecuteQuery( portable_stack, _T( "SELECT * FROM StackingTable WHERE stack_ID = %d" ), portable_stack_id ) ) 
								{
									do
									{
										// we intentionally re-declare these identifiers b/c the semantics have changed. Now they're associated with the 
										// record(s) in the stack as we step through the contents of the stack - 
										int		portable_file_recno ;	
										int		stack_position ;		
										stringt	file_name ;

										portable_stack.GetFieldValue( _T( "file_id" ), portable_file_recno ) ;
										portable_stack.GetFieldValue( _T( "stack_position" ), stack_position ) ;

										if ( portable_db.ExecuteQuery( portable_file_in_stack, _T( "SELECT * FROM PortableFileTable WHERE ROWID = %d" ), portable_file_recno ) ) 
										{
											portable_file_in_stack.GetFieldValue( _T( "file_name" ), file_name ) ;

											if ( lightroom_db.ExecuteQuery( lr_image, _T( "SELECT Adobe_images.id_local FROM Adobe_images, AgLibraryFile WHERE Adobe_images.rootFile = AgLibraryFile.id_local AND AgLibraryFile.lc_idx_filename = '%s'" ), file_name ) )
											{
												int	lr_image_recno ;

												lr_image.GetFieldValue( 0, lr_image_recno ) ;
												lightroom_db.ExecuteSQL( _T( "INSERT INTO AgLibraryFolderStackImage( collapsed, image, position, stack ) VALUES ( %d, %d, %d, %d )" ), collapsed, lr_image_recno, stack_position, lr_stack_record ) ;
											}
											else
												ASSERT( FALSE ) ;	// should never happen.... 

										}
										else
											ASSERT( FALSE ) ;	// should never happen!!
									}
									while ( portable_stack.StepNext( ) ) ;
								}
								else
									ASSERT( FALSE ) ;		// should NEVER happen. We're executing this line b/c we found the top file in this stack!!
							}
						}

						current_time = CurrentTimeString( ) ;

						portable_db.ExecuteSQL( _T( "UPDATE PortableFileTable SET imported_to_desktop=1, new_import_to_desktop=1, time_imported_to_desktop='%s' WHERE file_name = '%s'" ), (const TCHAR *) current_time, (const TCHAR *) file_name ) ;

						_tprintf( _T( "\n" ) ) ;
					}
				}
				while ( files_of_interest.StepNext( ) ) ;

				// clean up all the dreck in memory left after keyword processing 
				while ( !kw_list.empty( ) ) 
				{
					delete kw_list.front( ) ;
					kw_list.pop_front( ) ;
				}

				while ( !kw_by_image_list.empty( ) )
				{
					while ( !kw_by_image_list.front( )->applied_keyword.empty( ) )
						kw_by_image_list.front( )->applied_keyword.pop_front( ) ;	// even though this is a list of pointers, the memory has already been deleted

					delete kw_by_image_list.front( ) ;
					kw_by_image_list.pop_front( ) ;
				}
			}
		}
	}

	return isOK ;
}








/******************************************************************************
FindOrphanedFiles

	Identify files on the portable which have NOT been imported to the LR database 
	and warn the user about them 

******************************************************************************/
bool FindOrphanedFiles( 
	const stringt & portable_path		// I - path to root of portable drive 
)
{
	bool				orphan_found = false ;
	PortableDatabase	portable_db ;

	if ( portable_db.Open( portable_path ) ) 
	{
		SQLCursor		orphaned_file ;

		if ( portable_db.ExecuteQuery( orphaned_file, _T( "SELECT file_name, file_path FROM PortableFileTable WHERE imported_to_desktop=0" ) ) )
		{
			stringt		volume_name ;
			stringt		file_name ;
			stringt		file_path ;

			orphan_found = true ;

			volume_name = GetPortableVolumeName( portable_path ) ;

			_tprintf( _T( "Files were found on portable \"%s\" that were not imported to the lightroom database:\n" ), (const TCHAR *) volume_name ) ;

			do
			{
				orphaned_file.GetFieldValue( _T( "file_name" ), file_name ) ;
				orphaned_file.GetFieldValue( _T( "file_path" ), file_path ) ;

				_tprintf( _T( "%40s in %s\n" ), (const TCHAR *) file_name, (const TCHAR *) file_path ) ;
			}
			while ( orphaned_file.StepNext( ) ) ;

			_tprintf( _T( "\n\n" ) ) ;
			_tprintf( _T( "This may be the result of either forgetting to import some files, or importing files to different lightroom databases.\n\n" ) ) ;
		}
	}

	return orphan_found ;
}







/******************************************************************************
ChangeLightroomDB

	Offer the user a chance to swap lightroom databases underneath us. We print
	a prompt, and the user can load lightroom, select a new catalog, exit
	lightroom, and then enter "Y" here... 

******************************************************************************/
bool ChangeLightroomDB( )
{
	bool	retry = false ;
	bool	invalid_input = false ;
	stringt	input ;
	TCHAR *	ptr ;

	_tprintf( _T( "If you imported photographs to another Lightroom catalog, load Lightroom and\n" ) ) ;
	_tprintf( _T( "select that catalog.\n" ) ) ;
	_tprintf( _T( "\n" ) ) ;
	_tprintf( _T( "If you believe there are photographs on the portable which were not imported\n" ) ) ;
	_tprintf( _T( "to any Lightroom catalog, load Lightroom and import them to the appropriate\n" ) ) ;
	_tprintf( _T( "catalog.\n" ) ) ;
	_tprintf( _T( "\n" ) ) ;
	_tprintf( _T( "In either case, remember to exit Lighroom before letting this application\n" ) ) ;
	_tprintf( _T( "continue.\n" ) ) ;
	_tprintf( _T( "\n" ) ) ;

	do
	{
		invalid_input = false ;

		_tprintf( _T( "\n\nRe-check for imported files (Y/N)? " ) ) ;

		ptr = input.GetBufferSetLength( 10 ) ;
		_fgetts( ptr, 10, stdin ) ;

		input.ReleaseBuffer( ) ;
		input.Trim( ) ;
		input.MakeUpper( ) ;
		input = input.Left( 1 ) ;

		if ( 0 == input.Compare( _T( "Y" ) ) )
			retry = true ;
		else if ( 0 == input.Compare( _T( "N" ) ) ) 
			retry = false ;		
		else
		{
			_tprintf( _T( "\n\nPlease answer Y (or \"Yes\") or N (or \"No\")\n\n" ) ) ;
			invalid_input = true ;
		}
	}
	while ( invalid_input ) ;

	return retry ;
}
