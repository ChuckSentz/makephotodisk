/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


Classes which encapsulate SQLite3 strangeness
***************************************************************************************/
#include <stdarg.h>
#include <string.h>
#include "SQLiteDB.h"



#ifndef ASSERT

// define ASSERT and VERIFY for APPLE 

#ifdef DEBUG

#define	ASSERT( x )		if ( !(x) ) \
						{	\
							fprintf( stderr, "Assertion failed, %s, in %s, line %d\n", #x, __FILE__, __LINE__ ) ;	\
							exit( -1 ) ;		\
						} ;



#define VERIFY( x )		if ( !(x) ) \
						{	\
							fprintf( stderr, "VERIFY failed, %s, in %s, line %d\n", #x, __FILE__, __LINE__ ) ;	\
							exit( -1 ) ;		\
						} ;

#else

#define ASSERT( x )		
#define VERIFY( x )		x


#endif // DEBUG

#endif	// ifndef ASSERT


#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif




// map some sqlite3 api's based on whether we're compiled for UNICODE or not 
#ifdef UNICODE

	#define SQLite3Open					sqlite3_open16
	#define	SQLite3Prepare				sqlite3_prepare16_v2
	#define SQLite3ColumnBytes			sqlite3_column_bytes16
	#define SQLite3ColumnText			sqlite3_column_text16
	#define SQLite3ColumnOriginName		sqlite3_column_origin_name16
	#define SQLite3ColumnTableName		sqlite3_column_table_name16
	#define SQLiteErrMessage			sqlite3_errmsg16

#else

	#define SQLite3Open					sqlite3_open
	#define SQLite3Prepare				sqlite3_prepare_v2
	#define SQLite3ColumnBytes			sqlite3_column_bytes
	#define SQLite3ColumnText			sqlite3_column_text
	#define SQLite3ColumnOriginName		sqlite3_column_origin_name
	#define SQLite3ColumnTableName		sqlite3_column_table_name
	#define SQLiteErrMessage			sqlite3_errmsg

#endif // UNICODE

// just for consistency - map other sqlite3 api's, too
#define SQLite3Close		sqlite3_close
#define SQLite3Finalize		sqlite3_finalize
#define SQLite3Step			sqlite3_step
#define SQLiteColumnInt		sqlite3_column_int
#define SQLiteColumnInt64	sqlite3_column_int64
#define SQLiteColumnType	sqlite3_column_type
#define SQLiteColumnDouble	sqlite3_column_double
#define SQLiteColumnBlob	sqlite3_column_blob 
#define SQLiteExtendedErr	sqlite3_extended_errcode




/******************************************************************************
SQLDatabase::SQLDatabase

Simple-minded constructor

******************************************************************************/
SQLDatabase::SQLDatabase( )
{
	m_db = NULL ;
	m_last_rc = SQLITE_OK ;
	m_table_name = NULL ;
	m_table_count = 0 ;
//	m_attach_count = 0 ;
}


SQLDatabase::~SQLDatabase( )
{
	Close( ) ;
}







/******************************************************************************
SQLDatabase::Open

returns true if successful, false if not. If the SQLDatabase object is already 
attached to an open db, it will be closed first. 

******************************************************************************/
bool SQLDatabase::Open( 
	const TCHAR * path		// I - database filename. 
)
{
	if ( m_db ) 
	{
		Close( ) ;
		m_database_name.Empty( ) ;
	}

	if ( path && *path ) 
	{
		m_last_rc = SQLite3Open( path, &m_db ) ;

		if ( SQLITE_OK == m_last_rc ) 
		{
			m_database_name = path ;
			return true ;
		}
		else 
		{
			stringt	faux_sql ;

			faux_sql.Format( _T( "open file: \"%s\"" ), path ) ;

			throw SQLDatabaseException( m_last_rc, SQLiteExtendedErr( m_db ),  (const TCHAR *) faux_sql, (const TCHAR *) SQLiteErrMessage( m_db ), __FILE__, __LINE__ ) ;
		}
	}

	return false ;
}








/******************************************************************************
SQLDatabase::Close

******************************************************************************/
void SQLDatabase::Close( ) 
{
	if ( m_db ) 
		SQLite3Close( m_db ) ;

	m_db = NULL ;

	if ( m_table_count ) 
	{
		delete [] m_table_name ;
		m_table_name = NULL ;
		m_table_count = 0 ;
	}
}




#ifdef OBSOLETE_CODE

/************************************************************************************
SQLDatabase::Attach

Encapsulates the ATTACH syntax. The SQLite API requires an alias for the attached 
database, so we generate an alias if SQLDatabase client doesn't supply one

************************************************************************************/
bool SQLDatabase::Attach( 
	SQLDatabase & attachee,		// I - SQLDatabase to attach to 
	const TCHAR * alias			// I - optional name of aslias. defaults to =NULL
)
{
	stringt	sql_text ;
	stringt	default_alias ;

	if ( !alias || !*alias ) 
	{
		default_alias.Format( _T( "Attachee%d" ), ++m_attach_count ) ;
		alias = default_alias ;
	}

	sql_text.Format( _T( "ATTACH '%s' AS %s" ), (const TCHAR *) attachee.m_database_name, alias ) ;

	return ExecuteSQL( sql_text ) ;
}

#endif



/************************************************************************************
SQLDatabase::Attach

Wraps the Attach method. The old method required the user to Open both databases
first, and only then Attach one to the other. The alias was auto-generated and 
then discarded. This, it turns out, was a "clever" idea which was actually 
pretty stupid..... 

We now require the user to supply the path-name and an alias for the database. The user
needs to use an alias to disambiguate table references when both databases contain
tables with the same names... Use dot-notation with the alias name prefixed to the
table name, eg, "alias1.mytable" v. "alias2.mytable" 

This method also allows you to use Attach in lieu of Open, so you can specify aliases 
for both databases, which is also necessary when both datbases contain identically 
named tables. 

************************************************************************************/
bool SQLDatabase::Attach( 
	const TCHAR * database_path,	// I - SQLite database to attach to 
	const TCHAR * alias				// I - alias (see comment) 
)
{
	stringt	sql_text ;

	sql_text.Format( _T( "ATTACH '%s' AS %s" ), database_path, alias ) ;

	return ExecuteSQL( sql_text ) ;
}









/******************************************************************************
SQLDatabaseException::SQLDatabaseException

	Construct a database construction to be thrown... 

	typically thrown with code like so:

		throw SQLDatabaseException( m_last_rc, SQLiteExtendedErr( m_db ), sql_text, (const TCHAR *) SQLiteErrMessage( m_db ), __FILE__, __LINE__ ) ;

******************************************************************************/
SQLDatabaseException::SQLDatabaseException( 
	int native_error,			// I - the SQLite error code 
	int sub_error,				// I - the SQLite sub error, or 0 
	const TCHAR * sql_text,		// I - the full sql text that threw the error, if available 
	const TCHAR * err_msg,		// I - the error message from SQLite 
	const char * file,			// I - the file where the exception was thrown (from the __FILE__ macro) 
	int line_no					// I - the line number where the exception was thrown (from the __LINE__ macro) 
)
{
	m_native_error = native_error ;
	m_sub_error = sub_error ;
	m_filename = file ;
	m_line_no = line_no ;
	m_offer_retry = false ;
	m_error_text = err_msg ;
	m_sql_text = sql_text ;
}










/******************************************************************************
SQLDatabase::Prepare

PRIVATE member encapsulating SQLite prepare call 
	
******************************************************************************/
bool SQLDatabase::Prepare( 
	const TCHAR * sql_text,				// I - SQL statement to 
	sqlite3_stmt * & prepared_sql		// O - SQLite's private "prepared SQL" format 
)
{
#ifdef UNICODE
	const void *	tail ;		// UNICDODE version of prepare takes a 'void *'
#else
	const char *	tail ;		// non-UNICODE version of prepare takes a 'char *'
#endif

	int				sql_query_size ;

	sql_query_size = (int) ( sizeof( TCHAR ) * ( 1 + _tcslen( sql_text ) ) ) ;

	if ( SQLITE_OK == ( m_last_rc = SQLite3Prepare( m_db, sql_text, sql_query_size, &prepared_sql, &tail ) ) )
		return true ;
	else
		throw SQLDatabaseException( m_last_rc, SQLiteExtendedErr( m_db ), sql_text, (const TCHAR *) SQLiteErrMessage( m_db ), __FILE__, __LINE__ ) ;
}








/******************************************************************************
SQLDatabase::Step

PRIVATE member encapsulating SQLite step call. client should call StepNext	

******************************************************************************/
bool SQLDatabase::Step( 
	sqlite3_stmt * prepared_sql			// I/O - SQLite's private "prepared SQL" thing 
)
{
	m_last_rc = SQLite3Step( prepared_sql ) ;

	if ( SQLITE_ROW == m_last_rc )
		return true ;
	else if ( SQLITE_DONE == m_last_rc ) 
		return false ;
	else
		throw SQLDatabaseException( m_last_rc, SQLiteExtendedErr( m_db ), _T( "sql text unavailable" ), (const TCHAR *) SQLiteErrMessage( m_db ), __FILE__, __LINE__ ) ;
}








/******************************************************************************
SQLDatabase::CreateTable

	Create a table, or if it already exists, ensure its definition matches the
	definition passed in. 

******************************************************************************/
bool SQLDatabase::CreateTable( 
	const TCHAR * name,					// I - table name 
	const TCHAR * definition			// I - SQL definition of the table 
)
{
	bool isOK = false ;

	if ( TableExists( name ) )
	{
		if ( TableDefinitionsMatch( name, definition ) )
			isOK = true ;
	}
	else
	{
		sqlite3_stmt *	create_statement = NULL ;
		stringt			text ;

		text.Format( _T( "CREATE TABLE %s ( %s ) ;" ), name, definition ) ;

		if ( Prepare( text, create_statement ) )
		{
			Step( create_statement ) ;

			if ( m_last_rc == SQLITE_DONE ) 
			{
				isOK = true ;
			}

			// need to discard the compiled SQL statement 
			SQLite3Finalize( create_statement ) ;
		}
	}

	return isOK ;
}











/******************************************************************************
SQLDatabase::TableExists

	calls SQLite to check for the table in the sqlite_master table. Returns true
	iff the table was found. 

******************************************************************************/
bool SQLDatabase::TableExists( 
	const TCHAR * table_name		// I - table to check on
) 
{
	stringt			sql_text ;
	sqlite3_stmt *	query_statement = NULL ;
	bool			isOK = false ;

	sql_text.Format( _T( "SELECT tbl_name FROM sqlite_master WHERE tbl_name='%s'  AND type='table'" ), table_name ) ;

	if ( Prepare( sql_text, query_statement ) )
	{
		isOK = Step( query_statement ) ;
		SQLite3Finalize( query_statement ) ;
	}
	return isOK ;
}








/******************************************************************************
SQLDatabase::NormalizeDefinition

PRIVATE member called by TableDefinitionsMatch. 

	As implemented, this function does minimal "normalization" making it 
	possible to ignore differences in white space. 
	
	It will fail on equivalent definitions where the order of fields is different. 
	
******************************************************************************/
stringt SQLDatabase::NormalizeDefinition( 
	stringt definition					// I/O - definition to "normalize"
) 
{
	stringt	token ;
	int		token_pos ;
	stringt	normalized_def ;

	definition.MakeLower( ) ;
	token_pos = 0 ;
	
	do
	{
		token = definition.Tokenize( _T( " \n\r\t" ), token_pos ) ;
		if ( token != _T( "" ) ) 
		{
			normalized_def += token + _T( " " ) ;
		}
	}
	while ( _T( "" ) != token ) ;

	normalized_def.Trim( ) ;
    normalized_def.Replace( _T( " ," ), _T( "," ) ) ;
	normalized_def.MakeLower( ) ;

	/* 
		We CAN go further. At this point, can add a comma at the end of normalized_def,
		then re-parse it with token seperator ',' - this gives field name and keywords for ea
		field. We could then re-order those fields and field definitions by lexical order of
		the field name, and then re-construct the normalized def
	*/

	return normalized_def ;
}








/******************************************************************************
SQLDatabase::TableDefinitionsMatch

PRIVATE MEMBER 

	Compares the definition provided by the caller with the definition stored
	in the database for the specified table 'table_name' 

	This is a heuristic which negates differences in whitespace and capitalization,
	but will fail if the columns have been re-ordered. 

	For example, the following will FAIL even though they really are equivalent: 

	CREATE TABLE MyTable ( recno INTEGER PRIMARY KEY, name, value INTEGER )
	CREATE TABLE MyTable ( name, value INTEGER, recno INTEGER PRIMARY KEY ) 

******************************************************************************/
bool SQLDatabase::TableDefinitionsMatch( 
	const TCHAR * table_name,			//	I - table name 
	const TCHAR * definition			//	I - definition provided by caller 
) 
{
	stringt			sql_text ;
	sqlite3_stmt *	sql_statement = NULL ;
	bool			matches = false ;

	sql_text.Format( _T( "SELECT sql FROM sqlite_master WHERE tbl_name='%s' AND type='table'" ), table_name ) ;

	if ( Prepare( sql_text, sql_statement ) )
	{
		if ( Step( sql_statement ) )
		{
			stringt		current_def ;
			stringt		new_def ;

			GetColumnTextByIndex( sql_statement, 0, current_def ) ;

            // sql stored in master_table includes the actual SQL create command, etc.
            if ( -1 != current_def.Find( _T( '(' ) ) && -1 != current_def.Find( _T( ')' ) ) )
            {
                current_def = current_def.Mid( 1 + current_def.Find( _T( '(' ) ) ) ;
                current_def = current_def.Left( current_def.Find( _T( ')' ) ) ) ;
            }
			current_def = NormalizeDefinition( current_def ) ;
			new_def = NormalizeDefinition( definition ) ;

			matches = ( 0 == current_def.Compare( new_def ) ) ;
		}
		SQLite3Finalize( sql_statement ) ;
	}

	return matches ;
}








/******************************************************************************
SQLDatabase::GetColumnTextByIndex

PRIVATE MEMBER



******************************************************************************/
bool SQLDatabase::GetColumnTextByIndex( 
	sqlite3_stmt * prepared_sql,			// I/O - sqlite's private compiled sql 
	int index,								// I - 
	stringt & value							// O - 
) 
{
	int		bytes ;
	TCHAR *	ptr ;

	if ( ( bytes = SQLite3ColumnBytes( prepared_sql, index ) ) )
	{
		// GetBufferSetLength() takes a number of characters.... and number of characters is given by "bytes / sizeof( TCHAR )", and need to allocate enough memory for the characters + 1 for the nul 
		ptr = value.GetBufferSetLength( bytes / sizeof( TCHAR ) + 1 ) ;
		memcpy( ptr, SQLite3ColumnText( prepared_sql, index ), bytes ) ;
		value.ReleaseBuffer( ) ;
	}
	else
		value = _T( "" ) ;		// 0 bytes means we have a NULL field 

	return true ;
}








/******************************************************************************
SQLDatabase::GetTableName

	Allows user to enumerate the tables in a database. 

******************************************************************************/
bool SQLDatabase::GetTableName( 
	stringt & table_name,				//	O - receives the next table name 
	int & table_index					//	I/O - the index to use, incremented on return 
)
{
	bool isOK = false ;

	if ( 0 == m_table_count ) 
	{
		if ( ( m_table_count = GetTableCount( ) ) )
		{
			if ( m_table_count ) 
			{
				SQLCursor query ;

				m_table_name = new stringt[ m_table_count ] ;

				if ( ( ExecuteQuery( query, _T( "SELECT name, sql FROM sqlite_master WHERE type = 'table' " ) ) ) )
				{
					int	i = 0 ;

					do
					{
						stringt	table_name ;

						query.GetFieldValue( _T( "name" ), table_name ) ;

						ASSERT( i < m_table_count ) ;
						m_table_name[ i++ ] = table_name ;
					}
					while ( query.StepNext( ) ) ;

				}
			}
		}
	}
		
	if ( table_index < m_table_count ) 
	{
		table_name = m_table_name[ table_index ] ;
		table_index++ ;
		isOK = true ;
	}

	return isOK ;
}




/******************************************************************************
SQLDatabase::GetTableDefinition

		

******************************************************************************/
bool SQLDatabase::GetTableDefinition( 
	const TCHAR * table_name,			// I - table to get definition of 
	stringt & definition				// O - gets definition of table 
)
{
	SQLCursor	def_cursor ;
	bool		isOK = false ;

	if ( ExecuteQuery( def_cursor, _T( "SELECT sql FROM sqlite_master WHERE name = '%s' AND type = 'table'" ), table_name ) )
	{
		isOK = def_cursor.GetFieldValue( 0, definition ) ;
	}

	return isOK ;
}





/******************************************************************************
SQLDatabase::DropTable

	Deletes a table, if it exists 

******************************************************************************/
bool SQLDatabase::DropTable( 
	const TCHAR * name					// I - name of table to drop 
)
{
	bool isOK = false ;

	if ( TableExists( name ) )
	{
		isOK = ExecuteSQL( _T( "DROP TABLE %s" ), name ) ;
	}

	return isOK ;
}












/******************************************************************************
SQLDatabase::GetTableCount

	Returns the number of tables in the databse, simply by running a SELECT COUNT 
	query on the sqlite_master 

******************************************************************************/
int SQLDatabase::GetTableCount( )
{
	int			record_count = 0 ;
	SQLCursor 	query ;

	if ( ExecuteQuery( query, _T( "SELECT COUNT( * ) FROM sqlite_master WHERE type='table'" ) ) ) 
	{
		query.GetFieldValue( 0, record_count ) ;
	}
	return record_count ;
}




/******************************************************************************
SQLDatabase::ExecuteQuery

	Wrapper for SELECT statements. 

	Principal weakness of this function is that it does no validation of arguments 
	and relies on sqlite's exception mechanism to handle errors 

******************************************************************************/
bool SQLDatabase::ExecuteQuery( 
	SQLCursor & sql_cursor,				// O - gets SQL cursor user can use to step through results of query 
	const TCHAR * sql_format,			// I - SELECT statement with printf-style placeholders for args 
	...									// I - variable args 
) 
{
	stringt sql_text ;
	va_list v_ptr ;

	va_start( v_ptr, sql_format ) ;
	sql_text.FormatV( sql_format, v_ptr ) ;
	
	sql_cursor.InitializeQuery( *this, sql_text ) ;

	return sql_cursor.StepNext( ) ;
}




/******************************************************************************
SQLDatabase::ExecuteSQL

	Wrapper for all SQL operations other than SELECT statement 

******************************************************************************/
bool SQLDatabase::ExecuteSQL( 
	const TCHAR * sql_format,			// I - SQL statement with printf-style placeholders 
	...									// I - variable args as required by format string
)
{
	stringt			sql_text ;
	va_list			v_ptr ;
	sqlite3_stmt *	prepared_sql ;
	int				rc ;

	va_start( v_ptr, sql_format ) ;
	sql_text.FormatV( sql_format, v_ptr ) ;

	if ( Prepare( sql_text, prepared_sql ) )
	{
		rc = SQLite3Step( prepared_sql ) ;

		// need to discard the compiled SQL statement 
		SQLite3Finalize( prepared_sql ) ;
	}

	return SQLITE_DONE == rc ;
}




/******************************************************************************
SQLDatabase::GetLastInsertRowID

	Returns the ROWID of the last line added to a table. Typically want to 
	call this immediately after ExecuteSQL( _T( "INSERT ... " ) ) 

******************************************************************************/
int SQLDatabase::GetLastInsertRowID( ) 
{
	ASSERT( NULL != m_db ) ;

	return (int) sqlite3_last_insert_rowid( m_db ) ;
}








/******************************************************************************
SQLCursor::SQLCursor

	Public constructor 


******************************************************************************/
SQLCursor::SQLCursor( )
{
	m_db = NULL ;
	m_prepared_sql = NULL ;
}



/******************************************************************************
SQLCursor::InitializeQuery

	Public constructor 


******************************************************************************/
void SQLCursor::InitializeQuery( 
	SQLDatabase & db,						// the database to query 
	const TCHAR * sql_select_text			// the selection text defining the query 
)
{
	if ( m_db )
	{
		DiscardQuery( ) ;
	}
	m_db = &db ;
	m_sql_text = sql_select_text ;
	m_db->Prepare( m_sql_text, m_prepared_sql ) ;
}






void SQLCursor::DiscardQuery( )
{
	SQLite3Finalize( m_prepared_sql ) ;		// no harm if m_prepared_sql is NULL 

	m_prepared_sql = NULL ;	
	m_db = NULL ;
	m_column_def.Empty( ) ;
	m_sql_text.Empty( ) ;
}






/******************************************************************************
SQLCursor::~SQLCursor

	destructor just makes sure the prepared sql member is cleaned up 

******************************************************************************/
SQLCursor::~SQLCursor( )
{
	SQLite3Finalize( m_prepared_sql ) ;		// no harm if m_prepared_sql is NULL 
}









/******************************************************************************
SQLCursor::GetFieldCount

	returns the number of columns returned by the associated SQL query 
	
******************************************************************************/
int SQLCursor::GetFieldCount( ) 
{
	if ( 0 == m_column_def.GetLength( ) )
		InitializeColumnDef( ) ;

	int semi_ct = 0 ;
	int	current_pos = 0 ;

	while ( -1 != ( current_pos = m_column_def.Find( _T( ';' ), current_pos ) ) )
	{
		semi_ct++ ;
		current_pos++ ;
	}

	return semi_ct + 1 ;
}




/******************************************************************************
SQLCursor::GetRecordCount

	Not sure if I'll ever want this... but we basically just construct a 
	SELECT COUNT( ) 


******************************************************************************/
int SQLCursor::GetRecordCount( )
{
	stringt sub_query = m_sql_text ;
	stringt lc_sub_query ;
	int		select_start ;
	int		from_start ;
	SQLCursor sub_cursor ;
	int		rec_count ;

	/*	
	*	SQL keywords and field and table names are case-insensitive... but 
	*	we can't mess with any string literals passed in the WHERE clause....
	*/ 
	sub_query.TrimLeft( ) ;
	sub_query.Replace( _T( "\t" ), _T( " " ) ) ;
	lc_sub_query = sub_query ;
	lc_sub_query.MakeLower( ) ;

	select_start = lc_sub_query.Find( _T( "select " ) ) ;
	ASSERT( select_start == 0 ) ;
	from_start = lc_sub_query.Find( _T( " from " ) ) ;

	// if the SELECT is an agregate function, eg COUNT( * ), AVG( * ), MIN( * ), etc, then the select is just going to return the value of the aggregate function
	if ( -1 != sub_query.Mid( 7, from_start-7 ).Find( _T( '(' ) ) )
		return 1 ;
	
	/* 
	*	so we rewrite the query with the exact same FROM and WHERE, and even field names - It's important to 
	*	preserve the field names because we might have something like SELECT DISTINCT .... 
	*/
	sub_query = _T( "SELECT COUNT ( " ) + sub_query.Mid( 7, from_start - 7 ) + _T( " )" ) + sub_query.Mid( from_start ) ;
	
	if ( m_db->ExecuteQuery( sub_cursor, sub_query ) )
		sub_cursor.GetFieldValue( 0, rec_count ) ;
	else
		ASSERT( FALSE ) ;

	return rec_count ;
}





/******************************************************************************
SQLCursor::GetFieldName

	Return the n'th field name returned by the SELECT query underlying this SQLCursor

******************************************************************************/
stringt SQLCursor::GetFieldName( 
	int index							// I - index of field to return 
) 
{
	if ( 0 == m_column_def.GetLength( ) )
		InitializeColumnDef( ) ;

	// the 'index'th field name is between 'index'th and the 'index + 1'th semi-colon
	int	start_pos = 0 ;
	int	semi_ct = 0 ;
	int	semi_pos = 0 ;

	while ( true ) 
	{
		semi_pos = m_column_def.Find( _T( ';' ), start_pos )  ;
		if ( semi_ct == index ) 
		{
			stringt long_name ;

			if ( -1 == semi_pos )	// semi_pos will be -1 if matching last field 
				long_name = m_column_def.Mid( start_pos ) ;
			else
				long_name = m_column_def.Mid( start_pos, semi_pos - start_pos ) ;

			if ( -1 != long_name.Find( _T( '.' ) ) )
				return long_name.Mid( long_name.Find( _T( '.' ) ) + 1 ) ;
			else
				return long_name ;
		}
		else if ( semi_pos != -1 ) 
		{
			start_pos = semi_pos + 1 ;
			semi_ct++ ;
		}
		else
			return stringt( _T( "!undefined" ) ) ;
	}
}







/******************************************************************************
SQLCursor::StepNext

	Advance to the next row. 

Return
	true  - have another row

	false - no more rows 

******************************************************************************/
bool SQLCursor::StepNext( )
{
	bool isOK = false ;

	if ( m_prepared_sql ) 
		isOK = m_db->Step( m_prepared_sql ) ;
	
	return isOK ;
}




/******************************************************************************
SQLCursor::InitializeColumnDef

PRIVATE MEMBER

	called when required to get the column-name metadata 

******************************************************************************/
void SQLCursor::InitializeColumnDef( ) 
{
	const TCHAR * table ;
	const TCHAR * field ;
	int	col_no ;

	for ( col_no = 0 ; NULL != ( field = (const TCHAR *) SQLite3ColumnOriginName( m_prepared_sql, col_no ) ) ; col_no++ )
	{
		stringt table_dot_field ;

		table = (const TCHAR *) SQLite3ColumnTableName( m_prepared_sql, col_no ) ;
		table_dot_field.Format( _T( "%s.%s" ), table, field ) ;

		if ( col_no ) 
			m_column_def += _T( ";" ) ;
		m_column_def += table_dot_field ;
	}
}





/******************************************************************************
SQLCursor::GetColumnNumber

	Returns the column-number of a given column-name 

******************************************************************************/
int SQLCursor::GetColumnNumber( 
	const TCHAR * field_name			// I - field name 
) 
{
	if ( 0 == m_column_def.GetLength( ) )
		InitializeColumnDef( ) ;

	int col_no = 0 ;
	int token_pos = 0 ;
	bool fully_qualified_name = false ;

	stringt token ;

	if ( _tcschr( field_name, _T( '.' ) ) )
		fully_qualified_name = true ;

	token = m_column_def.Tokenize( _T( ";" ), token_pos ) ;
	while ( _T( "" != token ) )
	{
		if ( fully_qualified_name && 0 == token.CompareNoCase( field_name ) ) 
			return col_no ;
		else 
		{
			ASSERT( -1 != token.Find( _T( '.' ) ) ) ;
			if ( 0 == token.Mid( token.Find( _T( '.' ) ) + 1 ).CompareNoCase( field_name ) )
				return col_no ;
			else
				col_no++ ;
		}

		token = m_column_def.Tokenize( _T( ";" ), token_pos ) ;
	}

	return -1 ;
}





/******************************************************************************
SQLCursor::GetFieldValue

	overload of GetFieldValue to return a string representation 

	SQLite is happy to translate data to a compatible type regardless of the 
	native format it's stored in. This call will succeed if column_name designates
	a column that is integer or floating point, and just return the data formatted 
	in a string representation 

	All GetFieldValue( ) calls where client specifies a column name turn around
	to call the underlying GetFieldValue where column is specified by column number

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	const TCHAR * column_name,			// I - column name to return 
	stringt & text						// O - gets text representation of column data 
)
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return GetFieldValue( column_no, text ) ;

	return false ;
}


/******************************************************************************
SQLCursor::GetFieldValue

	overload to return int version of column data 

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	const TCHAR * column_name,			// I - column name to return data for 
	int & int_value						// O - gets integer representation of column data 
)
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return GetFieldValue( column_no, int_value ) ;

	return false ;
}



/******************************************************************************
SQLCursor::GetFieldValue

	overload to return 64-bit integer version of column data 

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	const TCHAR * column_name,			// I - column to return data for 
	uint64 & uint64_value				// O - gets 64-bit integer representation of column data 
) 
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return GetFieldValue( column_no, uint64_value ) ;

	return false ;
}



/******************************************************************************
SQLCursor::GetFieldValue

	overload to return double-precision floating point representation of data 

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	const TCHAR * column_name,			// I - column to return data for 
	double & float_value				// O - gets double representation of column data 
)
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return GetFieldValue( column_no, float_value ) ;

	return false ;
}



/******************************************************************************
SQLCursor::GetFieldValue

	overload to return untyped blob of data 

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	const TCHAR * column_name,			// I - column name 
	int * blob_size,					// I/O - I - size of buffer provided, O: size of data 
	void * blob_buffer					// I/O - address of data buffer, defaults to NULL 
)
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return GetFieldValue( column_no, blob_size, blob_buffer ) ;

	return false ;
}


/******************************************************************************
SQLCursor::GetFieldValue

	Returns a text representation of the data. With the exception of BLOB type,
	this can be used on any other type to return a usable presentation of the data

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	int column_pos,						// I - column number 
	stringt & text						// O - gets text representation of data 
)
{
	if ( m_prepared_sql ) 
	{
		text = (const TCHAR *) SQLite3ColumnText( m_prepared_sql, column_pos ) ;
		return true ;
	}
	else
		return false ;
}




/******************************************************************************
SQLCursor::GetFieldValue

	Used to get a 32-bit int representation of the column's data 		

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	int column_pos,						// I - column number 
	int & int_value						// O - integer representation of column's data 
)
{
	if ( m_prepared_sql ) 
	{
		int_value = SQLiteColumnInt( m_prepared_sql, column_pos ) ;
		return true ;
	}
	return false ;
}



/******************************************************************************
SQLCursor::GetFieldValue

	used to get a 64-bit integer 

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	int column_pos,						// I - column number
	uint64 & uint64_value				// O - gets uint64 integer 
) 
{
	if ( m_prepared_sql ) 
	{
		uint64_value = SQLiteColumnInt64( m_prepared_sql, column_pos ) ;
		return true ;
	}
	return false ;
}


/******************************************************************************
SQLCursor::GetFieldValue

	Gets a double-precision floating point representation of numeric data 

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	int column_pos,						// I - column number 
	double & float_value				// O - floating point representation of data 
)
{
	if ( m_prepared_sql ) 
	{
		float_value = SQLiteColumnDouble( m_prepared_sql, column_pos ) ;
		return true ;
	}
	return false ;
}





/******************************************************************************
SQLCursor::GetFieldValue

	On entry, blob_size specifies the size of the buffer to receive the data. On
	return it ALWAYS gives the actual size of the data. If the size on entry is
	large enough to contain the whole blob, the data is copied to the buffer and 
	the return value is true. If blob_size is too small, the return is false. 

	If blob_size is 0, blob_buffer may safely be NULL 

Example of use: 
	buff_size = 0 ;
	GetFieldValue( column_pos, &buff_size ) ;
	buffer = new byte[ buff_size ] ;
	GetFieldValue( column_pos, &buff_size, buffer ) ;

******************************************************************************/
bool SQLCursor::GetFieldValue( 
	int column_pos,						// I - column number 
	int * blob_size,					// I/O - size of buffer 
	void * blob_buffer					// I - address of buffer (default is NULL) 
)
{
	if ( m_prepared_sql ) 
	{
		if ( SQLite3ColumnBytes( m_prepared_sql, column_pos ) <= *blob_size ) 
		{
			ASSERT( blob_buffer != NULL ) ;	
			*blob_size = SQLite3ColumnBytes( m_prepared_sql, column_pos ) ;
            
            memcpy( blob_buffer, SQLiteColumnBlob( m_prepared_sql, column_pos ), *blob_size ) ;
			return true ;
		}
		else
			*blob_size = SQLite3ColumnBytes( m_prepared_sql, column_pos ) ;
	}

	return false ;
}


/******************************************************************************
SQLCursor::ColumnIsNull

	Returns true iff. the corresponding column is NULL 

	Note: NULL is NOT the same as blank or 0. NULL is a special SQL value
	which means that the data for the column is NOT specified. 

******************************************************************************/
bool SQLCursor::ColumnIsNull( 
	const TCHAR * column_name			// I - column to check 
)
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return ColumnIsNull( column_no ) ;

	ASSERT( FALSE ) ;
	return true ;		// I guess if the column doesn't exist, it must be null, eh?
}




/******************************************************************************
SQLCursor::ColumnIsNull

	Returns true iff. the corresponding column is NULL 

	Note: NULL is NOT the same as blank or 0. NULL is a special SQL value
	which means that the data for the column is NOT specified. 

******************************************************************************/
bool SQLCursor::ColumnIsNull( 
	int column_index					// I - column # to check on 
) 
{
	return SQLITE_NULL == GetFieldType( column_index ) ;
}





/******************************************************************************
SQLCursor::GetFieldType

	returns the native field type for a given field 


******************************************************************************/
SQLDatabase::FieldType SQLCursor::GetFieldType( 
	const TCHAR * column_name			// I - column to check on by name 
) 
{
	int	column_no ;

	if ( -1 != ( column_no = GetColumnNumber( column_name ) ) )
		return GetFieldType( column_no ) ;

	ASSERT( FALSE ) ;
	return SQLDatabase::UnknownType ;
}



/******************************************************************************
SQLCursor::GetFieldType

	returns the native field type for a given field 

******************************************************************************/
SQLDatabase::FieldType SQLCursor::GetFieldType( 
	int column_no						// I - column number to check on 
)
{
	switch ( SQLiteColumnType( m_prepared_sql, column_no ) )
	{
		case SQLITE_INTEGER :
			return SQLDatabase::TypeInteger ;
			
		case SQLITE_FLOAT :
			return SQLDatabase::TypeFloat ;

		case SQLITE_TEXT :
			return SQLDatabase::TypeText ;

		case SQLITE_BLOB :
			return SQLDatabase::TypeBlob ;

		case SQLITE_NULL : 
			return SQLDatabase::TypeNULL ;

		default :
			ASSERT( FALSE ) ;
			return SQLDatabase::UnknownType ;
	}
}



