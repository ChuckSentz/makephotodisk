/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	The MakePhotoDisk3Doc document object.  This encapsulates the options 
	selected for a disk project (stored primarily in the .dp file in the "hidden" 
	subdirectory under the disk's root directory) as well as all SQL interactions
	with the lightroom database and the only API's calling into the custom 
	metadata database. 

***************************************************************************************/

#include "stdafx.h"
#include "sqlitedb.h"
#include "MakePhotoDisk3.h"
#include "MakePhotoDisk3Doc.h"
#include "mainfrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMakePhotoDisk3Doc

IMPLEMENT_DYNCREATE(CMakePhotoDisk3Doc, CDocument)

BEGIN_MESSAGE_MAP(CMakePhotoDisk3Doc, CDocument)
	ON_COMMAND(ID_FILE_SAVE, &CMakePhotoDisk3Doc::OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CMakePhotoDisk3Doc::OnUpdateFileSave)
END_MESSAGE_MAP()


								// do NOT enclose these in _T() !!! They are ASCII strings!!!
#define CROPPED_HEIGHT_MARKER	"croppedHeight = "		
#define CROPPED_WIDTH_MARKER	"croppedWidth = "
#define	MIN_PREVIEW_DIMENSION	70






/******************************************************************************
CMakePhotoDisk3Doc::CMakePhotoDisk3Doc

	Only constructor for the document object

******************************************************************************/
CMakePhotoDisk3Doc::CMakePhotoDisk3Doc()
{
	m_disk_type = UndefinedDiskType ;
	m_sel_collection = NULL ;
	m_last_selection = -1 ;
	m_anchor_selection = -1 ;
	m_selection_count = 0 ;
	m_doc_state = Virgin ;
	m_disk_title = _T( "Untitled disk" ) ;
	m_disk_size = 0 ;
	m_custom_metadata_edits = false ;
	m_sort_by_capture_time = false ;
	m_ascending_sort = false ;

	m_do_big_presentation = false ;
	m_do_compressed = false ;
	m_do_facebook = false ;
	m_do_full_size = false ;

	m_watermark_presentation = false ;
	m_watermark_compressed = false ;
	m_watermark_facebook = false ;
	m_watermark_full_size = false ;

	TCHAR	module_path[ MAX_PATH ] ;
	TCHAR	path[ MAX_PATH ] ;
	TCHAR	drive[ MAX_PATH ] ;

	::GetModuleFileName( NULL, module_path, sizeof( module_path ) / sizeof( module_path[ 0 ] ) ) ;
	_tsplitpath_s( module_path, drive, sizeof( drive ) / sizeof( drive[ 0 ] ), path, sizeof( path ) / sizeof( path[ 0 ] ), NULL, 0, NULL, 0 ) ;
	_tmakepath_s( module_path, sizeof( module_path ) / sizeof( module_path[ 0 ] ), drive, path, NULL, NULL ) ;
	m_app_path = module_path ;
}





/******************************************************************************
CMakePhotoDisk3Doc::~CMakePhotoDisk3Doc

	Destructor. 

******************************************************************************/
CMakePhotoDisk3Doc::~CMakePhotoDisk3Doc()
{
	DeletePhotoList( ) ;

	while ( m_collection_list.GetCount( ) )
		delete m_collection_list.RemoveHead( ) ;
}






/******************************************************************************
CMakePhotoDisk3Doc::OnNewDocument

	Delete allocated memory and reset most variables 

******************************************************************************/
BOOL CMakePhotoDisk3Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	while ( m_collection_list.GetCount( ) )
		delete m_collection_list.RemoveHead( ) ;

	while ( m_collection_pic_list.GetCount( ) ) 
		delete m_collection_pic_list.RemoveHead( ) ;

	m_project_location.Empty( ) ;
	m_project_directory.Empty( ) ;
	m_catalog_path.Empty( ) ;

	m_disk_title = _T( "Untitled disk" ) ;

	m_disk_type = UndefinedDiskType ;
	m_sel_collection = NULL ;
	m_last_selection = -1 ;
	m_anchor_selection = -1 ;
	m_selection_count = 0 ;
	m_doc_state = Virgin ;
	m_disk_size = 0 ;
	m_major_revision = 1 ;
	m_minor_revision = 0 ;

	return TRUE;
}






/******************************************************************************
CMakePhotoDisk3Doc::ResetSelectFlags

	clear "selected" status for all photos

******************************************************************************/
void CMakePhotoDisk3Doc::ResetSelectFlags( )
{
	POSITION		pos ;
	PhotoImage *	photo ;

	for ( photo = GetFirstPhoto( pos ) ; NULL != photo ; photo = GetNextPhoto( pos ) )
		photo->SetSelectedFlag( false ) ;

	m_selection_count = 0 ;
}





/******************************************************************************
CMakePhotoDisk3Doc::DeletePhotoList

	Delete all photos in the list, after saving custom edits to metadata. Any
	custom edits for a photo will be available when that same photo is 
	included in another disk 

******************************************************************************/
void CMakePhotoDisk3Doc::DeletePhotoList( ) 
{
	CheckSaveCustomMetadata( ) ;

	while ( m_collection_pic_list.GetCount( ) )
		delete m_collection_pic_list.RemoveHead( ) ;
}






/******************************************************************************
CMakePhotoDisk3Doc::GetLastSelectedImage

	Return the last selected image, or NULL if none 

******************************************************************************/
PhotoImage * CMakePhotoDisk3Doc::GetLastSelectedImage( ) const
{
	PhotoImage * sel_image = NULL ;

	if ( m_last_selection != -1 )
	{
		POSITION	pos ;

		sel_image = FindPhoto( m_last_selection, pos ) ;
	}

	return sel_image ;
}







/******************************************************************************
CMakePhotoDisk3Doc::SetCatalog

	Called when user has selected a new catalog. We build a list (really a tree) 
	of the collections defined in the catalog. 

	We have to be savvy about collections which contain other collections (rather 
	than images) and need to ignore system-defined "smart collections" (eg, all 
	images of a given color, or rating, etc.).

******************************************************************************/
bool CMakePhotoDisk3Doc::SetCatalog( 
	const TCHAR * catalog					// I - path of lightroom catalog to open 
)
{
	bool		isOK = false ;
	SQLDatabase	lightroom_db ;
	bool		retry_db_open = false ;

	// new catalog ==> new document, too 
	OnNewDocument( ) ;

	do
	{
		retry_db_open = false ;

		try
		{
			if ( lightroom_db.Open( catalog ) )
			{
				m_catalog_path = catalog ;

				/*
				*	First thing we do is get all the top-level collections. That is, collections who's parent is NULL, meaning they're 
				*	not contained within any collection set. 
				*/
				SQLCursor	db_cursor ;
				int			smart_collections_id = 0 ;

					// list the collections at the "root" level, that is, their parent is NULL 
				if ( lightroom_db.ExecuteQuery( db_cursor, _T( "SELECT * FROM AgLibraryCollection WHERE parent IS NULL AND SystemOnly = 0.0 ORDER BY name" ) ) )
				{
					isOK = true ;

					do
					{
						int			id_local ;
						stringt		name ;
						stringt		creation_id ;

						db_cursor.GetFieldValue( _T( "id_local" ), id_local ) ;
						db_cursor.GetFieldValue( _T( "name" ), name ) ;
						db_cursor.GetFieldValue( _T( "creationId" ), creation_id ) ;

							// do NOT enque "Smart Collections"... 
						if ( 0 == name.CompareNoCase( _T( "Smart Collections" ) ) )
						{
							// ... but make note of the "smart collection" id, so we don't try to enque smart collections below
							smart_collections_id = id_local ;
						}
						else
							m_collection_list.AddTail( new Collection( id_local, -1 != creation_id.Find( _T( ".group" ) ), name ) ) ;
					}
					while ( db_cursor.StepNext( ) ) ;
				}


				/*
					Now that we have all the top-level collections, we query the catalog for collections which DO have parents. 
					We may have to do this repeatedly, since collections may be nested within collections
				*/
				int		retry_no = 0 ;
				bool	retry ;


				/*****
				
					FIXME - Now that I know a lot more about SQL, there's a more efficient query which returns just collections which are
					collection-sets, for ex: 
					
						SELECT * FROM AgLibraryCollection WEHRE id_local IN ( SELECT parent FROM AgLibraryCollection )


				****/

				do
				{
					retry = false ;

					if ( lightroom_db.ExecuteQuery( db_cursor, _T( "SELECT * FROM AgLibraryCollection WHERE parent IS NOT NULL AND SystemOnly = 0.0 ORDER BY NAME" ) ) )
					{
						do
						{
							int			id_local ;
							stringt		name ;
							stringt		creation_id ;
							int			parent_id ;
							Collection * parent ;

							db_cursor.GetFieldValue( _T( "id_local" ), id_local ) ;
							db_cursor.GetFieldValue( _T( "parent" ), parent_id ) ;
							db_cursor.GetFieldValue( _T( "creationId" ), creation_id ) ;
							db_cursor.GetFieldValue( _T( "name" ), name ) ;


							// and do NOT bother with collections which are children of "Smart Collections" 
							if ( parent_id != smart_collections_id )		
							{
								if ( !FindCollectionByID( id_local ) ) 
								{
									if ( parent = FindCollectionByID( parent_id ) ) 
									{
										// have new collection, and have already got its parent - 
										parent->AddChild( new Collection( id_local, -1 != creation_id.Find( _T( ".group" ) ), name ) ) ;
									}
									else
										retry = true ;	// unable to add this collection b/c we haven't yet encountered its parent
								}
							}
						}
						while ( db_cursor.StepNext( ) ) ;
					}
				}
				while ( retry && ++retry_no < 100 ) ;	// retry if necessary - but don't just loop forever in case of a lightroom_db error 
				/*
				*	Looping up to 100 times passes pretty quickly and handles the absolute worst case where collections are nested > 100 deep... 
				*/

				if ( retry_no == 100 ) 
				{
					AfxMessageBox( _T( "There appears to be a grave error in the collection table, or there are collections nested over 100 levels deep" ), MB_OK ) ;
				}
			}
		}
		catch ( SQLDatabaseException & sql_exception ) 
		{
			int			msg_style = MB_ICONEXCLAMATION ;
			stringt		msg_text ;

			if ( sql_exception.RetryPossible( ) )
			{
				msg_style |= MB_RETRYCANCEL ;
				msg_text.Format( _T( "A database error (%d) has occurred.\n\n\"%s\"\n\nClick Cancel to exit, or Retry after resolving the problem." ), sql_exception.GetNativeError( ), sql_exception.GetErrorText( ) ) ;
			}
			else
			{
				msg_style |= MB_OK ;
				msg_text.Format( _T( "An unrecoverable database error (%d) has occurred.\n\n\"%s\"\n\nClick OK to exit." ), sql_exception.GetNativeError( ), sql_exception.GetErrorText( ) ) ;
			}

			if (! ( retry_db_open = ( IDRETRY == AfxMessageBox( msg_text, msg_style ) ) ) )
				AfxAbort( ) ;
		}
	}
	while ( retry_db_open ) ;

	return isOK ;
}



 




/******************************************************************************
CMakePhotoDisk3Doc::DeselectCollection

	Called when user is changing the selected collection 

******************************************************************************/
bool CMakePhotoDisk3Doc::DeselectCollection( )
{
	DeletePhotoList( ) ;

	m_sel_collection = NULL ;
	m_last_selection = -1 ;
	m_doc_state = Virgin ;
	m_anchor_selection = -1 ;
	m_selection_count = 0 ;

	return true ;
}






/******************************************************************************
CMakePhotoDisk3Doc::FindCollectionByID

	Used only during building the tree of collections. Uses RecursiveCollectionSearch
	to search the collection tree for a collection matching the provided ID 

******************************************************************************/
Collection * CMakePhotoDisk3Doc::FindCollectionByID( 
	int collection_id												// I - collection ID to look for 
) 
{
	return RecursiveCollectionSearch( m_collection_list, collection_id ) ;
}






/******************************************************************************
CMakePhotoDisk3Doc::RecursiveCollectionSearch
	
	Only called by FindCollectionByID( ) above. 

	First called to search the member-variable list of collections, and will
	search the child queue of any colleciton-set encountered 

******************************************************************************/
Collection * CMakePhotoDisk3Doc::RecursiveCollectionSearch( 
	CTypedPtrList< CPtrList, Collection *> & collection_list,		// I - list of collections to search 
	int collection_id												// I - collection ID to look for 
)
{
	POSITION		pos ;
	Collection *	current_collection ;

	if ( pos = collection_list.GetHeadPosition( ) )
	{
		do
		{
			Collection *	child ;

			current_collection = collection_list.GetNext( pos ) ;

			if ( collection_id == current_collection->GetIDLocal( ) )
				return current_collection ;

			// go all recursive with the child queue, if there is one 
			if ( current_collection->GetIsCollectionSet( ) )
				if ( child = RecursiveCollectionSearch( current_collection->GetChildQueue( ), collection_id ) )
					return child ;
		}
		while ( NULL != pos ) ;
	}

	return NULL ;
}










/******************************************************************************
CMakePhotoDisk3Doc::SelectCollection

	Called when a collection has been selected. 

	Queries the database to get the contents of a collection, then extracts the 
	preview jpegs. 

	Although we have a tree of the collections for the selected catalog, the 
	document has its own separate list of PhotoImage's representing the 
	contents of the selected collection. 

******************************************************************************/
bool CMakePhotoDisk3Doc::SelectCollection( 
	const Collection * collection				//	I - selected collection 
)
{
	SQLDatabase	lightroom_db ;
	bool		isOK = false ;

	ASSERT( m_collection_pic_list.GetCount( ) == 0 ) ;

	m_sel_collection = collection ;
	m_doc_state = CatalogCollectionKnown ;
	m_disk_title = collection->GetName( ) ;

	if ( lightroom_db.Open( m_catalog_path ) )
	{
		SQLCursor collection_cursor ;
		stringt	lrdata_path ;

		// get the sorting for the collection first - 
		if ( lightroom_db.ExecuteQuery( collection_cursor, _T( "SELECT * FROM AgLibraryCollectionContent WHERE collection = %d" ), collection->GetIDLocal( ) ) )
		{
			bool	ascending = true ;
			stringt	content ;
			stringt	sort_by_field ;
			stringt	module ;

			isOK = true ;
			
			do
			{
				collection_cursor.GetFieldValue( _T( "content" ), content ) ;
				collection_cursor.GetFieldValue( _T( "owningModule" ), module ) ;

				if ( 0 == module.Compare( _T( "com.adobe.ag.library.filter" ) ) )
				{
					ASSERT( content.GetLength( ) == 0 ) ;
				}
				else if ( 0 == module.Compare( _T( "com.adobe.ag.library.sortType" ) ) )
				{
					/* 
					   With most sort types, the content gives the exact name of the field to order the query results on. 
					   The field-name may be a field in Adobe_images or AgLibraryCollectionImage, or AgLibraryFile. 
				   
					   However, with some content values, we need to do some extra logic

					   "fileName" becomes "baseName" 
					   "addedOrder" becomes "Adobe_images.id_local" since id_local auto-increments as rows are added
					   "pick" must become "Adobe_images.pick" to distinguish it from "AgLibraryCollectionImage.pick". This latter appears to always be 0. Flagging a picture in a collection updates the Adobe_images pick field.

					   "colorLabels" ("label text") is too complex and obscure to deal with. Basically, you have to sort based on the text-name of the color, that is, 
					   since "b" < "g" < "p" < "r" < "y", then the color labels are ordered: blue < green < purple < red < yellow. Evidently, this method
					   of sorting made some sense with an earlier version of lightroom where the user could define personally meaningful labels corresponding
					   to the color labels.... 

					   Meanwhile, content "labelColor" is good. This just sorts on the internal numbering of the colors. 

					   HOWEVER....   big point of confusion....  the field-name in Adobe_images which contains the numeric color label is named "colorLabels". 
					   So sort order "colorLabels" means we want to ignore the collection's sort order, but sort order "labelColor" means order by "Adobe_images.colorLabels" 
				   */
					if ( 0 == content.Compare( _T( "fileName" ) ) )
						sort_by_field = _T( "baseName" ) ;
					else if ( 0 == content.Compare( _T( "addedOrder" ) ) )
						sort_by_field = _T( "Adobe_images.id_local" ) ;
					else if ( 0 == content.Compare( _T( "colorLabels" ) ) )
					{
						AfxMessageBox( _T( "The selected collection is sorted by 'label text' which is not currently supported. Sorry." ), MB_OK ) ;
						ASSERT( FALSE ) ;
						return false ;
					}
					else if ( 0 == content.Compare( _T( "labelColor" ) ) )
						sort_by_field = _T( "Adobe_images.colorLabels" ) ;	// this is wierd but right -- see coment above 
					else if ( 0 == content.Compare( _T( "pick" ) ) )
						sort_by_field = _T( "Adobe_images.pick" ) ;
					else
						sort_by_field = content ;
				}
				else if ( 0 == module.Compare( _T( "com.adobe.ag.library.sortDirection" ) ) )
				{
					if ( 0 == content.Compare( _T( "ascending" ) ) )
						ascending = true ;
					else if ( 0 == content.Compare( _T( "descending" ) ) )
						ascending = false ;
					else
					{
						ascending = true ;
						ASSERT( FALSE ) ;
					}
				}
				else if ( 0 == module.Compare( _T( "com.adobe.ag.webGallery" ) ) || 0 == module.Compare( _T( "com.adobe.ag.slideshow" ) ) )
				{
					// just ignore it
				}
				else
					ASSERT( FALSE ) ;
			}
			while ( collection_cursor.StepNext( ) ) ;

			lrdata_path = m_catalog_path.Left( m_catalog_path.ReverseFind( _T( '.' ) ) ) ;
			lrdata_path += _T( " Previews.lrdata" ) ;

			m_sort_by_capture_time = ( 0 == sort_by_field.Compare( _T( "captureTime") ) ) ;
			m_ascending_sort = ascending ;

			SQLCursor image_cursor ;

				// now, query to get the images in the collection 
			if ( lightroom_db.ExecuteQuery(	image_cursor, 
															// what follows is a big string on multiple lines - no comments or commas allowed!
															_T( "SELECT " ) 
																_T( "AgLibraryFile.id_global, " )
																_T( "AgLibraryFile.baseName, " )
																_T( "AgLibraryFile.extension, " )
																_T( "Adobe_images.id_local, " )
																_T( "Adobe_images.aspectRatioCache, " )
																_T( "Adobe_images.captureTime, " )
																_T( "Adobe_images.colorLabels, " )
																_T( "Adobe_images.touchCount, " )
																_T( "Adobe_images.touchTime, " )
																_T( "Adobe_images.fileHeight, " )
																_T( "Adobe_images.fileWidth, " )
																_T( "Adobe_images.pick, " )
																_T( "Adobe_images.fileFormat, " ) 
																_T( "AgLibraryCollectionImage.positionInCollection, " )
																_T( "Adobe_images.fileFormat, " )
																_T( "Adobe_images.rating, " )
																_T( "AgLibraryFolder.pathFromRoot, " )
																_T( "AgLibraryRootFolder.absolutePath, " )
																_T( "Adobe_AdditionalMetadata.xmp " )
															_T( "FROM " )
																_T( "AgLibraryFile, " )
																_T( "AgLibraryFolder, " ) 
																_T( "AgLibraryRootFolder, " )
																_T( "Adobe_images, " )
																_T( "AgLibraryCollectionImage, " )
																_T( "Adobe_AdditionalMetadata " )
															_T( "WHERE " )
																_T( "AgLibraryFile.id_local = Adobe_images.rootFile " )
															_T( "AND " )
																_T( "Adobe_images.id_local = AgLibraryCollectionImage.image " )
															_T( "AND " )
																_T( "AgLibraryFolder.id_local = AgLibraryFile.folder " )
															_T( "AND " )
																_T( "AgLibraryRootFolder.id_local = AgLibraryFolder.rootFolder " )
															_T( "AND " )
																_T( "AgLibraryCollectionImage.collection = %d " )
															_T( "AND " )
																_T( "Adobe_AdditionalMetadata.image = Adobe_images.id_local " )
															_T( "ORDER BY " )
																_T( "%s  %s" ), 
															collection->GetIDLocal( ),
															(const TCHAR *) sort_by_field, 
															ascending ? _T( "ASC" ) : _T( "DESC" ) 
														) )
			{
				int	pic_no = 1 ;

				do
				{
					stringt	name ;
					stringt	extension ;
					stringt path_root ;
					stringt path_relative ;
					stringt	xmp_data ;

					stringt	fq_psd_path ;
					stringt	fq_preview_path ;
					stringt	guid ;
					stringt	title ;
					stringt	description ;
					stringt	camera ;
					stringt	lens ;
					stringt	exposure ;
					stringt	exposure_time ;
					stringt	fstop ;
					stringt	focal_length ;
					stringt	iso ;
					stringt file_format ;
					int		orientation ;
					int		pick ;
					SYSTEMTIME	create_time ;
					int pixel_height ;
					int pixel_width ;

					image_cursor.GetFieldValue( _T( "AgLibraryFile.baseName" ), name ) ;
					image_cursor.GetFieldValue( _T( "AgLibraryFile.id_global" ), guid ) ;
					image_cursor.GetFieldValue( _T( "AgLibraryFile.extension" ), extension ) ;
					image_cursor.GetFieldValue( _T( "AgLibraryFolder.pathFromRoot" ), path_relative ) ;
					image_cursor.GetFieldValue( _T( "AgLibraryRootFolder.absolutePath" ), path_root ) ;
					image_cursor.GetFieldValue( _T( "Adobe_AdditionalMetadata.xmp" ), xmp_data ) ;
					image_cursor.GetFieldValue( _T( "Adobe_images.fileFormat" ), file_format ) ;
					image_cursor.GetFieldValue( _T( "Adobe_images.pick" ), pick ) ;
					image_cursor.GetFieldValue( _T( "Adobe_images.fileHeight" ), pixel_height ) ;
					image_cursor.GetFieldValue( _T( "Adobe_images.fileWidth" ), pixel_width ) ;

					ExtractPreviewJpeg( fq_preview_path, lrdata_path, guid ) ;
					ExtractXMPData( xmp_data, title, description, camera, lens, exposure_time, fstop, focal_length, iso, orientation, create_time ) ;

					fq_psd_path.Format( _T( "%s%s%s.%s" ), (const TCHAR *) path_root, (const TCHAR *) path_relative, (const TCHAR *) name, (const TCHAR *) extension ) ;

					if ( exposure_time.GetLength( ) && fstop.GetLength( ) )
						exposure.Format( _T( "%s f/%s" ), (const TCHAR *) exposure_time, (const TCHAR *) fstop ) ;

					if ( 0 < lens.Find( _T( '-' ) ) && focal_length.GetLength( ) )
					{
						lens += _T( " at " ) ;
						lens += focal_length ;
					}

					AddPhotoToList( new PhotoImage( name, fq_psd_path, fq_preview_path, title, description, camera, lens, exposure, iso, pic_no, orientation, pixel_width, pixel_height, create_time, pick, file_format ) ) ;
					pic_no++ ;
				}
				while ( image_cursor.StepNext( ) ) ;

				LoadCustomMetadata( ) ;
				
				if ( m_sort_by_capture_time )
					ReOrderPhotosByCaptureTime( ) ;
			}
		}
		else
			ASSERT( FALSE ) ;		// a collection with no sorting order?!?!?! 
	}

	return isOK ;
}



/******************************************************************************
CMakePhotoDisk3Doc::ReOrderPhotosByCaptureTime

	This is called by SelectCollection( ) right after its loaded the custom 
	metadata, and if creation time is adjusted for a multi-selection, if the
	collection is sorted by capture time. 

	We just do a bubble sort on the list. This should work reasonably well, 
	since collections are most likely less than 100+ photos in size. 

******************************************************************************/
void CMakePhotoDisk3Doc::ReOrderPhotosByCaptureTime( )
{
	POSITION		cur_pos ;
	POSITION		last_pos ;
	PhotoImage *	photo ;
	PhotoImage *	last_photo = NULL ;
	bool			out_of_order_found ;

	do
	{
		out_of_order_found = false ;

		// first - scan the list and see if ANY two photos are out of order 
		for ( cur_pos = m_collection_pic_list.GetHeadPosition( ), last_pos = NULL ; NULL != cur_pos ; m_collection_pic_list.GetNext( cur_pos ) )
		{
			FILETIME		cur_photo_time ;
			FILETIME		last_photo_time ;
			SYSTEMTIME		cur_sys_time ;

			photo = m_collection_pic_list.GetAt( cur_pos ) ;
			cur_sys_time = photo->GetCreateTime( ) ;
			SystemTimeToFileTime( &cur_sys_time, &cur_photo_time ) ;

			if ( NULL != last_pos ) 
			{
				int cmp ;

				cmp = CompareFileTime( &last_photo_time, &cur_photo_time ) ;

				if ( ( cmp > 0 && m_ascending_sort ) || ( cmp < 0 && !m_ascending_sort ) )
				{
					out_of_order_found = true ;
					m_collection_pic_list.RemoveAt( last_pos ) ;
					m_collection_pic_list.InsertAfter( cur_pos, last_photo ) ;

					// we just moved 'last_photo' to the position immediately after 'cur_photo' - so it becomes the current photo 
					// .... which is critical below, when we set the last_photo to be the 'cur_photo' 
					cur_pos = last_pos ;
					cur_photo_time = last_photo_time ;
					photo = last_photo ;
				}
			}

			// the current photo now becomes the last photo 
			last_pos = cur_pos ;
			last_photo_time = cur_photo_time ;
			last_photo = photo ;
		}
	}
	while ( out_of_order_found ) ;

	RenumberPhotoList( ) ;
}





/******************************************************************************
swap16
	
	Simple utility to swap the byte order of a 16-bit integer 

******************************************************************************/
INT16 swap16( 
	BYTE * data							// I - the wrong-ended 16-bit integer 
) 
{
	union
	{
		BYTE	b[ 2 ];
		UINT16	v ;
	}
	x ;

	x.b[ 1 ] = data[ 0 ] ;
	x.b[ 0 ] = data[ 1 ] ;

	return x.v ;
}



/******************************************************************************
swap32

	Utility to swap the byte order of a 32-bit integer 

******************************************************************************/
INT32 swap32( 
	BYTE * data							// I - the wrong-ended 32-bit integer 
)
{
	union
	{
		BYTE	b[ 4 ] ;
		UINT32	v ;
	} 
	x ;
	int	i ;

	for ( i = 0 ; i < 4 ; i++ )
		x.b[ i ] = data[ 3 - i ] ;
	return x.v ;
}



/******************************************************************************
Next16ByteBoundary

	Utility to ensure a file-offset is on a 16-byte boundary, or round it up
	to the next 16-byte boundary

******************************************************************************/
UINT32 Next16ByteBoundary( 
	UINT32 addr							// I - the offset to check or round-up 
)
{
	if ( addr % 16 )
		addr += 16 - ( addr % 16 ) ;

	return addr ;
}




/******************************************************************************
CMakePhotoDisk3Doc::ExtractPreviewJpeg

	Preview jpeg's are stored in .lrprev files. You locate an image's lrprev file by using 
	the image's GUID and looking under the lrdata directory. 

	The lrdata directory is in the same directory containing the catalog, and named 
	after the catalog. For example, if the catalog is "Hoagy Catalog.lrcat", the 
	lrdata directory is "Hoagy Catalog Preview Data.lrdata". 

	Within this lrdata directory, you find 16 subdirectories named '0', '1', '2', 
	... 'd', 'e', 'f'. The name corresponds to the 1st digit of the GUID. Within 
	each such directory is a whole lotta subdirectories named with 4-digit hex 
	numbers corresponding to the 1st 4 digits of the GUID. 

	So given a catalog named "Hoagy Catalog" and a GUID like 

		35379126-C77F-4221-8CB7-D541D4361F7A, 

	we need to look in a directory named "...\Hoagy Catalog Preview Data.lrdata\3\3537". 

	In that directory we will find 1 or more .lrprev files. The lrprev file is 
	typically named using the full GUID, followed by another GUID. The meaning 
	and derivation of the 2nd GUID is not clear, but doesn't seem to matter. But I 
	have noticed that .lrprev files without the 2nd GUID are always very small 
	(about 13Kb or less) and contain only 1-2 thumbnails. In each instance I've seen, 
	an .lrprev without the 2nd GUID is always paired with another .lrprev WITH a 2nd 
	GUID. In some cases, the 2nd .lrprev file contains more recent preview jpegs and 
	always contains much higher resolution jpegs. 

	For now, all we're going to do is search for file(s) which start with the GUID 
	we're looking for. If, however, the match doesn't have the 2nd GUID, we're just
	going to skip it and insist on another one. 

******************************************************************************/
bool CMakePhotoDisk3Doc::ExtractPreviewJpeg( 
	stringt & fq_preview_path,				// O - gets the fully qualified path of the extracted jpeg 
	const TCHAR * lrdata_path,				// I - the lrdata root directory 
	const TCHAR * guid						// I - the GUID for the AgLibraryFile 
)
{
	// FIXME - I suspect the 2nd GUID is from the Adobe_imageDevelopSettings 

	bool			isOK = false ;
	stringt			lrprev_path ;
	WIN32_FIND_DATA	find_data ;
	HANDLE			find_handle ;

	lrprev_path.Format( _T( "%s\\%1.1s\\%4.4s\\%s-*.lrprev" ), lrdata_path, guid, guid, guid ) ;
	
	// try to locate the smallest jpeg preview which is larger than 70 x 105 - 
	if ( INVALID_HANDLE_VALUE != ( find_handle = FindFirstFile( lrprev_path, &find_data ) ) )
	{
		bool	keep_scanning = false ;

		do
		{
			CFile			lrprev_file ;

			lrprev_path.Format( _T( "%s\\%1.1s\\%4.4s\\%s" ), lrdata_path, guid, guid, find_data.cFileName ) ;

			// only open the file if the filename does NOT end in "-.lrprev" 
			if ( 0 != lrprev_path.Right( 8 ).CompareNoCase( _T( "-.lrprev" ) ) )
			{
				if ( lrprev_file.Open( lrprev_path, CFile::modeRead | CFile::typeBinary ) ) 
				{
					byte buffer[ 1024 ] ;

					if ( sizeof( buffer ) == lrprev_file.Read( buffer, sizeof( buffer ) ) )
					{
						if ( 0 == memcmp( buffer, "AgHg", 4 ) ) 
						{
							UINT16	header_len ;
							UINT32	data_len ;
							int		fs_height ;
							int		fs_width ;
							char *	shortside_tag ;
							char *	search_pos ;
							char *	find ;
							int		embedded_jpeg_no ;
							bool	any_preview_found ;

							header_len = swap16( buffer + 4 ) ;
							data_len = swap32( buffer + 12 ) ;

							/* Starting at buffer + header_len and running to buffer + data_len, we get a text section of the file
							header, which contains something like this: 

							pyramid = {
									colorProfile = "sRGB",
									croppedHeight = 4928,
									croppedWidth = 3280,
									digest = "",
									formatVersion = 3,
									levels = {
											{
													height = 80,
													width = 53,
											},
											{
													height = 160,
													width = 106,
											},
									},
									quality = "thumbnail",
									uuid = "35379126-C77F-4221-8CB7-D541D4361F7A",
							}

							each entry in the "levels" section gives the dimmensions of a preview embedded in
							the file. There are usually more than 2, but may be as few as 1. They always are
							arranged in ascending order of size. 

							We're going to do some really crude parsing here - just read the original fullsize
							dimmensions of the image (croppedHeight and croppedWidth), then scan the smaller 
							dimmension (either height or width) to find the first one that's big enough for us. 

							*/


							/*
							* IMPORTANT NOTE
							*
							*		The text we read out of the lrprev file is ASCII. So we WANT to use the good old 
							*		ASCII string functions, not the unicode/ansii agnostic versions, and strings here
							*		should NOT be enclosed in _T( ), etc. 
							*
							*/

							if ( strstr( (char *) buffer + header_len, CROPPED_HEIGHT_MARKER ) && strstr( (char *) buffer + header_len, CROPPED_WIDTH_MARKER ) )
							{
								fs_height = atoi( strstr( (char *) buffer + header_len, CROPPED_HEIGHT_MARKER ) + strlen( CROPPED_HEIGHT_MARKER ) ) ;
								fs_width = atoi( strstr( (char *) buffer + header_len, CROPPED_WIDTH_MARKER ) + strlen( CROPPED_WIDTH_MARKER ) ) ;
							}
							else
								fs_height = fs_width = 0 ;

							if ( fs_height < fs_width ) 
								shortside_tag = "height = " ;
							else
								shortside_tag = "width = " ;

							search_pos = (char *) buffer + header_len ;
							embedded_jpeg_no = 0 ;

							any_preview_found = false ;

							while ( find = strstr( search_pos, shortside_tag ) )
							{
								any_preview_found = true ;
								embedded_jpeg_no++ ;
								find += strlen( shortside_tag ) ;

								if ( atoi( find ) >= MIN_PREVIEW_DIMENSION ) 
								{
									break ;
								}
								search_pos = find ;
							}

							if ( any_preview_found ) 
							{
								ExtractNthJpeg( fq_preview_path, guid, embedded_jpeg_no, Next16ByteBoundary( header_len + data_len ), lrprev_file ) ;
								keep_scanning = false ;
								isOK = true ;
							}
							else
								keep_scanning = true ;		// try FindNextFile - 
						}
					}

					lrprev_file.Close( ) ;
				}
			}
			else
			{
				keep_scanning = true ;
			}
		} 
		while ( keep_scanning && FindNextFile( find_handle, &find_data ) ) ;

		FindClose( find_handle ) ;
	}

	return isOK ;
}









/******************************************************************************
CMakePhotoDisk3Doc::ExtractNthJpeg

	Called by ExtractPreviewJpeg to get the n'th jpeg in the lrdata file 

	lrprev_file is an open .lrprev file. We want to extract the n'th embedded jpeg. The 1st one
	starts at start_addr. We determine the path to use and fill in fq_preview_path with the fully
	qualified path of the extracted preview jpeg. 

******************************************************************************/
bool CMakePhotoDisk3Doc::ExtractNthJpeg( 
	stringt & fq_preview_path,				// O - gets the path of the extracted jpeg 
	const TCHAR * guid,						// I - GUID of the root file 
	int n,									// I - the n in n'th jpeg 
	UINT32 addr,							// I - the address of the image header 
	CFile & lrprev_file						// I/O - the CFile for the open lrprev file 
)
{
	// 
	struct JPEG_HEADER
	{
		BYTE	sig[ 4 ] ;			// had better be "AgHg"
		UINT16	header_len ;		// almost always 20 
		UINT8	data_type ;			// 
		BYTE	unknown[ 5 ] ;		// 5 bytes with unknown use 
		UINT32	data_len ;			// 32-bit length of the data following this record
		BYTE	unknown2[ 8 ] ;		// 8 bytes of more unknown use
		char	data_label[ 8 ] ;	// a label for the data 
	}
	header ;
	bool	isOK = false ;
	int		jpeg_no = 0 ;

	do
	{
		lrprev_file.Seek( addr, CFile::begin ) ;
	
		if ( sizeof( header ) == lrprev_file.Read( &header, sizeof( header ) ) )
		{
			header.header_len = swap16( (BYTE*) &header.header_len ) ;
			header.data_len = swap32( (BYTE *) &header.data_len ) ;

			if ( ++jpeg_no == n ) 
			{
				// we just read the header, so the file pointer is right at the start of the data containing
				// the disk image of the jpeg file we need to extract... and it's header.date_len bytes long. 
				byte * buffer ;

				if ( buffer = (byte *) malloc( header.data_len ) )
				{
					GetTempPath( MAX_PATH, fq_preview_path.GetBufferSetLength( MAX_PATH ) ) ;

					fq_preview_path.ReleaseBuffer( ) ;
					fq_preview_path += guid ;
					fq_preview_path += _T( ".jpg" ) ;

					if ( header.data_len == lrprev_file.Read( buffer, header.data_len ) ) 
					{
						CFile	jpeg_file ;

						if ( jpeg_file.Open( fq_preview_path, CFile::modeCreate | CFile::modeWrite ) ) 
						{
							jpeg_file.Write( buffer, header.data_len ) ;
							jpeg_file.Close( ) ;

							isOK = true ;
						}
					}

					free( buffer ) ;
				}
			}
			else 
			{
				addr = Next16ByteBoundary( addr + header.header_len + header.data_len ) ;
			} 
		}
	}
	while ( jpeg_no < n ) ;

	return isOK ;
}







/******************************************************************************
CMakePhotoDisk3Doc::ExtractDelimitedString

	Used in parsing xmp data to extract desired data fields

******************************************************************************/
bool CMakePhotoDisk3Doc::ExtractDelimitedString( 
	stringt & result,							// O - gets extracted field 
	const TCHAR * left_delim,					// I - left delimiter 
	const TCHAR * right_delim,					// I - right delimiter 
	const TCHAR * source						// I - source data 
) 
{
	const TCHAR * lfind ;
	const TCHAR * rfind ;

	if ( lfind = _tcsstr( source, left_delim ) )
	{
		if ( rfind = _tcsstr( lfind + _tcslen( left_delim ), right_delim ) )
		{
			result = lfind + _tcslen( left_delim ) ;
			result = result.Left( rfind - ( lfind + _tcslen( left_delim ) ) ) ;
			return true ;
		}
	}

	return false ;
}





/******************************************************************************
CMakePhotoDisk3Doc::EvaluateStringFraction

	Convert an EXIF fraction to a normalized fraction, or a real number, if the
	fraction is greater than 1. 
	
	A fraction with a numerator equal to 1 is left alone. This is typically an 
	exposure time like 1/125s, which we want to retain as a fraction, while an 
	exposure time which is multiple seconds should be rendered as something 
	like 1.6s or 30s, etc. 

******************************************************************************/
void CMakePhotoDisk3Doc::EvaluateStringFraction( 
	stringt & str, 
	int max_decimal_places, 
	int min_decimal_places 
)
{
	int numerator ;
	int denominator ;
	int	decimal_point ;

	if ( 2 == _stscanf_s( str, _T( "%d/%d" ), &numerator, &denominator ) )
	{
		if ( 0 != denominator && numerator != 1 )
		{
			str.Format( _T( "%.*f" ), max_decimal_places, ( (double) numerator ) / ( (double) denominator ) ) ;

			// now... trim trailing 0's which take us over the minimum decimal places
			if ( -1 != ( decimal_point = str.Find( _T( '.' ) ) ) )
			{
				while ( max_decimal_places > min_decimal_places ) 
				{
					if ( _T( '0' ) == str[ decimal_point + max_decimal_places ] )
						str = str.Left( decimal_point + max_decimal_places ) ;
					else
						break ;

					max_decimal_places-- ;
				}

				if ( 0 == max_decimal_places )
					str = str.Left( decimal_point ) ;
			}
		}
	}
}







/******************************************************************************
CMakePhotoDisk3Doc::ExtractXMPData

	Search the big old glob of xmp data for interesting fields. 

	The only significant complication here is that the same data may be stored 
	in a variety of different formats. The different formats appear to be a 
	result of using different versions of lightroom over the years: with some
	version-changes Adobe decided to change the format of the XMP data. 

******************************************************************************/
bool CMakePhotoDisk3Doc::ExtractXMPData( 
	stringt & xmp_data,					// I - the XMP data blob 
	stringt & title,					// O - the pic title
	stringt & description,				// O - the pic description
	stringt & camera,					// O - the camera used
	stringt & lens,						// O - the lens used 
	stringt & exposure_time,			// O - the exposure time 
	stringt & fstop,					// O - the f-stop 
	stringt & focal_length,				// O - the focal length 
	stringt & iso,						// O - the ISO
	int & orientation,					// O - vertical or horizontal 
	SYSTEMTIME & create_time			// O - capture time, as a SYSTEMTIME 
)
{
	bool isOK = false ;
	stringt	create_time_str ;

	if ( ExtractDelimitedString( title, _T( "<dc:title>" ), _T( "</dc:title>" ), xmp_data ) )
		if ( ExtractDelimitedString( title, _T( "<rdf:li" ), _T( "/rdf:li>" ), title ) )
			ExtractDelimitedString( title, _T( ">" ), _T( "<" ), title ) ;
		else
			title.Empty( ) ;

	if ( ExtractDelimitedString( description, _T( "<dc:description>" ), _T( "</dc:description>" ), xmp_data ) )
		if ( ExtractDelimitedString( description, _T( "<rdf:li" ), _T( "/rdf:li>" ), description ) )
			ExtractDelimitedString( description, _T( ">" ), _T( "<" ), description ) ;
		else
			description.Empty( ) ;

	/* 
	The XMP data in the database is formatted in one of several different ways... 
	depending, I presume, on the version of Camera Raw in use when the corresponding
	image was imported, or perhaps on the manner in which the image was imported, or
	perhaps the phase of the moon when the image was imported. 
	*/
	stringt	orientation_str ;

	if ( !ExtractDelimitedString( orientation_str, _T( "<tiff:Orientation>" ), _T( "</tiff:Orientation>" ), xmp_data ) )
		if ( !ExtractDelimitedString( orientation_str, _T( "tiff:Orientation=\"" ), _T( "\"" ), xmp_data ) )
			orientation_str = _T( "1" ) ;

	switch ( _ttoi( orientation_str ) )
	{
		default : 
		case 1 :
			orientation = 0 ;	// image not rotated at all - 
			break ;
		case 8 :
			orientation = 90 ;	// image rotated 90 to right - normal vertical composition
			break ;
		case 6 :
			orientation = -90 ;	// image rotated 90 to left - reverse vertical composition 
			break ;
	}

	if ( !ExtractDelimitedString( camera, _T( "<tiff:Model>" ), _T( "</tiff:Model>" ), xmp_data ) )
		ExtractDelimitedString( camera, _T( "tiff:Model=\"" ), _T( "\"" ), xmp_data ) ;
	camera.Replace( _T( "NIKON" ), _T( "Nikon" ) ) ;

   	if ( !ExtractDelimitedString( lens, _T( "<aux:Lens>" ), _T( "</aux:Lens>" ), xmp_data ) )
		ExtractDelimitedString( lens, _T( "aux:Lens=\"" ), _T( "\"" ), xmp_data ) ;
	lens.Replace( _T( ".0" ), _T( "" ) ) ;
	lens.Replace( _T( " mm" ), _T( "mm" ) ) ;

	if ( !ExtractDelimitedString( exposure_time, _T( "<exif:ExposureTime>" ), _T( "</exif:ExposureTime>" ), xmp_data ) )
		ExtractDelimitedString( exposure_time, _T( "exif:ExposureTime=\"" ), _T( "\"" ), xmp_data ) ;
	EvaluateStringFraction( exposure_time, 1, 0 ) ;

	if ( !ExtractDelimitedString( fstop, _T( "<exif:FNumber>" ), _T( "</exif:FNumber>" ), xmp_data ) )
		ExtractDelimitedString( fstop, _T( "exif:FNumber=\"" ), _T( "\"" ), xmp_data ) ;
	EvaluateStringFraction( fstop, 1, 0 ) ;

	if ( !ExtractDelimitedString( focal_length, _T( "<exif:FocalLength>" ), _T( "</exif:FocalLength>" ), xmp_data ) )
		ExtractDelimitedString( focal_length, _T( "exif:FocalLength=\"" ), _T( "\"" ), xmp_data  ) ;
	EvaluateStringFraction( focal_length, 0, 0 ) ;
	if ( focal_length.GetLength( ) )
		focal_length += _T( "mm" ) ;

	if ( ExtractDelimitedString( iso, _T( "<exif:ISOSpeedRatings>" ), _T( "</exif:ISOSpeedRatings>" ), xmp_data ) )
		ExtractDelimitedString( iso, _T( "<rdf:li>" ), _T( "</rdf:li>" ), iso ) ;

	if ( !ExtractDelimitedString( create_time_str, _T( "<exif:DateTimeOriginal>" ), _T( "</exif:DateTimeOriginal>" ), xmp_data ) ) 
		if ( !ExtractDelimitedString( create_time_str, _T( "exif:DateTimeOriginal=\"" ), _T( "\"" ), xmp_data ) )
			if ( !ExtractDelimitedString( create_time_str, _T( "<xap:CreateDate>" ), _T( "</xap:CreateDate>" ), xmp_data ) )
				if ( !ExtractDelimitedString( create_time_str, _T( "<xmp:CreateDate>" ), _T( "</xmp:CreateDate>" ), xmp_data ) )
					ExtractDelimitedString( create_time_str, _T( "xmp:CreateDate=\"" ), _T( "\"" ), xmp_data  ) ;

	create_time.wMilliseconds = 0 ;
	_stscanf_s( create_time_str, _T( "%d-%d-%dT%d:%d:%d" ), &create_time.wYear, &create_time.wMonth, &create_time.wDay, &create_time.wHour, &create_time.wMinute, &create_time.wSecond ) ;

	return isOK ;
}












/******************************************************************************
CMakePhotoDisk3Doc::Serialize

	not used - provided b/c Document class must have it 

******************************************************************************/
void CMakePhotoDisk3Doc::Serialize(
	CArchive & ar						// I - the MFC archive object 
)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}





// CMakePhotoDisk3Doc diagnostics

#ifdef _DEBUG
void CMakePhotoDisk3Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMakePhotoDisk3Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMakePhotoDisk3Doc commands




#define COMMON_TEMPLATE_SOURCE			_T( "c:\\users\\csentz\\documents\\photography\\photo disk templates\\common" )
#define FAMILY_AND_FRIENDS_SOURCE		_T( "c:\\users\\csentz\\documents\\photography\\photo disk templates\\family and friends" )
#define ASSIGNMENT_SOURCE				_T( "c:\\users\\csentz\\documents\\photography\\photo disk templates\\assignment" )
#define CLIENT_SOURCE					_T( "c:\\users\\csentz\\documents\\photography\\photo disk templates\\client" )
#define SAMPLER_SOURCE					_T( "c:\\users\\csentz\\documents\\photography\\photo disk templates\\sampler" )



/******************************************************************************
CMakePhotoDisk3Doc::CopyDirectoryTree

	Called by CreateProject (or recursively by itself) to copy the contents of one 
	directory (within a template) to another (within the disk project). 

	On entry, we assume the directory specified by dest_dir exists and is ready to receive the 
	contents of source_dir. Each directory encountered in source_dir is recursed into. 

******************************************************************************/
bool CMakePhotoDisk3Doc::CopyDirectoryTree( 
	const TCHAR * source_dir,					// I - source directory tree 
	const TCHAR * dest_dir						// I - destination directory tree 
) 
{
	WIN32_FIND_DATA	find_data ;
	HANDLE			find_handle ;
	stringt			source_name ;
	stringt			dest_name ;
	bool			isOK = false ;

	source_name.Format( _T( "%s\\*" ), source_dir ) ;
	if ( INVALID_HANDLE_VALUE != ( find_handle = FindFirstFile( source_name, &find_data ) ) ) 
	{
		isOK = true ;

		do
		{
			source_name.Format( _T( "%s\\%s" ), source_dir, find_data.cFileName ) ;
			dest_name.Format( _T( "%s\\%s" ), dest_dir, find_data.cFileName ) ;

			if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
			{
				if ( 0 != _tcscmp( find_data.cFileName, _T( "." ) ) && 0 != _tcscmp( find_data.cFileName, _T( ".." ) ) )
				{
					if ( isOK = ( 0 != CreateDirectory( dest_name, NULL ) ) )
					{
						isOK = CopyDirectoryTree( source_name, dest_name ) ;
					}
				}
			}
			else
			{
				// copy the source to the dest, and just overwrite the destination if it already exists
				isOK = ( 0 != CopyFile( source_name, dest_name, FALSE ) ) ;
			}
		}
		while ( isOK && FindNextFile( find_handle, &find_data ) ) ;

		FindClose( find_handle ) ;
	}

	return isOK ;
}









/******************************************************************************
CMakePhotoDisk3Doc::CreateProject

	Called by the view after the user has confirmed that he wants to create a 
	project	with the options selected. 

	1. Create the "directory" folder and the standard sub-tree under it. 
	2. Copy the appropriate starter files from the common template directory 
	3. Then copy the files (if any) from the disk-type-specific source directory 

******************************************************************************/
bool CMakePhotoDisk3Doc::CreateProject( )
{
	bool isOK = false ;
	stringt	directory ;

	directory.Format( _T( "%s\\%s" ), (const TCHAR *) m_project_location, (const TCHAR *) m_project_directory ) ;

	if ( isOK = ( 0 != CreateDirectory( directory, NULL ) ) )
	{
		if ( isOK = CopyDirectoryTree( COMMON_TEMPLATE_SOURCE, directory ) )
		{
			switch ( m_disk_type ) 
			{
				case FamilyAndFriendsDisk : 
					isOK = CopyDirectoryTree( FAMILY_AND_FRIENDS_SOURCE, directory ) ;
					break ;

				case AssignmentProduct :
					isOK = CopyDirectoryTree( ASSIGNMENT_SOURCE, directory ) ;
					break ;

				case ClientDisk : 
					isOK = CopyDirectoryTree( CLIENT_SOURCE, directory ) ;
					break ;

				case SamplerDisk :
					isOK = CopyDirectoryTree( SAMPLER_SOURCE, directory ) ;
					break ;

				default :
					ASSERT( FALSE ) ;
					break ;
			}
		}

		m_doc_state = CatalogCollectionProjectKnown ;

		// now - save the document in the "hidden" subdirectory 
		if ( isOK ) 
		{
			isOK = SaveDocument( ) ;
		}
	}
	else
	{
		stringt	msg ;

		msg.Format( _T( "Unable to create project directory:\n\n\"%s\"\n" ), (const TCHAR *) directory ) ;
		AfxMessageBox( msg, MB_ICONSTOP | MB_OK ) ;
	} 

	return isOK ;
}











/******************************************************************************
CMakePhotoDisk3Doc::LoadDocument
	
	load a project from disk 

******************************************************************************/
bool CMakePhotoDisk3Doc::LoadDocument( 
	const TCHAR * fq_path				// I - fully qualified path to project file 
)
{
	CStdioFile	doc_file ;
	bool		isOK = false ;

	if ( doc_file.Open( fq_path, CFile::typeText | CFile::modeRead ) ) 
	{
		stringt	sig ;
		stringt	doc_buffer ;

		PreloadDocBuffer( doc_file, doc_buffer ) ;

		LoadParameter( doc_buffer, _T( "diskproject" ), sig ) ;
		if ( 0 == sig.Compare( _T( "Make Photo Disk 3.0" ) ) ) 
		{
			int		collection_ID ;
			stringt	catalog_path ;

			LoadParameter( doc_buffer, _T( "catalogpath" ), catalog_path ) ;
			LoadParameter( doc_buffer, _T( "collectionid" ), collection_ID ) ;

			if ( SetCatalog( catalog_path ) )	// this will re-initialize the document and set m_catalog_path equal to the catalog path passed as an argument
			{
				if ( m_sel_collection = FindCollectionByID( collection_ID ) ) 
				{
					SelectCollection( m_sel_collection ) ;

					if ( AnySourceMissing( ) )
					{
						stringt	msg ;

						msg.Format( _T( "The selected collection \"%s\" includes missing source files" ), m_sel_collection->GetName( ) ) ;
						AfxMessageBox( msg, MB_OK | MB_ICONSTOP ) ;
					}
					else if ( AnySourceUnsupported( ) )
					{
						stringt	msg ;

						msg.Format( _T( "The selected collection \"%s\" includes source files in an unsupported format" ), m_sel_collection->GetName( ) ) ;
						AfxMessageBox( msg, MB_OK | MB_ICONSTOP ) ;
					}
					else
					{
						stringt	pic_name ;
						int		ondisk_no ;
						int		pic_count ;
						int		i ;
						int		tmp_int ;

						m_doc_state = CatalogCollectionProjectKnown ;
						isOK = true ;

						LoadParameter( doc_buffer, _T( "location" ), m_project_location ) ;
						LoadParameter( doc_buffer, _T( "directory" ), m_project_directory ) ;
						m_project_path.Format( _T( "%s\\%s" ), (const TCHAR *) m_project_location, (const TCHAR *) m_project_directory ) ;

						LoadParameter( doc_buffer, _T( "disktitle" ), m_disk_title ) ;
						LoadParameter( doc_buffer, _T( "keyimage" ), m_key_image_source ) ;
						LoadParameter( doc_buffer, _T( "diskicon" ), m_disk_icon_source ) ;

						LoadDiskType( doc_buffer ) ;
						
						LoadParameter( doc_buffer, _T( "majorrev" ), tmp_int ) ;
						m_major_revision = (UINT32) tmp_int ;
						LoadParameter( doc_buffer, _T( "minorev" ), tmp_int ) ;
						m_minor_revision = (UINT32) tmp_int ;
						LoadParameter( doc_buffer, _T( "piccount" ), pic_count ) ;

						for ( i = 0 ; i < pic_count; i++ )
						{
							PhotoImage * photo ;

							LoadDuple( doc_buffer, _T( "image" ), i, pic_name, ondisk_no ) ;

							if ( photo = FindPhotoByFilename( pic_name ) )
							{
								photo->SetOnDiskNumber( ondisk_no ) ;
							}
							else
							{
								// have a photo on disk that is NOT in the collection anymore - zap all instances of it. 
								stringt		ondisk_name ;

								ondisk_name.Format( _T( "%d.jpg" ), ondisk_no ) ;
								DeleteOrphanJpeg( ondisk_name ) ;

							}
						}
					}
				}
				else
					AfxMessageBox( _T( "Collection not found." ), MB_OK ) ;
			}
			// else - catalog not found - reported elsewhere
		}
		else
			AfxMessageBox( _T( "Invalid file format." ), MB_ICONSTOP | MB_OK ) ;

		doc_file.Close( ) ;
	}
	else 
	{
		stringt		msg ;

		msg.Format( _T( "Unable to open file \"%s\".\n" ), fq_path ) ;

		AfxMessageBox( msg, MB_OK | MB_ICONSTOP ) ;
	}

	return isOK ;
}






/******************************************************************************
CMakePhotoDisk3Doc::PreloadDocBuffer

	Load the document file's contents into a big string 

******************************************************************************/
bool CMakePhotoDisk3Doc::PreloadDocBuffer( 
	CStdioFile & doc_file,					// I - opened document file 
	stringt & doc_buffer					// O - gets contents of file 
)
{
	bool isOK = false ;
	TCHAR * buffer ;
	TCHAR * cur_pos ;
	int		len ;

	len = (int) doc_file.GetLength( ) ;

	if ( buffer = doc_buffer.GetBufferSetLength( len ) ) 
	{
		cur_pos = buffer ;

		while ( doc_file.ReadString( cur_pos, len - ( cur_pos - buffer ) ) ) 
			cur_pos += _tcslen( cur_pos ) ;

		doc_buffer.ReleaseBuffer( ) ;

		isOK = ( doc_buffer.GetLength( ) > 0 ) ;
	}

	return isOK ;
}





/******************************************************************************
CMakePhotoDisk3Doc::LoadDiskType

	Used to just save and read the numeric equivalent of the disk type. But we now
	save/load disk type PLUS the individual selections (fullsize, facebook, etc)

	The format is a tuple consisting of the disk type followed by the output types.
	A typical FamilyAndFriends disk (value 1) will be: 

		<disktype>1,bigpresent,compressed,facebook,fulsize</disktype>

	But if fullsize and facebeeok have been de-selected, it would be:

		<disktype>1,bigpresent,compressed</disktype>
	
******************************************************************************/
bool CMakePhotoDisk3Doc::LoadDiskType( 
	CString & doc_buffer					// I - document buffer 
) 
{
	stringt	disk_type_string ;
	stringt	token ;
	int		token_pos ;
	bool	isOK = false ;
	bool	first_extra = true ;

	LoadParameter( doc_buffer, _T( "disktype" ), disk_type_string ) ;

	token_pos = 0 ;
	token = disk_type_string.Tokenize( _T( "," ), token_pos ) ;

	if ( !token.IsEmpty( ) )
	{
		m_disk_type = (DiskTypes) ( _ttoi( token ) ) ;

		switch ( m_disk_type ) 
		{
			case FamilyAndFriendsDisk :
				m_do_big_presentation = true ;
				m_do_compressed = true ;
				m_do_facebook = true ;
				m_do_full_size = true ;
				break ;

			case AssignmentProduct :
				m_do_big_presentation = true ;
				m_do_compressed = true ;
				m_do_full_size = true ;
				break ;

			case ClientDisk :
				m_do_compressed = true ;
				m_do_facebook = true ;
				break ;

			case SamplerDisk :
				break ;

			default :
				ASSERT( FALSE ) ;
				break ;
		}

		isOK = true ;
	}

	do
	{
		token = disk_type_string.Tokenize( _T( "," ), token_pos ) ;

		if ( !token.IsEmpty( ) ) 
		{
			if ( first_extra ) 
			{
				// OK - looks like this is a new document style, reset all defaults and only set true if found on this line - 
				m_do_big_presentation = false ;
				m_do_compressed = false ;
				m_do_facebook = false ;
				m_do_full_size = false ;

				m_watermark_presentation = false ;
				m_watermark_compressed = false ;
				m_watermark_facebook = false ;
				m_watermark_full_size = false ;

				first_extra = false ;
			}

			if ( 0 == token.CompareNoCase( _T( "bigpresent" ) ) )
				m_do_big_presentation = true ;
			if ( 0 == token.CompareNoCase( _T( "compressed" ) ) )
				m_do_compressed = true ;
			if ( 0 == token.CompareNoCase( _T( "facebook" ) ) )
				m_do_facebook = true ;
			if ( 0 == token.CompareNoCase( _T( "fullsize" ) ) ) 
				m_do_full_size = true ;
		}
	}
	while ( !token.IsEmpty( ) ) ;

	return isOK ;
}






/******************************************************************************
AppendCommaDelimitedToken

	Trivial helper function used by SaveDiskType to optionally append a comma

******************************************************************************/
void AppendCommaDelimitedToken( 
	stringt & token_str,				// I/O - string of comma-delimited tokens 
	const TCHAR * token					// I - new token to add to token_str 
) 
{
	if ( 0 != token_str.Right( 1 ).Compare( _T( "," ) ) )
		token_str += _T( "," ) ;
	token_str += token ;
}







/******************************************************************************
CMakePhotoDisk3Doc::SaveDiskType

	Used to just save the numeric equivalent of the disk type. But we now
	save/load disk type PLUS the individual selections (fullsize, facebook, etc)

	The format is a tuple consisting of the disk type followed by the output types.
	A typical FamilyAndFriends disk (value 1) will be: 

		<disktype>1,bigpresent,compressed,facebook,fulsize</disktype>

	But if fullsize and facebeeok have been de-selected, it would be:

		<disktype>1,bigpresent,compressed</disktype>

******************************************************************************/
void CMakePhotoDisk3Doc::SaveDiskType( 
	CStdioFile & doc_file					// O - the file to write to 
)
{
	stringt	disk_type_str ;		// becomes a comma-delimited string like: "1,bigpresent,compressed,facebook,fullsize"

	disk_type_str.Format( _T( "%d" ), (int) m_disk_type ) ;

	// FIXME - these string literals should at least be #defined constants.... 
	if ( m_do_big_presentation )
		AppendCommaDelimitedToken( disk_type_str, _T( "bigpresent" ) ) ;

	if ( m_do_compressed )
		AppendCommaDelimitedToken( disk_type_str, _T( "compressed" ) ) ;

	if ( m_do_facebook )
		AppendCommaDelimitedToken( disk_type_str, _T( "facebook" ) ) ;

	if ( m_do_full_size )
		AppendCommaDelimitedToken( disk_type_str, _T( "fullsize" ) ) ;

	if ( m_watermark_presentation )
		AppendCommaDelimitedToken( disk_type_str, _T( "watermarkpres" ) ) ;

	if ( m_watermark_compressed )
		AppendCommaDelimitedToken( disk_type_str, _T( "watermarkcomp" ) ) ;

	if ( m_watermark_facebook )
		AppendCommaDelimitedToken( disk_type_str, _T( "watermarkfbook" ) ) ;

	if ( m_watermark_full_size ) 
		AppendCommaDelimitedToken( disk_type_str, _T( "watermarkfull" ) ) ;

	SaveParameter( doc_file, _T( "disktype" ), disk_type_str ) ;
}






/******************************************************************************
CMakePhotoDisk3Doc::LoadParameter

	Returns a string value delimited by xml-style delimiters - eg:

		<disktitle>My disk title</disktitle>

	returns true if delimited value found, false if not 

******************************************************************************/
bool CMakePhotoDisk3Doc::LoadParameter( 
	stringt & buffer,					// I - document buffer 
	const TCHAR * tag,					// I - tag to look for 
	stringt & value						// O - gets the value delimited by tag 
) 
{
	stringt	ltarget ;
	stringt	rtarget ;
	bool	isOK = false ;
	int		lpos ;

	ltarget.Format( _T( "<%s>" ), tag ) ;
	rtarget.Format( _T( "</%s>" ), tag ) ;

	if ( -1 != ( lpos = buffer.Find( ltarget ) ) )
	{
		int	rpos ;

		if ( -1 != ( rpos = buffer.Find( rtarget, lpos ) ) )
		{
			lpos += ltarget.GetLength( ) ;
			value = buffer.Mid( lpos, rpos - lpos ) ;
			isOK = true ;
		}
	}

	return isOK ;
}




/******************************************************************************
bool CMakePhotoDisk3Doc::LoadParameter

	Searches the buffer for an integer value delimited by xml-style delimiters, eg:

		<diskrevision>3</diskrevision>

	returns true if delimited value found, false if not 

******************************************************************************/
bool CMakePhotoDisk3Doc::LoadParameter( 
	stringt & buffer,					// I - document contents 
	const TCHAR * tag,					// I - the tag 
	int & value							// O - gets the integer 
) 
{
	stringt	value_str ;
	bool	isOK = false ;

	if ( isOK = LoadParameter( buffer, tag, value_str ) )
	{
		value = (UINT32) _ttoi64( value_str ) ;
	}

	return isOK ;
}



/******************************************************************************
CMakePhotoDisk3Doc::LoadDuple

	Read a the n'th delimited duple consisting of a paired string and integer 
	value, eg

		<image>Hoagie (4 months) 033,-1</image>

	returns true if the duple is found, false if not 

******************************************************************************/
bool CMakePhotoDisk3Doc::LoadDuple( 
	stringt & buffer,						// I - the document contents 
	const TCHAR * tag,						// I - the tag to search for 
	int duple_no,							// I - the number of the duple to get
	stringt & value1,						// O - gets the string value 
	int & value2							// O - gets the integer value 
) 
{
	int	match_no ;
	stringt	ltarget ;
	stringt	rtarget ;
	int		lpos ;
	bool	isOK = false ;

	ltarget.Format( _T( "<%s>" ), tag ) ;
	rtarget.Format( _T( "</%s>" ), tag ) ;

	lpos = 0 ;
	match_no = 0 ;
	while ( -1 != ( lpos = buffer.Find( ltarget, lpos ) ) )
	{
		if ( match_no == duple_no ) 
		{
			int	rpos ;

			lpos += ltarget.GetLength( ) ;
			if ( -1 != ( rpos = buffer.Find( rtarget, lpos ) ) )
			{
				stringt	duple_str ;
				int		comma ;

				duple_str = buffer.Mid( lpos, rpos - lpos ) ;
				if ( -1 != ( comma = duple_str.Find( _T( ',' ) ) ) )
				{
					isOK = true ;
					value1 = duple_str.Left( comma ) ;
					value2 = _ttoi( duple_str.Mid( comma + 1 ) ) ;
					break ;
				}
			}
		}
		else
			lpos++ ;

		match_no++ ;
	}

	return isOK ;
}





/******************************************************************************
CMakePhotoDisk3Doc::SaveDocument

	Save all the parameters of a disk project 

	The project-specific parameters are stored in a .dp file in the "hidden" 
	subdirectory under the disk image on the hard drive 

	Custom metadata is stored in a common database where it can be used by
	other disk projects 

******************************************************************************/
bool CMakePhotoDisk3Doc::SaveDocument( ) 
{
	bool		isOK = false ;
	stringt		doc_filename ;
	CStdioFile	doc_file ;

	// create the directory, if necessary - 
	doc_filename.Format( _T( "%s\\%s\\hidden" ), (const TCHAR *) m_project_location, (const TCHAR *) m_project_directory ) ;

	if ( CreateDirectory( doc_filename, NULL ) || ERROR_ALREADY_EXISTS == ::GetLastError( ) ) 
	{
		doc_filename.Format( _T( "%s\\%s\\hidden\\docdef.dp" ), (const TCHAR *) m_project_location, (const TCHAR *) m_project_directory ) ;

		if ( doc_file.Open( doc_filename, CFile::typeText | CFile::modeCreate | CFile::modeWrite ) )
		{
			isOK = true ;

			SaveParameter( doc_file, _T( "diskproject" ), _T( "Make Photo Disk 3.0" ) ) ;
			SaveParameter( doc_file, _T( "catalogpath" ), m_catalog_path ) ;
			SaveParameter( doc_file, _T( "collectionid" ), m_sel_collection->GetIDLocal( ) ) ;
			SaveParameter( doc_file, _T( "location" ), m_project_location ) ;
			SaveParameter( doc_file, _T( "directory" ), m_project_directory ) ;
			SaveParameter( doc_file, _T( "disktitle" ), m_disk_title ) ;
			SaveParameter( doc_file, _T( "keyimage" ), m_key_image_source ) ;
			SaveParameter( doc_file, _T( "diskicon" ), m_disk_icon_source ) ;
			SaveDiskType( doc_file ) ;
			SaveParameter( doc_file, _T( "majorrev" ), m_major_revision ) ;
			SaveParameter( doc_file, _T( "minorev" ), m_minor_revision ) ;
			SaveParameter( doc_file, _T( "piccount" ), m_collection_pic_list.GetCount( ) ) ;

			PhotoImage * photo ;
			POSITION pos ;

			for ( photo = GetFirstPhoto( pos ) ; photo != NULL ; photo = GetNextPhoto( pos ) ) 
			{
				SaveDuple( doc_file, _T( "image" ), photo->GetName( ), photo->GetNumberOnDisk( ) ) ;
			}

			// we should save image overrides and extended comments here... 

			doc_file.Close( ) ;

			SetModifiedFlag( FALSE ) ;

			AfxGetApp( )->AddToRecentFileList( doc_filename ) ;
		}
		else
		{
			stringt		err_message ;

			// FIXME - should really be using FormatMessage, perhaps in a central area, to report OS errors 
			err_message.Format( _T( "Unable to create file\n\n\"%s\"\n\nOS error code %d (0x%X)." ), (const TCHAR *) doc_filename, GetLastError( ) ) ;
			AfxMessageBox( err_message, MB_ICONSTOP | MB_OK ) ;
		}
	}
	else
		AfxMessageBox( _T( "Unable to create directory for project file" ), MB_ICONSTOP | MB_OK ) ;

	CheckSaveCustomMetadata( ) ;

	return isOK ;
}




/******************************************************************************
CMakePhotoDisk3Doc::SaveParameter

	Save string parameter 

******************************************************************************/
void CMakePhotoDisk3Doc::SaveParameter( 
	CStdioFile & doc_file,					// I - .dp file 
	const TCHAR * param_id,					// I - tag for string param 
	const TCHAR * str_param					// I - value of string parameter 
) 
{
	stringt	output ;

	output.Format( _T( "<%s>%s</%s>\n" ), param_id, str_param, param_id ) ;
	doc_file.WriteString( output ) ;
}


/******************************************************************************
CMakePhotoDisk3Doc::SaveParameter

	Save int parameter 

******************************************************************************/
void CMakePhotoDisk3Doc::SaveParameter( 
	CStdioFile & doc_file,					// I - .dp file 
	const TCHAR * param_id, 				// I - tag for string param 
	int value 								// I - value of int parameter 
) 
{
	stringt	value_str ;

	value_str.Format( _T( "%d" ), value ) ;
	SaveParameter( doc_file, param_id, value_str ) ;
}


/******************************************************************************
CMakePhotoDisk3Doc::SaveParameter

	Save int32 param 

******************************************************************************/
void CMakePhotoDisk3Doc::SaveParameter( 
	CStdioFile & doc_file,					// I - .dp file 
	const TCHAR * param_id, 				// I - tag for string param 
	UINT32 value 							// I - value of int parameter 
) 
{
	stringt	value_str ;

	value_str.Format( _T( "%d" ), value ) ;
	SaveParameter( doc_file, param_id, value_str ) ;
}






/******************************************************************************
CMakePhotoDisk3Doc::SaveDuple

	Save duple param 

******************************************************************************/
void CMakePhotoDisk3Doc::SaveDuple( 
	CStdioFile & doc_file,					// I - .dp file 
	const TCHAR * param_id, 				// I - tag for string param 
	const TCHAR * value1, 					// I - value of string parameter 
	const TCHAR * value2					// I - value of 2nd string parameter 
) 
{
	stringt	duple ;

	duple.Format( _T( "%s,%s" ), value1, value2 ) ;
	SaveParameter( doc_file, param_id, duple ) ;
}






/******************************************************************************
CMakePhotoDisk3Doc::SaveDuple

	Save a duple consisting of a string and an int. This is the typical case,
	where we save a picture's title and its output number 

******************************************************************************/
void CMakePhotoDisk3Doc::SaveDuple( 
	CStdioFile & doc_file,					// I - .dp file 
	const TCHAR * param_id, 				// I - tag for string param 
	const TCHAR * str_arg, 					// I - value of string parameter 
	int numeric_arg 						// I - value of 2nd parameter, an int
) 
{
	stringt	value_str ;

	value_str.Format( _T( "%d" ), numeric_arg ) ;
	SaveDuple( doc_file, param_id, str_arg, value_str ) ;
}





/******************************************************************************
CMakePhotoDisk3Doc::AnyRefreshNeeded

	Scan the list of pictures and determine if any photo needs refresh, based
	on the file modify date of the source file (a .psd) and its destination
	file(s) - the .jpg files destined for the disk 

******************************************************************************/
bool CMakePhotoDisk3Doc::AnyRefreshNeeded( )
{
	bool			any_refresh_needed = false ;
	POSITION		pos ;
	PhotoImage *	photo_image ;

	for ( photo_image = GetFirstPhoto( pos ) ; !any_refresh_needed && photo_image != NULL ; photo_image = GetNextPhoto( pos ) ) 
		if ( photo_image->JpegRefreshNeeded( this ) ) 
			any_refresh_needed = true ;

	return any_refresh_needed ;
}




/******************************************************************************
CMakePhotoDisk3Doc::AnySourceMissing

	Scan the list of pictures and determine if any photo is missing its 
	source file  

******************************************************************************/
bool CMakePhotoDisk3Doc::AnySourceMissing( ) 
{
	bool	any_source_missing = false ;
	POSITION		pos ;
	PhotoImage *	photo_image ;

	for ( photo_image = GetFirstPhoto( pos ) ; !any_source_missing && photo_image != NULL ; photo_image = GetNextPhoto( pos ) ) 
		if ( photo_image->SourceIsMissing( ) ) 
			any_source_missing = true ;

	return any_source_missing ;
}



/******************************************************************************
CMakePhotoDisk3Doc::AnySourceUnsupported

	Scan the list of pictures for any photo in the collection with an 
	unsupported source file format 

******************************************************************************/
bool CMakePhotoDisk3Doc::AnySourceUnsupported( ) 
{
	bool	any_source_unsupported = false ;
	POSITION		pos ;
	PhotoImage *	photo_image ;

	for ( photo_image = GetFirstPhoto( pos ) ; !any_source_unsupported && photo_image != NULL ; photo_image = GetNextPhoto( pos ) ) 
		if ( photo_image->UnsupportedFileFormat( ) ) 
			any_source_unsupported = true ;

	return any_source_unsupported ;
}




/******************************************************************************
CMakePhotoDisk3Doc::RenameOutputJpeg

	Called after images have been deleted or added to the collection requiring 
	all images following the deletion (or insertion) to be renumbered. 

******************************************************************************/
void CMakePhotoDisk3Doc::RenameOutputJpeg( 
	int current_number,					// I - old photo number 
	int new_number						// I - new photo number 
) const
{
	static const TCHAR * jpeg_path[ ] = 
	{
		_T( "images\\presentation" ),
		_T( "images\\minip" ), 
		_T( "images\\facebook" ),
		_T( "images\\compressed" ),
		_T( "images\\fullsize" ),
		_T( "pages\\thumbnail" )
	} ;
	int	i ;
	stringt	old_name ;
	stringt	new_name ;

	for ( i = 0 ; i < sizeof( jpeg_path ) / sizeof( jpeg_path[ 0 ] ) ; i++ )
	{
		old_name.Format( _T( "%s\\%s\\%d.jpg" ), GetProjectPath( ), jpeg_path[ i ], current_number ) ;
		new_name.Format( _T( "%s\\%s\\%d.jpg" ), GetProjectPath( ), jpeg_path[ i ], new_number ) ;

		// rename files. But depending on the file type, the 'old_name' file may not exist...
		if ( INVALID_FILE_ATTRIBUTES != GetFileAttributes( old_name ) ) 
			VERIFY( MoveFileEx( old_name, new_name, 0 ) ) ;
	}
}



/******************************************************************************
CMakePhotoDisk3Doc::DetermineOutputNumberByListPosition

	Determine what the output number of a picture should be 
	based on its position in the list. 

******************************************************************************/
int CMakePhotoDisk3Doc::DetermineOutputNumberByListPosition( 
	const PhotoImage * target_image				// I - the PhotoImage in the list
) const 
{
	POSITION		pos ;
	int				pic_no ;
	PhotoImage *	cur_image ;

	for ( pic_no = 1, cur_image = GetFirstPhoto( pos ) ; cur_image != target_image ; cur_image = GetNextPhoto( pos ), pic_no++ )
		;

	ASSERT( cur_image != NULL && pic_no <= m_collection_pic_list.GetCount( ) ) ;

	return pic_no ;
}




/******************************************************************************
CMakePhotoDisk3Doc::FixMisalignedOutputNumbers

	Scan the list of photos for any photo with an out-of-order pic number.

	Note that the pics in the in-memory list are always in the correct order, 
	since they were loaded from the database in the correct sort order for 
	the selected collection.

	Uses a slightly arcane recursive function to re-number output files correctly 
	with only 1 rename operation per mis-named file. 

******************************************************************************/
void CMakePhotoDisk3Doc::FixMisalignedOutputNumbers( )
{
	int				pic_no ;
	POSITION		pos ;
	PhotoImage *	photo_image ;


	for ( photo_image = GetFirstPhoto( pos ), pic_no = 0 ; photo_image != NULL ; photo_image = GetNextPhoto( pos ), pic_no++ )
	{
		if ( photo_image->GetNumberOnDisk( ) != -1 && photo_image->GetNumberOnDisk( ) != pic_no + 1 ) 
		{
			int		start_of_chain = photo_image->GetNumberOnDisk( ) ;

			ChainRenumber( start_of_chain, start_of_chain, pic_no + 1, photo_image ) ;
		}
	}
}



/******************************************************************************
CMakePhotoDisk3Doc::ChainRenumber

	Recursive function to renumber files. 

	Developed in test aplicaiton ChainRenumTest and extensively tested there 
	in massive simulations 

	We need to re-number files, but can't do so immediately if another file is
	using our desired number. So we have to first fix the number for that other
	file. We may continue following this chain, until we either get to the 
	end, or find ourselves coming back to the start number. 

******************************************************************************/
void CMakePhotoDisk3Doc::ChainRenumber( 
	int start_of_chain,						// I - start of this "chain" 
	int cur_actual,							// I - the current output-number for cur_image
	int cur_ideal,							// I - the output-number cur_image SHOULD have 
	PhotoImage * cur_image					// I/O - cur_image 
) 
{
	PhotoImage * next_link ;

	// we don't have the correct output number - find the PhotoImage which is using our output number 
	next_link = FindPhotoByOutputNumber( cur_ideal ) ;

	if ( NULL == next_link ) 
	{
		// no one is using our desired output number - just renumber us 
		RenameOutputJpeg( cur_actual, cur_ideal ) ;
		cur_image->SetOnDiskNumber( cur_ideal ) ;
	}
	else
	{
		if ( next_link->GetNumberOnDisk( ) == start_of_chain ) 
		{
			// next_link's photo is the start of the chain - reassign it to number 0 
			RenameOutputJpeg( start_of_chain, 0 ) ;
			FindPhotoByOutputNumber( start_of_chain )->SetOnDiskNumber( 0 ) ;
			// the file we just renamed as 0.jpg will be renamed appropriately after we return from the recursive call below - 

			RenameOutputJpeg( cur_actual, cur_ideal ) ;
			cur_image->SetOnDiskNumber( cur_ideal ) ;
		}
		else
		{
			// must renumber next_link first.... 
			ChainRenumber( start_of_chain, next_link->GetNumberOnDisk( ), DetermineOutputNumberByListPosition( next_link ), next_link ) ;

			// now we can rename our file and return... 
			// Usually, cur_image->GetNumberOnDisk( ) will be the same as cur_actual, but need to use cur_image->GetNumberOnDisk( ) b/c we might have set it to 0 above.... 
			RenameOutputJpeg( cur_image->GetNumberOnDisk( ), cur_ideal ) ;
			cur_image->SetOnDiskNumber( cur_ideal ) ;
		}
	}
}





/******************************************************************************
CMakePhotoDisk3Doc::RealignOrAssignOutputNumbers

	Called in preparation to generating HTML & jpeg's for disk content. 

	First: make sure any leftover output jpegs match the photo's position in 
	the list - it may have changed if photos have been added to, or deleted 
	from the collection, or if the collection's sorting has changed, since 
	the last time output jpegs were generated. 
		
	Second: assign output numbers to any PhotoImage's in the list where the
	number is currently -1 (meaning, "unassigned"). 

******************************************************************************/
void CMakePhotoDisk3Doc::RealignOrAssignOutputNumbers( ) 
{
	int				pic_no ;
	POSITION		pos ;
	PhotoImage *	photo_image ;

	// if this isn't our first go-round with this disk, we may have old output numbers... 
	FixMisalignedOutputNumbers( ) ;

	// now, assign numbers to all photo's with -1 for an output number 
	for ( photo_image = GetFirstPhoto( pos ), pic_no = 1 ; photo_image != NULL ; photo_image = GetNextPhoto( pos ), pic_no++ )
		if ( photo_image->GetNumberOnDisk( ) == -1 ) 
			photo_image->SetOnDiskNumber( pic_no ) ;
#ifdef _DEBUG
		else if ( photo_image->GetNumberOnDisk( ) != pic_no ) 
		{
			// something is very wrong!!!
			ASSERT( FALSE ) ;
		}
#endif
}





/******************************************************************************
CMakePhotoDisk3Doc::DeleteOrphanJpeg

	Check the disk for "orphaned jpegs", that is, jpegs lying around under the 
	images subdir which don't correspond to any image currently in the 
	collection. This occurs if pictures are removed from the active 
	collection then the disk is re-generated

	FIXME - should also handle cases where the disk-type has been changed 
	to exclude fullsize presentation, fullsize, compressed, etc....  

******************************************************************************/
void CMakePhotoDisk3Doc::DeleteOrphanJpeg( 
	const TCHAR * file_name					// I - base filename of jpeg to delete 
)
{
	stringt	path ;

	path.Format( _T( "%s\\images\\presentation\\%s" ), GetProjectPath( ), file_name ) ;
	DeleteFile( path ) ;

	path.Format( _T( "%s\\images\\fullsize\\%s" ), GetProjectPath( ), file_name ) ;
	DeleteFile( path ) ;

	path.Format( _T( "%s\\images\\compressed\\%s" ), GetProjectPath( ), file_name ) ;
	DeleteFile( path ) ;

	path.Format( _T( "%s\\images\\facebook\\%s" ), GetProjectPath( ), file_name ) ;
	DeleteFile( path ) ;

	path.Format( _T( "%s\\pages\\thumbnail\\%s" ), GetProjectPath( ), file_name ) ;
	DeleteFile( path ) ;
}





/******************************************************************************
DeleteJpegsInDir

	called by CMakePhotoDisk3Doc::DeleteUnwantedJpegOutput( ) to scan the 
	specified directory for any jpeg files and delete them all. 

******************************************************************************/
static void DeleteJpegsInDir( const TCHAR * path ) 
{
	stringt				match_path ;
	WIN32_FIND_DATA		find_data ;
	HANDLE				find_handle ;

	match_path.Format( _T( "%s\\*.jpg" ), path ) ;
	if ( INVALID_HANDLE_VALUE != ( find_handle = FindFirstFile( match_path, &find_data ) ) )
	{
		do
		{
			if ( !( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
			{
				match_path.Format( _T( "%s\\%s" ), path, find_data.cFileName ) ;
				::DeleteFile( match_path ) ;
			}
		}
		while ( FindNextFile( find_handle, &find_data ) ) ;

		FindClose( find_handle ) ;
	}
}





/******************************************************************************
CMakePhotoDisk3Doc::DeleteUnwantedJpegOutput

	Called when user clicks Update Jpeg and HTML button on the Edit/Update panel. 
	We check the output directories to determine if there are any extant
	jpeg output types out there that are no longer wanted, eg, the user has
	unselected "Full Size" and yet we've got files in the fullsize subdirectory 

******************************************************************************/
void CMakePhotoDisk3Doc::DeleteUnwantedJpegOutput( )
{
	stringt	path ;

	if ( !GetDoBigPresentation( ) )
	{
		path.Format( _T( "%s\\images\\presentation" ), GetProjectPath( ) ) ;
		DeleteJpegsInDir( path ) ;
	}

	if ( !GetDoCompressed( ) ) 
	{
		path.Format( _T( "%s\\images\\compressed" ), GetProjectPath( ) ) ;
		DeleteJpegsInDir( path ) ;
	}

	if ( !GetDoFacebook( ) )
	{
		path.Format( _T( "%s\\images\\facebook" ), GetProjectPath( ) ) ;
		DeleteJpegsInDir( path ) ;
	}

	if ( !GetDoFullSize( ) ) 
	{
		path.Format( _T( "%s\\images\\fullsize" ), GetProjectPath( ) ) ;
		DeleteJpegsInDir( path ) ;
	}
}





/******************************************************************************
CMakePhotoDisk3Doc::DeleteOrphanedHTML
	
	Check for and delete "orphaned HTML" pages for pictures. Whenever we re-
	generate any content, we always re-write the HTML for all active pages. 
	But we might have extra HTML pages if the collection has been reduced 
	in size. 

******************************************************************************/
void CMakePhotoDisk3Doc::DeleteOrphanedHTML( ) 
{
	int		pic_no ;
	bool	old_html_found ;

	/* 
		With 'n' pics in the list, UpdateHTMLWorker already over-wrote any outdated presentation pages 
		for the first 'n' pics. We just have to over-write outdated pages for higher-numbered pics
	*/
	pic_no = GetPhotoListSize( ) ;

	do
	{
		stringt		old_html_page_name ;

		old_html_found = false ;

		old_html_page_name.Format( _T( "%s\\pages\\pic%d.htm" ), GetProjectPath( ), pic_no + 1 ) ;
		if ( INVALID_FILE_ATTRIBUTES != GetFileAttributes( old_html_page_name ) ) 
		{
			old_html_found = true ;
			DeleteFile( old_html_page_name ) ;
		}
		pic_no++ ;
	}
	while ( old_html_found ) ;
}





/******************************************************************************
CMakePhotoDisk3Doc::TallyUpJpegRefreshCount

	Called to determine how many jpeg's we need to update. 

******************************************************************************/
void CMakePhotoDisk3Doc::TallyUpJpegRefreshCount( int & refresh_file_ct )  
{
	POSITION		pos ;
	PhotoImage *	photo_image ;

	refresh_file_ct = 0 ;

	for ( photo_image = GetFirstPhoto( pos ) ; NULL != photo_image ; photo_image = GetNextPhoto( pos ) )
	{
		// photo_image may not yet have valid file times if it hasn't yet been displayed on the light table - 
		if ( !photo_image->HasValidFileTimes( ) )
			photo_image->JpegRefreshNeeded( this ) ;

		if ( photo_image->ThumbnailRefreshNeeded( this ) )
			refresh_file_ct++ ;
		if ( photo_image->PresentationRefreshNeeded( this ) ) 
			refresh_file_ct++ ;
		if ( photo_image->MiniPresentationRefreshNeeded( this ) ) 
			refresh_file_ct++ ;
		if ( photo_image->CompressedRefreshNeeded( this ) )
			refresh_file_ct++ ;
		if ( photo_image->FullsizeRefreshNeeded( this ) ) 
			refresh_file_ct++ ;
		if ( photo_image->FacebookRefreshNeeded( this ) ) 
			refresh_file_ct++ ;
	}
}





/******************************************************************************
CMakePhotoDisk3Doc::ResetPhotoTimes

	Called after a jpeg refresh, we step through the image list and reset the 
	date/time for each one. 

******************************************************************************/
void CMakePhotoDisk3Doc::ResetPhotoTimes( )
{
	POSITION		pos ;
	PhotoImage *	image ;

	for ( image = GetFirstPhoto( pos ) ; NULL != image ; image = GetNextPhoto( pos ) )
		image->ResetPhotoTimes( ) ;
}










/******************************************************************************
CMakePhotoDisk3Doc::FindPhotoByFullyQualifiedPathname
	
	This variation of FindPhotoBy____fill in the blank_____ is only used from 
	within the refresh dialog. 

******************************************************************************/
PhotoImage * CMakePhotoDisk3Doc::FindPhotoByFullyQualifiedPathname( 
	const TCHAR * name					// I - fully qualified source file name 
) const
{
	POSITION		pos ;
	PhotoImage *	image ;
	stringt			source_path ;

	for ( image = GetFirstPhoto( pos ) ; NULL != image ; image = GetNextPhoto( pos ) ) 
	{
		source_path = image->GetSourceFilePath( ) ;
		if ( 0 == source_path.CompareNoCase( name ) ) 
			return image ;

		source_path.MakeLower( ) ;

		if ( _tcsstr( source_path, name ) )
			return image ;
	}

	return image ;
}






/******************************************************************************
CMakePhotoDisk3Doc::FindPhotoByFilename

	This variation of FindPhotoBy____fill in the blank_____ finds the PhotoImage
	with matching basename 

******************************************************************************/
PhotoImage * CMakePhotoDisk3Doc::FindPhotoByFilename(
	const TCHAR * name							// I - base name of source file 
) const
{
	POSITION		pos ;
	PhotoImage *	image ;
	stringt			source_path ;

	for ( image = GetFirstPhoto( pos ) ; NULL != image ; image = GetNextPhoto( pos ) ) 
	{
		if ( 0 == _tcsicmp( image->GetName( ), name ) )
			return image ;
	}

	return NULL ;
} 



/******************************************************************************
CMakePhotoDisk3Doc::FindPhotoByOutputNumber

	This variation of FindPhotoBy____fill in the blank_____ finds the PhotoImage
	with a particular output number 

******************************************************************************/
PhotoImage * CMakePhotoDisk3Doc::FindPhotoByOutputNumber( 
	int output_number					// I - output number of photo 
) const
{
	POSITION		pos ;
	PhotoImage *	image ;

	for ( image = GetFirstPhoto( pos ) ; NULL != image ; image = GetNextPhoto( pos ) )
		if ( image->GetNumberOnDisk( ) == output_number ) 
			return image ;

	return NULL ;
}





/******************************************************************************
CMakePhotoDisk3Doc::CalculateDiskUsage

	Estimate the amount of disk space to store a disk's generated content 

******************************************************************************/
void CMakePhotoDisk3Doc::CalculateDiskUsage( )
{
	RecurseCalcDiskUse( GetProjectPath( ) ) ;
}



/******************************************************************************
CMakePhotoDisk3Doc::GetDiskUsage

	Estimate the amount of disk space to store a disk's generated content 

******************************************************************************/
UINT64 CMakePhotoDisk3Doc::GetDiskUsage( )
{
	if ( 0 == m_disk_size ) 
		CalculateDiskUsage( ) ;

	return m_disk_size ;
}







/******************************************************************************
CMakePhotoDisk3Doc::RecurseCalcDiskUse

	Estimate the amount of disk space to store a disk's generated content 

******************************************************************************/
void CMakePhotoDisk3Doc::RecurseCalcDiskUse( 
	const TCHAR * dir_name					// I - directory to tote up
)
{
	stringt				path ;
	HANDLE				find_handle ;
	WIN32_FIND_DATA		find_data ;

	path.Format( _T( "%s\\*" ), dir_name ) ;

	if ( INVALID_HANDLE_VALUE != ( find_handle = FindFirstFile( path, &find_data ) ) ) 
	{
		do
		{
			if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
			{
				if ( 0 != _tcscmp( find_data.cFileName, _T( "." ) ) && 0 != _tcscmp( find_data.cFileName, _T( ".." ) ) && 0 != _tcscmp( find_data.cFileName, _T( "hidden" ) ) ) 
				{
					path.Format( _T( "%s\\%s" ), dir_name, find_data.cFileName ) ;

					RecurseCalcDiskUse( path ) ;
				}
			}
			else
			{
				stringt debug_trace ;

				debug_trace.Format( _T( "%s\n" ), find_data.cFileName ) ;
				TRACE( debug_trace ) ;

				m_disk_size += MAKEUINT64( find_data.nFileSizeLow, find_data.nFileSizeHigh ) ;
			}
		}
		while ( FindNextFile( find_handle, &find_data ) ) ;

		FindClose( find_handle ) ;
	}
}





/******************************************************************************
CMakePhotoDisk3Doc::OnFileSave

	I think this is just here to prevent the default MFC version from getting 
	called.  

******************************************************************************/
void CMakePhotoDisk3Doc::OnFileSave()
{
}









/******************************************************************************
CMakePhotoDisk3Doc::OnUpdateFileSave

	Disable the file save menu option. We don't save/load documents the standard
	MFC way. 

******************************************************************************/
void CMakePhotoDisk3Doc::OnUpdateFileSave(
	CCmdUI *pCmdUI
)
{
	pCmdUI->Enable( FALSE ) ;
}





/******************************************************************************
PrepareDataString

	Format a string for SQL insert/update operations. The string must be enclosed in single quotes, and 
	any embedded quotes need to be duplicated, eg, the string

		"I can't live without apostrophe's," He said. 

	becomes

		'""I can''t live without apostrophe''s,"" He said.' 

******************************************************************************/
void PrepareDataString( 
	stringt & output,					// O - gets formatted string 
	const TCHAR * input_string			// I - string before formatting... 
) 
{
	stringt work ;

	work = input_string ;
	work.Replace( _T( "'" ), _T( "''" ) ) ;
	work.Replace( _T( "\"" ), _T( "\"\"" ) ) ;
	output.Format( _T( "'%s'" ), (const TCHAR *) work ) ;
}





/******************************************************************************
DigestDataString

	Remove double-quotes and double-apostrophes from a string 

******************************************************************************/
void DigestDataString( 
	stringt & output,					// O - gets the massaged string 
	const TCHAR * input_string			// I - the SQL-ready formatted string 
) 
{
	output = input_string ;

	output.Replace( _T( "''" ), _T( "'" ) ) ;
	output.Replace( _T( "\"\"" ), _T( "\"" ) ) ;
}





/******************************************************************************
CMakePhotoDisk3Doc::OpenCustomEditsDBAndGuaranteeTables

	Used by functions which save and load metadata to open the metadata db. 

******************************************************************************/
bool CMakePhotoDisk3Doc::OpenCustomEditsDBAndGuaranteeTables( 
	SQLDatabase & db				// O - gets open DB 
) 
{
	stringt	db_path ;

	// first - open the db and make sure the tables are there.... 
	db_path.Format( _T( "%s\\metadata.db" ), (const TCHAR *) m_app_path ) ;
	db.Open( db_path ) ;

	if ( db.CreateTable( _T( "DiskProjects" ), _T( "\n     record_ID   INTEGER PRIMARY KEY,\n     project_name NOT NULL\n" ) ) )
	{
		if ( db.CreateTable( _T( "CustomEdits" ), _T( "\n     record_ID   INTEGER PRIMARY KEY,\n     base_name NOT NULL,\n     timestamp_hi INTEGER,\n     timestamp_lo INTEGER,\n     title,\n     description,\n     create_time_string,\n     adjust_seconds INTEGER,\n     photographer,\n     camera,\n     comment\n" ) ) ) 
		{
			if ( db.CreateTable( _T( "ProjectXEdits" ), _T( "\n     record_ID INTEGER PRIMARY KEY,\n     disk_project INTEGER NOT NULL,\n     custom_edit INTEGER NOT NULL\n" ) ) )
			{
				return true ;
			}
		}
	}

	return false ;
}







/******************************************************************************
CMakePhotoDisk3Doc::CheckSaveCustomMetadata

	Go through the photo list and make sure any new metadata edits are saved to the db. 

******************************************************************************/
void CMakePhotoDisk3Doc::CheckSaveCustomMetadata( )
{
	SQLDatabase	db ;

	if ( OpenCustomEditsDBAndGuaranteeTables( db ) )
	{
		stringt			project_name_fmt ;
		PhotoImage *	photo ;
		POSITION		pos ;

		if ( CatalogCollectionProjectKnown == m_doc_state )
			PrepareDataString( project_name_fmt, m_project_directory.MakeLower( ) ) ;
		else
			project_name_fmt = _T( "'unspecified project'" ) ;

		// zip through our photo collection looking for any with new edits (HasDirtyEdits = true)
		for ( photo = GetFirstPhoto( pos ) ; NULL != photo ; photo = GetNextPhoto( pos ) )
		{
			if ( photo->HasDirtyEdits( ) )
			{
				// We have a photo with new edits to it.... and they need to be saved to the database... 
				stringt		where_clause ;
				SYSTEMTIME	create_time ;
				int			edit_recno ;
				int			proj_recno ;
				int			adjust_seconds ;
				stringt		tmp_string ;
				stringt		photo_name ;

				// these variables store the representation of the corresponding data as used in SQL statements. 
				// strings are enclosed in single quotes, and any apostrophes or double-quotes within the strings need to double up - eg, "it's easy" ==> "'it''s easy'" 
				stringt		base_name_fmt ;
				stringt		title_fmt ;
				stringt		description_fmt ;
				stringt		create_time_fmt ;
				stringt		adjust_seconds_fmt ;
				stringt		photographer_fmt ;
				stringt		camera_fmt ;
				stringt		comment_fmt ;
				stringt		timestamp_hi_fmt ;
				stringt		timestamp_lo_fmt ;


				// Now, format the variables - we're going to need them for either an INSERT or an UPDATE operation 
				photo_name = photo->GetName( ) ;
				PrepareDataString( base_name_fmt, photo_name.MakeLower( ) ) ;

				photo->GetOverrideTitle( tmp_string ) ;
				PrepareDataString( title_fmt, tmp_string ) ;

				photo->GetOverrideDescr( tmp_string ) ;
				PrepareDataString( description_fmt, tmp_string ) ;

				// SYSTEMTIME and integers need different handling... 
				if ( photo->GetOverrideCreateTime( create_time ) ) 
					create_time_fmt.Format( _T( "'%d/%d/%d-%d:%d:%d'" ), create_time.wYear, create_time.wMonth, create_time.wDay, create_time.wHour, create_time.wMinute, create_time.wSecond ) ;
				else
					create_time_fmt = _T( "''" ) ;

				if ( photo->GetCreateTimeAdjust( adjust_seconds ) )
					adjust_seconds_fmt.Format( _T( "%d" ), adjust_seconds ) ;
				else
					adjust_seconds_fmt = _T( "0" ) ;

				// we're assuming 'int' is 32-bit size
				ASSERT( sizeof( int ) == sizeof( __int32 ) ) ;
				timestamp_hi_fmt.Format( _T( "%u" ), photo->GetOverrideTimestamp( ).dwHighDateTime ) ;
				timestamp_lo_fmt.Format( _T( "%u" ), photo->GetOverrideTimestamp( ).dwLowDateTime ) ;

				photo->GetOverridePhotographer( tmp_string ) ;
				PrepareDataString( photographer_fmt, tmp_string ) ;

				photo->GetOverrideCameraName( tmp_string ) ;
				PrepareDataString( camera_fmt, tmp_string ) ;

				photo->GetExtendedComment( tmp_string ) ;
				PrepareDataString( comment_fmt, tmp_string ) ;

				SQLCursor edit_cursor ;

				if ( db.ExecuteQuery( edit_cursor, _T( "SELECT CustomEdits.* FROM CustomEdits, DiskProjects, ProjectXEdits " )
													_T( "WHERE base_name='%s' AND project_name=%s AND DiskProjects.record_ID = ProjectXEdits.disk_project AND CustomEdits.record_ID = ProjectXEdits.custom_edit" ), 
													(const TCHAR *) photo_name, (const TCHAR *) project_name_fmt ) )
				{
					stringt	set_list ;

					// there IS a record for this photo in this project - we're just going to update it. 
					edit_cursor.GetFieldValue( _T( "CustomEdits.record_ID" ), edit_recno ) ;

					set_list.Format( _T( "timestamp_hi=%s, timestamp_lo=%s, title=%s, description=%s, create_time_string=%s, adjust_seconds=%s, photographer=%s, camera=%s, comment=%s" ), 
											static_cast< const TCHAR *>( timestamp_hi_fmt ),
											static_cast< const TCHAR *>( timestamp_lo_fmt ),
											static_cast< const TCHAR *>( title_fmt ), 
											static_cast< const TCHAR *>( description_fmt ), 
											static_cast< const TCHAR *>( create_time_fmt ), 
											static_cast< const TCHAR *>( adjust_seconds_fmt ),  
											static_cast< const TCHAR *>( photographer_fmt ), 
											static_cast< const TCHAR *>( camera_fmt ), 
											static_cast< const TCHAR *>( comment_fmt ) ) ;

					db.ExecuteSQL( _T( "UPDATE CustomEdits SET %s WHERE record_ID = %d" ), (const TCHAR *) set_list, edit_recno ) ;
				}
				else
				{
					stringt		value_list ;
					SQLCursor	project_cursor ;

					/* 
						We know there's no record for this photo in this project. But check to see if there's already a record for this project....
						if there is no record for the current project, create it
					*/
					if ( db.ExecuteQuery( project_cursor, _T( "SELECT * FROM DiskProjects WHERE project_name = %s" ), (const TCHAR *) project_name_fmt ) )
					{
						project_cursor.GetFieldValue( _T( "record_ID" ), proj_recno ) ;
					}
					else
					{
						db.ExecuteSQL( _T( "INSERT INTO DiskProjects ( project_name ) VALUES ( %s )" ), (const TCHAR *) project_name_fmt ) ;
						proj_recno = db.GetLastInsertRowID( ) ;
					}

					// whatever happened above, we now have a project record number in 'proj_recno' - 

					value_list.Format( _T( "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s" ), 
											static_cast< const TCHAR *>( base_name_fmt ), 
											static_cast< const TCHAR *>( title_fmt ), 
											static_cast< const TCHAR *>( timestamp_hi_fmt ),
											static_cast< const TCHAR *>( timestamp_lo_fmt ),
											static_cast< const TCHAR *>( description_fmt ), 
											static_cast< const TCHAR *>( create_time_fmt ), 
											static_cast< const TCHAR *>( adjust_seconds_fmt ), 
											static_cast< const TCHAR *>( photographer_fmt ), 
											static_cast< const TCHAR *>( camera_fmt ), 
											static_cast< const TCHAR *>( comment_fmt ) ) ;

					VERIFY( db.ExecuteSQL( _T( "INSERT INTO CustomEdits ( base_name, title, timestamp_hi, timestamp_lo, description, create_time_string, adjust_seconds, photographer, camera, comment ) VALUES ( %s )" ), (const TCHAR *) value_list ) ) ;
					edit_recno = db.GetLastInsertRowID( ) ;

					// now create the ProjectXEdits record which ties the Project record to the CustomEdits record. 
					value_list.Format( _T( "%d, %d" ), proj_recno, edit_recno ) ;
					VERIFY( db.ExecuteSQL( _T( "INSERT INTO ProjectXEdits ( disk_project, custom_edit ) VALUES ( %s )" ), (const TCHAR *) value_list ) ) ;
				}

				// reset the dirty edits flag - 
				photo->SetHasDirtyEdits( false ) ;
			}
		}
	}
}




/******************************************************************************
CMakePhotoDisk3Doc::LoadCustomMetadata

	We have just loaded the image list. We may, or we may not, have a peoject selected at this point. So...
	if we do have a project selected...
		we query the metadata database for the image and the given project 

		if we find a match - that's what we set as the metadata else, we try again, without specifying the project.... 

	if we do not have a peoject selected... 
		we query the metadata database for each image

		if we find a match - we use the metadata as "tentative" metadata.... 

	Custom metadata is stored in a small private database consisting of 3 tables. 

	CustomEdits, which stores override data for a file in a particular project 

	DiskProjects, which contains a record for each project, plus 1 record for "unspecified project" 

	ProjectXEdits, encodes the many-to-many relationship between CustomEdits and DiskProjects 

******************************************************************************/
void CMakePhotoDisk3Doc::LoadCustomMetadata( )
{
	SQLDatabase	db ;

	if ( OpenCustomEditsDBAndGuaranteeTables( db ) )
	{
		PhotoImage *	photo ;
		POSITION		pos ;
		stringt			project_name ;

		if ( CatalogCollectionProjectKnown == m_doc_state )
			project_name = m_project_directory.MakeLower( ) ;

		for ( photo = GetFirstPhoto( pos ) ; NULL != photo ; photo = GetNextPhoto( pos ) )
		{
			stringt			photo_name ;
			stringt			where_clause ;
			SQLCursor 		edits_cursor ;

			photo_name = photo->GetName( ) ;
			photo_name = photo_name.MakeLower( ) ;

			if ( CatalogCollectionProjectKnown == m_doc_state )
			{
				// FIXME: if project name's ever contain apostrophes, we need to use PrepareDataString( ) on project_name 
				where_clause.Format( _T( "base_name='%s' AND project_name='%s' AND ProjectXEdits.custom_edit = CustomEdits.record_ID AND ProjectXEdits.disk_project = DiskProjects.record_ID" ), (const TCHAR *) photo_name, (const TCHAR *) project_name ) ;

				if ( db.ExecuteQuery( edits_cursor, _T( "SELECT * FROM CustomEdits, DiskProjects, ProjectXEdits WHERE %s" ), (const TCHAR *) where_clause ) )
				{
					SetOverrides( photo, edits_cursor ) ;
					continue ;
				}
			}

			// get here if either project is NOT known, or the query failed - 
			where_clause.Format( _T( "base_name='%s' AND ProjectXEdits.custom_edit = CustomEdits.record_ID AND ProjectXEdits.disk_project = DiskProjects.record_ID" ), (const TCHAR *) photo_name ) ;

			if ( db.ExecuteQuery( edits_cursor, _T( "SELECT * FROM CustomEdits, DiskProjects, ProjectXEdits WHERE %s" ), (const TCHAR *) where_clause ) )
			{
				SetOverrides( photo, edits_cursor ) ;
			}
		}
	}
}







/******************************************************************************
CMakePhotoDisk3Doc::SetOverrides

	Called from LoadCustomMetadata. We've found custom metadata edits 
	applying to the PhotoImage passed in. 

******************************************************************************/
void CMakePhotoDisk3Doc::SetOverrides( 
	PhotoImage * photo,					// O - PhotoImage which may be updated 
	SQLCursor & cursor					// I - current record in metadata db applicable to PhotoImage
) 
{
	stringt		data_string ;
	stringt		work ;
	int			data_int ;
	FILETIME	timestamp ;

	// have metadata edits specific to this photo in this project - 
	if ( cursor.GetFieldValue( _T( "title" ), work ) ) 
	{
		DigestDataString( data_string, work ) ;
		photo->SetOverrideTitle( data_string ) ;
	}

	if ( cursor.GetFieldValue( _T( "description" ), work ) )
	{
		DigestDataString( data_string, work ) ;
		photo->SetOverridedDescr( data_string ) ;
	}

	cursor.GetFieldValue( _T( "timestamp_lo" ), data_int ) ;
	timestamp.dwLowDateTime = data_int ;
	cursor.GetFieldValue( _T( "timestamp_hi" ), data_int ) ;
	timestamp.dwHighDateTime = data_int ;
	photo->SetOverrideTimestamp( timestamp ) ;

	if ( cursor.GetFieldValue( _T( "create_time_string" ), data_string ) )
	{
		SYSTEMTIME	create_time ;

		if ( data_string.GetLength( ) )
		{
			int			year ;
			int			month ;
			int			day ;
			int			hour ;
			int			minute ;
			int			second ;

			// parse create_time string, which is in the format: yyyy/mm/dd-hh:mm:ss
			_stscanf_s( data_string, _T( "%d/%d/%d-%d:%d:%d" ), &year, &month, &day, &hour, &minute, &second ) ;

			create_time.wYear = year ;
			create_time.wMonth = month ;
			create_time.wDay = day ;
			create_time.wHour = hour ;
			create_time.wMinute = minute ;
			create_time.wSecond = second ;
			create_time.wMilliseconds = 0 ;
		}
		else
			ZeroMemory( &create_time, sizeof( create_time ) ) ;

		photo->SetOverrideCreateTime( create_time ) ;
	}

	if ( cursor.GetFieldValue( _T( "adjust_seconds" ), data_int ) )
	{
		photo->SetCreateTimeAdjust( data_int ) ;
	}

	if ( cursor.GetFieldValue( _T( "photographer" ), work ) ) 
	{
		DigestDataString( data_string, work ) ;
		photo->SetOverridePhotographer( data_string ) ;
	}

	if ( cursor.GetFieldValue( _T( "camera" ), work ) ) 
	{
		DigestDataString( data_string, work ) ;
		photo->SetOverrideCameraName( data_string ) ;
	}

	if ( cursor.GetFieldValue( _T( "comment" ), work ) )
	{
		DigestDataString( data_string, work ) ;
		photo->SetExtendedComment( data_string ) ;
	}
}





/******************************************************************************
CMakePhotoDisk3Doc::ChangedCreateTime

	Called after the user-edited metadata has effectively changed the photo create
	time. If the collection is sorted based on create-time, that means that the 
	PhotoImage list may need to be updated. 

	We might have a list which is sorted in ascending order, or descending order. We might have
	a photo which has had its create-time advanced to a later time or rolled back to an earlier
	time. 

	sort order		change to photo		result
	--------------	-----------------	----------------------------------
	ascending		advance time		move down list until find later time or end of list
	ascending		rollback time		move up list until find earlier time or start of list
	descending		advance time		move up list until find later time or start of list
	descending		rollback time		move down list until find earlier time or end of list

	Arguments:
		altered_image		the PhotoImage that has been changed. 

		change				result of compare between altered_image's old time and its
							current time. If < 0, altered_image's create time has been rolled back
							and if > 0, altered_image's create time has been advanced 

		update_views		true if this function should call UpdateAllViews. This argument
							will be false in case this is called by the wrapper which handles
							multi-select updates, in which case the wrapper handles the update

******************************************************************************/
bool CMakePhotoDisk3Doc::ChangedCreateTime( 
	PhotoImage * altered_image,					// I - the PhotoImage with user-edits changing the effective create time 
	int change,									// I - code indicating change to create time (see function comment) 
	bool update_views /*=true*/					// I - should we call UpdateAllViews 
)
{
	bool	reordered_list = false ;

	if ( m_sort_by_capture_time )
	{
		// and m_ascending_sort tells us if the list is ascending 
		bool			moving_up ;			// true if moving up list 
		POSITION		image_pos ;			// set to the position of altered_image. 
		POSITION		current_pos ;		// our current position as we step through the list
		PhotoImage *	candidate ;			// the photo @ current position
		bool			keep_going ;
		int				skip_count ;

		// we're going to be moving up if either 
		moving_up = ( m_ascending_sort && change < 0 ) || ( !m_ascending_sort && change > 0 ) ;

		current_pos = image_pos = m_collection_pic_list.Find( altered_image ) ;

		skip_count = 0 ;

		// scan up or down the list until we hit a stopping point - 
		do
		{
			keep_going = false ;

			if ( moving_up )
				m_collection_pic_list.GetPrev( current_pos ) ;
			else
				m_collection_pic_list.GetNext( current_pos ) ;

			if ( current_pos ) 
			{
				candidate = m_collection_pic_list.GetAt( current_pos ) ;

				if ( change > 0 )		
				{
					// increased time of altered_image, keep going as long as candidate is earlier 
					keep_going = candidate->CompareCreateTime( *altered_image ) < 0 ;
				}
				else
				{
					// decreased time of altered_image, keep going as long as candidate is later...
					keep_going = candidate->CompareCreateTime( *altered_image ) > 0 ;
				}

				if ( keep_going )
					skip_count++ ;
			}
		}
		while ( current_pos && keep_going ) ;

		// skip_count gives us the number of Photos to skip over - if 0, there's no change to the list 
		if ( skip_count ) 
		{
			reordered_list = true ;

			// first - remove the image from the list.... 
			m_collection_pic_list.RemoveAt( image_pos ) ;

			if ( moving_up )
			{
				if ( current_pos )
					m_collection_pic_list.InsertAfter( current_pos, altered_image ) ;
				else
					m_collection_pic_list.AddHead( altered_image ) ;
			}
			else
			{
				if ( current_pos )
					m_collection_pic_list.InsertBefore( current_pos, altered_image ) ;
				else
					m_collection_pic_list.AddTail( altered_image ) ;
			}

			if ( update_views )
			{
				ASSERT( GetSelectionCount( ) == 1 ) ;

				CRange	cell_range ;

				if ( moving_up ) 
				{
					cell_range.first = m_last_selection - skip_count ;
					cell_range.last = m_last_selection ;
					m_last_selection = cell_range.first ;
				}
				else
				{
					cell_range.first = m_last_selection ;
					cell_range.last = m_last_selection + skip_count ;
					m_last_selection = cell_range.last ;
				}

				m_anchor_selection = m_last_selection ;

				UpdateAllViews( NULL, ReorderImages, &cell_range ) ;
			}
		}

		if ( reordered_list ) 
			RenumberPhotoList( ) ;
	}

	return reordered_list ;
}


/******************************************************************************
CMakePhotoDisk3Doc::RenumberPhotoList

	Called 
	
	1. after re-sorting the list after edits which change the create-time of 
	any photo in a collection sorted based on create time. 

	2. at the end of RenumberPhotoList( ), which may be called after loading 
	a project 

******************************************************************************/
void CMakePhotoDisk3Doc::RenumberPhotoList( )
{
	int				pos_no ;
	POSITION		pos ;
	PhotoImage *	cur_photo ;

	for ( pos_no = 1, cur_photo = GetFirstPhoto( pos ) ; cur_photo != NULL ; pos_no++, cur_photo = GetNextPhoto( pos ) ) 
		cur_photo->m_number_in_collection = pos_no ;
}
