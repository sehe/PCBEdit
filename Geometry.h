
//Geometry.h

#pragma once

#ifndef GEOMETRY_H
#define GEOMETRY_H

// ..........................................................................
typedef TCHAR TPCB_CHAR;
typedef __int64 TPCB_NUM;
// max signed  9,223,372,036,854,775,807

//milimeters to nanometers
#define MILL_MULT 1000000

//typedef double TPCB_NUM;
typedef unsigned int TPCB_UINT;

template < typename NumType >
struct bg_point_
{
	TPCB_NUM x;
	TPCB_NUM y;
	bg_point_( )
		:x( 0 )
		,y( 0 )
	{ }
	bg_point_( TPCB_NUM ix, TPCB_NUM iy )
		:x( ix )
		,y( iy )
	{ }
    template <std::size_t K>
   inline NumType const& get( ) const
    {
        BOOST_STATIC_ASSERT(K < 2);
		if( K == 0 )
	        return x;
		//else
		return y;
   }

    template <std::size_t K>
    inline void set(NumType const& value)
    {
        BOOST_STATIC_ASSERT(K < 2);
		if( K == 0 )
	        x= value;
		else
			y= value;
    }
	TPCB_NUM get_x( ) const { return x; }
	TPCB_NUM get_y( ) const { return y; }
	void set_x( TPCB_NUM ix ) { x= ix; }
	void set_y( TPCB_NUM iy ) { y= iy; }
};


namespace boost { namespace geometry
{

///*!
//\brief Meta-function defining return type of distance function
//\ingroup distance
//\note The strategy defines the return-type (so this situation is different
//    from length, where distance is sqr/sqrt, but length always squared)
// */
////template <typename Geometry1, typename Geometry2 = Geometry1>
//struct default_distance_result
//{
//    typedef typename strategy::distance::services::return_type
//        <
//            typename strategy::distance::services::default_strategy
//                <
//                    point_tag,
//                    typename point_type<__int64>::type,
//                    typename point_type<__int64>::type
//                >::type,
//            typename point_type<__int64>::type,
//            typename point_type<__int64>::type
//        >::type type;
//};


}} // namespace boost::geometry
BOOST_GEOMETRY_REGISTER_POINT_2D( bg_point_< TPCB_NUM >, TPCB_NUM, bg::cs::cartesian, x, y );
namespace boost { namespace geometry { namespace traits {
}}}

typedef bg_point_< TPCB_NUM > bg_point;
typedef bg::model::segment< bg_point > bg_line;
typedef bg::model::box< bg_point > bg_box;
typedef bg::model::polygon< bg_point > bg_polygon;

// ............................................................................
inline bool operator == ( const bg_point& a, const bg_point& b );
inline bool operator != ( const bg_point& a, const bg_point& b );

// ............................................................................
inline bool operator == ( const bg_point& a, const bg_point& b )
{
	return a.get< 0 >( ) == b.get< 0 >( ) && a.get< 1 >( ) == b.get< 1 >( );
}

// ............................................................................
inline bool operator != ( const bg_point& a, const bg_point& b )
{
	return ! ( a == b );
}

// ..........................................................................
inline bg_point	operator + ( bg_point const& a, bg_point const& b )
{
	return bg_point( a.get_x( ) + b.get_x( ), a.get_y( ) + b.get_y( ) );
}

// ..........................................................................
inline bg_point	operator + ( bg_point const& a, TPCB_NUM const& b )
{
	return bg_point( a.get_x( ) + b, a.get_y( ) + b );
}

// ............................................................................
inline bg_point operator - ( bg_point& a, const bg_point& b )
{
	return bg_point( a.get< 0 >( ) - b.get< 0 >( ), a.get< 1 >( ) - b.get< 1 >( ) );
}

// ............................................................................
inline bg_point operator - ( bg_point& a, const TPCB_NUM& b )
{
	return bg_point( a.get< 0 >( ) - b, a.get< 1 >( ) - b );
}

// ..........................................................................
inline bool operator < ( const bg_point& a, const bg_point& b )
{
	if( a.get< 0 >( ) < b.get< 0 >( ) )
		return true;
	else if( a.get< 0 >( ) > b.get< 0 >( ) )
		return false;
	return a.get< 1 >( ) < b.get< 1 >( );
}

// ............................................................................
inline void operator += ( bg_point& a, const bg_point& b )
{
	a.set< 0 >( a.get< 0 >( ) + b.get< 0 >( ) );
	a.set< 1 >( a.get< 1 >( ) + b.get< 1 >( ) );
}

// ............................................................................
inline void operator += ( bg_point& a, TPCB_NUM const& b )
{
	a.set< 0 >( a.get< 0 >( ) + b );
	a.set< 1 >( a.get< 1 >( ) + b );
}

// ............................................................................
inline void operator -= ( bg_point& a, const bg_point& b )
{
	a.set< 0 >( a.get< 0 >( ) - b.get< 0 >( ) );
	a.set< 1 >( a.get< 1 >( ) - b.get< 1 >( ) );
}

// ............................................................................
inline void operator -= ( bg_point& a, TPCB_NUM const& b )
{
	a.set< 0 >( a.get< 0 >( ) - b );
	a.set< 1 >( a.get< 1 >( ) - b );
}

// ............................................................................
inline bg_point operator / ( bg_point& a, const bg_point& b )
{
	return bg_point( a.get< 0 >( ) / b.get< 0 >( ), a.get< 1 >( ) / b.get< 1 >( ) );
}

// ............................................................................
inline bg_point operator / ( bg_point& a, const TPCB_NUM& b )
{
	return bg_point( a.get< 0 >( ) / b, a.get< 1 >( ) / b );
}

// ............................................................................
inline std::wostream& operator << ( std::wostream& out, const bg_point& p )
{
	out << p.get_x( ) << ',' << p.get_y( );
	return out;
}

#endif // GEOMETRY_H
