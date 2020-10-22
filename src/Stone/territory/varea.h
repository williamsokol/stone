//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef VAREA_H
#define VAREA_H

#include <BWAPI.h>
#include "../utils.h"
#include "../defs.h"


namespace stone
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VArea
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class VArea
{
public:
	static VArea *					Get(const BWEM::Area * pBWEMPart);

									VArea(const BWEM::Area * pBWEMPart);
									~VArea();

	const BWEM::Area *				BWEMPart() const		{ return m_pBWEMPart; }

	VArea &							operator=(const VArea &) = delete;

private:
	const BWEM::Area *				m_pBWEMPart;
};



} // namespace stone


#endif

