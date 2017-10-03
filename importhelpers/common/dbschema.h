/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



Database table and field definitions 

***************************************************************************************/

#ifndef laptopapp_dbschema_h
#define laptopapp_dbschema_h

// db on laptop 
#define LAPTOP_DB_NAME								_T( "laptop.db" ) 

#define LAPTOP_DB_FILE_TABLE						_T( "MasterFileTable" )
#define LAPTOP_DB_FILE_TABLE_DEFINITION				_T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "file_name UNIQUE NOT NULL,\n" ) \
													_T( "file_size NOT NULL,\n" ) \
													_T( "source_path,\n" ) \
													_T( "portable_count INTEGER,\n" ) \
													_T( "imported_to_desktop INTEGER,\n" ) \
													_T( "time_imported_to_desktop,\n" ) \
													_T( "deleted_from_laptop INTEGER,\n" ) \
													_T( "required_redundancy INTEGER\n" )

// external drive table 
#define LAPTOP_DB_PORTABLE_DRIVES_TABLE				_T( "PortableDriveTable" )
#define LAPTOP_DB_PORTABLE_DRIVES_TABLE_DEFINITION  _T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "volume_name UNIQUE NOT NULL,\n" ) \
                                                    _T( "last_seen_date,\n" ) \
                                                    _T( "first_seen_date,\n" ) \
                                                    _T( "file_write_count INTEGER,\n" ) \
                                                    _T( "disk_use_count INTEGER\n" )

#define LAPTOP_DB_X_PORTABLE_MAP_TABLE				_T( "MasterFileXPortableMapping" )
#define	LAPTOP_DB_X_PORTABLE_MAP_DEFINITION			_T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "master_file_record INTEGER,\n" ) \
													_T( "portable_drive_record INTEGER,\n" ) \
													_T( "import_known_to_portable INTEGER\n" ) 
// db on portable drive
#define PORTABLE_DB_NAME							_T( "portable.db" )
#define PORTABLE_DB_FILE_TABLE						_T( "PortableFileTable" )
#define PORTABLE_DB_FILE_TABLE_DEFINITION			_T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "file_name UNIQUE NOT NULL,\n" ) \
													_T( "file_size NOT NULL,\n" ) \
													_T( "file_path,\n" ) \
													_T( "rating INTEGER,\n" ) \
													_T( "color_label,\n" ) \
													_T( "pick INTEGER,\n" ) \
													_T( "imported_to_desktop INTEGER,\n" ) \
													_T( "new_import_to_desktop INTEGER,\n" ) \
													_T( "time_imported_to_desktop,\n" ) \
													_T( "time_copied_to_portable\n" )

#define PORTABLE_DB_KEYWORDS_TABLE					_T( "PortableKeywordsTable" )
#define	PORTABLE_DB_KEYWORDS_TABLE_DEFINITION		_T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "keyword UNIQUE NOT NULL,\n" ) \
													_T( "parent INTEGER,\n" ) \
													_T( "date_created,\n" ) \
													_T( "last_applied\n" )

#define PORTABLE_DB_KEYWORDS_X_IMAGES_TABLE			_T( "KeywordsXImagesTable" )
#define PORTABLE_DB_KEYWORDS_X_IMAGES_TABLE_DEF		_T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "keyword_rec NOT NULL,\n" ) \
													_T( "image_rec NOT NULL\n" ) 

#define PORTABLE_DB_STACKING_TABLE					_T( "StackingTable" ) 
#define	PORTABLE_DB_STACKING_TABLE_DEFINITION		_T( "rec_no INTEGER PRIMARY KEY,\n" ) \
													_T( "file_id INTEGER,\n" ) \
													_T( "stack_position INTEGER,\n" ) \
													_T( "collapsed INTEGER,\n" ) \
													_T( "stack_ID INTEGER\n" )

#endif
