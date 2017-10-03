/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



The document object, which represents a disk project

***************************************************************************************/

#pragma once

#include "DocRecords.h"


class SQLCursor ;
class SQLDatabase ;


class CRange : public CObject
{
public:
	int	first ;
	int	last ;

public:
	CRange( ) 
	{
		first = last = 0 ;
	} ;
} ;




class CMakePhotoDisk3Doc : public CDocument
{
public:
	enum UpdateMessages
	{
		CatalogSelected	= 1, 
		CatalogDeselected,
		CollectionSelected,
		CollectionDeselected,
		ImageSelected,
		ImageDeselected,
		MetadataUpdated,					// user has updated the metadata for a photo 
		JpegsRefreshed,						// just finished refreshing the jpeg's in the project directory 
		ReorderImages,						// images in a specified range need to be re-ordered (principially due to changing effective creation time in a collection ordered by creation time 
	} ;


	enum DiskTypes
	{
		UndefinedDiskType = 0,			
		FamilyAndFriendsDisk = 1,		// defaults to presentation, fullsize, facebook, compressed...	with a generous release
		AssignmentProduct,				// defaults to presentation, fullsize, compressed...			with a tailored release (per terms of contract) 
		ClientDisk,						// defaults to small presentation, facebook, compressed...		with a limited release w/ instructions on printing 
		SamplerDisk,					// small presentation...										with an explict non-release + contact info...	
	} ;

	enum DocState
	{
		Virgin = 0,						// virginal - light table should be blank 
		CatalogCollectionKnown,			// have a collection selected, light table should show slides 
		CatalogCollectionProjectKnown	// have a collection and a project directory - able to do updates, and display out-of-date files, etc. 
	} ;

protected: // create from serialization only
	CMakePhotoDisk3Doc();

	CString		m_catalog_path ;		// fully qualified path for the lrcat file
	CString		m_project_location ;	// "location" part of path - the containing directory 
	CString		m_project_directory ;	// "directory" part of the project path - the newly created root directory for the disk project
	CString		m_project_path ;		// location + directory - the fully qualified pathname of the project root dir. 
	DiskTypes	m_disk_type ;			// selected disk type. This 

	bool		m_do_big_presentation ;
	bool		m_do_compressed ;
	bool		m_do_facebook ;
	bool		m_do_full_size ;

	bool		m_watermark_presentation ;
	bool		m_watermark_compressed ;
	bool		m_watermark_facebook ;
	bool		m_watermark_full_size ;

	DocState	m_doc_state ;
	CString		m_disk_title ;
	UINT64		m_disk_size ;
	UINT32		m_major_revision ;
	UINT32		m_minor_revision ;
	CString		m_key_image_source ;
	CString		m_disk_icon_source ;	
	bool		m_custom_metadata_edits ;
	CString		m_app_path ;

	CTypedPtrList< CPtrList, Collection *>		m_collection_list ;
	const Collection *							m_sel_collection ;
	CTypedPtrList< CPtrList, PhotoImage *>		m_collection_pic_list ;
	int											m_last_selection ;
	int											m_anchor_selection ;
	int											m_selection_count ;
	bool										m_sort_by_capture_time ;
	bool										m_ascending_sort ;

	void DeletePhotoList( ) ;
	bool ExtractPreviewJpeg( CString & fq_preview_path, const TCHAR * lrdata_path, const TCHAR * guid ) ;
	bool ExtractNthJpeg( CString & fq_preview_path, const TCHAR * guid, int n, UINT32 addr, CFile & lrprev_file ) ;
	bool ExtractXMPData( CString & xmp_data, CString & title, CString & description, CString & camera, CString & lens, CString & exposure_time, CString & fstop, CString & focal_length, CString & iso, int & orientation, SYSTEMTIME & create_time ) ;
	bool ExtractDelimitedString( CString & result, const TCHAR * left_delim, const TCHAR * right_delim, const TCHAR * source ) ;
	void EvaluateStringFraction( CString & str, int max_decimal_places, int min_decimal_places ) ;
	bool PreloadDocBuffer( CStdioFile & doc_file, CString & doc_buffer ) ;
	bool LoadDiskType( CString & doc_buffer ) ;
	bool LoadParameter( CString & buffer, const TCHAR * tag, CString & value ) ;
	bool LoadParameter( CString & buffer, const TCHAR * tag, int & value ) ;
	bool LoadDuple( CString & buffer, const TCHAR * tag, int duple_no, CString & value1, int & value2 ) ;
	void SaveParameter( CStdioFile & doc_file, const TCHAR * param_id, const TCHAR * str_param ) ;
	void SaveParameter( CStdioFile & doc_file, const TCHAR * param_id, UINT32 value ) ;
	void SaveParameter( CStdioFile & doc_file, const TCHAR * param_id, int value ) ;
	void SaveDiskType( CStdioFile & doc_file ) ;
	void SaveDuple( CStdioFile & doc_file, const TCHAR * param_id, const TCHAR * value1, const TCHAR * value2 ) ;
	void SaveDuple( CStdioFile & doc_file, const TCHAR * param_id, const TCHAR * str_arg, int numeric_arg ) ;
	bool OpenCustomEditsDBAndGuaranteeTables( SQLDatabase & db ) ;

	void RenumberPhotoList( ) ;

public:
	bool GetSortByCaptureTime( ) const ;
	bool GetAscendingSort( ) const ;

	inline void AddPhotoToList( PhotoImage * photo ) 
	{
		m_collection_pic_list.AddTail( photo ) ;
	} ;
	bool CopyDirectoryTree( const TCHAR * source_dir, const TCHAR * dest_dir ) ;
	void RecurseCalcDiskUse( const TCHAR * path ) ;
	void DeleteOrphanJpeg( const TCHAR * file_name ) ;
	void DeleteOrphanedHTML( ) ;
	void DeleteUnwantedJpegOutput( ) ;
	void FixMisalignedOutputNumbers( ) ;
	void ChainRenumber( int start_of_chain, int cur_actual, int cur_ideal, PhotoImage * cur_image ) ;

	void CheckSaveCustomMetadata( ) ;
	void LoadCustomMetadata( ) ;
	void SetOverrides( PhotoImage * photo, SQLCursor & sql ) ;
	void SelectRange( int sel1, int sel2 ) ;

// Attributes
public:
	int GetCollectionListSize( ) const ;
	Collection * GetFirstCollection( POSITION & pos ) const ;
	CTypedPtrList< CPtrList, Collection *> & GetCollectionList( ) ;

	Collection * FindCollectionByID( int collection_id ) ;
	Collection * RecursiveCollectionSearch( CTypedPtrList< CPtrList, Collection *> & collection_list, int collection_id ) ;

	Collection * GetNextCollection( POSITION & pos ) const ;
	PhotoImage * GetFirstPhoto( POSITION & pos ) const ;
	PhotoImage * GetNextPhoto( POSITION & pos ) const ;
	PhotoImage * GetFirstSelectedPhoto( POSITION & pos ) const ;
	PhotoImage * GetNextSelectedPhoto( POSITION & pos ) const ;
	PhotoImage * FindPhoto( int index,  POSITION & pos ) const ;
	PhotoImage * FindPhotoByFullyQualifiedPathname( const TCHAR * name ) const ;
	PhotoImage * FindPhotoByFilename( const TCHAR * name ) const ;
	PhotoImage * FindPhotoByOutputNumber( int output_number ) const ;
	int DetermineOutputNumberByListPosition( const PhotoImage * photo_image ) const ;

	void RenameOutputJpeg( int current_number, int new_number ) const ;

	int GetLastSelectedImageNo( ) const ;
	PhotoImage * GetLastSelectedImage( ) const ;
	void SetLastSelectedImageNo( int number, bool ctrl_select, bool shift_select ) ;
	int GetAnchorSelectionNo( ) const ;
	int GetSelectionCount( ) const ;
	void ResetSelectFlags( ) ;

	void SetDiskType( DiskTypes disk_type ) ;
	void SetDoBigPresentation( bool value ) ;
	void SetDoCompressed( bool value ) ;
	void SetDoFacebook( bool value ) ;
	void SetDoFullSize( bool value ) ;
	void SetWatermarkPresentation( bool value ) ;
	void SetWatermarkCompressed( bool value ) ;
	void SetWatermarkFacebook( bool value ) ;
	void SetWaterarkFullsize( bool value ) ;

	int GetPhotoListSize( ) const ;
	const Collection * GetSelectedCollection( ) const ;
	const TCHAR * GetCatalog( ) const ;
	DiskTypes GetBasicDiskType( ) const ;
	bool GetDoBigPresentation( ) const ;
	bool GetDoCompressed( ) const ;
	bool GetDoFacebook( ) const ;
	bool GetDoFullSize( ) const ;
	bool GetWatermarkPresentation( ) const ;
	bool GetWatermarkCompressed( ) const ;
	bool GetWatermarkFacebook( ) const ;
	bool GetWaterarkFullsize( ) const ;

	const TCHAR * GetDiskTitle( ) const ;
	void SetDiskTitle( const TCHAR * disk_title ) ;
	void SetLocation( const TCHAR * location ) ;
	void SetDirectory( const TCHAR * directory ) ;
	const TCHAR * GetProjectPath( ) const ;
	const TCHAR * GetProjectLocation( ) const ;
	const TCHAR * GetProjectDirectory( ) const ;
	DocState GetDocState( ) const ;
	void SetDocState( DocState new_doc_state ) ;
	UINT64 GetDiskUsage( ) ;
	const TCHAR * GetKeyImageSource( ) const ;
	const TCHAR * GetDiskIconSource( ) const ;
	UINT32 GetMajorRevision( ) const ;
	UINT32 GetMinorRevision( ) const ;
	void SetKeyImageSource( const TCHAR * source_path ) ;
	void SetDiskIconSource( const TCHAR * source_path ) ;
	void SetRevision( UINT32 major, UINT32 minor ) ;
	bool ChangedCreateTime( PhotoImage * altered_image, int change, bool update_views = true ) ;
	void ReOrderPhotosByCaptureTime( ) ;

// Operations
public:
	bool SetCatalog( const TCHAR * catalog ) ;
	void SetProjectLocation( const TCHAR * location ) ;
	void SetProjectDirectory( const TCHAR * directory ) ;

	bool SelectCollection( const Collection * collection ) ;
	bool DeselectCollection( ) ;

	bool CreateProject( ) ;
	bool SaveDocument( ) ;
	bool LoadDocument( const TCHAR * fq_path ) ;

	void RealignOrAssignOutputNumbers( ) ;
	void TallyUpJpegRefreshCount( int & refresh_jpeg_ct ) ;
	void ResetPhotoTimes( ) ;
	void CalculateDiskUsage( ) ;
	bool AnyRefreshNeeded( ) ;
	bool AnySourceUnsupported( ) ;
	bool AnySourceMissing( ) ;
	inline void SetCustomMetadataEdits( ) 
	{
		m_custom_metadata_edits = true ;
	} ;

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMakePhotoDisk3Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

	DECLARE_DYNCREATE(CMakePhotoDisk3Doc)

public:
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
};







inline int CMakePhotoDisk3Doc::GetCollectionListSize( ) const 
{
	return m_collection_list.GetSize( ) ;
}


inline Collection * CMakePhotoDisk3Doc::GetFirstCollection( POSITION & pos ) const
{
	if ( pos = m_collection_list.GetHeadPosition( ) )
		return GetNextCollection( pos ) ;
	else
		return NULL ;
}


inline CTypedPtrList< CPtrList, Collection *> & CMakePhotoDisk3Doc::GetCollectionList( ) 
{
	return m_collection_list ;
}
	


inline Collection * CMakePhotoDisk3Doc::GetNextCollection( POSITION & pos ) const
{
	if ( pos != NULL ) 
		return m_collection_list.GetNext( pos ) ;
	else
		return NULL ;
}


inline PhotoImage * CMakePhotoDisk3Doc::GetFirstPhoto( POSITION & pos ) const
{
	if ( pos = m_collection_pic_list.GetHeadPosition( ) ) 
		return GetNextPhoto( pos ) ;
	else
		return NULL ;
}


inline PhotoImage * CMakePhotoDisk3Doc::GetNextPhoto( POSITION & pos ) const 
{
	if ( pos != NULL )
		return m_collection_pic_list.GetNext( pos ) ;
	else
		return NULL ;
}



inline PhotoImage * CMakePhotoDisk3Doc::GetFirstSelectedPhoto( POSITION & pos ) const
{
	PhotoImage * image ;

	if ( image = GetFirstPhoto( pos ) )
		while ( image && !image->GetSelectedFlag( ) ) 
			image = GetNextPhoto( pos ) ;

	return image ;
}



inline PhotoImage * CMakePhotoDisk3Doc::GetNextSelectedPhoto( POSITION & pos ) const
{
	PhotoImage * image ;

	if ( image = GetNextPhoto( pos ) )
		while ( image && !image->GetSelectedFlag( ) ) 
			image = GetNextPhoto( pos ) ;

	return image ;
}




inline PhotoImage * CMakePhotoDisk3Doc::FindPhoto( int index,  POSITION & pos ) const
{
	if ( pos = m_collection_pic_list.FindIndex( index ) )
		return GetNextPhoto( pos ) ;
	else
		return NULL ;
}


inline int CMakePhotoDisk3Doc::GetLastSelectedImageNo( ) const
{
	return m_last_selection ;
}


inline int CMakePhotoDisk3Doc::GetSelectionCount( ) const
{
	return m_selection_count ;
} ;


inline int CMakePhotoDisk3Doc::GetAnchorSelectionNo( ) const 
{
	return m_anchor_selection ;
} ;


inline void CMakePhotoDisk3Doc::SetLastSelectedImageNo( int number, bool ctrl_select, bool shift_select ) 
{
	ASSERT( number >= -1 && number < m_collection_pic_list.GetSize( ) ) ;

	POSITION		pos ;
	PhotoImage *	photo ;

	if ( !ctrl_select && !shift_select ) 
	{
		ResetSelectFlags( ) ;

		m_anchor_selection = number ;
		m_selection_count = 1 ;
		photo = FindPhoto( number, pos ) ;
		photo->SetSelectedFlag( ) ;
	}
	else if ( !ctrl_select && shift_select ) 
	{
		ResetSelectFlags( ) ;

		// select all photos between anchor and number - 
		SelectRange( m_anchor_selection, number ) ;
	}
	else if ( ctrl_select && !shift_select ) 
	{
		photo = FindPhoto( number, pos ) ;

		if ( photo->GetSelectedFlag( ) )
		{
			photo->SetSelectedFlag( false ) ;
			m_selection_count-- ;
		}
		else
		{
			photo->SetSelectedFlag( true ) ;
			m_selection_count++ ;
		}
		m_anchor_selection = number ;
	}
	else if ( ctrl_select && shift_select ) 
	{
		SelectRange( m_anchor_selection, number ) ;
		m_anchor_selection = number ;
	}
	else
		ASSERT( FALSE ) ;

	m_last_selection = number ;
} 





inline void CMakePhotoDisk3Doc::SelectRange( int sel1, int sel2 ) 
{
	if ( sel1 > sel2 ) 
		SelectRange( sel2, sel1 ) ;
	else
	{
		PhotoImage *	photo ;
		POSITION		pos ;

		do
		{
			photo = FindPhoto( sel1, pos ) ;
			if ( !photo->GetSelectedFlag( ) )
			{
				photo->SetSelectedFlag( true ) ;
				m_selection_count++ ;
			}
			sel1++ ;
		}
		while ( sel1 <= sel2 ) ;
	}
}







inline void CMakePhotoDisk3Doc::SetDiskType( DiskTypes disk_type ) 
{
	// assume there's no watermarking 
	m_watermark_presentation = false ;
	m_watermark_compressed = false ;
	m_watermark_facebook = false ;
	m_watermark_full_size = false ;

	switch ( m_disk_type = disk_type )
	{
		case UndefinedDiskType : 
			m_do_big_presentation = false ;
			m_do_compressed = false ;
			m_do_facebook = false ;
			m_do_full_size = false ;
			break ;

		case FamilyAndFriendsDisk :
			// defaults to presentation, fullsize, facebook, compressed...	with a generous release
			m_do_big_presentation = true ;
			m_do_compressed = true ;
			m_do_facebook = true ;
			m_do_full_size = true ;
			break ;

		case AssignmentProduct :
			// defaults to presentation, fullsize, compressed...			with a tailored release (per terms of contract) 
			m_do_big_presentation = true ;
			m_do_compressed = true ;
			m_do_facebook = true ;

			m_do_full_size = false;
			break ;

		case ClientDisk : 
			// defaults to small presentation, facebook, compressed...		with a limited release w/ instructions on printing 
			m_do_compressed = true ;
			m_do_facebook = true ;

			m_do_big_presentation = false ;
			m_do_full_size = false ;
			break ;

		case SamplerDisk : 
			// small presentation...										with an explict non-release + contact info...	
			m_do_big_presentation = false ;
			m_do_compressed = false ;
			m_do_facebook = false ;
			m_do_full_size = false ;
			break ;
	}
} 



inline int CMakePhotoDisk3Doc::GetPhotoListSize( ) const
{
	return m_collection_pic_list.GetSize( ) ;
} 

inline const Collection * CMakePhotoDisk3Doc::GetSelectedCollection( ) const
{
	return m_sel_collection ;
} 

inline const TCHAR * CMakePhotoDisk3Doc::GetCatalog( ) const
{
	return m_catalog_path ;
} 

inline CMakePhotoDisk3Doc::DiskTypes CMakePhotoDisk3Doc::GetBasicDiskType( ) const
{
	return m_disk_type ;
} 




inline bool CMakePhotoDisk3Doc::GetDoBigPresentation( ) const 
{
	return m_do_big_presentation ;
}

inline bool CMakePhotoDisk3Doc::GetDoCompressed( ) const
{
	return m_do_compressed ;
}

inline bool CMakePhotoDisk3Doc::GetDoFacebook( ) const
{
	return m_do_facebook ;
}

inline bool CMakePhotoDisk3Doc::GetDoFullSize( ) const
{
	return m_do_full_size ;
}

inline bool CMakePhotoDisk3Doc::GetWatermarkPresentation( ) const
{
	return m_watermark_presentation ;
}

inline bool CMakePhotoDisk3Doc::GetWatermarkCompressed( ) const 
{
	return m_watermark_compressed ;
}

inline bool CMakePhotoDisk3Doc::GetWatermarkFacebook( ) const
{
	return m_watermark_facebook ;
}

inline bool CMakePhotoDisk3Doc::GetWaterarkFullsize( ) const
{
	return m_watermark_full_size ;
}



inline void CMakePhotoDisk3Doc::SetDoBigPresentation( bool value ) 
{
	m_do_big_presentation = value ;
}

inline void CMakePhotoDisk3Doc::SetDoCompressed( bool value )
{
	m_do_compressed = value ;
}

inline void CMakePhotoDisk3Doc::SetDoFacebook( bool value )
{
	m_do_facebook = value ;
}

inline void CMakePhotoDisk3Doc::SetDoFullSize( bool value )
{
	m_do_full_size = value ;
}




inline void CMakePhotoDisk3Doc::SetWatermarkPresentation( bool value )
{
	m_watermark_presentation = value ;
}

inline void CMakePhotoDisk3Doc::SetWatermarkCompressed( bool value ) 
{
	m_watermark_compressed = value ;
}

inline void CMakePhotoDisk3Doc::SetWatermarkFacebook( bool value )
{
	m_watermark_facebook = value ;
}

inline void CMakePhotoDisk3Doc::SetWaterarkFullsize( bool value )
{
	m_watermark_full_size = value ;
}










inline const TCHAR * CMakePhotoDisk3Doc::GetDiskTitle( ) const
{
	return m_disk_title ;
} 

inline void CMakePhotoDisk3Doc::SetDiskTitle( const TCHAR * disk_title ) 
{
	m_disk_title = disk_title ;
}

inline void CMakePhotoDisk3Doc::SetLocation( const TCHAR * location ) 
{
	m_project_location = location ;
	m_project_path.Format( _T( "%s\\%s" ), location, (const TCHAR *) m_project_directory ) ;
} 

inline void CMakePhotoDisk3Doc::SetDirectory( const TCHAR * directory )
{
	m_project_directory = directory ;
	m_project_path.Format( _T( "%s\\%s" ), (const TCHAR *) m_project_location, directory ) ;
} 

inline const TCHAR * CMakePhotoDisk3Doc::GetProjectPath( ) const
{
	return m_project_path ;
} 

inline const TCHAR * CMakePhotoDisk3Doc::GetProjectLocation( ) const 
{
	return m_project_location ;
} 

inline const TCHAR * CMakePhotoDisk3Doc::GetProjectDirectory( ) const
{
	return m_project_directory ;
} 


inline bool	CMakePhotoDisk3Doc::GetSortByCaptureTime( ) const
{
	return m_sort_by_capture_time ;
}

inline bool CMakePhotoDisk3Doc::GetAscendingSort( ) const
{
	return m_ascending_sort ;
}

inline CMakePhotoDisk3Doc::DocState CMakePhotoDisk3Doc::GetDocState( ) const
{
	return m_doc_state ;
} 

inline void CMakePhotoDisk3Doc::SetDocState( DocState new_doc_state ) 
{
	ASSERT( new_doc_state == Virgin || new_doc_state == CatalogCollectionKnown || new_doc_state == CatalogCollectionProjectKnown ) ;

	m_doc_state = new_doc_state ;
} 


inline const TCHAR * CMakePhotoDisk3Doc::GetKeyImageSource( ) const
{
	if ( m_key_image_source.GetLength( ) )
		return m_key_image_source ;
	else
		return NULL ;
}


inline const TCHAR * CMakePhotoDisk3Doc::GetDiskIconSource( ) const
{
	if ( m_disk_icon_source.GetLength( ) )
		return m_disk_icon_source ;
	else
		return NULL ;
}


inline UINT32 CMakePhotoDisk3Doc::GetMajorRevision( ) const
{
	return m_major_revision ;
}


inline UINT32 CMakePhotoDisk3Doc::GetMinorRevision( ) const
{
	return m_minor_revision ;
}


inline void CMakePhotoDisk3Doc::SetKeyImageSource( const TCHAR * source_path )
{
	m_key_image_source = source_path ;
}


inline void CMakePhotoDisk3Doc::SetDiskIconSource( const TCHAR * source_path )
{
	m_disk_icon_source = source_path ;
}


inline void CMakePhotoDisk3Doc::SetRevision( UINT32 major, UINT32 minor )
{
	m_major_revision = major ;
	m_minor_revision = minor ;
}



