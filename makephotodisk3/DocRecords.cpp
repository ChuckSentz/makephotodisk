/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	Implementation of Collection and PhotoImage classes. Usually, a Collection consists
	of images, but it can optionally contain other collections. In this case, Lightroom
	calls it a collection set. 

***************************************************************************************/
#include "stdafx.h"

#include "MakePhotoDisk3Doc.h"



/******************************************************************************
Collection::Collection

	Generic constructor for collection class. If the collection is a collection-set
	(a collection of collections), then its child queue will be populated with
	the collections in the set. 

	The list of PhotoImage's in a collection is only loaded for the selected
	collection and is stored in the document object 

******************************************************************************/
Collection::Collection( 
	int id_local,					// I - id of the collection in the Lightroom DB
	bool collection_set,			// I - true if collection set, false if regular collection
	const TCHAR * name				// I - name of the collection 
)
{
	m_id_local = id_local ;
	m_collection_set = collection_set ;
	m_name = name ;
}


/******************************************************************************
Collection::~Collection

	plain vanilla destructor 

******************************************************************************/
Collection::~Collection( ) 
{
	while ( m_child_q.GetCount( ) )
		delete m_child_q.RemoveHead( ) ;
} 


/******************************************************************************
Collection::AddChild

	Add a child collection 

******************************************************************************/
void Collection::AddChild( Collection * child_collection )
{
	ASSERT( m_collection_set == true ) ;

	m_child_q.AddTail( child_collection ) ;
}

/******************************************************************************
Collection::GetFirstChild

	Start iterating through child collections 

******************************************************************************/
Collection * Collection::GetFirstChild( 
	POSITION & pos				// O - gets position of next item on exit 
) const
{
	if ( pos = m_child_q.GetHeadPosition( ) )
		return GetNextChild( pos ) ;
	return NULL ;
}

/******************************************************************************
Collection::GetNextChild

	Continue iterating the child queue

******************************************************************************/
Collection * Collection::GetNextChild( 
	POSITION & pos				// I/O - 
) const
{
	if ( NULL != pos ) 
		return m_child_q.GetNext( pos ) ;
	return NULL ;
}




/******************************************************************************
PhotoImage::PhotoImage

	Construct a PhotoImage object from data loaded from DB 

******************************************************************************/
PhotoImage::PhotoImage( 
			const TCHAR * name,					// I - name of photo 
			const TCHAR * psd_path,				// I - 
			const TCHAR * jpeg_file,			// I - 
			const TCHAR * title,				// I - 
			const TCHAR * description,			// I - 
			const TCHAR * camera,				// I - 
			const TCHAR * lens,					// I - 
			const TCHAR * exposure,				// I - 
			const TCHAR * iso,					// I - 
			int			  pic_no,				// I - 
			int			  orientation,			// I - 
			int			  pixel_width,			// I - 
			int			  pixel_height,			// I - 
			const SYSTEMTIME &	create_time,	// I - 
			int			pick,					// I - 
			const TCHAR * file_format			// I -
)
{
	m_preview_jpeg = jpeg_file ;
	m_psd_file = psd_path ;
	m_name = name ;
	m_pick = pick ;
	m_file_format = file_format ;
	m_source_missing = -1 ;
	m_valid_file_times = false ;

	m_number_in_collection = pic_no ;
	m_number_on_disk = -1 ;				// unknown until we check

	m_extracted_data.title = title ;
	m_extracted_data.description = description ;
	m_extracted_data.camera = camera ;
	m_extracted_data.lens = lens ;
	m_extracted_data.exposure = exposure ;
	m_extracted_data.ISO = iso ;
	m_extracted_data.orientation = orientation ;
	m_extracted_data.create_time = create_time ;
	m_extracted_data.height = pixel_height ;
	m_extracted_data.width = pixel_width ;

	m_override_data.flag = 0 ;
	m_override_data.create_adjust = 0 ;
	m_dirty_edits = false ;
	m_selected = false ;

	m_fullp_width = 0 ;
	m_minip_width = 0 ;
	m_fullp_height = 0 ;
	m_minip_height = 0 ;

	memset( &m_source_mod_time, 0, sizeof( m_source_mod_time ) ) ;
	memset( &m_presentation_mod_time, 0, sizeof( m_presentation_mod_time ) ) ;
	memset( &m_minip_mod_time, 0, sizeof( m_minip_mod_time ) ) ;
	memset( &m_compressed_mod_time, 0, sizeof( m_compressed_mod_time ) ) ;
	memset( &m_facebook_mod_time, 0, sizeof( m_facebook_mod_time ) ) ;
	memset( &m_fullsize_mod_time, 0, sizeof( m_fullsize_mod_time ) ) ;
	memset( &m_thumbnail_mod_time, 0, sizeof( m_thumbnail_mod_time ) ) ;

	memset( &m_extracted_data_timestamp, 0, sizeof( m_extracted_data_timestamp ) ) ;
	memset( &m_override_data_timestamp, 0, sizeof( m_override_data_timestamp ) ) ;
}





/******************************************************************************
PhotoImage::PhotoImage
	
	Default constructor 

******************************************************************************/
PhotoImage::PhotoImage( )
{
	m_pick = 0 ;
	m_number_on_disk = -1 ;				// undefined 
	m_number_in_collection = -1 ;		//     "

	m_extracted_data.orientation = 0 ;
	m_extracted_data.height = 0 ;
	m_extracted_data.width = 0 ;
	ZeroMemory( &m_extracted_data.create_time, sizeof( m_extracted_data.create_time ) ) ;

	m_override_data.flag = 0 ;
	ZeroMemory( &m_override_data.create_time, sizeof( m_override_data.create_time ) ) ;
	m_override_data.create_adjust = 0 ;

	m_valid_file_times = false ;
	m_source_missing = 1 ;
	m_dirty_edits = false ;
	m_selected = false ;

	ZeroMemory( &m_source_mod_time,			sizeof( m_source_mod_time ) ) ;
	ZeroMemory( &m_presentation_mod_time,	sizeof( m_presentation_mod_time ) ) ;
	ZeroMemory( &m_minip_mod_time,			sizeof( m_minip_mod_time ) ) ;
	ZeroMemory( &m_compressed_mod_time,		sizeof( m_compressed_mod_time ) ) ;
	ZeroMemory( &m_facebook_mod_time,		sizeof( m_facebook_mod_time ) ) ;
	ZeroMemory( &m_fullsize_mod_time,		sizeof( m_fullsize_mod_time ) ) ;
	ZeroMemory( &m_thumbnail_mod_time,		sizeof( m_thumbnail_mod_time ) ) ;
	ZeroMemory( &m_extracted_data_timestamp,sizeof( m_extracted_data_timestamp ) ) ;
	ZeroMemory( &m_override_data_timestamp, sizeof( m_override_data_timestamp ) ) ;
}





/******************************************************************************
PhotoImage::operator=

	Assignment operator 

******************************************************************************/
PhotoImage & PhotoImage::operator=( const PhotoImage & other )
{
	if ( this != &other )
	{
		m_preview_jpeg		   	= other.m_preview_jpeg ;
		m_psd_file			   	= other.m_psd_file ;
		m_name 				   	= other.m_name ;
		m_pick 				   	= other.m_pick ;
		m_file_format 		   	= other.m_file_format ;
		m_number_on_disk 	   	= other.m_number_on_disk ;
		m_number_in_collection 	= other.m_number_in_collection ;

		m_extracted_data 	   	= other.m_extracted_data ;
		m_override_data 	   	= other.m_override_data ;

		m_valid_file_times	   	= other.m_valid_file_times ;
		m_source_missing	   	= other.m_source_missing ;

		m_source_mod_time	   	= other.m_source_mod_time ;
		m_minip_mod_time		= other.m_minip_mod_time ;
		m_presentation_mod_time	= other.m_presentation_mod_time ;
		m_compressed_mod_time  	= other.m_compressed_mod_time ;
		m_facebook_mod_time	   	= other.m_facebook_mod_time ;
		m_fullsize_mod_time	   	= other.m_fullsize_mod_time ;
		m_thumbnail_mod_time   	= other.m_thumbnail_mod_time ;
		m_dirty_edits			= other.m_dirty_edits ;
		m_selected				= other.m_selected ;

		m_extracted_data_timestamp = other.m_extracted_data_timestamp ;
		m_override_data_timestamp = other.m_override_data_timestamp ;
	}

	return *this ;
}





/******************************************************************************
PhotoImage::~PhotoImage

	destructor 

******************************************************************************/
PhotoImage::~PhotoImage( )
{
	DeleteFile( m_preview_jpeg ) ;
}




/******************************************************************************
PhotoImage::GetFormattedCaption

	Formats the photo's caption, which normally consists of the title, a colon,
	and the description. 

	The results will reflect whatever overrides the user has specified. 

	If there is no title or description, nor any overrides, the caption
	defaults to the last choice, which is just the filename 

******************************************************************************/
CString PhotoImage::GetFormattedCaption( ) const
{
	CString	work ;
	CString title ;
	CString descr ;

	// first choice for title is the override, 2nd is the extracted title, 3rd is the filename 
	if ( m_override_data.title.GetLength( ) )
		title = m_override_data.title ;
	else if ( m_extracted_data.title.GetLength( ) ) 
		title = m_extracted_data.title ;
	else 
		title = m_name ;

	// first choice for description is the override, then the extracted, or NULL if neither 
	if ( m_override_data.description.GetLength( ) )
		descr = m_override_data.description ;
	else if ( m_extracted_data.description.GetLength( ) )
		descr = m_extracted_data.description ;

	descr.Trim( ) ;

	// if there is a description, format the caption as title: description, otherwise, just use the title 
	if ( descr.GetLength( ) )
		work.Format( _T( "%s: %s" ), (const TCHAR *) title, (const TCHAR *) descr ) ;
	else
		work = title ;

	return work ;
}




/******************************************************************************
PhotoImage::GetFormattedCaptionOverrides

	Returns true iff. there are overrides of either title or description 

******************************************************************************/
bool PhotoImage::GetFormattedCaptionOverrides( ) const 
{
	return 0 != m_override_data.title.GetLength( ) || 0 != m_override_data.description.GetLength( ) ;
}






/******************************************************************************
PhotoImage::AddSeconds

	A class static. 

	This function returns the SYSTEMTIME which would result after adding a given 
	number of seconds (which can be negative) 

******************************************************************************/
SYSTEMTIME PhotoImage::AddSeconds( 
	const SYSTEMTIME & date_time,			// I - the starting SYSTEMTIME 
	INT32 adjust_seconds					// I - number of seconds to adjust it by 
) 
{
	FILETIME	ft ;
	INT64		adjust_64 ;
	INT64		ft_64 ;
	SYSTEMTIME	result_time ;
		
	SystemTimeToFileTime( &date_time, &ft ) ;

	// convert the FILETIME structure to the equivalent 64-bit integer
	ft_64 = (INT64)( ( ( (UINT64) ft.dwHighDateTime ) << 32 ) + (UINT64) ft.dwLowDateTime ) ;

	// convert 32-bit adjust_seconds (in seconds) to 64-bit equivalent in 100-nanosecond intervals 
	adjust_64 = adjust_seconds ;
	adjust_64 *= 10000000 ;		
	ft_64 += adjust_64 ;

	ft.dwLowDateTime  = (DWORD) (ft_64 & 0xFFFFFFFF );
	ft.dwHighDateTime = (DWORD) (ft_64 >> 32 );

	FileTimeToSystemTime( &ft, &result_time ) ;

	return result_time ;
}





/******************************************************************************
PhotoImage::FormatSystemTime

	This is a protected helper function, not an API. 
	
	GetCreationSubCaption( ) is the public member. 

******************************************************************************/
void PhotoImage::FormatSystemTime( 
	CString & time_str,						// O - gets formatted date & time 
	SYSTEMTIME date_time,					// I - SYSTEMTIME of create time 
	INT32 adjust_seconds					// I - seconds to add/sub to create time, default is 0 
) const
{
	TCHAR formatted_date[ 128 ] ;
	TCHAR formatted_time[ 128 ] ;

	/*	
		Have we been asked to adjust_seconds the time by a sepcified number of seconds? 
		
		This feature is useful when we've created photos in another time zone or the 
		camera time has gotten out-of-date, etc. 

		A 32-bit second adjust_seconds lets us fudge the time over a range of about +/- 50 years
	*/
	date_time = AddSeconds( date_time, adjust_seconds ) ;

	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &date_time, _T( "dddd dd MMMM yyyy" ), formatted_date, sizeof( formatted_date ) / sizeof( formatted_date[ 0 ] ) ) )
	{
		if ( GetTimeFormat( LOCALE_USER_DEFAULT, 0, &date_time, _T( "hh:mm tt" ), formatted_time, sizeof( formatted_time ) / sizeof( formatted_time[ 0 ] ) ) )
		{
			_tcslwr_s( formatted_time, 1 + _tcslen( formatted_time ) ) ;
		}

		time_str.Format( _T( "%s at %s" ), formatted_date, formatted_time ) ;
	}
}






/******************************************************************************
PhotoImage::GetCreationSubCaption

	Format the "created" sub-caption. Typically consists of the 
	camera model used and the date and time, eg something like: 

	"Created with Nikon D300 on Sunday 21 December 2008 at 11:56 pm."

******************************************************************************/
CString PhotoImage::GetCreationSubCaption( ) const
{
	CString	work ;
	CString camera ;
	CString	created ;
		
	if ( m_override_data.camera.GetLength( ) )
		camera = m_override_data.camera ;
	else if ( m_extracted_data.camera.GetLength( ) )
		camera = m_extracted_data.camera ;

	if ( m_override_data.flag & SystemTimeValid ) 
	{
		FormatSystemTime( created, m_override_data.create_time ) ;
	}
	else if ( m_override_data.create_time_str.GetLength( ) )
	{
		created = m_override_data.create_time_str ;
	}
	else if ( m_override_data.create_adjust ) 
	{
		FormatSystemTime( created, m_extracted_data.create_time, m_override_data.create_adjust ) ;
	}
	else
	{
		FormatSystemTime( created, m_extracted_data.create_time ) ;
	}

	if ( camera.GetLength( ) )
		work.Format( _T( "Created %s, with %s" ), (const TCHAR *) created, (const TCHAR *) camera ) ;
	else
		work.Format( _T( "Created %s" ), (const TCHAR *) created ) ;

	if ( m_override_data.photog.GetLength( ) ) 
	{
		CString	byline ;

		byline.Format( _T( " by %s" ), (const TCHAR *) m_override_data.photog ) ;
		work += byline ;
	}

	return work ;
}




/******************************************************************************
PhotoImage::GetCreationSubCaptionOverrides

	Returns true if any component of the creation sub-caption has been changed
	by user overrides 

******************************************************************************/
bool PhotoImage::GetCreationSubCaptionOverrides( ) const
{
	return	0 != m_override_data.camera.GetLength( ) 
			||
			m_override_data.flag & SystemTimeValid
			||
			0 != m_override_data.create_time_str.GetLength( ) 
			||
			m_override_data.create_adjust 
			||
			0 != m_override_data.photog.GetLength( ) ;
}







/******************************************************************************
PhotoImage::GetExposureSubCaption

	Returns a formatted CString with the exposure, lens and ISO data for the 
	PhotoImage. 

	Eg, something like: 

	"1/320 f/8 using 24-70mm f/2.8 zoom at 70mm (ISO=400)" 

	If the ISO or lens data is unavailable, it is simply omitted. 

******************************************************************************/
CString PhotoImage::GetExposureSubCaption( ) const
{
	CString	work ;
	CString exposure ;
	CString lens ;
	CString iso ;

	if ( m_extracted_data.exposure.GetLength( ) ) 
		exposure = m_extracted_data.exposure ;

	if ( m_extracted_data.lens.GetLength( ) )
		lens =  m_extracted_data.lens ;

	if ( m_extracted_data.ISO.GetLength( ) )
		iso = m_extracted_data.ISO ;

	work = exposure ;
	
	if ( lens.GetLength( ) )
	{
		if ( work.GetLength( ) )
			work += _T( " using " ) ;

		work += lens ;
	}

	if ( iso.GetLength( ) )
	{
		if ( work.GetLength( ) )
			work += _T( " " ) ;

		work += _T( "(ISO=" ) ;
		work += iso ;
		work += _T( ")" ) ;
	}

	return work ;
}




/******************************************************************************
PhotoImage::GetExposureSubCaptionOverrides

	At present, there's no way to override the exposure data 

******************************************************************************/
bool PhotoImage::GetExposureSubCaptionOverrides( ) const
{
	return false ;
}






/******************************************************************************
CheckFileDateTime

	Static utility function, used by PhotoImage::CheckDiskForFileTimes( ) to 
	check the actual disk for the file's write time 	

******************************************************************************/
static bool CheckFileDateTime( 
	const TCHAR * fq_path,				// I - path of file on disk 
	FILETIME & write_time,				// O - gets the write time of the file (or default if no file found) 
	const FILETIME & default_time		// I - default time to return if file does not exist 
) 
{
	WIN32_FILE_ATTRIBUTE_DATA	file_data ;
	bool found ;

	if ( found = ( 0 != GetFileAttributesEx( fq_path, GetFileExInfoStandard, &file_data ) ) )
		write_time = file_data.ftLastWriteTime ;
	else
		write_time = default_time ;
	
	return found ;
}









/******************************************************************************
PhotoImage::CheckDiskForFileTimes

	We only want to generate jpegs for files where the source file has been
	modified later than the jpeg file. 

	Compare the modified time of the source file with the modified time of 
	each of the required jpegs. The set of required disk types depends on 
	the disk type, but we always initialize the mod_time member variables for
	all types to 'end_of_time', then update mod times only for those files
	we actually need. If a jpeg we need is NOT found, its mod time is set
	to 'beginning_of_time', which will force an update 

******************************************************************************/
void PhotoImage::CheckDiskForFileTimes( 
	CMakePhotoDisk3Doc * doc				// I - doc pointer. Used to get the disk type & path to the disk root 
)
{
	CString			dest_jpeg_path ;
	const FILETIME	end_of_time = { 0xFFFFFFFF, 0xFFFFFFFF } ;
	const FILETIME	beginning_of_time = { 0 } ;

	ASSERT( m_source_missing == 0 ) ;
	VERIFY( CheckFileDateTime( m_psd_file, m_source_mod_time, end_of_time ) ) ;

	if ( m_number_on_disk == -1 ) 
		m_thumbnail_mod_time = beginning_of_time ;
	else
	{
		// try to check the actual file on disk
		dest_jpeg_path.Format( _T( "%s\\pages\\thumbnail\\%d.jpg" ), doc->GetProjectPath( ), m_number_on_disk ) ;
		CheckFileDateTime( dest_jpeg_path, m_thumbnail_mod_time, beginning_of_time ) ;
	}



	/* 
		We now determine what jpegs we need to create (or update), based on (a) whether the user has selected 
		that particular jpeg type and (b) the last write time 

	*/ 
	m_minip_mod_time = end_of_time ;
	m_presentation_mod_time = end_of_time ;
	m_compressed_mod_time = end_of_time ;
	m_facebook_mod_time = end_of_time ;
	m_fullsize_mod_time = end_of_time ;

	if ( -1 == m_number_on_disk )
		m_minip_mod_time = end_of_time ;
	else
	{
		dest_jpeg_path.Format( _T( "%s\\images\\minip\\%d.jpg" ), doc->GetProjectPath( ), m_number_on_disk ) ;
		CheckFileDateTime( dest_jpeg_path, m_minip_mod_time, beginning_of_time ) ;
	}


	if ( doc->GetDoBigPresentation( ) )
	{
		if ( -1 == m_number_on_disk ) 
			m_presentation_mod_time = beginning_of_time ;
		else
		{
			dest_jpeg_path.Format( _T( "%s\\images\\presentation\\%d.jpg" ), doc->GetProjectPath( ), m_number_on_disk ) ;
			CheckFileDateTime( dest_jpeg_path, m_presentation_mod_time, beginning_of_time ) ;
		}
	}


	if ( doc->GetDoCompressed( ) )
	{
		if ( -1 == m_number_on_disk ) 
			m_compressed_mod_time = beginning_of_time ;
		else
		{
			dest_jpeg_path.Format( _T( "%s\\images\\compressed\\%d.jpg" ), doc->GetProjectPath( ), m_number_on_disk ) ;
			CheckFileDateTime( dest_jpeg_path, m_compressed_mod_time, beginning_of_time ) ;
		}
	}

	if ( doc->GetDoFacebook( ) )
	{
		if ( -1 == m_number_on_disk ) 
			m_facebook_mod_time = beginning_of_time ;
		else
		{
			dest_jpeg_path.Format( _T( "%s\\images\\facebook\\%d.jpg" ), doc->GetProjectPath( ), m_number_on_disk ) ;
			CheckFileDateTime( dest_jpeg_path, m_facebook_mod_time, beginning_of_time ) ;
		}
	}

	if ( doc->GetDoFullSize( ) )
	{
		if ( -1 == m_number_on_disk ) 
			m_fullsize_mod_time = beginning_of_time ;
		else
		{
			dest_jpeg_path.Format( _T( "%s\\images\\fullsize\\%d.jpg" ), doc->GetProjectPath( ), m_number_on_disk ) ;
			CheckFileDateTime( dest_jpeg_path, m_fullsize_mod_time, beginning_of_time ) ;
		}
	}
}




/******************************************************************************
PhotoImage::FormatAdjustTime

	class static 
	
	Formats a user-input 'adjust_seconds' value into a more readable format. 
	As cryptic as it might be, a string like "5d 3h 12m 30s" is something I 
	can grasp better than "443,550", which is the equivalent number of seconds 

******************************************************************************/
CString PhotoImage::FormatAdjustTime( 
	INT32 adjust_seconds					// I - number of seconds 
)
{
	int		secs ;
	int		mins ;
	int		hours ;
	int		days ;
	bool	negative ;
	CString	work ;

	if ( negative = adjust_seconds < 0 ) 
		adjust_seconds = -adjust_seconds ;

	secs = adjust_seconds % 60 ;
	
	if ( adjust_seconds /= 60 )
	{
		mins = adjust_seconds % 60 ;

		if ( adjust_seconds /= 60 )
		{
			hours = adjust_seconds % 24 ;

			if ( adjust_seconds /= 24 )
			{
				days = adjust_seconds ;
				ASSERT( days < 365 ) ;			// we are NOT going to support corrections over a year

				work.Format( _T( "%dd %dh %dm %ds" ), days, hours, mins, secs ) ;

			}
			else
			{
				work.Format( _T( "%dh %dm %ds" ), hours, mins, secs ) ;
			}
		}
		else
		{
			work.Format( _T( "%dm %ds" ), mins, secs ) ;
		}
	}
	else
	{
		work.Format( _T( "%ds" ), secs ) ;
	}

	if ( negative ) 
		work.Insert( 0, _T( "- " ) ) ;

	return work ;
}



/******************************************************************************
PhotoImage::ParseAdjustTimeString

	class static 

	Translates a time string with hours, minutes, seconds into a 32-bit integer 
	with the number of seconds. 

	Translates something like "5d 3h 12m 30s" to integer 443550 (the equivalent
	number of seconds) 

	Companion function to PhotoImage::FormatAdjustTime

******************************************************************************/
bool PhotoImage::ParseAdjustTimeString( 
	const TCHAR * str,				// I - time string 
	INT32 & adjust_secs				// O - gets seconds 
) 
{
	CString	adjust_str = str ;
	INT32	adjust_time = 0 ;
	bool	negative = false ;
	int		days ;
	int		hours ;
	int		mins ;
	int		secs ;
	int		day_index ;
	int		hour_index ;
	int		min_index ;
	int		sec_index ;
	int		unit_index ;

	days = hours = mins = secs = 0 ;
	day_index = hour_index = min_index = sec_index = -1 ;

	// we don't need no stinking whitespace
	adjust_str.Replace( _T( " " ), _T( "" ) ) ;

	if ( _T( '-' ) == adjust_str.GetAt( 0 ) )
	{
		negative = true ;
		adjust_str = adjust_str.Mid( 1 ) ;
	}

	while ( 0 < adjust_str.GetLength( ) ) 
	{
		if ( -1 != ( unit_index = adjust_str.FindOneOf( _T( "dhmsDHMS" ) ) ) )
		{
			switch ( adjust_str.GetAt( unit_index ) )
			{
				case _T( 'd' ) :
				case _T( 'D' ) :
					if ( -1 == day_index ) 
					{
						days = _ttoi( adjust_str ) ;
						adjust_str = adjust_str.Mid( unit_index + 1 ) ;
						day_index = unit_index ;

						if ( days >= 365 ) 
							return false ;
					}
					else
						return false ;
					break ;

				case _T( 'h' ) :
				case _T( 'H' ) :
					if ( -1 == hour_index ) 
					{
						hours = _ttoi( adjust_str ) ;
						adjust_str = adjust_str.Mid( unit_index + 1 ) ;
						hour_index = unit_index ;
					}
					else
						return false ;
					break ;

				case _T( 'm' ) :
				case _T( 'M' ) :
					if ( -1 == min_index ) 
					{
						mins = _ttoi( adjust_str ) ;
						adjust_str = adjust_str.Mid( unit_index + 1 ) ;
						min_index = unit_index ;
					}
					else
						return false ;
					break ;

				case _T( 's' ) :
				case _T( 'S' ) :
					if ( -1 == sec_index )
					{
						secs = _ttoi( adjust_str ) ;
						adjust_str = adjust_str.Mid( unit_index + 1 ) ;
						sec_index = unit_index ;
					}
					else
						return false ;
					break ;

				default :
					// hey - you need to add a case handler for this letter, or remove it for the string passed to CString::FindOneOf() !!!!
					ASSERT( FALSE ) ;
					return false ;
			}
		}
		else if ( 0 == _tcscmp( str, _T( "0" ) ) )
		{
			adjust_secs = 0 ;
			return true ;				// special case - allow "0" without units 
		}
		else
		{
			return false ;		// we have non-zero length, and possibly non-zero digits, but no valid units... 
		}
	}

	adjust_secs = secs + 60 * mins + 3600 * hours + (24 * 3600 ) * days ;
	if ( negative )
		adjust_secs = -adjust_secs ;

	return true ;
}




/******************************************************************************
PhotoImage::AddHTMLCoding

	class static

	Take multi-line edit contents and add appropriate HTML coding to get
	paragraph breaks. 

******************************************************************************/
void PhotoImage::AddHTMLCoding( 
	CString & text						// I/O - edit box text turned into HTML 
) 
{
	CString	work ;
	CString	encoding ;
	int		newline_index ;
	int		p_index ;

	work = text ;
	text = _T( "" ) ;

	// don't allow the user to manually add <p> and </p>
	while ( -1 != ( p_index = work.Find( _T( "<p>" ) ) ) )
		work = work.Left( p_index ) + work.Mid( p_index + 3 ) ;

	while ( -1 != ( p_index = work.Find( _T( "</p>" ) ) ) )
		work = work.Left( p_index ) + work.Mid( p_index + 4 ) ;

	// remove carriage returns 
	while ( -1 != ( newline_index = work.Find( _T( '\r' ) ) ) )
		work = work.Left( newline_index ) + work.Mid( newline_index + 1 ) ;


	if ( -1 != ( work.Find( _T( '\n' ) ) ) )
	{
		while ( -1 != ( newline_index = work.Find( _T( '\n' ) ) ) ) 
		{
			encoding.Format( _T( "<p>%s</p>" ), (const TCHAR *) work.Left( newline_index ) ) ;
			text += encoding ;
			work = work.Mid( newline_index + 1 ) ;
		}
		text += work ;
	}
	else
		text.Format( _T( "<p>%s</p>" ), (const TCHAR *) work ) ;	// want <p> and </p> enclosing comment even if no newlines are embedded 
}




/******************************************************************************
PhotoImage::RemoveHTMLCoding

	class static

	Translate HTML-encoded text into a string with \r\n inserted in place of 
	any </p>

	Complement of PhotoImage::AddHTMLCoding

******************************************************************************/
void PhotoImage::RemoveHTMLCoding( 
	CString & text					// I/O - Replace </p> with newline + carriage return 
) 
{
	CString	work ;
	CString	encoding ;
	int		endp_index ;
	int		p_index ;

	work = text ;
	text = _T( "" ) ;

	while ( -1 != ( p_index = work.Find( _T( "<p>" ) ) ) )
	{
		VERIFY( -1 != ( endp_index = work.Find( _T( "</p>" ) ) ) ) ;
		ASSERT( endp_index > p_index ) ; 
		ASSERT( -1 == work.Find( _T( "<p>" ), p_index + 3 ) || endp_index < work.Find( _T( "<p>" ), p_index + 3 ) ) ;

		encoding.Format( _T( "%s\r\n" ), (const TCHAR *) work.Mid( p_index + 3, endp_index - p_index - 3 ) ) ;
		text += encoding ;
		work = work.Mid( endp_index + 4 ) ;
	}
	text += work ;
}




/******************************************************************************
PhotoImage::???RefreshNeeded accessors - 

	return true if the corresponding image-type needs to be refreshed. 

******************************************************************************/
bool PhotoImage::PresentationRefreshNeeded( CMakePhotoDisk3Doc * doc ) 
{
	ASSERT( m_valid_file_times ) ;

	if ( doc->GetDoBigPresentation( ) )
		return CompareFileTime( &m_override_data_timestamp, &m_presentation_mod_time ) > 0 || CompareFileTime( &m_source_mod_time, &m_presentation_mod_time ) > 0 ;
	else
		return false ;
} 


bool PhotoImage::MiniPresentationRefreshNeeded( CMakePhotoDisk3Doc * doc ) 
{
	ASSERT( m_valid_file_times ) ;

	return CompareFileTime( &m_override_data_timestamp, &m_minip_mod_time ) > 0 || CompareFileTime( &m_source_mod_time, &m_minip_mod_time ) > 0 ;
} 


bool PhotoImage::CompressedRefreshNeeded( CMakePhotoDisk3Doc * doc ) 
{
	ASSERT( m_valid_file_times ) ;

	if ( doc->GetDoCompressed( ) )
		return CompareFileTime( &m_override_data_timestamp, &m_compressed_mod_time ) > 0 || CompareFileTime( &m_source_mod_time, &m_compressed_mod_time ) > 0 ;
	else
		return false ;
} 


bool PhotoImage::FacebookRefreshNeeded( CMakePhotoDisk3Doc * doc ) 
{
	ASSERT( m_valid_file_times ) ;

	if ( doc->GetDoFacebook( ) )
		return CompareFileTime( &m_override_data_timestamp, &m_facebook_mod_time ) > 0 || CompareFileTime( &m_source_mod_time, &m_facebook_mod_time ) > 0 ;
	else
		return false ;
} 

bool PhotoImage::FullsizeRefreshNeeded( CMakePhotoDisk3Doc * doc ) 
{
	ASSERT( m_valid_file_times ) ;

	if ( doc->GetDoFullSize( ) )
		return CompareFileTime( &m_override_data_timestamp, &m_fullsize_mod_time ) > 0 || CompareFileTime( &m_source_mod_time, &m_fullsize_mod_time ) > 0 ;
	else
		return false ;
}


bool PhotoImage::ThumbnailRefreshNeeded( CMakePhotoDisk3Doc * doc ) 
{
	ASSERT( m_valid_file_times ) ;

	return CompareFileTime( &m_override_data_timestamp, &m_thumbnail_mod_time ) > 0 || CompareFileTime( &m_source_mod_time, &m_thumbnail_mod_time ) > 0 ;
}

