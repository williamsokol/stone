//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef DEFS_H
#define DEFS_H

#include <BWAPI.h>
#include "bwem/src/bwem.h"
#include "vect.h"

using namespace BWAPI;
using namespace BWAPI::UnitTypes::Enum;

using namespace BWEM;
using namespace BWEM::BWAPI_ext;
using namespace BWEM::utils;

namespace stone
{


typedef int frame_t;
typedef int gameTime_t;
typedef int delay_t;
typedef BWAPI::UnitTypes::Enum::Enum tid_t;

#define FORWARD_DECLARE_MY(tid)	template<tid_t> class My; template<> class My<tid>;

const double pi = acos(-1);



inline Position toPosition(const Vect & v)	{ return Position(int(0.5 + v.x), int(0.5 + v.y)); }
inline Vect toVect(const Position & p)		{ return Vect(p.x,p.y); }


} // namespace Stone


#endif

