/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Classes which encapsulate SQLite3 strangeness
***************************************************************************************/

#pragma once

#include <string.h>
#include "sqlite3.h"
#include "stdtypes.h"

#if defined( WIN32 ) && !defined( USE_TSTRING ) 

	#include "stdafx.h"
	#define stringt		CString

#else
	#include "tstring.h"

	#define stringt		tstring

#endif


class SQLDatabase ;
class SQLCursor ;



/******************************************************************************
SQLDatabaseException

	As the name implies, this is the exception thrown by the database. The 
	database classes throw exceptions for catastrophic failures (that is, 
	failures where there's really no point in trying to carry on)

******************************************************************************/
class SQLDatabaseException
{
public:
	enum ExceptionCode
	{
		GenericError	=	1,	
		InternalError,
		Permission,
		DatabaseBusy,
		TableLocked,
		OutOfMemory,
		UnexpectedError
	} ;

protected:
	int				m_native_error ;
	int				m_sub_error ;
	stringt 		m_error_text ;
	stringt			m_sql_text ;
	const char *	m_filename ;
	int				m_line_no ;
	bool			m_offer_retry ;

public:
	SQLDatabaseException( int native_error, int sub_error, const TCHAR * sql_text, const TCHAR * err_msg, const char * file, int line_no ) ;

	inline int GetNativeError( ) const
	{
		return m_native_error ;
	} ;

	inline int GetSubError( ) const
	{
		return m_sub_error ;
	} ;

	inline const TCHAR * GetErrorText( ) const
	{
		return m_error_text ;
	} ;

	inline const TCHAR * GetSQLText( ) const
	{
		return m_sql_text ;
	} ;

	inline const char * GetFileName( ) const
	{
		return m_filename ;
	} ;

	inline int GetLineNo( ) const
	{
		return m_line_no ;
	} ;

	inline bool RetryPossible( ) const
	{
		return m_offer_retry ;
	} ;
} ;






/******************************************************************************
SQLDatabase

	The main database wrapper. To use a SQL database: 

	1. instantiate a SQLDatabase object 
	2. Open a database specified by its file path
	3. perform SELECT queries by calling SQLDatabase::ExecuteQuery
	4. perform other SQL actions on the database through SQLDatabase::ExecuteSQL

******************************************************************************/
class SQLDatabase
{
	friend SQLCursor ;

	sqlite3 *	m_db ;
	int			m_last_rc ;
	stringt	*	m_table_name ;
	stringt		m_database_name ;
	int			m_table_count ;
//	int			m_attach_count ;

	bool Prepare( const TCHAR * sql_text, sqlite3_stmt * & prepared_sql ) ;
	bool NextRow( sqlite3_stmt * prepared_sql ) ;
	bool Step( sqlite3_stmt * prepared_sql ) ;
	bool TableExists( const TCHAR * table_name ) ;
	bool TableDefinitionsMatch( const TCHAR * table_name, const TCHAR * definition ) ;
	stringt NormalizeDefinition( stringt definition ) ;
	bool GetColumnTextByIndex( sqlite3_stmt * prepared_sql, int index, stringt & value ) ;

public:
	enum FieldType
	{
		TypeInteger = 1,
		TypeFloat = 2, 
		TypeText = 3,
		TypeBlob = 4,
		TypeNULL = 5,
		UnknownType = 666													// should never see this one, unless a disaster of biblical proportions has occurred in the code... or we're trying to link with a new version of sqlite
	} ;

public:
	SQLDatabase( ) ;
	~SQLDatabase( ) ;

	bool Open( const TCHAR * path ) ;
	void Close( ) ;

	// working with tables - 
	bool GetTableName( stringt & table_name, int & table_index ) ;				// can step through the list of tables
	bool GetTableDefinition( const TCHAR * table_name, stringt & definition ) ;		// gets the sql-style definition of a table's fields 
	bool CreateTable( const TCHAR * name, const TCHAR * definition ) ;
	bool DropTable( const TCHAR * name ) ;
	int GetTableCount( ) ;
	int GetLastInsertRowID( ) ;
	bool ExecuteQuery( SQLCursor &sql_cursor, const TCHAR * sql_format, ... ) ;		// can use for SELECT queries - which let you then Step through a recordset
	bool ExecuteSQL( const TCHAR * sql_format, ... ) ;								// can use for UPDATE, DELETE, etc - which either succeed or fail 
	// bool Attach( SQLDatabase & attachee, const TCHAR * alias = NULL ) ;
	bool Attach( const TCHAR * db_path, const TCHAR * alias ) ;
} ;










/******************************************************************************
SQLCursor

	SQLDatabase::ExecuteQuery returns a poitner to a SQLCursor. The db client
	can then extract column values by SQLCursor::GetFieldValue

******************************************************************************/
class SQLCursor
{
	friend SQLDatabase ;

	SQLDatabase *		m_db ;
	sqlite3_stmt *		m_prepared_sql ;
	stringt				m_column_def ;
	stringt				m_sql_text ;

	// stringt				m_select_from ;
	// stringt				m_select_where ;

	void InitializeColumnDef( ) ;
	int GetColumnNumber( const TCHAR * field_name ) ;

	void InitializeQuery( SQLDatabase & db, const TCHAR * sql_select_text ) ;
	void DiscardQuery( ) ;

	SQLCursor & operator=( const SQLCursor & other ) 
	{
        return *this ;
	} ;

	SQLCursor( SQLCursor & other ) 
	{
	} ;

public:
	SQLCursor( ) ;
	~SQLCursor( ) ;
	
	bool StepNext( ) ;

	bool GetFieldValue( const TCHAR * column_name, stringt & text ) ;
	bool GetFieldValue( const TCHAR * column_name, int & int_value ) ;
	bool GetFieldValue( const TCHAR * column_name, uint64 & uint64_value ) ;
	bool GetFieldValue( const TCHAR * column_name, double & float_value ) ;
	bool GetFieldValue( const TCHAR * column_name, int * blob_size, void * blob_buffer = NULL ) ;

	bool GetFieldValue( int column_pos, stringt & text ) ;
	bool GetFieldValue( int column_pos, int & int_value ) ;
	bool GetFieldValue( int column_pos, uint64 & uint64_value ) ;
	bool GetFieldValue( int column_pos, double & float_value ) ;
	bool GetFieldValue( int column_pos, int * blob_size, void * blob_buffer = NULL ) ;

	bool GetFieldName( int column_pos, stringt & field_name ) ;

	int GetDataType( const TCHAR * column_name ) ;
	int GetDataType( int column_index ) ;

	int GetDataSize( const TCHAR * column_name ) ;
	int GetDataSize( int column_index ) ;

	int GetRecordCount( ) ;

	int GetFieldCount( ) ;
	stringt GetFieldName( int field_no ) ;

	bool ColumnIsNull( const TCHAR * column_name ) ;
	bool ColumnIsNull( int column_index ) ;

	SQLDatabase::FieldType GetFieldType( const TCHAR * column_name ) ;
	SQLDatabase::FieldType GetFieldType( int column_no ) ;
} ;


