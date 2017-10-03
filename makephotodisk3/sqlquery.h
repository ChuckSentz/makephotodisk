/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




Wrapper for sqlite3 database API's. To use a SQLIte3 database, do the following: 

Instantiate a SQLiteDB object, and use SQLiteDB::Open to open the required db. You do not
need to close the database object - the connection is closed and cleaned up within the 
class destructor. 

To query the database, instantiate a SQLiteQuery object and use SetSQLText() to to set
the SQL query to run. Then call SQLiteQUery::Execute(). If this function returns true, 
you can then retrieve individual columns. You can either retrieve columns by column 
number or by table and field name using dot notation, eg "tablename.fieldname". 

***************************************************************************************/
#pragma once

/*lint -save -e*     don't want any lint doing any error-checking for this 3rd party file */
#include "sqlite3.h"
/*lint -restore */


class SQLiteQuery ;

class SQLiteDB
{
	friend SQLiteQuery ;

	sqlite3 *	m_db ;
	int			m_last_rc ;

	bool Prepare( const TCHAR * sql_text, sqlite3_stmt * & prepared_sql ) ;
	bool NextRow( sqlite3_stmt * prepared_sql ) ;
	bool Step( sqlite3_stmt * prepared_sql ) ;

public:
	SQLiteDB( ) ;
	~SQLiteDB( ) ;

	bool Open( const TCHAR * path ) ;
	void Close( ) ;
} ;











class SQLiteQuery
{

public:
	enum DataType
	{
		NoType	=	0,
		Integer	=	1,
		Float	=	2,
		Text	=	3,
		Blob	=	4,
		Null	=	5, 
	} ;

protected:
	SQLiteDB &		m_db ;
	CString			m_text ;
	sqlite3_stmt *	m_prepared_sql ;
	CStringArray	m_column ;
	int				m_row_ID ;

	void InitializeColumnArray( ) ;
	int GetColumnNumber( const TCHAR * table_dot_field ) ;

public:
	SQLiteQuery( SQLiteDB & db ) ;
	SQLiteQuery( SQLiteDB & db, const TCHAR * sql_text ) ;

	~SQLiteQuery(void);

	void SetSQLText( const TCHAR * sql_text ) ;
	bool Execute( ) ;
	bool NextRow( ) ;

	bool Select( const TCHAR * field_list, const TCHAR * from_list, const TCHAR * where_clause = NULL ) ;
	bool Update( const TCHAR * table_name, const TCHAR * set_list, const TCHAR * where_clause = NULL ) ;
	bool Insert( const TCHAR * table_name, const TCHAR * field_list, const TCHAR * value_list ) ;
	bool Delete( const TCHAR * table_name, const TCHAR * where_clause = NULL ) ;
	bool CreateTable( const TCHAR * table_name, const TCHAR * field_list ) ;
	bool GuaranteeTable( const TCHAR * table_name, const TCHAR * field_list ) ;
	bool DropTable( const TCHAR * table_name ) ;

	int GetInsertRowID( ) ;		// only valid after INSERT 

	DataType GetColumnType( int column_no ) ;
	DataType GetColumnType( const TCHAR * table_dot_field ) ;

	bool GetColumnText( CString & data, int column_no ) ;
	bool GetColumnText( CString & data, const TCHAR * table_dot_field ) ;

	bool GetColumnInt( int & i, int column_no ) ;
	bool GetColumnInt( int & i, const TCHAR * table_dot_field ) ;

	bool GetColumnFloat( double & data, int column_no ) ;
	bool GetColumnFloat( double & data, const TCHAR * table_dot_field ) ;

	bool GetColumnBlob( byte ** blob, int & size, int column_no ) ;
	bool GetColumnBlob( byte ** blob, int & size, const TCHAR * table_dot_field ) ;
};









class SQLiteException
{
	enum 
	{
		SQLERR_Generic =	1,	// SQL error or missing db
		SQLERR_Internal,		// Internal logic error in SQLite
		SQLERR_Permission,		// access permission denied 
		SQLERR_Busy,			// db is locked by another user
		SQLERR_TableLocked,		// an individual table locked??
		SQLERR_OutOfMemory,		// out of memory
		SQLERR_FatalError,		// any of a number of catastrophic failures 
	}
	m_exception_code ;
	int			m_native_code ;
	CString		m_text ;
	bool		m_offer_retry ;

public:
	SQLiteException( int native_error_code ) ;

	inline int GetExceptionCode( ) const 
	{
		return m_exception_code ;
	} ;

	inline int GetNativeError( ) const
	{
		return m_native_code ;
	} ;

	inline const TCHAR * GetErrorText( ) const
	{
		return m_text ;
	} ;

	inline bool RetryPossible( ) const
	{
		return m_offer_retry ;
	} ;
} ;

