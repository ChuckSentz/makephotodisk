/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

// #include "JpegFile.h"

class JPegSection
{
public:
	enum	// JPEG section ID's 
	{
		StartFrameC0	= 0xC0,		// Start Of Frame N
		StartFrameC1	= 0xC1,		// N indicates which compression process
		StartFrameC2	= 0xC2,		// Only StartFrameC0-StartFrameC2 are now in common use
		StartFrameC3	= 0xC3,	
									// C4 is not a StartFrame marker 
		StartFrame5		= 0xC5,
		StartFrame6  	= 0xC6,
		StartFrame7  	= 0xC7,
		StartFrame9  	= 0xC9,
		StartFrame10 	= 0xCA,
		StartFrame11 	= 0xCB,
									// CC is not a StartFrame marker 
		StartFrame13 	= 0xCD,
		StartFrame14 	= 0xCE,
		StartFrame15 	= 0xCF,
		
		StartOfImage	= 0xD8,		// Start Of Image (beginning of datastream)
		EndOfImage		= 0xD9,		// End Of Image (end of datastream)
		StartOfScan		= 0xDA,		// Start Of Scan (begins compressed data)
		JFIFData  		= 0xE0,		// Jfif marker
		EXIFData  		= 0xE1,		// Exif marker
		Comment   		= 0xFE,		// Comment 
	} ;


protected:
									// a section consists of...
	int			m_padding ;			// 'm_padding' 0xFF's in a row
	uchar		m_type ;			// a 1-byte type code for the section 
	uint16		m_length ;			// a 16-bit length for the section - always in Motorola byte order
	uchar *		m_data ;			// 'length' bytes of data 

public:
	JPegSection( JPegSection & source ) ;
	JPegSection(void);
	virtual ~JPegSection(void);
	JPegSection & operator=( JPegSection & source ) ;
	
	inline bool IsEXIF( ) const 
	{
		return m_type == EXIFData ;
	} ;
	
	inline bool IsEOF( ) const
	{
		return m_type == EndOfImage ;
	} ;
	
	inline bool IsStartOfScan( ) const
	{
		return m_type == StartOfScan ;
	} ;
	
	virtual bool Read( CFile & source_file ) ;
	virtual bool Write( CFile & dest_file ) ;
	
	inline virtual void DEBUG_DUMP( )
	{
		return ;
	} ;
	
	inline virtual bool GetCameraName( CString & name ) const
	{
		return false ;
	} ;
	
	inline virtual bool GetCreateDate( CString & date ) const
	{
		return false ;
	} ;
	
	inline virtual bool GetCreateTime( CString & time ) const
	{
		return false ;
	} ;
	
	inline virtual bool GetExposure( CString & exposure ) const
	{
		return false ;
	} ;
	
	inline virtual bool GetAperture( CString & aperture ) const
	{
		return false ;
	} ;
	
	inline virtual bool GetFocalLength( CString & focal_len ) const
	{
		return false ;
	} ;
	
	inline virtual bool GetZoomLens( bool & zoom_lens ) const 
	{
		return false ;
	} ;
	
	inline virtual bool GetLensFocalLength( CString & focal_len ) const 
	{
		return false ;
	} ;
};
