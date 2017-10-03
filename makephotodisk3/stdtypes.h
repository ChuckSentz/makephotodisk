/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



***************************************************************************************/

#pragma once


#ifdef WIN32
	typedef __int64 int64 ;
	typedef unsigned __int64 uint64 ;
	typedef int int32 ;
	typedef unsigned int uint32 ;
	typedef short int16 ;
	typedef unsigned short uint16 ;
	typedef char int8 ;
	typedef unsigned char uint8 ;
	typedef unsigned char byte ;
	typedef unsigned char uchar ;
#else
    typedef long int64 ;
	typedef unsigned long uint64 ;
    typedef int int32 ;
	typedef unsigned int uint32 ;
    typedef short int16 ;
	typedef unsigned short uint16 ;
	typedef char int8 ;
	typedef unsigned char uint8 ;
	typedef unsigned char byte ;
	typedef unsigned char uchar ;
#endif
