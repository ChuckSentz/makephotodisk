/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




Collections, PhotoImages and any other structures and classes which make up the stuff
of a document

***************************************************************************************/

#pragma once

class CMakePhotoDisk3Doc ;


class Collection
{
	int										m_id_local ;
	bool									m_collection_set ;
	CString									m_name ;
	CTypedPtrList< CPtrList, Collection *>	m_child_q ;

protected:
	Collection( ) ;

public:
	Collection( int id_local, bool collection_set, const TCHAR * name ) ;
	~Collection( ) ;

	inline UINT32 GetIDLocal( ) const
		{
			return static_cast< UINT32 >( m_id_local ) ;
		} ;

	inline const TCHAR * GetName( ) const
		{
			return m_name ;
		} ;

	inline bool GetIsCollectionSet( ) const
		{
			return m_collection_set ;
		} ;

	inline CTypedPtrList< CPtrList, Collection *> & GetChildQueue( ) 
		{
			return m_child_q ;
		} ;

	void AddChild( Collection * child_collection ) ;
	Collection * GetFirstChild( POSITION & pos ) const ;
	Collection * GetNextChild( POSITION & pos ) const ;
} ;





class PhotoImage
{
	friend class CMakePhotoDisk3Doc ;

	CString		m_preview_jpeg ;			// fully qualified path to the preview jpeg
	CString		m_psd_file ;				// fully qualified path to the source file (usually a psd file) 
	CString		m_name ;
	int			m_pick ;					// db pick status: -1 = rejected, 0 = unflagged, 1 = pick 
	CString		m_file_format ;				// file format extracted from the db 
	int			m_number_on_disk ;			// the pic number of this photo on disk 
	int			m_number_in_collection ;	// the pic number of this photo in the collection (same as it's 1-based position in the in-memory list)

	struct 
	{
		CString		title ;				// title given to the pic 
		CString		description ;		
		CString		camera ;			// camera, eg, "Nikon D300" 
		CString		lens ;				// lens string, eg, "28-70mm f/2.8"
		CString		exposure ;			// exposure, eg, "1/250s at f/5.6" 
		CString		ISO ;				 // ISO, eg, "800" 
		int			orientation ;		// 0=not rotated, 90=vertical, right-rotated, -90=vertical, left-rotated
		int			height ;
		int			width ;
		SYSTEMTIME	create_time ;		// time/date the image was snapped
	} 
	m_extracted_data ;

	FILETIME	m_extracted_data_timestamp ;

	enum OverrideFlags
	{
		SystemTimeValid		=	0x00000001,
	} ;

	struct
	{
		UINT32		flag ;
		CString		title ;
		CString		description ;
		CString		camera ;
		CString		photog ;
		CString		create_time_str ;	// with some images - the best guess on create time may be something like "Summer, 1968" 
		SYSTEMTIME	create_time ;		// precise create time, if known
		INT32		create_adjust ;		// +/- seconds to adjust the extracted create time
		CString		m_extended_comment ;
	}
	m_override_data ;

	FILETIME	m_override_data_timestamp ;

	bool			m_valid_file_times ;
	int				m_source_missing ;		// -1 = unknown status, 0 = not mising, 1 = missing 
	bool			m_dirty_edits ;			// user has edited the overrides 
	bool			m_selected ;

	/*
	*	if ( m_valid_file_times) is true, the following store the file modified 
	*	times for the corresponding files. If m_valid_file_times is true and 
	*	a FILETIME is all-zero, this indicates the file does not exist. 
	*
	*/ 
	FILETIME		m_source_mod_time ;		
	FILETIME		m_presentation_mod_time ;
	FILETIME		m_compressed_mod_time ;
	FILETIME		m_facebook_mod_time ;
	FILETIME		m_fullsize_mod_time ;
	FILETIME		m_thumbnail_mod_time ;
	FILETIME		m_minip_mod_time ;

	// width and height of the presentation versions of this image 
	int m_fullp_width ;
	int m_minip_width ;
	int m_fullp_height ;
	int m_minip_height ;

	void FormatSystemTime( CString & time_str, SYSTEMTIME date_time, INT32 adjust_seconds = 0 ) const ; 


	/* 
		because of the occasional HTML-encoded characters, these functions, which
		give direct access to the raw strings, should only be public if and when
		they're really essential. 

		The functions which return strings for an image should be sophisticated about
		translating HTML-encodings to the appropriate characters. Eg, &amp; should
		become just '&' 
	*/
	inline const TCHAR * GetTitle( ) const 
	{
		return m_extracted_data.title ;
	} ;

	inline const TCHAR * GetDescription( ) const
	{
		return m_extracted_data.description ;
	} ;

	inline const TCHAR * GetCamera( ) const 
	{
		return m_extracted_data.camera ;
	} ;

	inline const TCHAR * GetLens( ) const
	{
		return m_extracted_data.lens ;
	} ;

	inline const TCHAR * GetExposure( ) const
	{
		return m_extracted_data.exposure ;
	} ;

	inline const TCHAR * GetISO( ) const
	{
		return m_extracted_data.ISO ;
	} ;

	/*
	*
	* function used by SourceIsOutdated() to actually check file dates. It needs a doc 
	* pointer b/c we need to know where to look for the jpegs and also which jpegs
	* to be concerned with 
	*
	*/
	void CheckDiskForFileTimes( CMakePhotoDisk3Doc * doc ) ;

	inline void SetOnDiskNumber( int ondisk_no ) 
	{
		m_number_on_disk = ondisk_no ;
	} ;

public:
	PhotoImage(	const TCHAR * name, 
			const TCHAR * psd_path, 
			const TCHAR * jpeg_file, 
			const TCHAR * title, 
			const TCHAR * description, 
			const TCHAR * camera,
			const TCHAR * lens,
			const TCHAR * exposure,
			const TCHAR * iso,
			int			  pic_no, 
			int			  orientation,
			int			  pixel_width,
			int			  pixel_height, 
			const SYSTEMTIME &	create_time, 
			int			  pick,
			const TCHAR * file_format
			) ;

	PhotoImage( ) ;

	PhotoImage & operator=( const PhotoImage & other ) ;

	~PhotoImage( ) ;

	static bool ParseAdjustTimeString( const TCHAR * str, INT32 & adjust_secs ) ;
	static CString FormatAdjustTime( INT32 adjust_seconds ) ;
	static void AddHTMLCoding( CString & text ) ;
	static void RemoveHTMLCoding( CString & text ) ;

	inline const TCHAR * GetPreviewJpeg( ) const
	{
		return m_preview_jpeg ;
	} ;

	inline const TCHAR * GetName( ) const
	{
		return m_name ;
	} ;

	inline int GetNumberOnDisk( ) const 
	{
		return m_number_on_disk ;
	} ;

	inline int GetNumberInCollection( ) const
	{
		return m_number_in_collection ;
	} ;

	inline bool GetExtendedComment( CString & comment ) const
	{
		comment = m_override_data.m_extended_comment ;
		return !comment.IsEmpty( ) ;
	} ;

	inline int GetOrientation( ) const 
	{
		return m_extracted_data.orientation ;
	} ;

	inline int GetHeight( ) const
	{
		if ( GetOrientation( ) % 180 )
			return m_extracted_data.width ;
		else
			return m_extracted_data.height ;
	} ;

	inline int GetWidth( ) const
	{
		if ( GetOrientation( ) % 180 )
			return m_extracted_data.height ;
		else
			return m_extracted_data.width ;
	} ;

	inline int GetPresentationWidth( ) const 
	{
		return m_fullp_width ;
	} ;

	inline void SetPresentationWidth( int width ) 
	{
		m_fullp_width = width ;
	} ;

	inline int GetMiniPresentationWidth( ) const 
	{
		return m_minip_width ;
	} ;

	inline void SetMiniPresentationWidth( int width ) 
	{
		m_minip_width = width ;
	} ;

	inline int GetPresentationHeight( ) const 
	{
		return m_fullp_height ;
	} ;

	inline void SetPresentationHeight( int height ) 
	{
		m_fullp_height = height ;
	} ;

	inline int GetMiniPresentationHeight( ) const
	{
		return m_minip_height ;
	} ;

	inline void SetMiniPresentationHeight( int height )
	{
		m_minip_height = height ;
	} ;

	SYSTEMTIME GetCreateTime( ) const
	{
		if ( m_override_data.flag & PhotoImage::SystemTimeValid ) 
			return m_override_data.create_time ;
		else if ( m_override_data.create_adjust )
			return AddSeconds( m_extracted_data.create_time, m_override_data.create_adjust ) ;
		else
			return m_extracted_data.create_time ;
	} ;

	// class static 
	static SYSTEMTIME AddSeconds( const SYSTEMTIME & date_time, INT32 adjust_seconds ) ;

	// <0 if our time is less than other time, 0 if equal, >0 if our time is greater 
	int CompareCreateTime( const PhotoImage & other ) const
	{
		return CompareCreateTime( other.GetCreateTime( ) ) ;
	} ;

	// < 0 if our time is less than test time, 0 if equal, > 0 if our time is greater than test time 
	int CompareCreateTime( const SYSTEMTIME &test_time ) const 
	{
		FILETIME	our_time ;
		FILETIME	other_time ;

		SystemTimeToFileTime( &GetCreateTime( ), &our_time ) ;
		SystemTimeToFileTime( &test_time, &other_time ) ;

		return CompareFileTime( &our_time, &other_time ) ;
	} ;

	inline bool IsRejected( ) const 
	{
		return -1 == m_pick ;
	} ;

	inline bool IsPick( ) const
	{
		return 1 == m_pick ;
	} ;

	inline bool IsUnflagged( ) const
	{
		return 0 == m_pick ;
	} ;

	inline const TCHAR * GetFileFormat( ) const 
	{
		return m_file_format ;
	} ;

	inline bool UnsupportedFileFormat( ) const
	{
		return 0 == m_file_format.CompareNoCase( _T( "RAW" ) ) ;
	} ;

	inline bool HasEdits( ) const
	{
		return GetFormattedCaptionOverrides( ) || GetCreationSubCaptionOverrides( ) || GetExposureSubCaptionOverrides( ) || m_override_data.m_extended_comment.GetLength( ) ;
	} ;

	inline bool HasDirtyEdits( ) const
	{
		return m_dirty_edits ;
	} ;

	inline bool HasValidFileTimes( ) const
	{
		return m_valid_file_times ;
	} ;

	void ResetPhotoTimes( ) 
	{
		m_valid_file_times = false ;
	} ;
	
	bool JpegRefreshNeeded( CMakePhotoDisk3Doc * doc ) // NOT const !
	{
		// should never call this without first verifying that the source file at least exists...
		ASSERT( m_source_missing == 0 ) ;

		if ( !m_valid_file_times ) 
		{
			CheckDiskForFileTimes( doc ) ;
			m_valid_file_times = true ;
		}

		// need refresh if override metadata is more recent than jpeg's, too - 
		if ( CompareFileTime( &m_override_data_timestamp, &m_presentation_mod_time ) > 0 
				|| CompareFileTime( &m_override_data_timestamp, &m_minip_mod_time ) > 0 
				|| CompareFileTime( &m_override_data_timestamp, &m_compressed_mod_time ) > 0 
				|| CompareFileTime( &m_override_data_timestamp, &m_facebook_mod_time ) > 0 
				|| CompareFileTime( &m_override_data_timestamp, &m_fullsize_mod_time ) > 0 
				|| CompareFileTime( &m_override_data_timestamp, &m_thumbnail_mod_time ) > 0 )
				return true ;

		// CheckDiskForFileTimes will set the FILETIME for unneded files to all 1's (a time about 58,000 years hence)  
		return ( CompareFileTime( &m_source_mod_time, &m_presentation_mod_time ) > 0 
				|| CompareFileTime( &m_source_mod_time, &m_minip_mod_time ) > 0 
				|| CompareFileTime( &m_source_mod_time, &m_compressed_mod_time ) > 0 
				|| CompareFileTime( &m_source_mod_time, &m_facebook_mod_time ) > 0 
				|| CompareFileTime( &m_source_mod_time, &m_fullsize_mod_time ) > 0 
				|| CompareFileTime( &m_source_mod_time, &m_thumbnail_mod_time ) > 0 ) ;
	} ;

	bool JpegRenumberNeeded( ) const
	{
		return m_number_on_disk != m_number_in_collection ;
	} ;

	bool PresentationRefreshNeeded( CMakePhotoDisk3Doc * doc ) ;
	bool MiniPresentationRefreshNeeded( CMakePhotoDisk3Doc * doc ) ;
	bool CompressedRefreshNeeded( CMakePhotoDisk3Doc * doc ) ;
	bool FacebookRefreshNeeded( CMakePhotoDisk3Doc * doc ) ;
	bool FullsizeRefreshNeeded( CMakePhotoDisk3Doc * doc ) ;
	bool ThumbnailRefreshNeeded( CMakePhotoDisk3Doc * doc ) ;

	bool SourceIsMissing( )
	{
		if ( -1 == m_source_missing ) 
		{
			if ( INVALID_FILE_ATTRIBUTES == GetFileAttributes( m_psd_file ) )
				m_source_missing = 1 ;
			else
				m_source_missing = 0 ;
		}

		return m_source_missing == 1 ;
	}

	inline void SetSelectedFlag( bool selected = true ) 
	{
		m_selected = selected ;
	} ;

	inline bool GetSelectedFlag( ) const 
	{
		return m_selected ;
	} ;

	CString GetFormattedCaption( ) const ;
	CString GetCreationSubCaption( ) const ;
	CString GetExposureSubCaption( ) const ;

	bool GetFormattedCaptionOverrides( ) const ;
	bool GetCreationSubCaptionOverrides( ) const ;
	bool GetExposureSubCaptionOverrides( ) const ;

	bool GetExtractedTitle( CString & extracted_title ) const ;
	bool GetOverrideTitle( CString & override_title ) const ;
	void SetOverrideTitle( const TCHAR * override_title ) ;
	bool GetExtractedDescr( CString & extracted_descr ) const ;
	bool GetOverrideDescr( CString & override_descr ) const ;
	void SetOverridedDescr( const TCHAR * override_descr ) ;
	bool GetExtractedCreateTime( SYSTEMTIME & create_time ) const ;
	bool GetOverrideCreateTime( SYSTEMTIME & override_time ) const ;
	void SetOverrideCreateTime( const SYSTEMTIME & override_time ) ;
	bool GetCreateTimeAdjust( INT32 & adjust_seconds ) const ;
	void SetCreateTimeAdjust( const INT32 & adjust_seconds ) ;
	FILETIME GetOverrideTimestamp( ) const ;
	void SetOverrideTimestamp( const FILETIME & timestamp ) ;
	bool GetExtractedCameraName( CString & camera_name ) const ;
	bool GetOverrideCameraName( CString & camera_name ) const ;
	void SetOverrideCameraName( const TCHAR * camera_name ) ;
	bool GetOverridePhotographer( CString & photog ) const ;
	void SetOverridePhotographer( const TCHAR * photog_name ) ;
	void SetExtendedComment( const TCHAR * comment ) ;

	inline const TCHAR * GetSourceFilePath( ) const 
	{
		return m_psd_file ;
	} ;

	inline void SetHasDirtyEdits( bool dirty = true )
	{
		m_dirty_edits = dirty ;

		if ( dirty ) 
			GetSystemTimeAsFileTime( &m_override_data_timestamp ) ;		// file times are always UTC, not local 
	} ;
} ;



inline bool PhotoImage::GetExtractedTitle( CString & extracted_title ) const
{
	extracted_title = m_extracted_data.title ;
	return !extracted_title.IsEmpty( ) ;
}


inline bool PhotoImage::GetOverrideTitle( CString & override_title ) const
{
	override_title = m_override_data.title ;
	return !override_title.IsEmpty( ) ;
}

inline void PhotoImage::SetOverrideTitle( const TCHAR * override_title )
{
	if ( override_title ) 
		m_override_data.title = override_title ;
	else
		m_override_data.title.Empty( ) ;
}


inline bool PhotoImage::GetExtractedDescr( CString & extracted_descr ) const
{
	extracted_descr = m_extracted_data.description ;
	return !m_extracted_data.description.IsEmpty( ) ;
}


inline bool PhotoImage::GetOverrideDescr( CString & override_descr ) const
{
	override_descr = m_override_data.description ;
	return !m_override_data.description.IsEmpty( ) ;
}

inline void PhotoImage::SetOverridedDescr( const TCHAR * override_descr )
{
	if ( override_descr ) 
		m_override_data.description = override_descr ;
	else
		m_override_data.description.Empty( ) ;
}


inline bool PhotoImage::GetExtractedCreateTime( SYSTEMTIME & create_time ) const
{
	create_time = m_extracted_data.create_time ;
	return m_extracted_data.create_time.wYear != 0 ;
}

inline bool PhotoImage::GetOverrideCreateTime( SYSTEMTIME & override_time ) const
{
	if ( m_override_data.flag & PhotoImage::SystemTimeValid ) 
	{
		override_time = m_override_data.create_time ;
		return true ;
	}
	else
		return false ;
}


inline void PhotoImage::SetOverrideCreateTime( const SYSTEMTIME & override_time )
{
	// set SystemTimeValid flag for override create time only if it's nonzero and it's different than the extracted create time 
	if ( 
			override_time.wYear != 0 
			&&
			(
				override_time.wYear	!= m_extracted_data.create_time.wYear 
				||
				override_time.wMonth != m_extracted_data.create_time.wMonth
				||
				override_time.wDay != m_extracted_data.create_time.wDay
				||
				override_time.wHour != m_extracted_data.create_time.wHour		
				||
				override_time.wMinute != m_extracted_data.create_time.wMinute	
				||
				override_time.wSecond != m_extracted_data.create_time.wSecond	
			)
		)
	{
		m_override_data.create_time = override_time ;
		m_override_data.flag |= PhotoImage::SystemTimeValid ;
	}
	else
	{
		m_override_data.flag &= ~PhotoImage::SystemTimeValid ;
		ZeroMemory( &m_override_data.create_time, sizeof( m_override_data.create_time ) ) ;
	}
}



inline bool PhotoImage::GetCreateTimeAdjust( INT32 & adjust_seconds ) const
{
	adjust_seconds = m_override_data.create_adjust ;
	return 0 != adjust_seconds ;
}



inline void PhotoImage::SetCreateTimeAdjust( const INT32 & adjust_seconds )
{
	m_override_data.create_adjust = adjust_seconds ;
}


inline FILETIME PhotoImage::GetOverrideTimestamp( ) const
{
	return m_override_data_timestamp ;
}

inline void PhotoImage::SetOverrideTimestamp( const FILETIME & timestamp ) 
{
	m_override_data_timestamp = timestamp ;
}


inline bool PhotoImage::GetExtractedCameraName( CString & camera_name ) const
{
	camera_name = m_override_data.camera ;
	return !camera_name.IsEmpty( ) ;
}

inline bool PhotoImage::GetOverrideCameraName( CString & camera_name ) const 
{
	camera_name = m_override_data.camera ;
	return !camera_name.IsEmpty( ) ;
}

inline void PhotoImage::SetOverrideCameraName( const TCHAR * camera_name )
{
	if ( camera_name ) 
		m_override_data.camera = camera_name ;
	else
		m_override_data.camera.Empty( ) ;
}


inline bool PhotoImage::GetOverridePhotographer( CString & photog ) const
{
	photog = m_override_data.photog ;
	return !photog.IsEmpty( ) ;
}



inline void PhotoImage::SetOverridePhotographer( const TCHAR * photog_name )
{
	if ( photog_name ) 
		m_override_data.photog = photog_name ;
	else
		m_override_data.photog.Empty( ) ;
}


inline void PhotoImage::SetExtendedComment( const TCHAR * comment )
{
	m_override_data.m_extended_comment = comment ;
}

