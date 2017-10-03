/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

#include "jpegsection.h"
#include "Rational.h"
#include "ExifDirectory.h"
#include "ExifDirectoryItem.h"

class EXIFHeader : public JPegSection
{
public:
	enum DataTags
	{
		// This is not an exhaustive list of EXIF tags - these are mostly the ones which are known and which are of interest for epscial processing 
		// TAGS must appear in ascending numerical order - 
		TAG_COMPRESSION =		0x0103,
		TAG_TITLE =				0x010E,		// image title 
		TAG_MAKE =              0x010F,		// camera make
		TAG_MODEL =             0x0110,		// camera model. The camera make and model tell us whether we can understand the maker comment
		TAG_ORIENTATION =       0x0112,
		TAG_XRESOLUTION =		0x011A,
		TAG_YRESOLUTION =		0x011B,
		TAG_RESOLUTION_UNIT =	0x0128,
		TAG_SOFTWARE_VER =		0x0131,		// defined in TIFF 6.0
		TAG_DATE_TIME =			0x0132,

		TAG_THUMBNAIL_OFFSET =  0x0201,
		TAG_THUMBNAIL_LENGTH =  0x0202,
		TAG_SUBCHROM_POS =		0x213,		// defined in TIFF 6.0 - 
		TAG_PHOTOGRAPHER =		0x0315,		// artist 

		TAG_COPYRIGHT_HOLDER =	0x8298,		// copyright holder 
		TAG_EXPOSURETIME =		0x829A,
		TAG_FNUMBER =           0x829D,
		TAG_EXIF_OFFSET =		0x8769,
		TAG_EXPOSURE_PROGRAM =  0x8822,
		TAG_ISO_EQUIVALENT =    0x8827,

		TAG_COMP_CONFIG =		0x9101,
		TAG_CMP_BITS_PER_PIXL =	0x9102,

		TAG_SHUTTERSPEED =      0x9201,
		TAG_APERTURE =          0x9202,
		TAG_MAXAPERTURE =       0x9205,
		TAG_FOCALLENGTH =       0x920A,

		TAG_EXIF_VERSION =		0x9000,
		TAG_DATETIME_ORIGINAL = 0x9003,
		TAG_DATE_TIME_DIGITIZ =	0x9004,

		TAG_BRIGHTNESS =		0x9203, 

		TAG_EXPOSURE_BIAS =     0x9204,
		TAG_SUBJECT_DISTANCE =  0x9206,
		TAG_METERING_MODE =     0x9207,
		TAG_WHITEBALANCE =      0x9208,
		TAG_FLASH =             0x9209,

		TAG_MAKER_COMMENT =		0x927C,		// MakerNote - manufacturer's data, in proprietary format - currently only parse these for Nikon D100's
		TAG_USERCOMMENT =       0x9286,		// UserComment - starts with 8-byte code identifying the character data type then 

		TAG_SUBSEC_TIME	=		0x9290,
		TAG_SUBSEC_TIME_ORIG =	0x9291,
		TAG_SUBSEC_TIME_DIGI =	0x9292,

		TAG_XP_TITLE =			0x9C9B,		// these are not documented in any EXIF or TIFF spec I've found. Appear to be unicode strings storing corresponding fields
		TAG_XP_COMMENT =		0x9c9C,
		TAG_XP_AUTHOR =			0x9c9d,
		TAG_XP_KEYWORD =		0x9c9e,
		TAG_XP_SUBJECT =		0x9c9f,
		
		TAG_FLASHPIX_VERSION =	0xA000,
		TAG_COLORSPACE =		0xA001,
		TAG_EXIF_IMAGEWIDTH =   0xA002,
		TAG_EXIF_IMAGELENGTH =  0xA003,
		TAG_RELATED_SOUND =		0xA004,
		TAG_INTEROP_OFFSET =	0xa005,

		TAG_FOCALPLANEXRES =    0xa20E,
		TAG_FOCALPLANEYRES =	0xA20F,		
		TAG_FOCALPLANEUNITS =   0xa210,
		TAG_SENSING_METHOD =	0xA217,
		TAG_FILE_SOURCE =		0xA300,
		TAG_SCENE_TYPE =		0xA301,
		TAG_CFA_PATTERN =		0xA302,

		TAG_CUST_RENDERED =		0xA401,
		TAG_EXPOSURE_MODE =		0xA402,
		TAG_WHITE_BALANCE =		0xA403,
		TAG_DIGITAL_ZOOM =		0xA404,
		TAG_FOCALLEN_35MM =		0xA405,
		TAG_SCENE_CAP_TYPE =	0xA406,
		TAG_GAIN_CONTROL =		0xA407,
		TAG_CONTRAST =			0xA408,
		TAG_SATURATION =		0xA409,
		TAG_SHARPNESS =			0xA40A,
		TAG_DISTANCE_RANGE =	0xA40C,
	
		// Nikon tags - for Nikon D100 
		NTAG_NIKON_VERSION =	0x0001,
		NTAG_ISO_EQUIVALENT =	0x0002,
		NTAG_COLOR_BW =			0x0003,
		NTAG_IMG_QUALITY =		0x0004,
		NTAG_WHITE_BAL =		0x0005,
		NTAG_SHARPENING =		0x0006,
		NTAG_FOCUS =			0x0007,
		NTAG_FLASH_MODE =		0x0008,
		NTAG_FLASH_METERING =	0x0009,
		NTAG_WHITE_BAL_ADJ =	0x000B,
		NTAG_ISO_SELECTION =	0x000F,
		NTAG_IMG_ADJUSTMENT =	0x0081,
		NTAG_LENS_ADAPTER =		0x0082,
		NTAG_LENS =				0x0084,
		NTAG_FOCUS_DIST =		0x0085,
		NTAG_DIGITAL_ZOOM =		0x0086,
		NTAG_FOCUS_POSIT =		0x0088,
		NTAG_FLASH_TYPE =		0x0090,
		NTAG_SATURATION =		0x0094,
		NTAG_NOISE_REDUCTION =	0x0095,
	} ;

	enum DataType
	{
		DTYPE_UINT8		=	1,		// an array of ASCII characters
		DTYPE_STRING	=	2,		// an ASCII string, 
		DTYPE_UINT16	=	3,		// a 16-bit unsigned short 
		DTYPE_UINT32	=	4,
		DTYPE_URATIONAL	=	5,
		DTYPE_INT8		=	6,
		DTYPE_SUNDEF	=	7,
		DTYPE_INT16		=	8,
		DTYPE_INT32		=	9,
		DTYPE_RATIONAL	=	10,
		DTYPE_SINGLE	=	11,
		DTYPE_DOUBLE	=	12,
	} ;


protected:
	bool				m_valid_exif ;				// ParseEXIF( ) understood the exif data 
	EXIFDirectory *		m_ifd ;
	
	// because Nikon comment fields contain their own TIFF context, with different offset values and conceivably different "endian-ness",
	// we need to preserve the base of the TIFF header for the enclosing TIFF directories, and whether they reverse bytes 
	bool		m_reverse_bytes ;			// true if MM format
	uchar *		m_tiff ;					// base of the TIFF header - always equal to the start of the "II" or "MM" 
	CPtrList	m_tiff_stack ;				// saves prior copy of reverse_bytes and tiff pointer 

protected:
	bool	ParseEXIF( ) ;

	EXIFDirectory * ParseNikon( uint32 start_of_tiff ) ;
	EXIFDirectory * ParseImageFileDir( uint32 start_ifd ) ;

	uint16	GetUint16At( const uchar * data, bool reverse_bytes ) const ;
	uint32	GetUint32At( const uchar * data, bool reverse_bytes ) const ;
	Rational GetRationalAt( const uchar * data, bool reverse_bytes ) const ;

	uint16	GetUint16At( int offset ) const ;
	uint32	GetUint32At( int offset ) const ;
	Rational GetRationalAt( int offset ) const ;

	void PushTiffContext( bool reverse_bytes, uchar * tiff ) ;
	void PopTiffContext( ) ;

	bool GetEXIFDirectoryItem( uint16 tag, uint16 & data_type, uint32 & data_count, bool & reverse, const uchar * & data_reference ) const ;
	bool GetEXIFDirectoryItem( const EXIFDirectory * dir, uint16 tag, uint16 & data_type, uint32 & data_count, bool & reverse, const uchar * & data_reference ) const ;
	const char * GetEXIFDirectoryItemString( uint16 tag ) const ;

	int DataSize( uint16 entry_type, uint32 entry_ct ) ;
	bool RecursiveSearchDirectory( int tag, const EXIFDirectoryItem * & dir_item ) const ;
	bool RecursiveSearchDirectory( const EXIFDirectory * dir, int tag, const EXIFDirectoryItem * & dir_item ) const ;


public:
	EXIFHeader( JPegSection & source ) ;
	EXIFHeader & operator=( JPegSection & source ) ;
	EXIFHeader & operator=( EXIFHeader & source ) ;
	
	~EXIFHeader( void ) ;

	const char * GetCameraMake( ) const ;
	const char * GetCameraModel( ) const ;
	
	virtual bool Write( CFile & dest_file ) ;
	
	void ShowIFD( const EXIFDirectory * dir, int level ) ;
	void ShowDirItemData( int dir_no, int level, const EXIFDirectoryItem & item ) ;
	virtual void DEBUG_DUMP( ) ;
	
	virtual bool GetCameraName( CString & name ) const ;
	virtual bool GetCreateTime( SYSTEMTIME & create_time ) const ;
	virtual bool GetExposure( Rational & exposure ) const ;
	virtual bool GetAperture( Rational & aperture ) const ;
	virtual bool GetFocalLength( Rational & focal_len ) const ;
	virtual bool GetLensFocalLength( CString & focal_len ) const ;
	virtual bool GetDescription( CString & description ) const ;
	virtual bool GetTitle( CString & title ) const ;
	virtual bool GetFlashUsed( bool & flash_used ) const ;
	virtual bool GetISO( int & iso ) const ;
	virtual bool GetHeight( int & height ) const ;
	virtual bool GetWidth( int & width ) const ;
};
