//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "vcp.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }

namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VChokePoint
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


VChokePoint * VChokePoint::Get(const BWEM::ChokePoint * pBWEMPart)
{
	return static_cast<VChokePoint *>(pBWEMPart->Ext());
}


VChokePoint::VChokePoint(const BWEM::ChokePoint * pBWEMPart)
	: m_pBWEMPart(pBWEMPart)
{
	assert_throw(!VChokePoint::Get(m_pBWEMPart));
	m_pBWEMPart->SetExt(this);

}


VChokePoint::~VChokePoint()
{
	m_pBWEMPart->SetExt(nullptr);

}


int VChokePoint::AirDistanceFrom(Position p) const
{
	int minDist = 0;
	for (auto node : {ChokePoint::end1, ChokePoint::middle, ChokePoint::end2})
		minDist = min(minDist, p.getApproxDistance(Position(GetChokePoint()->Pos(node))));

	return minDist;
}
	
} // namespace stone



