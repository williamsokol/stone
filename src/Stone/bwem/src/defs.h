//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef BWEM_DEFS_H
#define BWEM_DEFS_H

#include <assert.h>
#include "../../myassert.h"

namespace BWEM
{

namespace detail
{
	void onAssertThrowFailed(const std::string & file, int line, const std::string & condition, const std::string & message);

} // namespace details

/*
#define bwem_assert_debug_only(expr)			assert(expr)
#define bwem_assert_plus(expr, message)			assert(expr)
#define bwem_assert(expr)						bwem_assert_plus(expr, "")
#define bwem_assert_throw_plus(expr, message)   ((expr)?(void)0:detail::onAssertThrowFailed(__FILE__,__LINE__, #expr, message))
#define bwem_assert_throw(expr)					bwem_assert_throw_plus(expr, "")
*/

#define bwem_assert_debug_only(expr)			assert_(expr)
#define bwem_assert_plus(expr, message)			assert_throw_plus(expr, message)
#define bwem_assert(expr)						bwem_assert_plus(expr, "")
#define bwem_assert_throw_plus(expr, message)   assert_throw_plus(expr, message)
#define bwem_assert_throw(expr)					bwem_assert_throw_plus(expr, "")





class BWEMException : public std::runtime_error
{
public:
	explicit                BWEMException(const char * message) : std::runtime_error(message) {}
	explicit                BWEMException(const std::string & message) : BWEMException(message.c_str()) {}
};







typedef int16_t altitude_t;		// type of the altitudes, in pixels




namespace utils
{

const bool no_check = true;

} // namespace utils


namespace detail
{

const int lake_max_size = 300;
const int lake_max_width = 8*4;

const int area_min_miniTiles = 64;

const int max_tiles_between_CommandCenter_and_ressources = 10;
const int min_tiles_between_Bases = 10;

} // namespace detail


} // namespace BWEM


#endif

