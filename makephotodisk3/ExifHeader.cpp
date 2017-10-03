/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   PORTED FROM MakePhotoDisk2

   Rev 1.3   30 Oct 2012 12:29:36   CSentz(new desktop)
Fixed compile errors in VS2008. Pointer to non-const can no longer be passed as argument declared
as taking a reference to a pointer to const. Fix was just declaring pointers as pointers to 
const data.

   Rev 1.2   02 Dec 2007 16:25:56   CSentz(desktop)
Resotred logic to extract detailed lens description for jpeg's created with CS2, so that now
we can still handle the old format, but then try the new CS3 format if we don't find the CS2
version. This way, we can re-create old disks.

   Rev 1.1   01 Dec 2007 02:39:46   CSentz(desktop)
Fixed detection of lens to match changes with CS3

   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/
#include "StdAfx.h"
// #include "JpegFile.h"
#include "EXIFHeader.h"



EXIFHeader::EXIFHeader( JPegSection & source )
	: JPegSection( source ) 
{
	m_tiff = m_data ;
	m_ifd = NULL ;
	
	m_valid_exif = ParseEXIF( ) ;
}



EXIFHeader::~EXIFHeader(void)
{
	delete m_ifd ;
}



EXIFHeader & EXIFHeader::operator=( JPegSection & source )
{
	if ( this != & source )
	{
		( (JPegSection) *this ) = source ;
		m_tiff = m_data ;
		m_ifd = NULL ;
		
		m_valid_exif = ParseEXIF( ) ;
	}

	return *this ;
}


EXIFHeader & EXIFHeader::operator=( EXIFHeader & source )
{
	if ( this != & source )
	{
		// use the base class to assign its stuff 
		( (JPegSection) *this ) = source ;

		// now copy over the exif fields.... 
		if ( m_valid_exif = source.m_valid_exif )
		{
			m_reverse_bytes = source.m_reverse_bytes ;
			m_tiff = m_data + ( source.m_tiff - source.m_data ) ;
		}

		m_ifd = new EXIFDirectory( * source.m_ifd ) ;
		
		ASSERT( FALSE ) ;
	}
	
	return *this ;
}

	
uint16 EXIFHeader::GetUint16At( const uchar * data, bool reverse_bytes ) const
{
	uint16 tmp = *( (uint16 *) data ) ;
	
	if ( reverse_bytes )
		_swab( (char *) &tmp, (char *) &tmp, sizeof( uint16 ) ) ;
	return tmp ;
}


uint16	EXIFHeader::GetUint16At( int offset ) const
{
	return GetUint16At( m_tiff + offset, m_reverse_bytes ) ;
} 




uint32 EXIFHeader::GetUint32At( const uchar * data, bool reverse_bytes ) const
{
	uint32	tmp ;
	uint16	lsw ;
	uint16	msw ;
		
	if ( reverse_bytes )
	{
		msw = GetUint16At( data, reverse_bytes ) ;
		lsw = GetUint16At( data + 2, reverse_bytes ) ;
		tmp = MAKELONG( lsw, msw ) ;	
	}
	else
		tmp = *( (uint32 *) data ) ;

	return tmp ;
}


uint32	EXIFHeader::GetUint32At( int offset ) const
{
	return GetUint32At( m_tiff + offset, m_reverse_bytes ) ;
}



Rational EXIFHeader::GetRationalAt( int offset ) const
{
	return GetRationalAt( m_tiff + offset, m_reverse_bytes ) ;
}

Rational EXIFHeader::GetRationalAt( const uchar * data, bool reverse_bytes ) const
{
	return Rational( GetUint32At( data, reverse_bytes ), GetUint32At( data + sizeof( uint32 ), reverse_bytes ) ) ;
}




//	CPtrList	m_tiff_stack ;

struct TiffStackEntry
{
	bool	reverse_bytes ;
	uchar *	tiff ;
	
	TiffStackEntry( bool _reverse_bytes, uchar * _tiff )
	{
		reverse_bytes = _reverse_bytes ;
		tiff = _tiff ;
	} ;	
} ;

void EXIFHeader::PushTiffContext( bool reverse_bytes, uchar * tiff )
{
	TiffStackEntry *	tse = new TiffStackEntry( m_reverse_bytes, m_tiff ) ;

	m_tiff_stack.AddTail( tse ) ;
	m_reverse_bytes = reverse_bytes ;
	m_tiff = tiff ;
}

void EXIFHeader::PopTiffContext( )
{
	TiffStackEntry *	tse ;
	
	if ( m_tiff_stack.GetCount( ) )
	{
		tse = (TiffStackEntry *) m_tiff_stack.RemoveTail( ) ;
		m_reverse_bytes = tse->reverse_bytes ;
		m_tiff = tse->tiff ;
		
		delete tse ;
	}	
}




/*

From TIFF6.pdf - 

	A TIFF file begins with an 8-byte image file header, containing the following
	information:

		Bytes 0-1: The byte order used within the file. Legal values are:

			II (4949.H)
			MM (4D4D.H)

			In the II format, byte order is always from the least significant byte to the most
			significant byte, for both 16-bit and 32-bit integers This is called little-endian byte
			order. In the MM format, byte order is always from most significant to least
			significant, for both 16-bit and 32-bit integers. This is called big-endian byte
			order.
			
		Bytes 2-3 An arbitrary but carefully chosen number (42) that further identifies the file as a
			TIFF file. The byte order depends on the value of Bytes 0-1.
			
		Bytes 4-7 The offset (in bytes) of the first IFD. The directory may be at any location in the
			file after the header but must begin on a word boundary. In particular, an Image
			File Directory may follow the image data it describes. Readers must follow the
			pointers wherever they may lead.
			
			The term byte offset is always used in this document to refer to a location with
			respect to the beginning of the TIFF file. The first byte of the file has an offset of 0.
		
*/
bool EXIFHeader::ParseEXIF( )
{
	bool	isOK = false ;
	
	// m_data should have the 2-byte length followed by "Exif", an indication of byte-ordering, then a 2-byte magic number - 
	if ( 0 == memcmp( m_data + 2, "Exif", 4 ) )
	{
		bool	reverse_bytes ;
	
		if ( 0 == memcmp( m_data + 8, "II", 2 ) )
			reverse_bytes = false ;
		else if ( 0 == memcmp( m_data + 8, "MM", 2 ) )
			reverse_bytes = true ;
		else
			return false ;
			
		PushTiffContext( reverse_bytes, m_data + 8 ) ;

		if ( 0x002a == GetUint16At( 2 ) )
			m_ifd = ParseImageFileDir( GetUint32At( 4 ) ) ;
		
		PopTiffContext( ) ;
	}
	
	return isOK ;
}


EXIFDirectory * EXIFHeader::ParseNikon( uint32 start_of_tiff )
{
	EXIFDirectory *	exif_dir = NULL ;

	// The Nikon comment starts with a 0-terminated string, "Nikon" 
	if ( 0 == strcmp( (const char *) m_tiff + start_of_tiff, "Nikon" ) )
	{
		bool	reverse_bytes ;
	
		// after "Nikon" we have what appears to be a 4-byte version number, then the usual "II" or "MM", magic number, and IFD's 
		if ( 0 == memcmp( m_tiff + start_of_tiff + 10, "II", 2 ) )
			reverse_bytes = false ;
		else if ( 0 == memcmp( m_tiff + start_of_tiff + 10, "MM", 2 ) )
			reverse_bytes = true ;
		else
			return false ;

		PushTiffContext( reverse_bytes, m_tiff + start_of_tiff + 10 ) ;
		
		if ( 0x002a == GetUint16At( 2 ) )
			exif_dir = ParseImageFileDir( GetUint32At( 4 ) ) ;
		
		PopTiffContext( ) ;
	}
	
	return exif_dir ;
}






/*
	EXIF information cnsists of the following: 
	
	We have a header with certain identifying fields - 
	
		2-byte EXIF tag (FFE1)
		2-byte section length (always in moto-order, and length includes the 2 bytes for the length field)
		6-byte string "Exif\0\0"
		2-byte byte-order code (either "MM" or "II" - if "MM" integers are motorola order, if "II", integers are intel order)
		2-byte integer containing the magic number 42 
		4-byte offset of first IFD - this, like all IFD offsets, is relative to the offset of the start of the "II" or "MM"
	
	An IFD consists of the following - 
		2-byte directory count
		12-byte directory entries
		4-byte "next pointer" to the next IFD at the current level - if 0, this is the last IFD at the level
	
	An IFD must begin on an even byte address
	
	Each directory entry consists of
		2-byte tag field
		2-byte data type code
		4-byte data count 
		4-byte data value (if, based on the data-type and count, the total size of the data is 4 or less bytes, this value field
			contains the actual value. If, however, the total size of the data is greater than 4 bytes, this value field is actually
			an offset to the data. This offset, as always, is relative to the start of the byte-order field with "II" or "MM")
	
	There's no stated requirement exactly where the data referenced by value pointers must go. It appears, though, that it typically 
	follows the directory referencing it. That is not a requirement, in fact the documentation in TIFF6.pdf says at some point the
	value pointer may actually reference a location in the file following the image data. Evidently, the value referenced must 
	begin on an even byte boundary. 
	
*/

EXIFDirectory * EXIFHeader::ParseImageFileDir( uint32 start_ifd )
{
	uint16	dir_ct ;
	bool	isOK = false ;

	uint32	cur_addr ;
	uint16	entry_tag ;
	uint16	entry_type ;
	int32	entry_ct ;
	uint32	entry_value ;
	uchar *	entry_data ;
	
	EXIFDirectory *	first_dir = NULL ;
	EXIFDirectory *	exif_dir = NULL ;

	do
	{
		// dbgPrintf( TEXT( "\n         IFD at %08.8X\n" ), start_ifd ) ;

		if ( 1 <= ( dir_ct = GetUint16At( start_ifd ) ) )
		{
			{
				EXIFDirectory *	new_exif_dir = new EXIFDirectory( dir_ct, start_ifd, m_tiff ) ;
				
				// if we were parsing a previous directory, this must be a 2nd (or later) time through the loop, so set its 'next' pointer to this one 
				if ( exif_dir )
					exif_dir->SetNext( new_exif_dir ) ;
				exif_dir = new_exif_dir ;

				// if first_dir is NULL, must be parsing the first dir in the chain
				if ( NULL == first_dir )
					first_dir = exif_dir ;
					
				// if m_ifd is also NULL, we must be parsing the first IFD in the file...
				if ( NULL == m_ifd )
					m_ifd = exif_dir ;
			}
	
			uint16	i ;

			isOK = true ;
			
			// set our current address to the byte immediately after the directory's entry count 
			cur_addr = start_ifd + 2 ;
			
			for ( i = 0 ; i < dir_ct ; i++ )
			{
				EXIFDirectory * sub_dir = NULL ;
				
				entry_tag = GetUint16At( cur_addr ) ;
				entry_type = GetUint16At( cur_addr + 2 ) ;
				entry_ct = GetUint32At( cur_addr + 4 ) ;
				entry_value = GetUint32At( cur_addr + 8 ) ;

				if ( DataSize( entry_type, entry_ct ) > 4 )
					entry_data = m_tiff + entry_value ;
				else
					entry_data = m_tiff + cur_addr + 8 ;

				// dbgPrintf( TEXT( "              Entry %2.2d: tag=0x%04.4X, type=0x%04.4X, n=%4d, v=%08.8X\n" ), i, entry_tag, entry_type, entry_ct, entry_value ) ;

				// special processing for certain tags - 
				switch ( entry_tag )
				{
					case TAG_MAKER_COMMENT:
						if ( 0 == strcmp( GetCameraMake( ), "NIKON CORPORATION" ) && 0 == strcmp( GetCameraModel( ), "NIKON D100 " ) )
							sub_dir = ParseNikon( entry_value ) ;
						break ;

					case TAG_EXIF_OFFSET:
						sub_dir = ParseImageFileDir( entry_value ) ;
						break ;

					case TAG_INTEROP_OFFSET :
						sub_dir = ParseImageFileDir( entry_value ) ;
						break ;
						
					default :
						break ;
				}

				// FIX ME - this is moderately dumb. Consider what's happening here - we're constructing a temporary EXIFDirectoryItem, a reference
				// to this is passed down to EXIFItemDirectoryArray::AddItem, which then invokes the EXIFItemDirectoryItem assign operator which
				// effectively creates a copy this object. This is then immediately destructed. 
				exif_dir->AddItem( EXIFDirectoryItem( entry_tag, entry_type, entry_ct, m_reverse_bytes, entry_data, sub_dir ) ) ;
				cur_addr += 12 ;
			}
		
			// dbgPrintf( TEXT( "\n" ) ) ;
		}
		else
			return false ;
	}
	while ( start_ifd = GetUint32At( cur_addr ) ) ;
	
	return first_dir ;
}





struct InterestingTags
{
	uint16	tag ;
	int		sub_ifd ;
	int		parent_ifd ;
	int		ifd_no ;
} ;


/**********************************************************************************
Selectively write EXIF data - 

We DO NOT write the following:
	
	maker comment fields - these contain proprietary and undocumented fields which may be meaningless
	thumbnails - 
	interoperability IFD - can't find any good documentation on what goes in there, and have yet to see an interop IFD with more than 
	2 fields - one of which simply advertises that it's "R98" compatible. 
	
We DO write the following:
	author, title, description and copyright data
	date/time of creation
	photographic detail - 


**********************************************************************************/
bool EXIFHeader::Write( CFile & dest_file )
{
	return false ;
}



void CreateIFDImages( InterestingTags tags[ ], int tag_ct, BYTE * & ifd_base, uint16 & ifd_len, BYTE * & overflow_base, uint16 & overflow_len )
{

}
	










/*****************



******************/

// returns TRUE if matching tag is found 
bool EXIFHeader::GetEXIFDirectoryItem( const EXIFDirectory * dir, uint16 tag, uint16 & data_type, uint32 & data_count, bool & reverse_bytes, const uchar * & data_reference ) const
{
	int	i ;

	for ( i = 0 ; i < dir->GetCount( ) ; i++ )
	{
		const EXIFDirectoryItem & dir_item( dir->GetItemAt( i ) ) ;

		if ( dir_item.GetTag( ) == tag )
		{
			data_type = dir_item.GetType( ) ;
			data_count = dir_item.GetCount( ) ;
			reverse_bytes = dir_item.GetReverseBytes( ) ;
			data_reference = dir_item.GetData( ) ;
			
			return true ;
		}

		if ( NULL != dir_item.GetDir( ) )
		{
			if ( GetEXIFDirectoryItem( dir_item.GetDir( ), tag, data_type, data_count, reverse_bytes, data_reference ) )
				return true ;
		}
	}

	// nothing matched
	return false ;	
}





bool EXIFHeader::GetEXIFDirectoryItem( uint16 tag, uint16 & data_type, uint32 & data_count, bool & reverse_bytes, const uchar * & data_reference ) const
{
	if ( m_ifd )
		return GetEXIFDirectoryItem( m_ifd, tag, data_type, data_count, reverse_bytes, data_reference ) ;
	else
		return false ;
}




const char * EXIFHeader::GetEXIFDirectoryItemString( uint16 tag ) const
{
	uint16	data_type = 0 ;
	uint32	count = 0 ;
	bool	reverse_bytes ;
	const uchar *	data_reference = NULL ;

	if ( GetEXIFDirectoryItem( tag, data_type, count, reverse_bytes, data_reference ) )
	{
		if ( DTYPE_STRING == data_type && NULL != data_reference ) 
			return (const char *) data_reference ;
	}

	return NULL ;
}


const char * EXIFHeader::GetCameraMake( ) const
{
	return GetEXIFDirectoryItemString( TAG_MAKE ) ;
}

const char * EXIFHeader::GetCameraModel( ) const
{
	return GetEXIFDirectoryItemString( TAG_MODEL ) ;
}



int EXIFHeader::DataSize( uint16 entry_type, uint32 entry_ct )
{
	switch ( entry_type )
	{
		case DTYPE_UINT8 :
		case DTYPE_INT8 :
		case DTYPE_STRING :
		case DTYPE_SUNDEF :
			return entry_ct ;

		case DTYPE_UINT16 :
		case DTYPE_INT16 :
			return 2 * entry_ct ;

		case DTYPE_SINGLE :
		case DTYPE_UINT32 :
		case DTYPE_INT32 :
			return 4 * entry_ct ;

		case DTYPE_URATIONAL :
		case DTYPE_RATIONAL :
		case DTYPE_DOUBLE :
			return 8 * entry_ct ;
		
		default:
			ASSERT( FALSE ) ;
			return entry_ct ;
	}
}






struct TagTable
{
	uint16			tag ;
	const TCHAR *	label ;
} tag_strings[ ] = 
{
	{	EXIFHeader::TAG_COMPRESSION,		TEXT( "TAG_COMPRESSION" )		},
	{	EXIFHeader::TAG_TITLE,				TEXT( "TAG_TITLE" )		},
	{	EXIFHeader::TAG_MAKE,				TEXT( "TAG_MAKE" )		},
	{	EXIFHeader::TAG_MODEL,				TEXT( "TAG_MODEL" )		},
	{	EXIFHeader::TAG_ORIENTATION,		TEXT( "TAG_ORIENTATION" )		},
	{	EXIFHeader::TAG_XRESOLUTION,		TEXT( "TAG_XRESOLUTION" )		},
	{	EXIFHeader::TAG_YRESOLUTION,		TEXT( "TAG_YRESOLUTION" )		},
	{	EXIFHeader::TAG_RESOLUTION_UNIT,	TEXT( "TAG_RESOLUTION_UNIT" )		},
	{	EXIFHeader::TAG_SOFTWARE_VER,		TEXT( "TAG_SOFTWARE_VER" )		},
	{	EXIFHeader::TAG_DATE_TIME,			TEXT( "TAG_DATE_TIME" )		},
	{	EXIFHeader::TAG_THUMBNAIL_OFFSET,	TEXT( "TAG_THUMBNAIL_OFFSET" )		},
	{	EXIFHeader::TAG_THUMBNAIL_LENGTH,	TEXT( "TAG_THUMBNAIL_LENGTH" )		},
	{	EXIFHeader::TAG_SUBCHROM_POS,		TEXT( "TAG_SUBCHROM_POS" )	},
	{	EXIFHeader::TAG_PHOTOGRAPHER,		TEXT( "TAG_PHOTOGRAPHER" )		},
	{	EXIFHeader::TAG_COPYRIGHT_HOLDER,	TEXT( "TAG_COPYRIGHT_HOLDER" )		},
	{	EXIFHeader::TAG_EXPOSURETIME,		TEXT( "TAG_EXPOSURETIME" )		},
	{	EXIFHeader::TAG_FNUMBER,			TEXT( "TAG_FNUMBER" )		},
	{	EXIFHeader::TAG_EXIF_OFFSET,		TEXT( "TAG_EXIF_OFFSET" )		},
	{	EXIFHeader::TAG_EXPOSURE_PROGRAM,	TEXT( "TAG_EXPOSURE_PROGRAM" )		},
	{	EXIFHeader::TAG_ISO_EQUIVALENT,		TEXT( "TAG_ISO_EQUIVALENT" )		},
	{	EXIFHeader::TAG_COMP_CONFIG,		TEXT( "TAG_COMP_CONFIG" )		},
	{	EXIFHeader::TAG_CMP_BITS_PER_PIXL,	TEXT( "TAG_CMP_BITS_PER_PIXL" )	},
	{	EXIFHeader::TAG_SHUTTERSPEED,		TEXT( "TAG_SHUTTERSPEED" )		},
	{	EXIFHeader::TAG_APERTURE,			TEXT( "TAG_APERTURE" )		},
	{	EXIFHeader::TAG_MAXAPERTURE,		TEXT( "TAG_MAXAPERTURE" )		},
	{	EXIFHeader::TAG_FOCALLENGTH,		TEXT( "TAG_FOCALLENGTH" )		},
	{	EXIFHeader::TAG_EXIF_VERSION,		TEXT( "TAG_EXIF_VERSION" )		},
	{	EXIFHeader::TAG_DATETIME_ORIGINAL,	TEXT( "TAG_DATETIME_ORIGINAL" )		},
	{	EXIFHeader::TAG_DATE_TIME_DIGITIZ,	TEXT( "TAG_DATE_TIME_DIGITIZ" )		},
	{	EXIFHeader::TAG_BRIGHTNESS,			TEXT( "TAG_BRIGHTNESS" )	},
	{	EXIFHeader::TAG_EXPOSURE_BIAS,		TEXT( "TAG_EXPOSURE_BIAS" )		},
	{	EXIFHeader::TAG_SUBJECT_DISTANCE,	TEXT( "TAG_SUBJECT_DISTANCE" )		},
	{	EXIFHeader::TAG_METERING_MODE,		TEXT( "TAG_METERING_MODE" )		},
	{	EXIFHeader::TAG_WHITEBALANCE,		TEXT( "TAG_WHITEBALANCE" )		},
	{	EXIFHeader::TAG_FLASH,				TEXT( "TAG_FLASH" )		},
	{	EXIFHeader::TAG_MAKER_COMMENT,		TEXT( "TAG_MAKER_COMMENT" )		},
	{	EXIFHeader::TAG_USERCOMMENT,		TEXT( "TAG_USERCOMMENT" )		},
	{	EXIFHeader::TAG_SUBSEC_TIME,		TEXT( "TAG_SUBSEC_TIME" )		},
	{	EXIFHeader::TAG_SUBSEC_TIME_ORIG,	TEXT( "TAG_SUBSEC_TIME_ORIG" )		},
	{	EXIFHeader::TAG_SUBSEC_TIME_DIGI,	TEXT( "TAG_SUBSEC_TIME_DIGI" )		},
	{	EXIFHeader::TAG_FLASHPIX_VERSION,	TEXT( "TAG_FLASHPIX_VERSION" )		},
	{	EXIFHeader::TAG_COLORSPACE,			TEXT( "TAG_COLORSPACE" )		},
	{	EXIFHeader::TAG_EXIF_IMAGEWIDTH,	TEXT( "TAG_EXIF_IMAGEWIDTH" )		},
	{	EXIFHeader::TAG_EXIF_IMAGELENGTH,	TEXT( "TAG_EXIF_IMAGELENGTH" )		},
	{	EXIFHeader::TAG_RELATED_SOUND,		TEXT( "TAG_RELATED_SOUND" )		},
	{	EXIFHeader::TAG_INTEROP_OFFSET,		TEXT( "TAG_INTEROP_OFFSET" )		},
	{	EXIFHeader::TAG_FOCALPLANEXRES,		TEXT( "TAG_FOCALPLANEXRES" )		},
	{	EXIFHeader::TAG_FOCALPLANEYRES,		TEXT( "TAG_FOCALPLANEYRES" )		},
	{	EXIFHeader::TAG_FOCALPLANEUNITS,	TEXT( "TAG_FOCALPLANEUNITS" )		},
	{	EXIFHeader::TAG_SENSING_METHOD,		TEXT( "TAG_SENSING_METHOD" )		},
	{	EXIFHeader::TAG_FILE_SOURCE,		TEXT( "TAG_FILE_SOURCE" )		},
	{	EXIFHeader::TAG_SCENE_TYPE,			TEXT( "TAG_SCENE_TYPE" )		},
	{	EXIFHeader::TAG_CFA_PATTERN,		TEXT( "TAG_CFA_PATTERN" )		},
	{	EXIFHeader::TAG_CUST_RENDERED,		TEXT( "TAG_CUST_RENDERED" )		},
	{	EXIFHeader::TAG_EXPOSURE_MODE,		TEXT( "TAG_EXPOSURE_MODE" )		},
	{	EXIFHeader::TAG_WHITE_BALANCE,		TEXT( "TAG_WHITE_BALANCE" )		},
	{	EXIFHeader::TAG_DIGITAL_ZOOM,		TEXT( "TAG_DIGITAL_ZOOM" )		},
	{	EXIFHeader::TAG_FOCALLEN_35MM,		TEXT( "TAG_FOCALLEN_35MM" )		},
	{	EXIFHeader::TAG_SCENE_CAP_TYPE,		TEXT( "TAG_SCENE_CAP_TYPE" )	},
	{	EXIFHeader::TAG_GAIN_CONTROL,		TEXT( "TAG_GAIN_CONTROL" )		},
	{	EXIFHeader::TAG_CONTRAST,			TEXT( "TAG_CONTRAST" )			},
	{	EXIFHeader::TAG_SATURATION,			TEXT( "TAG_SATURATION" )		},
	{	EXIFHeader::TAG_SHARPNESS,			TEXT( "TAG_SHARPNESS" )			},
	{	EXIFHeader::TAG_DISTANCE_RANGE,		TEXT( "TAG_DISTANCE_RANGE" )	},
	{	EXIFHeader::NTAG_NIKON_VERSION,		TEXT( "NTAG_NIKON_VERSION" )		},
	{	EXIFHeader::NTAG_ISO_EQUIVALENT,	TEXT( "NTAG_ISO_EQUIVALENT" )		},
	{	EXIFHeader::NTAG_COLOR_BW,			TEXT( "NTAG_COLOR_BW" )		},
	{	EXIFHeader::NTAG_IMG_QUALITY,		TEXT( "NTAG_IMG_QUALITY" )		},
	{	EXIFHeader::NTAG_WHITE_BAL,			TEXT( "NTAG_WHITE_BAL" )		},
	{	EXIFHeader::NTAG_SHARPENING,		TEXT( "NTAG_SHARPENING" )		},
	{	EXIFHeader::NTAG_FOCUS,				TEXT( "NTAG_FOCUS" )		},
	{	EXIFHeader::NTAG_FLASH_MODE,		TEXT( "NTAG_FLASH_MODE" )		},
	{	EXIFHeader::NTAG_FLASH_METERING,	TEXT( "NTAG_FLASH_METERING" )		},
	{	EXIFHeader::NTAG_WHITE_BAL_ADJ,		TEXT( "NTAG_WHITE_BAL_ADJ" )		},
	{	EXIFHeader::NTAG_ISO_SELECTION,		TEXT( "NTAG_ISO_SELECTION" )		},
	{	EXIFHeader::NTAG_IMG_ADJUSTMENT,	TEXT( "NTAG_IMG_ADJUSTMENT" )		},
	{	EXIFHeader::NTAG_LENS_ADAPTER,		TEXT( "NTAG_LENS_ADAPTER" )		},
	{	EXIFHeader::NTAG_LENS,				TEXT( "NTAG_LENS" )		},
	{	EXIFHeader::NTAG_FOCUS_DIST,		TEXT( "NTAG_FOCUS_DIST" )		},
	{	EXIFHeader::NTAG_DIGITAL_ZOOM,		TEXT( "NTAG_DIGITAL_ZOOM" )		},
	{	EXIFHeader::NTAG_FOCUS_POSIT,		TEXT( "NTAG_FOCUS_POSIT" )		},
	{	EXIFHeader::NTAG_FLASH_TYPE,		TEXT( "NTAG_FLASH_TYPE" )		},
	{	EXIFHeader::NTAG_SATURATION,		TEXT( "NTAG_SATURATION" )		},
	{	EXIFHeader::NTAG_NOISE_REDUCTION,	TEXT( "NTAG_NOISE_REDUCTION" )	},
} ;

struct DTypeTable
{
	uint16	dtype ;
	TCHAR *	label ;
}
type_strings[ ] =
{
	{	EXIFHeader::DTYPE_UINT8,			TEXT( "DTYPE_UINT8" )		},
	{	EXIFHeader::DTYPE_STRING,			TEXT( "DTYPE_STRING" )		},
	{	EXIFHeader::DTYPE_UINT16,			TEXT( "DTYPE_UINT16" )		},
	{	EXIFHeader::DTYPE_UINT32,			TEXT( "DTYPE_UINT32" )		},
	{	EXIFHeader::DTYPE_URATIONAL,		TEXT( "DTYPE_URATIONAL" )	},
	{	EXIFHeader::DTYPE_INT8,				TEXT( "DTYPE_INT8" )		},
	{	EXIFHeader::DTYPE_SUNDEF,			TEXT( "DTYPE_SUNDEF" )		},
	{	EXIFHeader::DTYPE_INT16,			TEXT( "DTYPE_INT16" )		},
	{	EXIFHeader::DTYPE_INT32,			TEXT( "DTYPE_INT32" )		},
	{	EXIFHeader::DTYPE_RATIONAL,			TEXT( "DTYPE_RATIONAL" )	},
	{	EXIFHeader::DTYPE_SINGLE,			TEXT( "DTYPE_SINGLE" )		},
	{	EXIFHeader::DTYPE_DOUBLE,			TEXT( "DTYPE_DOUBLE" )		},
} ;



void EXIFHeader::ShowDirItemData( int dir_no, int level, const EXIFDirectoryItem & item )
{
	const TCHAR * tag_str =  TEXT( "   ???   " ) ;
	const TCHAR * type_str = TEXT( "   ???   " ) ;

	int i ;
	int	tmp ;
	Rational	rat ;
		
	for ( i = 0 ; i < sizeof( tag_strings ) / sizeof( tag_strings[ 0 ] ) ; i++ )
	{
		if ( item.GetTag( ) == tag_strings[ i ].tag )
		{
			tag_str = tag_strings[ i ].label ;
			break ;
		}
	}

	for ( i = 0 ; i < sizeof( type_strings ) / sizeof( type_strings[ 0 ] ) ; i++ )
	{
		if ( item.GetType( ) == type_strings[ i ].dtype )
		{
			type_str = type_strings[ i ].label ;
			break ;
		}
	}

	_tprintf( TEXT( "%*c%2d: Tag=%4.4X (%-22s)  Type=%4.4X (%-16s)  Count=%-5d  " ), level * 5, TEXT( ' ' ), dir_no + 1, item.GetTag( ), tag_str, item.GetType( ), type_str, item.GetCount( ) ) ;

	switch ( item.GetType( ) )
	{
		case EXIFHeader::DTYPE_STRING :
			// this MUST be ASCII 
			printf( "\"%.30s\"", item.GetData( ) ) ;
			break ;
		
		case EXIFHeader::DTYPE_INT16 :
		case EXIFHeader::DTYPE_UINT16 :
			tmp = GetUint16At( item.GetData( ), item.GetReverseBytes( ) ) ;
			_tprintf( TEXT( "%d" ), tmp ) ;
			break ;
			
		case EXIFHeader::DTYPE_INT32 :
		case EXIFHeader::DTYPE_UINT32 :
			tmp = GetUint32At( item.GetData( ), item.GetReverseBytes( ) ) ;
			_tprintf( TEXT( "%d" ), tmp ) ;
			break ;

		case EXIFHeader::DTYPE_INT8 :
		case EXIFHeader::DTYPE_UINT8 :
			tmp = *( item.GetData( ) ) ;
			_tprintf( TEXT( "%d" ), tmp ) ;
			break ;
		
		case EXIFHeader::DTYPE_URATIONAL :
		case EXIFHeader::DTYPE_RATIONAL :
			rat = GetRationalAt( item.GetData( ), item.GetReverseBytes( ) ) ;
			_tprintf( TEXT( "%u / %u" ), rat.GetNumerator( ), rat.GetDenominator( ) ) ;
			break ;

		case EXIFHeader::DTYPE_SINGLE :
			_tprintf( TEXT( "single precision float" ) ) ;
			break ;
			
		case EXIFHeader::DTYPE_DOUBLE :
			_tprintf( TEXT( "double precision float" ) ) ;
			break ;
	}
	
	_tprintf( TEXT( "\n" ) ) ;
}



void EXIFHeader::ShowIFD( const EXIFDirectory * dir, int level )
{
	_tprintf( TEXT( "\n%*c-------------------------------------------------------------------------------\n%*cImage File Directory - %d items\n\n" ), 5 * level, TEXT( ' ' ), 5 * level, TEXT( ' ' ), dir->GetCount( ) ) ;

	for ( int i = 0 ; i < dir->GetCount( ) ; i++ )
	{
		ShowDirItemData( i, level, dir->GetItemAt( i ) ) ;
	
		if ( dir->GetItemAt( i ).GetDir( ) )
			ShowIFD( dir->GetItemAt( i ).GetDir( ), level + 1 ) ;
	}
	
	_tprintf( TEXT( "\n" ) ) ;
	
	if ( dir->GetNext( ) )
		ShowIFD( dir->GetNext( ), level ) ;
}



bool EXIFHeader::RecursiveSearchDirectory( int tag, const EXIFDirectoryItem * & dir_item ) const
{
	return RecursiveSearchDirectory( m_ifd, tag, dir_item ) ;
}

bool EXIFHeader::RecursiveSearchDirectory( const EXIFDirectory * dir, int tag, const EXIFDirectoryItem * & dir_item ) const
{	
	for ( int i = 0 ; i < dir->GetCount( ) ; i++ )
	{
		if ( dir->GetItemAt( i ).GetTag( ) == tag )
		{
			dir_item = &( dir->GetItemAt( i ) ) ;
			return true ;
		}
		else if ( dir->GetItemAt( i ).GetDir( ) )
			if ( RecursiveSearchDirectory( dir->GetItemAt( i ).GetDir( ), tag, dir_item ) )
				return true ;
	}
	
	return false ;	
}





void EXIFHeader::DEBUG_DUMP( ) 
{
/*
	if ( m_ifd )
	{
		_tprintf( TEXT( "\n\n" ) ) ;
		ShowIFD( m_ifd, 0 ) ;
		_tprintf( TEXT( "\n\n" ) ) ;
	}
*/
}



bool EXIFHeader::GetCameraName( CString & name ) const
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_MODEL, dir_item ) )
		{
			got_it = true ;
			name.Format( _T( "%.*s" ), dir_item->GetCount( ), dir_item->GetData( ) ) ;
			name.Replace( _T( "NIKON" ), _T( "Nikon" ) ) ;
		}
	}
	return got_it ;
}

bool EXIFHeader::GetCreateTime( SYSTEMTIME & create_time ) const
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_DATETIME_ORIGINAL, dir_item ) )
		{
			int		year ;
			int		mon	;
			int		day ;
			int		hour ;
			int		minute ;
			int		second ;
			CString	work ;
						
			got_it = true ;
			
			work.Format( _T( "%.*s" ), dir_item->GetCount( ), dir_item->GetData( ) ) ;
			_stscanf( work, _T( "%d:%d:%d %d:%d:%d" ), &year, &mon, &day, &hour, &minute, &second ) ;

			create_time.wYear = year ;
			create_time.wMonth = mon ;
			create_time.wDay = day ;

			create_time.wHour = hour ;
			create_time.wMinute = minute ;
			create_time.wSecond = second ;
			create_time.wMilliseconds = 0 ;
			create_time.wDayOfWeek = 0 ;
		}
	}
	return got_it ;
}






bool EXIFHeader::GetExposure( Rational & exposure ) const 
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_EXPOSURETIME, dir_item ) )
		{
			exposure = GetRationalAt( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ;
			got_it = true ;
		}	
	}
	return got_it ;
}


bool EXIFHeader::GetAperture( Rational & aperture ) const
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_FNUMBER, dir_item ) )
		{
			aperture = GetRationalAt( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ;
			got_it = true ;
		}	
	}
	
	return got_it ;
}




bool EXIFHeader::GetFocalLength( Rational & focal_len ) const
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_FOCALLENGTH, dir_item ) )
		{
			focal_len = GetRationalAt( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ;
			got_it = true ;
		}
	}
	
	return got_it ;
}



char * memstr( const char * data, int length, const char * target ) 
{
	int		remaining_len = length ;
	char *	possible_hit ;
	char *	match = NULL ;
	int		target_len = (int) strlen( target ) ;

	do
	{
		if ( possible_hit = (char *) memchr( data, *target, remaining_len ) )
		{
			remaining_len -= (int) ( possible_hit - data ) ;
			
			if ( remaining_len >= target_len )
			{
				if ( 0 == memcmp( possible_hit, target, target_len ) )
					match = possible_hit ;
				else
				{
					data = possible_hit + 1 ;
					remaining_len-- ;
				}
			}
		}
		else
			remaining_len = 0 ;
	}
	while ( NULL == match && remaining_len >= target_len ) ;
	
	return match ;
}




bool EXIFHeader::GetLensFocalLength( CString & focal_len ) const
{
	bool	got_it = false ;

		// in m_data - 
		// <aux:Lens>lens string</aux:Lens>

	if ( !m_ifd )		// yes - this field is only in the adobe exif entry. 
	{
		char  *	match ;
		
		// Handle a CS2-generated jpeg - we look in m_data for <aux:Lens>...</aux:Lens>
		if ( match = memstr( (const char *) m_data, m_length, "<aux:Lens>" ) )
		{
			int		fix_pos ;
			
			match += 10 ;
			
			focal_len.Format( _T( "%.*s" ), strstr( match, "</aux:Lens>" ) - match, match ) ;

			while ( -1 != ( fix_pos = focal_len.Find( _T( ".0" ) ) ) )
				focal_len = focal_len.Left( fix_pos ) + focal_len.Mid( fix_pos + 2 ) ;
			
			while ( -1 != ( fix_pos = focal_len.Find( _T( " mm" ) ) ) )
				focal_len = focal_len.Left( fix_pos ) + focal_len.Mid( fix_pos + 1 ) ;
			
			got_it = true ;	
		}
		// else - handle a CS3 generated jpeg
		else if ( match = memstr( (const char *) m_data, m_length, "aux:Lens=\"" ) )
		{
			int		fix_pos ;
			char *	end_quote ;
						
			match += 10 ;

			if ( NULL != ( end_quote = strchr( match, '\"' ) ) )
			{
				focal_len.Format( _T( "%.*s" ), end_quote - match, match ) ;
				
				// get rid of the .0's added to the focal length - 
				while ( -1 != ( fix_pos = focal_len.Find( _T( ".0" ) ) ) )
					focal_len = focal_len.Left( fix_pos ) + focal_len.Mid( fix_pos + 2 ) ;

				// and scootch the mm over so it's snuggly against the focal length			
				while ( -1 != ( fix_pos = focal_len.Find( _T( " mm" ) ) ) )
					focal_len = focal_len.Left( fix_pos ) + focal_len.Mid( fix_pos + 1 ) ;
			
				got_it = true ;	
			}
		}
		// else - looks like we don't understand this format
	}
	return got_it ;
}



bool EXIFHeader::GetDescription( CString & description ) const 
{
	bool	got_it = false ;
	
	if ( !m_ifd ) 
	{
		char * match ;
		
		if ( match = memstr( (const char *) m_data + 2, m_length, "<dc:description>" ) )
		{		
			if ( match = strstr( match, "<rdf:li xml:lang=\"x-default\">" ) )
			{
				match = strchr( match, '>' ) + 1 ;
				description.Format( _T( "%.*s" ), strstr( match, "</rdf:li>" ) - match, match ) ;
				description.TrimLeft( ) ;
				description.TrimRight( ) ;
				got_it = 0 != description.GetLength( ) ;
			}
		}
	}
	
	return got_it ;
}

bool EXIFHeader::GetTitle( CString & title ) const 
{
	bool	got_it = false ;
	
	if ( !m_ifd ) 
	{
		char * match ;

		if ( match = memstr( (const char *) m_data + 2, m_length, "<dc:title>" ) )
		{		
			if ( match = strstr( match, "<rdf:li xml:lang=\"x-default\">" ) )
			{
				match = strchr( match, '>' ) + 1 ;
				title.Format( _T( "%.*s" ), strstr( match, "</rdf:li>" ) - match, match ) ;
				title.TrimLeft( ) ;
				title.TrimRight( ) ;
				got_it = 0 != title.GetLength( ) ;
			}
		}
	}
	
	return got_it ;
}




bool EXIFHeader::GetFlashUsed( bool & flash_used ) const
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_FLASH, dir_item ) )
		{
			flash_used = ( 0 != GetUint16At( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ) ;
			got_it = true ;
		}
	}

	return got_it ;
}



bool EXIFHeader::GetISO( int & iso ) const
{
	bool	got_it = false ;

	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;
		
		if ( RecursiveSearchDirectory( TAG_ISO_EQUIVALENT, dir_item ) )
		{
			iso = GetUint16At( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ;
			got_it = true ;
		}
		else if ( RecursiveSearchDirectory( NTAG_ISO_EQUIVALENT, dir_item ) )
		{
			got_it = true ;
		}
	}
	else
	{
		char * match ;
		
		// look in m_data for <png:Description>...</png:Description>
		if ( match = memstr( (const char *) m_data, m_length, "<exif:ISOSpeedRatings" ) )
		{
			got_it = false ;
		}
	}
	
	return got_it ;
}



bool EXIFHeader::GetHeight( int & height ) const
{
	bool	got_it = false ;
	
	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;

		if ( RecursiveSearchDirectory( TAG_EXIF_IMAGELENGTH, dir_item ) )
		{
			height = (int) GetUint32At( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ;
			got_it = true ;
		}
	}

	return got_it ;
}



bool EXIFHeader::GetWidth( int & width ) const
{
	bool	got_it = false ;
	
	if ( m_ifd )
	{
		const EXIFDirectoryItem *	dir_item ;

		if ( RecursiveSearchDirectory( TAG_EXIF_IMAGEWIDTH, dir_item ) )
		{
			width = (int) GetUint32At( dir_item->GetData( ), dir_item->GetReverseBytes( ) ) ;
			got_it = true ;
		}
	}

	return got_it ;
}



