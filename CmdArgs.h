
#pragma once

/*

This was copied from another project so much infrastucture is inaplicable

Usage
required
	spec: working folder
	--resolution [-r]: number of pixels across width of image
	--photo-list [-l]: list of photos to resample
		or
	--all-photos [-a]: all photos in working folder and sub folders
optional
	--help [-h]: print operational commands
	--dest-folder [-d]: resample source files to destination folder
	--measure [-m]: just log the operations that would be performed

~~~~~ typical ini, note 'measure' is just a switch:

[arguments]
source-folder= C:\Users\Dan\Pictures\work
resolution= 800
log= test.log
measure
dest-folder= C:\Users\Dan\Pictures\work\TestFolder

[files]
DSCN0212.JPG
DSCN0213.JPG
DSCN0214.JPG
DSCN0215.JPG
DSCN0216.JPG
DSCN0217.JPG

~~~~~~
*/

//......................................................

//special case as folder_exclude can be absolute or just folder name
struct less_folder_exclude : public std::less< bfs::path >
{
    bool operator( ) (const bfs::path& left, const  bfs::path& right ) const
    {
			return left.filename( ) < right.filename( );
    }
};

//..........................................................................
//new stuff for .ini
/*
	ini file items would usually come in pairs of:
		item label
		item value
	They don't have to, you could have just labels as bool switch
	The categories are kept in the 'container': ProgCnt
	ProgCnt wrapped by ProgCntObj for ease of use
*/
//label value item
typedef std::pair< std::string, std::string > ini_pair;
//items set
typedef std::vector< ini_pair > ini_set;
//category of items, [ cat label, items set ]
typedef std::pair< std::string, ini_set > ProgCntItem;
//mapped categories of items
typedef std::map< std::string, ini_set > ProgCnt;
typedef std::map< std::string, ini_set >::iterator ProgCntIt;
typedef std::pair< ini_pair&, bool > ProgCntInsertPair;

struct ProgCntObj
{
	ProgCnt container;
	ProgCntIt find( const std::string& in ) { return container.find( in ); }
	ProgCntIt end( ) { return container.end( ); }
	ini_set& insert( std::string& tag ) { return container.insert( ProgCntItem( tag, ini_set( ) ) ).first->second; }
	ProgCnt::size_type remove( std::string& in_tag ) { return container.erase( in_tag ); }
};

//..........................................................................
class my_ini_parser : po::basic_parsed_options<char>
{
public:
	my_ini_parser( const po::options_description& desc )
		:basic_parsed_options( &desc )
	{ }
	po::parsed_options parse_my_array( ini_set& set )
	{
		po::parsed_options result( description );
        
		for( auto it= set.begin( ); it != set.end( ); ++it )
		{
//			std::string option_name= it->first;
			po::option opt;
			opt.string_key= it->first;
			if( ! it->second.empty( ) )
				opt.value.push_back( it->second );

			result.options.push_back( opt );
		}
		return result;
	}
};

// .....................................................................
typedef std::vector< std::string > vstring;

//just unicode for now
namespace boost { namespace program_options {


typedef wparsed_options tparsed_options;
}};

// .....................................................................
class argstate
{
public:
	std::string user_folder;	//sink
	std::string source_folder;//source
	std::string log_file;
	std::string fuel_file;
	std::string response_file;
	size_t resolution;
	bool bMeasure;
	bool bAll;
	vstring file_list;

	po::options_description desc;//("Allowed options");
	po::variables_map vm;
	int ierr;
//
protected:
	ProgCntObj ini_obj;
	bool option_dependency( const char* for_what, const char* required_option );
	bool option_required( const char* for_what );
	bool option_declined( const char* for_what, const char* declined_option );

public:
	argstate( )
		:desc( "Usage" )
		,bMeasure( false )
		,bAll( false )
	{ }

	bool init( int argc,  wchar_t ** argv );
	int err( ) const { return ierr; };
	size_t count( char* arg ) { return vm.count( arg ); }

	template<class T>
	const T value( char* arg ) const { return boost::any_cast< T >( vm.at( arg ).value( ) ); }

	bool argstate::read_respose( po::tparsed_options& options );
	void store_single( char* arg, po::variable_value& val );
};

