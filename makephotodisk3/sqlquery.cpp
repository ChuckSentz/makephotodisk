/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.




Implementation of the SQLite wrapper classes. SQLiteDB, SQLiteQuery, SQLiteException... 

***************************************************************************************/
#include "stdafx.h"
#include "SQLQuery.h"
// #include "sqlite3.h" - included by sqlquery.h




SQLiteDB::SQLiteDB( )
{
	m_db = NULL ;
	m_last_rc = SQLITE_OK ;
}

SQLiteDB::~SQLiteDB( )
{
	Close( ) ;
}





SQLiteException::SQLiteException( int native_error_code )
{
	m_native_code = native_error_code ;
	m_offer_retry = false ;
	m_text.Format( _T( "Unexpected SQLite error (code %d)" ), native_error_code ) ;

	switch ( m_native_code = native_error_code )
	{
		case SQLITE_ERROR :
				m_exception_code = SQLERR_Generic ;
				m_text = _T( "SQL error or missing database." ) ;
				break ;

		case SQLITE_INTERNAL :
				m_exception_code = SQLERR_Internal ;
				m_text = _T( "SQLite internal error." ) ;
				break ;

		case SQLITE_BUSY :
		case SQLITE_PERM :
				m_exception_code = SQLERR_Permission ;		// access permission denied 
				m_text = _T( "Access to the catalog has been denied. Check for another instance of this program or lightroom using the catalog." ) ;
				m_offer_retry = true ;
				break ;

		case SQLITE_LOCKED :
				m_exception_code = SQLERR_TableLocked ;		// an individual table locked??
				m_text = _T( "Access to an individual table has been denied. Check for an instance of lightroom using the catalog." ) ;
				m_offer_retry = true ;
				break ;

		case SQLITE_NOMEM :
				m_exception_code = SQLERR_OutOfMemory ;		// out of memory
				m_text = _T( "SQLite has run out of memory." ) ;
				break ;

		case SQLITE_READONLY :		/* Attempt to write a readonly database */
		case SQLITE_INTERRUPT :		/* Operation terminated by sqlite3_interrupt()*/
		case SQLITE_IOERR :			/* Some kind of disk I/O error occurred */
		case SQLITE_CORRUPT :		/* The database disk image is malformed */
		case SQLITE_NOTFOUND :		/* Unknown opcode in sqlite3_file_control() */
		case SQLITE_ABORT :			
		case SQLITE_FULL :			/* Insertion failed because database is full */
		case SQLITE_CANTOPEN :		/* Unable to open the database file */
		case SQLITE_PROTOCOL :		/* Database lock protocol error */
		case SQLITE_EMPTY :			/* Database is empty */
		case SQLITE_SCHEMA :
		case SQLITE_TOOBIG :
		case SQLITE_CONSTRAINT :
		case SQLITE_MISMATCH :
		case SQLITE_MISUSE :
		case SQLITE_NOLFS :
		case SQLITE_AUTH :
		case SQLITE_FORMAT :
		case SQLITE_RANGE :
		case SQLITE_NOTADB :
				m_exception_code = SQLERR_FatalError ;
				m_text.Format( _T( "A fatal database error (%d) has occurred." ), m_native_code ) ;
				break ;
	}
} 





bool SQLiteDB::Open( const TCHAR * path )
{
	if ( m_db )
		Close( ) ;		// allow consumer to re-use the same SQLiteDB for different db, or to retry Open after handling an exception 

	if ( path && *path ) 
	{
		if ( SQLITE_OK == ( m_last_rc = sqlite3_open16( path, &m_db ) ) )
			return true ;
		else 
			throw SQLiteException( m_last_rc ) ;
	}
	else
	{
		/* 
			we have to check for empty path b/c sqlite3 WILL accept (for reasons unknown) a filename which is an empty string. There is some discussion 
			in the sqlite documentation of accepting special db names (like ":memory"), but no explanation of what the sqlite library thinks it's going
			to do with an empty database name.... 
		*/ 
		ASSERT( FALSE ) ;
		return false ;
	}
}




void SQLiteDB::Close( )
{
	// sqlite3_close MUST be called, regardless of whether the open call succeeded. Hence, we call Close( ) 
	// from within the destructor, or allow the user to call Close( ) explicitly. But prevent 
	// redundant calls to sqlite3_close by testing m_db for NULL... 
	if ( m_db ) 
		sqlite3_close( m_db ) ;
	m_db = NULL ;
}





bool SQLiteDB::Prepare( const TCHAR * sql_text, sqlite3_stmt * & prepared_sql ) 
{
	const void *	tail ;
	int				sql_query_size ;

	sql_query_size = sizeof( TCHAR ) * ( 1 + _tcslen( sql_text ) ) ;
	
	if ( SQLITE_OK == ( m_last_rc = sqlite3_prepare16_v2( m_db, sql_text, sql_query_size, &prepared_sql, &tail ) ) )
		return true ;
	else 
		throw SQLiteException( m_last_rc ) ;
}		





bool SQLiteDB::Step( sqlite3_stmt * prepared_sql ) 
{
	m_last_rc = sqlite3_step( prepared_sql ) ;

	if ( SQLITE_ROW == m_last_rc )				// got some data 
		return true ;
	else if ( SQLITE_DONE == m_last_rc )		// no more data 
		return false ;
	else
		throw SQLiteException( m_last_rc ) ;	// got an error condition.... 
}












SQLiteQuery::SQLiteQuery( SQLiteDB & db )
	: m_db( db ) 
{
	m_prepared_sql = NULL ;
	m_row_ID = 0 ;
}




SQLiteQuery::SQLiteQuery( SQLiteDB & db, const TCHAR * sql_text )
	: m_db( db ) 
{
	SetSQLText( sql_text ) ;
	m_prepared_sql = NULL ;
}



SQLiteQuery::~SQLiteQuery(void)
{
	if ( m_prepared_sql ) 
		sqlite3_finalize( m_prepared_sql ) ;
}




bool SQLiteQuery::Execute( ) 
{
	bool isOK = false ;

	if ( m_prepared_sql ) 
		sqlite3_finalize( m_prepared_sql ) ;

	if ( m_db.Prepare( m_text, m_prepared_sql ) )
	{
		if ( !( isOK = NextRow( ) ) )
		{
			/*
				NextRow() returns true if the underlying sqlite call returns SQLITE_ROW. 
				But... SQL statements which do not return data will return SQLITE_DONE
				to indicate success. 
			*/
			if ( 0 != m_text.Left( 6 ).TrimLeft( ).CompareNoCase( _T( "SELECT" ) ) )
			{
				isOK = ( m_db.m_last_rc == SQLITE_DONE ) ;
			}
		}
	}

	return isOK ;
}




bool SQLiteQuery::NextRow( )
{
	return m_db.Step( m_prepared_sql ) ;
}



SQLiteQuery::DataType SQLiteQuery::GetColumnType( int column_no )
{
	ASSERT( SQLiteQuery::Integer == SQLITE_INTEGER ) ;
	ASSERT( SQLiteQuery::Float == SQLITE_FLOAT ) ;
	ASSERT( SQLiteQuery::Text == SQLITE_TEXT ) ;	
	ASSERT( SQLiteQuery::Blob == SQLITE_BLOB ) ; 
	ASSERT( SQLiteQuery::Null == SQLITE_NULL ) ; 

	return (SQLiteQuery::DataType) sqlite3_column_type( m_prepared_sql, column_no ) ;
}



SQLiteQuery::DataType SQLiteQuery::GetColumnType( const TCHAR * table_dot_field )
{
	int column_no ;

	if ( -1 != ( column_no = GetColumnNumber( table_dot_field ) ) )
		return GetColumnType( column_no ) ;

	return SQLiteQuery::NoType ;
}





bool SQLiteQuery::GetColumnText( CString & text, int column_no )
{
	int			bytes ;
	TCHAR *		ptr ;

	// sqlite3_column_text16 can return NULL, in which case the bytes is going to be 0 
	if ( bytes = sqlite3_column_bytes16( m_prepared_sql, column_no ) )
	{
		ptr = text.GetBufferSetLength( bytes / sizeof( TCHAR ) + 1 ) ;

		memcpy( ptr, sqlite3_column_text16( m_prepared_sql, column_no ), bytes ) ;
		ptr[ bytes / sizeof( TCHAR ) ] = _T( '\0' ) ;
		text.ReleaseBufferSetLength( bytes / sizeof( TCHAR ) ) ;
	}
	else
		text = _T( "" ) ;		// 0 bytes may mean we have a NULL field 

	return true ;
}



bool SQLiteQuery::GetColumnInt( int & i, int column_no )
{
	i = sqlite3_column_int( m_prepared_sql, column_no ) ;
	return true ;
}


bool SQLiteQuery::GetColumnFloat( double & f, int column_no )
{
	f = sqlite3_column_double( m_prepared_sql, column_no ) ;
	return true ;
}


bool SQLiteQuery::GetColumnBlob( byte ** blob, int & size, int column_no )
{
	// SQLIte3 helpfully converts data from any given type into the requested type...
	// this complicates the data size a bit - 
	switch ( sqlite3_column_type( m_prepared_sql, column_no ) )
	{
		case SQLITE_TEXT :
		case SQLITE_INTEGER : 
		case SQLITE_FLOAT :
			// these types will be returned as unicode strings - 
			if ( size = sqlite3_column_bytes16( m_prepared_sql, column_no ) )
			{
				*blob = new byte[ size ] ;
				memcpy( *blob, sqlite3_column_blob( m_prepared_sql, column_no ), size ) ;
			}
			else
				*blob = NULL ;
			break ;
		
		case SQLITE_BLOB :
			if ( size = sqlite3_column_bytes( m_prepared_sql, column_no ) )
			{
				*blob = new byte[ size + sizeof( TCHAR ) ] ;
				memcpy( *blob, sqlite3_column_text16( m_prepared_sql, column_no ), size ) ;
				( (TCHAR *)(*blob) ) [ size / sizeof( TCHAR ) ] = _T( '\0' ) ;
			}
			else
				*blob = NULL ;
			break ;

		case SQLITE_NULL :
			// this is just returned as a null pointer 
			*blob = NULL ;
			size = 0 ;
			break ;
	}
	return true ;
}




bool SQLiteQuery::GetColumnBlob( byte ** blob, int & size, const TCHAR * table_dot_field )
{
	int column_no ;

	if ( -1 != ( column_no = GetColumnNumber( table_dot_field ) ) )
		return GetColumnBlob( blob, size, column_no ) ;

	return false ;
}

bool SQLiteQuery::GetColumnFloat( double & f, const TCHAR * table_dot_field )
{
	int column_no ;

	if ( -1 != ( column_no = GetColumnNumber( table_dot_field ) ) )
		return GetColumnFloat( f, column_no ) ;

	return false ;
}



bool SQLiteQuery::GetColumnText( CString & text, const TCHAR * table_dot_field )
{
	int column_no ;

	if ( -1 != ( column_no = GetColumnNumber( table_dot_field ) ) )
		return GetColumnText( text, column_no ) ;

	return false ;
}


bool SQLiteQuery::GetColumnInt( int & i, const TCHAR * table_dot_field )
{
	int column_no ;

	if ( -1 != ( column_no = GetColumnNumber( table_dot_field ) ) )
		return GetColumnInt( i, column_no ) ;

	return false ;
}







void SQLiteQuery::SetSQLText( const TCHAR * sql_text )
{
	m_text = sql_text ;

	if ( m_prepared_sql ) 
		sqlite3_finalize( m_prepared_sql ) ;
	m_prepared_sql = NULL ;
	m_column.RemoveAll( ) ;
}





void SQLiteQuery::InitializeColumnArray( )
{
	int	col_no ;
	const TCHAR * table ;
	const TCHAR * field ;

	for ( col_no = 0 ; NULL != ( field = (const TCHAR *) sqlite3_column_origin_name16( m_prepared_sql, col_no ) ) ; col_no++ )
	{
		CString table_dot_field ;

		table = (const TCHAR *) sqlite3_column_table_name16( m_prepared_sql, col_no ) ;
		ASSERT( table != NULL ) ;

		table_dot_field.Format( _T( "%s.%s" ), table, field ) ;
		m_column.Add( table_dot_field ) ;
	}
}


// user can pass a fully qualified field designation, formatted as tablename.fieldname, or
// just pass a field_name - in which case the user should verify that field name is unique
// when using a join
int SQLiteQuery::GetColumnNumber( const TCHAR * table_dot_field ) 
{
	if ( !m_column.GetSize( ) ) 
		InitializeColumnArray( ) ;

	int	i ;

	if ( _tcschr( table_dot_field, _T( '.' ) ) )
	{
		for ( i = 0 ; i < m_column.GetSize( ) ; i++ )
			if ( 0 == m_column.GetAt( i ).CompareNoCase( table_dot_field ) )
				return i ;
	}
	else
	{
		for ( i = 0 ; i < m_column.GetSize( ) ; i++ )
			if ( 0 == m_column.GetAt( i ).Mid( m_column.GetAt( i ).Find( _T( '.' ) ) + 1 ).CompareNoCase( table_dot_field ) )
				return i ;
	}

	return -1 ;		// not found 
}





bool SQLiteQuery::Select( const TCHAR * field_list, const TCHAR * from_list, const TCHAR * where_clause /* =NULL */ )
{
	CString	sql_statement ;

	if ( where_clause ) 
		sql_statement.Format( _T( "SELECT %s FROM %s WHERE %s" ), field_list, from_list, where_clause ) ;
	else
		sql_statement.Format( _T( "SELECT %s FROM %s" ), field_list, from_list ) ;
	
	SetSQLText( sql_statement ) ;
	return Execute( ) ;
}




bool SQLiteQuery::Update( const TCHAR * table_name, const TCHAR * set_list, const TCHAR * where_clause /* =NULL */ )
{
	CString sql_statement ;

	if ( where_clause ) 
		sql_statement.Format( _T( "UPDATE %s SET %s WHERE %s" ), table_name, set_list, where_clause ) ;
	else
		sql_statement.Format( _T( "UPDATE %s SET %s" ), table_name, set_list ) ;

	SetSQLText( sql_statement ) ;
	return Execute( ) ;
}



bool SQLiteQuery::Insert( const TCHAR * table_name, const TCHAR * field_list, const TCHAR * value_list )
{
	CString sql_statement ;
	bool	isOK ;

	sql_statement.Format( _T( "INSERT INTO %s ( %s ) VALUES ( %s )" ), table_name, field_list, value_list ) ;
	SetSQLText( sql_statement ) ;
	
	isOK = Execute( ) ; 
	if ( isOK ) 
		m_row_ID = (int) sqlite3_last_insert_rowid( m_db.m_db ) ;		

	return isOK ;
}




bool SQLiteQuery::Delete( const TCHAR * table_name, const TCHAR * where_clause /* =NULL */ )
{
	CString	sql_statement ;

	if ( where_clause ) 
		sql_statement.Format( _T( "DELETE FROM %s WHERE %s" ), table_name, where_clause ) ;
	else
		sql_statement.Format( _T( "DELETE FROM %s" ), table_name ) ;

	SetSQLText( sql_statement ) ;
	return Execute( ) ;
}




bool SQLiteQuery::CreateTable( const TCHAR * table_name, const TCHAR * field_list )
{
	CString	sql_statement ;

	sql_statement.Format( _T( "CREATE TABLE %s ( %s ) " ), table_name, field_list ) ;

	SetSQLText( sql_statement ) ;

	return Execute( ) ;
}






void NormalizeFieldList( CString & field_list )
{
	field_list = field_list.Trim( ) ;

	int pos = 0 ;

	while ( pos < field_list.GetLength( ) )
	{
		if ( !isgraph( field_list.GetAt( pos ) ) ) 
			field_list.SetAt( pos, _T( ' ' ) ) ;
		pos++ ;
	}

	while ( 0 < field_list.Replace( _T( "  " ), _T( " " ) ) ) 
		;

	field_list = field_list.MakeLower( ) ;
}




bool SQLiteQuery::GuaranteeTable( const TCHAR * table_name, const TCHAR * field_list )
{
	CString where_clause ;

	where_clause.Format( _T( "type='table' AND name='%s'" ), table_name ) ;

	if ( !Select( _T( "*" ), _T( "sqlite_master" ), where_clause ) )
	{
		// table does not exist - create it from scratch 
		return CreateTable( table_name, field_list ) ;
	}
	else
	{
		CString	new_field_list ;
		CString existing_field_list ;

		new_field_list = field_list ;

		if ( GetColumnText( existing_field_list, _T( "sql" ) ) )
		{
			existing_field_list = existing_field_list.Mid( 1 + existing_field_list.Find( _T( '(' ) ) ) ;
			existing_field_list = existing_field_list.Left( existing_field_list.Find( _T( ')' ) ) ) ;

			// remove extraneous white space & make everything lowercase.... 
			NormalizeFieldList( existing_field_list ) ;
			NormalizeFieldList( new_field_list ) ;

			return 0 == existing_field_list.Compare( new_field_list ) ;
		}
		else
			return false ;
	}
}







bool SQLiteQuery::DropTable( const TCHAR * table_name )
{
	CString	sql_statement ;

	sql_statement.Format( _T( "DROP TABLE %s" ), table_name ) ;

	SetSQLText( sql_statement ) ;

	return Execute( ) ;
}






int SQLiteQuery::GetInsertRowID( )		// only valid after INSERT 
{
	return m_row_ID ;
}

