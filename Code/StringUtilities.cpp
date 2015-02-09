#include "Engine.hpp"

const char *Format ( const char *fmt, ... )
{
	static char textString[35][READSIZ];

	static int _FormatTicker;

	// -- reset to 0 if we exceed MAX_NESTED_FORMAT , we should never nest this many Formats
	// -- but if we do, we will have problems and need to expand at that point in time.
	// -- however I do not forsee us experiencing that large of a upgrade
	if ( ++_FormatTicker >= 35 ) {
		_FormatTicker = 0;
	}

	try {
		int lTick = _FormatTicker;

		// -- GCC has deprecated this method, hopefully this will change
		// -- in the future so we can use this.
		//              TODO ( "Variable name Assignment - Unique identifiers" )

		// -- attempt to memset the current tick
		memset ( textString[lTick], 0, sizeof ( textString[0] ) );

		// -- Assign our variable length in totality!
		va_list args;
		va_start ( args, fmt );
		int length = vsnprintf ( textString[lTick], READSIZ, fmt, args );
		va_end ( args );

		if ( length == 0 ) {
			abort();
		}

		return textString[lTick];
	} catch ( ... ) {
		std::cout << "Unknown exception in Format has been caught."  << std::endl;
	}
	return "";
}


// case-insensitive string comparisons
bool SameString ( const std::string &a, const std::string &b )
{
	try {
		// they are not the same if they are empty
		if ( a.empty() ) { return false; }
		if ( b.empty() ) { return false; }

		if ( a.length() != b.length() )
		{ return false; }

		for ( int x = 0; ( a[x] != '\0' && b[x] != '\0' ); x++ ) {
			if ( tolower ( a[x] ) != tolower ( b[x] ) )
			{ return false; }
		}
	} catch ( ... ) {
		CATCH ( false );
		return false;
	}
	return true;
}

bool SameString ( const char *astr, const char *bstr )
{
	if ( astr == NULL ) {
		log_hd ( LOG_ERROR, "!SameString: null astr." );
		return false;
	}

	if ( bstr == NULL ) {
		log_hd ( LOG_ERROR, "!SameString: null bstr." );
		return false;
	}

	std::string a = astr;
	std::string b = bstr;
	return SameString ( a, b );
}

bool SameString ( const char *astr, const std::string bstr )
{
	if ( astr == NULL ) {
		log_hd ( LOG_ERROR, "!SameString: null astr." );
		return false;
	}

	std::string a = astr;
	return SameString ( a, bstr );
}

bool SameString ( const std::string &astr, const char *bstr )
{
	if ( bstr == NULL ) {
		log_hd ( LOG_ERROR, "!SameString: null bstr." );
		return false;
	}

	std::string b = bstr;
	return SameString ( astr, b );
}

// is the keyword/name within the 'namelist'
bool IsSameList ( const std::string &nameToFind, const std::string &namelist )
{
	try {
		if ( namelist.empty() )
		{ return false; }

		std::string nList = namelist;
		std::string name;

		while ( true ) {
			// chop chop, each word.
			nList = ChopString ( nList, name );

			// is the name empty? break.
			if ( name.empty() )
			{ break; }

			// are the string the same?
			if ( SameString ( nameToFind, name ) )
			{ return false; }

			// is the namelist empty now?
			if ( nList.empty() )
			{ break; }
		}
	} catch ( ... ) {
		CATCH ( false );
		return false;
	}
	return false;
}

std::string SmashTilde ( std::string str )
{
	std::string outstr;
	for ( int x = 0; str[x] != '\0'; x++ ) {
		if ( str[x] == '~' )
		{ outstr.append ( "-" ); }
		else {	outstr.append ( Format ( "%c", str[x] ) ); }
	}

	return outstr;
}

std::string FixAmp ( const std::string &str )
{
	std::string outstr;

	for ( int x = 0; str[x] != '\0'; x++ ) {
		if ( str[x] == '&' )
		{ outstr.append ( "&&" ); }
		else {
			char b[10];
			snprintf ( b, 10, "%c", str[x] );
			outstr.append ( b );
		}
	}

	return outstr;
}


std::string RemoveChar ( std::string str, char v )
{
	std::string outstr;
	for ( int x = 0; str[x] != '\0'; x++ ) {
		if ( str[x] != v ) {
			char b[10];
			snprintf ( b, 10, "%c", str[x] );
			outstr.append ( b );
		}
	}

	return outstr;
}

char* CapitalizeC ( const char *str )
{
	static char strcap [ READSIZ ];
	int  i;

	memset ( strcap, 0, sizeof ( strcap ) );

	for ( i = 0; str[i] != '\0'; i++ )
	{ strcap[i] = toupper ( str[i] ); }
	strcap[i] = '\0';

	return strcap;
}

char *ChopC ( const char *argument, char *first_arg )
{
	char cEnd;

	while ( isspace ( *argument ) )
	{ argument++; }

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
	{ cEnd = *argument++; }

	while ( *argument != '\0' ) {
		if ( *argument == cEnd ) {
			argument++;
			break;
		}
		*first_arg = LOWER ( *argument );
		first_arg++;
		argument++;
	}
	*first_arg = '\0';

	while ( isspace ( *argument ) )
	{ argument++; }

	return ( char * ) argument;
}

std::string ChopString ( const std::string &argument, std::string &first )
{
	std::string::size_type start, stop, stop2;
	char find;

	start = 0;

	if ( argument.length() < 1 || argument.empty() ) {
		first = "";

		return "";
	}

	if ( argument[0] == ' ' ) {
		start = argument.find_first_not_of ( ' ' );

		if ( start == argument.npos ) {
			first = "";

			return "";
		}
	}

	// Quotes or space?
	switch ( argument[start] ) {
		case '\'':
			find = '\'';
			start++;
			break;
		case '\"':
			find = '\"';
			start++;
			break;
		default:
			find = ' ';
	}

	// Find end of argument.
	stop = argument.find_first_of ( find, start );

	// Empty leftovers?
	if ( stop == argument.npos ) {
		first = argument.substr ( start );

		return "";
	}

	// Update first
	first = argument.substr ( start, ( stop - start ) );

	// Strip leading spaces from leftovers
	stop2 = argument.find_first_not_of ( ' ', stop + 1 );

	// Empty leftovers?
	if ( stop2 == argument.npos )
	{  return ""; }



	// Return leftovers.
	return argument.substr ( stop2 );
}

const char *StripNewLine ( const char *str )
{
	static char newstr[READSIZ];
	char buffer[READSIZ];
	int i, j;

	memset ( buffer, 0, sizeof ( buffer ) );

	strncpy ( buffer, str, sizeof ( buffer ) );

	// -- memset the string
	memset ( newstr, 0, sizeof ( newstr ) );

	for ( i = j = 0; buffer[i] != '\0'; i++ ) {
		if ( buffer[i] != '\r' && buffer[i] != '\n' ) {
			newstr[j++] = buffer[i];
		}
	}
	newstr[j] = '\0';

	return newstr;
}

std::string CapitalSentence ( const std::string &str )
{
	std::string _buf ( "" );
	bool didPeriod = false;

	for ( size_t _x = 0; _x <= str.length(); _x++ ) {
		if ( _x == 0 ) // first letter of the string is capitalized.
		{ _buf = toupper ( str[0] ); }
		else {
			// are we a letter that needs to be capitalized.
			if ( didPeriod && isalpha ( str[_x] ) ) {
				_buf[_x] = toupper ( str[_x] );
				didPeriod = false;
				continue;
			}
			// are we a period?  Setup our next sentence :)
			if ( str[_x] == '.' )
			{ didPeriod = true; }

			// just a letter/number/etc, push it to the new buffer.
			_buf[_x] = str[_x];
		}
	}

	return ( _buf );
}

std::string GetArg ( const std::string &str, int arg )
{
	if ( str.empty() )
	{  return ""; }

	std::string data;
	std::string argument;
	data = ChopString ( str, argument );
	if ( arg == 0 )
	{  return argument; }

	for ( int x = 1; x <= arg; x++ ) {
		data = ChopString ( data, argument );
		if ( arg == x ) {
			return argument;
		}
	}

	// return an empty string.
	return "";
}

// return everything after arg.
std::string GetAfter ( const std::string &str, int arg )
{
	if ( str.empty() )
	{  return ""; }

	std::string data;
	std::string argument;
	data = ChopString ( str, argument );
	if ( arg == 0 )
	{  return data; }

	for ( int x = 1; x <= arg; x++ ) {
		data = ChopString ( data, argument );
		if ( arg == x ) {
			return data;
		}
	}

	return "";
}

const char *SmashSpace ( const std::string &str_sm )
{
	if ( str_sm.empty() ) {
		log_hd ( LOG_ERROR, "SmashSpace called with a empty string!" );
		return NULL;
	}
	char *str = ( char * ) str_sm.c_str();
	static char strwrap[READSIZ] = {'\0'};
	memset ( strwrap, 0, sizeof ( strwrap ) );

	for ( int i = 0; str[i] != '\0'; i++ ) {
		if ( str[i] == ' ' )
		{ strwrap[i] = '_'; }
		else { strwrap[i] = str[i]; }
	}

	return strwrap;
}

