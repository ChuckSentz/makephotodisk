/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   PORTED FROM MakePhotoDisk2

   Rev 1.1   20 Oct 2007 22:48:04   CSentz(desktop)
Fixed ExifDigest constructor to initialize m_create_time member

   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/
#include "StdAfx.h"
#include "exifdigest.h"
#include "JpegSection.h"
#include "ExifHeader.h"




EXIFDigest::EXIFDigest( const TCHAR * filename )
{
	m_filename = filename ;
	memset( &m_create_time, 0xCC, sizeof( &m_create_time ) ) ;

	m_iso = 0 ;
	m_flash_used = false ;
	m_width = 0 ;
	m_height = 0 ;

	ParseEXIF( ) ;	
}

EXIFDigest::~EXIFDigest(void)
{
}


void EXIFDigest::ParseEXIF( ) 
{
	CFile	file ;
	bool	isOK = false ;
	
	if ( file.Open( m_filename, CFile::modeRead ) )
	{
		uchar	sig[ 2 ] ;

		if ( sizeof( sig ) == file.Read( sig, sizeof( sig ) ) )
		{
			if ( sig[ 0 ] == 0xFF && sig[ 1 ] == JPegSection::StartOfImage ) 
			{
				JPegSection	*	jpg_section ;
				CTypedPtrArray< CPtrArray, EXIFHeader *>	sections ;
				int				i ;
				
				do
				{
					jpg_section = NULL ;

					try
					{	
						jpg_section = new JPegSection ;
						isOK = jpg_section->Read( file ) ;

						if ( jpg_section->IsEXIF( ) )
						{
							EXIFHeader * exif_section = new EXIFHeader( *jpg_section ) ;

							sections.Add( exif_section ) ;
						}
						delete jpg_section ;
					}
					catch ( ... )
					{
						isOK = false ;
					}
				}
				while ( isOK && !jpg_section->IsStartOfScan( ) ) ;

				GetCameraName( sections ) ;
				GetCreateDateTime( sections ) ;
				GetTitle( sections ) ;
				GetDescription( sections ) ;

				GetAperture( sections ) ;
				GetFocalLength( sections ) ;
				GetExposure( sections ) ;

				GetLens( sections ) ;
				GetISO( sections ) ;
				GetFlashUsed( sections ) ;
	
				GetWidth( sections ) ;
				GetHeight( sections ) ;
				
				for ( i = 0 ; i < sections.GetCount( ) ; i++ )
					delete sections.GetAt( i ) ;
			}
		}

		file.Close( ) ;
	}
}




void EXIFDigest::GetCameraName( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	 i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetCameraName( m_camera ) )
			return ;
}



void EXIFDigest::GetCreateDateTime( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetCreateTime( m_create_time ) )
			return ;
}


void EXIFDigest::GetTitle( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetTitle( m_title ) )
			return ;
}

void EXIFDigest::GetDescription( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetDescription( m_description ) ) 
			return ;
}


void EXIFDigest::GetAperture( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetAperture( m_aperture ) )
			return ;
}



void EXIFDigest::GetFocalLength( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetFocalLength( m_focal_length ) ) 
			return ;
}

void EXIFDigest::GetExposure( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetExposure( m_exposure ) )
			return ;
}


void EXIFDigest::GetLens( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetLensFocalLength( m_lens ) )
			return ;
}

void EXIFDigest::GetISO( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetISO( m_iso ) ) 
			return ;
}

void EXIFDigest::GetFlashUsed( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetFlashUsed( m_flash_used ) )
			return ;
}


void EXIFDigest::GetWidth( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetWidth( m_width ) )
			return ;
}

void EXIFDigest::GetHeight( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections )
{
	int	i ;
	for ( i = 0 ; i < sections.GetCount( ) ; i++ )
		if ( sections.GetAt( i )->GetHeight( m_height ) ) 
			return ;
}







