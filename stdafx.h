
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
#define _SCL_SECURE_NO_WARNINGS

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#define _INTSAFE_H_INCLUDED_
#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <fstream>

//for boost
#include <string>
#include <memory>
#include <locale>
#include <iostream>
#include <exception>

// ..........................................................................
//http://stackoverflow.com/questions/2055849/visual-c-migrating-traditional-c-and-c-string-code-to-a-unicode-world
namespace std
{

#ifdef _MSC_VER

#ifdef UNICODE
typedef             wstring                         tstring ;
typedef             wistream                        tistream ;
// etc.
#else // Not UNICODE
typedef             string                          tstring ;
typedef             istream                         tistream ;
// etc.
#endif

#else // Not UNICODE
typedef             string                          tstring ;
typedef             istream                         tistream ;

#endif

} // namespace std

#include <vector>
#include <list>
#include <stack>

//#include <BoostBasic.h>

// BOOST GEOMETERY
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244)      // we are mixing doubles and __int64
#endif
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/core/point_order.hpp> 
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Optional includes to handle c-arrays as points, std::vectors as linestrings
//#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
//#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>
namespace bg= boost::geometry;
namespace bgstrat = bg::strategy;

#include <boost/any.hpp>
typedef std::list<boost::any> boost_many;
#include <boost/variant.hpp>
#include <boost/tokenizer.hpp>

#include <boost/program_options.hpp>
namespace po= boost::program_options;

#include <boost/algorithm/string.hpp>    
namespace ba= boost::algorithm;

#include <boost/unordered_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/assign/list_inserter.hpp> // for 'insert()'

//#include <boost/lambda/lambda.hpp>
//#include <boost/lambda/bind.hpp>
#include <boost/range/algorithm/find_if.hpp>

#define USE_WINDOWS_EX
//#include <ctrlext.h>

#define USING_XML
//#include <xml.h>

#define _T(a) a
using TCHAR = char;
using __int64 = std::int64_t;
using LPCTSTR = TCHAR const*;

//this is here so avaliable globaly pcb and xmlsettingsstore
static char const* const DUMMY_REGISTRY_PATH= _T("DummyPCBRegistryPath");
static char const* const USER_SETTINGS_FILENAME= _T("settings.xml");

template <typename T>
using AStrType = char const*;

// ................................


