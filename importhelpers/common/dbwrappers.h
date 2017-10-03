/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




databaase wrapper classes. These just ensure consistent location and table definitions 
for all databases 
***************************************************************************************/

#ifndef _dbwrappers_h_
#define	_dbwrappers_h_



class LaptopDatabase : public SQLDatabase
{
public:
	bool Open( const TCHAR * location = NULL ) ;
	static bool GetLaptopDBPath( stringt & path ) ;
} ;




class PortableDatabase : public SQLDatabase
{

public:
	bool Open( const TCHAR * portable_path ) ;

} ;





class LightroomDatabase : public SQLDatabase
{
	stringt	m_db_path ;

public: 
	bool Open( const TCHAR * lightroom_path = NULL ) ;	
	void GetLightroomDBPath( stringt & lightroom_path ) ;

} ;





#endif