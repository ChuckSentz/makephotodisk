/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



	This is the main implementation of a DLL compiled with CLR. This DLL is able to call
	the C# wrapper DLL used to interface with Photoshop, while permitting the main application
	to be compiled and run in native C++ 

	We use a technique described at http://pragmateek.com/using-c-from-native-c-with-the-help-of-ccli-v2/

***************************************************************************************/

#include "stdafx.h"

using namespace System;

#include "CLRPhotoshopWrapper.h"

#ifdef _DEBUG
#using "..\Debug\CSharpPhotoshopWrapper.dll"
#else
#using "..\Release\CSharpPhotoshopWrapper.dll"
#endif

#include <msclr\auto_gcroot.h>


using namespace System::Runtime::InteropServices ;		// Marshall 



class PrivateAPIWrapper
{
	public:	msclr::auto_gcroot< CSharpPhotoshopWrapper^ > api ;
} ;


PhotoshopWrapper::PhotoshopWrapper( )
{
	m_csdll = new PrivateAPIWrapper( ) ;
	m_csdll->api = gcnew CSharpPhotoshopWrapper( ) ;
} 

PhotoshopWrapper::~PhotoshopWrapper( ) 
{
	delete m_csdll ;
} 

bool PhotoshopWrapper::SetDiskRoot( const TCHAR * disk_root ) 
{
	return m_csdll->api->SetDiskRoot( gcnew System::String( disk_root ) ) ;
}


bool PhotoshopWrapper::LoadAndPrepareSourceFile( const TCHAR * source_file, const TCHAR * dest_filename ) 
{
	return m_csdll->api->LoadAndPrepareSourceFile( gcnew System::String( source_file ), gcnew System::String( dest_filename ) ) ;
} 

bool PhotoshopWrapper::OutputJpeg( int output_type, int * width, int * height )
{
	bool isOK ;
	int _width ;
	int _height  ;

	isOK = m_csdll->api->OutputJpeg( (JpegType) output_type, _width, _height ) ;

	if ( width ) 
		*width = _width ;
	
	if ( height ) 
		*height = _height ;

	return isOK ;
}  

bool PhotoshopWrapper::UnloadSourceFile( ) 
{
	return m_csdll->api->UnloadSourceFile( ) ;
}  
	
bool PhotoshopWrapper::QuitApp( )
{
	return m_csdll->api->QuitApp( ) ;
}  
 
int PhotoshopWrapper::GetErrorCode( )
{
	return m_csdll->api->GetErrorCode( ) ;
}  
