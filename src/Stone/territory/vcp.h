//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef VCP_H
#define VCP_H

#include <BWAPI.h>
#include "../utils.h"
#include "../defs.h"


namespace stone
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VChokePoint
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class VChokePoint
{
public:
	static VChokePoint *				Get(const BWEM::ChokePoint * pBWEMPart);

										VChokePoint(const BWEM::ChokePoint * pBWEMPart);
										~VChokePoint();

	const BWEM::ChokePoint *			GetChokePoint() const		{ return m_pBWEMPart; }

	int									AirDistanceFrom(Position p) const;

	VChokePoint &						operator=(const VChokePoint &) = delete;

private:
	const BWEM::ChokePoint *			m_pBWEMPart;
};



} // namespace stone


#endif

