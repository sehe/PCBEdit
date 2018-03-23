
#include "stdafx.h"
#include "CmdArgs.h"

///...........................................................
//hack from boost::property_tree::ini_parser
bool read_ini( std::basic_istream< char >& stream, ProgCntObj& pt )
{
	typedef char Ch;
	typedef std::basic_string<Ch> Str;
	const Ch semicolon = stream.widen(';');
	const Ch hash = stream.widen('#');
	const Ch lbracket = stream.widen('[');
	const Ch rbracket = stream.widen(']');

	Str line;
	ProgCntObj& local= pt;
	ini_set* section = NULL;
	std::string ini_set_tag;
	unsigned long line_no = 0;

	// For all lines
	for( ; stream.good( ); )
	{
		// Get line from stream
		++line_no;
		std::getline( stream, line );
		if( ! stream.good( ) && ! stream.eof( ) )
			return false;

		if( ! line.empty( ) )
		{
			// Comment, section or key?
			if( line[0] == semicolon || line[0] == hash )
			{
				// Ignore comments
			}
			else if( line[0] == lbracket )
			{
				// If the previous section was empty, drop it.
				if( section && section->empty( ) )
					local.remove( ini_set_tag );

				Str::size_type end= line.find( rbracket );
				if( end == Str::npos )
					return false;

				Str key= line.substr( 1, end - 1 );
				ba::trim( key, stream.getloc( ) );
				if( local.find( key ) != local.end( ) )
					return false;

				section = &local.insert( key );
			}
			else
			{
				if( ! section )
					return false;

				Str::size_type eqpos= line.find( Ch( '=' ) );
				if (eqpos == Str::npos)
					eqpos= 0;

				Str key= line.substr(0, eqpos);
				ba::trim( key, stream.getloc( ) );
				Str data= line.substr( eqpos ? eqpos + 1 : 0, Str::npos );
				ba::trim( data, stream.getloc( ) );

				section->push_back( ini_pair( key, data ) );
			}
		}
	}
	// If the last section was empty, drop it again.
	if( section && section->empty( ) )
		local.remove( ini_set_tag );

	return true;
}

bool argstate::read_respose( po::tparsed_options& options )
{
	// Load the file and tokenize it
//	std::ifstream ifs( vm["response-file"].as< bfs::path >( ).string( ).c_str( ) );
	std::ifstream ifs( vm["response-file"].as< std::string >( ) );
	if( ! ifs )
	{
		std::cerr << "Could not open the response file: " << vm["response-file"].as< std::string >( ) << std::endl;
		return false;
	}
	if( ! read_ini( ifs, ini_obj ) )
	{
		std::cerr << "Failed to parse the response file\n";
		return false;
	}

	ProgCntIt it= ini_obj.find( "arguments" );
	if( it != ini_obj.end( ) )
	{
		my_ini_parser parse( desc );
		po::store( parse.parse_my_array( it->second ), vm );
	}

	it= ini_obj.find( "files" );
	if( it != ini_obj.end( ) )
	{
		for( auto bit= it->second.begin( ); bit != it->second.end( ); ++bit )
			file_list.push_back( bit->second );
	}
	//just need to tag as in use for arg check
	if( ! file_list.empty( ) )
		store_single( "photo-list", po::variable_value( ) );

	return true;
	// will ifs.close( ) when out of scope;
}

void argstate::store_single( char* arg, po::variable_value& val )
{
	//quick hack
	std::map< std::string, po::variable_value >& m = vm;
	m[ arg ]= val;
}

///...........................................................
// Function used to check that of 'for_what' requires 'required_option'
bool argstate::option_dependency( const char* for_what, const char* required_option )
{
    if ( vm.count( for_what ) && ! vm[ for_what ].defaulted( )
		&& ( ! vm.count( required_option ) || vm[ required_option ].defaulted( ) ) )
		{
			std::cerr << "Option '" << for_what <<"' requires option '" << required_option << "'." << std::endl;
			return false;
		}
		return true;
}

///...........................................................
// Function used to check that of 'for_what' declines 'required_option'
bool argstate::option_declined( const char* for_what, const char* declined_option )
{
    if ( vm.count( for_what ) && vm.count( declined_option ) )
		{
			std::cerr << "Option '" << for_what <<"' not valid with '" << declined_option << "'." << std::endl;
			return false;
		}
		return true;
}

///...........................................................
// Function to check for required options
bool argstate::option_required( const char* for_what )
{
    if ( ! vm.count( for_what ) )
		{
			std::cerr << "Option '" << desc.find( for_what, false ).format_name( ) << "' is required." << std::endl;
			return false;
		}
		return true;
}

//#undef _X
//#define _X(x)      L ## x

///...........................................................
bool argstate::init( int argc,  wchar_t ** argv  )
{
	try
	{
//		desc.
		desc.add_options( )
			(_X("help,h"), _X("print usage message"))
			(_X("user-folder,u"), po::value( &user_folder ), _X("override default appdata path"))
			(_X("log,l"), po::value( &log_file ), _X("log file"))
			(_X("log-no-fail,n"), _X("log file, continue if logging fails"))
			(_X("source-folder,s"), po::value( &source_folder ), _X("source folder"))
			(_X("measure,m"), po::value( &bMeasure ), _X("just log needed actions, log must be specified"))
			(_X("photo-list,p"), po::value( &fuel_file ), _X("list of files to process"))
			(_X("resolution,r"), po::value( &resolution ), _X("resolution to resample to"))
			(_X("response-file,i"), po::value( &response_file ), _X("ini file to use for parameters"))
			(_X("all,a"), _X("Process all files in source-folder and sub folders"))
		;
//		std::vector< std::string > args = po::split_winmain( std::wstring( argv ) );
//		po::parse_command_line( argc, argv, desc );
		po::tparsed_options options= po::parse_command_line( argc, argv, desc );
//		po::parsed_options options= po::parse_command_line( args ).options( desc ).run( );
		try
		{
	        po::store( options, vm );
		}
		catch( po::error_with_option_name& e )
		{
			std::cout << e.get_option_name( ) << " failed to store" << std::endl;
			return false;
		}
        if( vm.count( "help" ) )
		{
            std::cout << desc << "\n";
            return false;
        }
		if( vm.count( "response-file" ) )
			read_respose( options );

		//if( ! vm.count( "source-folder" ) )
		//	source_folder= dest_folder;

		//test
		//store_single( "all", po::variable_value( ) );
		//if( ! option_dependency( "measure", "log" )
		//	|| ! option_required( "resolution" )
		//	|| ! option_declined( "all", "photo-list" ) )
		//	return false;

		po::notify( vm );

	}
    catch( std::exception& e )
	{
        std::cerr << "failed setup: " << e.what( ) << "\n";
		return false;
    }
	return true;
}


