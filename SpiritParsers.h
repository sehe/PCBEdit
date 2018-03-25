#include "spirit.h"

// ...........................................................................
// SHARED RESOURCES
// ...........................................................................
namespace SpiritShare
{
	//all reals will have a decimal point, pcnew rule
	//No exponent is done because of the likes of word: '3E15900'
	template <typename T>
	struct ts_real_policies : boost::spirit::qi::strict_real_policies<T>
	{
        // No exponent
		template <typename Iterator>
		static bool
		parse_exp(Iterator&, Iterator const&) {
			return false;
		}
    };

    namespace qi = boost::spirit::qi;
    using qi::char_;
    using qi::double_;
    using qi::lit;
    using qi::alpha;
    using qi::_val;
    using qi::omit;
    using qi::no_skip;
    using qi::int_;
    using qi::_val;
    using qi::_1;
    using qi::on_error;

}//namespace SpiritShare

// ...........................................................................
//KiCadParser
// ...........................................................................
BOOST_PHOENIX_ADAPT_FUNCTION( void, kipcb_pop_dest, kipcb_pop_dest_expr, 1 )
BOOST_PHOENIX_ADAPT_FUNCTION( void, kipcb_push_dest, kipcb_push_dest_expr, 2 )
BOOST_PHOENIX_ADAPT_FUNCTION( void, kipcb_post_dest, kipcb_post_dest_expr, 2 )
BOOST_PHOENIX_ADAPT_FUNCTION( void, kipcb_word_dest, kipcb_word_dest_expr, 2 )

// ...........................................................................
namespace KiCadParser     //  The skipper grammar
{
	using namespace SpiritShare;

    template <typename Iterator>
    struct skipper : qi::grammar<Iterator>
    {
        skipper() : skipper::base_type( skip )
        {
			qi::eol_type eol;

			comment=
				"#"
				>> *( char_ - eol )
				>> eol
                ;

			skip=
			comment
			| char_( " \x09\x0a\0x0d" )
			;
		}
private:		
        qi::rule<Iterator> skip;
        qi::rule<Iterator> comment;
    };
}

// ...........................................................................
namespace KiCadParser     //  The grammar
{
	using namespace SpiritShare;

	template <typename Iterator>
	struct parser : qi::grammar< Iterator, pcnew_wrapper( ), skipper< Iterator > >
	{
        parser( )
            :parser::base_type( start )
		{
			//main parser
			start=
				//assume well formed
                lit("(kicad_pcb") >> lit( "(version")
				>>  double_ [ bind( &KiCadParser::pcnew_wrapper::version, _val )= _1 ]
				>> ')'
				//pop off the rest of first line, unused for now
				>> s_skip >> s_skip
				//end assume;
				//parse the rest...
				>> *( prop			[ kipcb_push_dest( _val, _1 ) ]
					| pair			[ kipcb_post_dest( _val, _1 ) ]
					| lit(')')		[ kipcb_pop_dest( _val ) ]
					| word			[ kipcb_word_dest( _val, _1 ) ] //dangling attributes are rare but exist
					)
			;
			prop= ( '(' >> word >> parse_data >> & lit('(') );
			pair= '(' >> word >> parse_data >> ')';
			word= ( quoted_string | +( char_ - char_( "() \x09\x0a\0x0d\"" ) ) );

			parse_data= *( uint_p | double_p | word );
			quoted_string= omit [ char_('\"') ] >> no_skip [ *(char_ - char_('\"') )  ] >> lit( '\"' );
			uint_p= int_ >> & ! ( alpha | char_("-,+._") );
			double_p= ts_real >> & ! alpha;
			s_skip= *( qi::char_ - ')' ) >> ')';

//#define _DEBUG_OUT
#ifdef _DEBUG_OUT
//			BOOST_SPIRIT_DEBUG_NODE( start );
			BOOST_SPIRIT_DEBUG_NODE( pair );
			BOOST_SPIRIT_DEBUG_NODE( prop );
			BOOST_SPIRIT_DEBUG_NODE( word );
			BOOST_SPIRIT_DEBUG_NODE( s_skip );
			BOOST_SPIRIT_DEBUG_NODE( quoted_string );
#endif
		}
private:
		//with skipper
		qi::rule< Iterator, pcnew_wrapper( ), skipper< Iterator > > start;
        qi::rule< Iterator, pair_type( ), skipper< Iterator > > pair;
		qi::rule< Iterator, pair_type( ), skipper< Iterator > > prop;
		qi::rule< Iterator, values( ), skipper< Iterator > > parse_data;
		qi::rule< Iterator, skipper< Iterator > > s_skip;
		//without skipper
		qi::rule< Iterator, std::tstring( ) > word;
		qi::rule< Iterator, unsigned int( ) > uint_p;
		qi::real_parser< double, ts_real_policies< double > > ts_real;
		qi::rule< Iterator, double( ) > double_p;
		qi::rule< Iterator, std::tstring( ) > quoted_string;
	};
}

