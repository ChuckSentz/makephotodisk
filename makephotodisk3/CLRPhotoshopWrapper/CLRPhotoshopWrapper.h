/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


	Header for CLR wrapper DLL. 

	The calling app must instantiate an instance of PhotoshopWrapper, then...

	Call SetDiskRoot( ) 1x to tell the wrapper where the root of the disk project is 

	Perform the following 3 steps for each source file 

		1.  Call LoadAndPrepareSourceFile, passing the fully qualified pathname of the source .psd file
			and the filename for the destination (something like 1.jpg, 18.jpg, etc)

		2.	Call OutputJpeg( ) for each type of jpeg to output - eg, Thumbnail, Presentation, etc.

		3.	Call UnloadSourceFile( ) 

	Then call QuitApp( ) when done

***************************************************************************************/
// CLRPhotoshopWrapper.h

#pragma once


    // need to ensure these match the enums in PSWrapper.Class1.cs
enum ERROR_CODE
{
	NoError = 0,
	PhotoshopNotFound,					// unable to load Photoshop application
	NoDiskRootSet,						// need to specify the disk root directory before calling OutputJpeg 
	NoActiveSourceDoc,					// need to call LoadAndPrepareSourceFile before calling OutputJpeg
	AlreadyHaveActiveSourceDoc,			// calling LoadAndPrepareSourceFile with an active source file (need to call UnloadSourceFile)
	IllegalCall,						// generic error 
} ;

    // need to ensure these match the enums in PSWrapper.Class1.cs
enum JPEG_TYPE
{
	Thumbnail = 0,
	Presentation,
	MiniPresentation, 
	FullSize,
	Facebook,
	Compressed
} ;



class PrivateAPIWrapper ;

class __declspec( dllexport ) PhotoshopWrapper
{
private: 
	PrivateAPIWrapper * m_csdll ;

public:
	PhotoshopWrapper( ) ;
	~PhotoshopWrapper( ) ;

	bool LoadAndPrepareSourceFile( const TCHAR * source_file, const TCHAR * dest_rootname ) ;
	bool SetDiskRoot( const TCHAR * disk_root ) ;
	bool OutputJpeg( int output_type, int * width = NULL, int * height = NULL ) ;
	bool UnloadSourceFile( ) ;
	bool QuitApp( ) ;
    int GetErrorCode( ) ;
} ;