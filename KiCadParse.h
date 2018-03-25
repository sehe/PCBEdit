// ...........................................................................
//KiCadParse.h
#include "stdafx.h"

// ...........................................................................
class PCBoard;

// ...........................................................................
typedef boost::variant
<
	unsigned int
	,double
	,std::tstring

>value;

// ...........................................................................
typedef std::vector< value > values;
typedef std::pair< std::tstring, values > pair_type;

// ...........................................................................
template< class Ty >
class estack : public std::stack< Ty >
{
    using std::stack<Ty>::c;
public:
    using std::stack<Ty>::size;
	Ty& bottom( ) { return *c.begin( ); }
	Ty& down( size_t i ) { assert( i < size( ) && i > 0 ); return *( c.end( ) - i ); }
};

// ...........................................................................
//debug
namespace debug_
{
	static char const* which_str[ ]= {
		"uint",
		"double",
		"string",
	};
}

// ...........................................................................
namespace boost {

	std::ostream& operator<<(std::ostream& out, const value& v );
	std::ostream& operator<<(std::ostream& out, const values& v );
    std::ostream& operator<<(std::ostream& out, const pair_type& v);
}

// ...........................................................................
namespace KiCadParser
{
	typedef std::map<std::tstring, int> b_map;
	typedef std::map<std::tstring, int>::iterator b_map_it;


	// ...........................................................................
	#ifdef KICAD_PARSE_SPIRIT
	typedef boost::shared_ptr< int > SP_Base;
	#endif
	//#ifndef KICAD_PARSE_SPIRIT
	struct target
	{
		target( )
			:id( 0 )
			,layer( 0 )
		{ }
		target( std::tstring const& name )
			:name( name )
			,id( 0 )
			,layer( 0 )
		{ }

		target( std::tstring const& name, size_t id, size_t layer, SP_Base sp )
			:name( name )
			,id( id )
			,layer( layer )
			,sp_target( sp )
		{ }

		std::tstring name;
		size_t id;
		size_t layer;
		SP_Base sp_target;
	};
	typedef estack< target > s_target;
	//#endif

	// ...........................................................................
	struct pcnew_wrapper
	{
		PCBoard* pcboard;

		double version;
		size_t count;

		s_target targets;
		void push_target( target const& t ) { targets.push( t ); }
		void push_target( target && t ) { targets.push( std::move(t) ); }
		b_map main;
		b_map general;
		b_map module;
		b_map subs;

		pcnew_wrapper( PCBoard* pBoard )
			:pcboard( pBoard )
			,count( 0 )
		{
			Init( );
		}

		void Init( );
	//the real stuff
		//calls from parser
		void kipcb_pop_dest( );
		void kipcb_push_dest( const pair_type &pair );
		void kipcb_post_dest( const pair_type &pair );
		void kipcb_word_dest( const std::tstring& str );

		//break it down
	
		//when push_dest has values
		void push_values( target& targ, values const& vals );

		void push_obj( const pair_type& pair );
		void post_obj( const pair_type& pair );
		void top_post_obj( const pair_type& pair );

		void post_general( const pair_type& pair );

		//test stuff
		struct _info
		{
			std::string test_str;
			long links;
			size_t no_connects;
			size_t tracks;
			size_t modules;
			size_t nets;
	
		}info;
		//Layer* GetLayer( TPCB_UINT layer_id ) { return pcboard->GetLayer( layer_id ); }
		size_t GetLayerID( std::tstring const& str );

	};
	// ...........................................................................
	//context parser interface
	inline void kipcb_pop_dest_expr( pcnew_wrapper& a ){ a.kipcb_pop_dest( ); }
	inline void kipcb_push_dest_expr( pcnew_wrapper& a, pair_type const& pair ){ a.kipcb_push_dest( pair ); }
	inline void kipcb_post_dest_expr( pcnew_wrapper& a, pair_type const& pair ){ a.kipcb_post_dest( pair ); }
	inline void kipcb_word_dest_expr( pcnew_wrapper& a, std::tstring const& str ){ a.kipcb_word_dest( str ); }


}//namespace KiCadParser

bool KiCadParse( std::ifstream& in, PCBoard* pBoard );

bool PostKicadParse( PCBoard* pBoard );

