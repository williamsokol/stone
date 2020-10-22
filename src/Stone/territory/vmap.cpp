//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "vmap.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }

namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VMap
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


VMap::VMap(BWEM::Map * pMap)
	: m_pMap(pMap)
{
	m_Bases.reserve(pMap->BaseCount());
	m_Areas.reserve(pMap->Areas().size());
	m_ChokePoints.reserve(pMap->ChokePointCount());

	ChokePoint::UnmarkAll();
	for (const Area & area : pMap->Areas())
	{
		m_Areas.emplace_back(&area);

		for (const Base & base : area.Bases())
			m_Bases.emplace_back(&base);

		for (const ChokePoint * cp : area.ChokePoints())
			if (!cp->Marked())
			{
				cp->SetMarked();
				m_ChokePoints.emplace_back(cp);
			}
	}

	vector<const Neutral *> Neutrals;
	for (const auto & m : pMap->Minerals()) Neutrals.push_back(m.get());
	for (const auto & g : pMap->Geysers()) Neutrals.push_back(g.get());
	for (const auto & sb : pMap->StaticBuildings()) Neutrals.push_back(sb.get());

	for (const Neutral * n : Neutrals)
	{
		vector<TilePosition> Border = outerBorder(n->TopLeft(), n->Size(), bool("noCorner"));
		for (auto t : Border)
			if (pMap->Valid(t))
				ai()->GetVMap().SetNearNeutral(pMap->GetTile(t, no_check));

		if (n->IsRessource())
			for (int dy = -3 ; dy < n->Size().y + 3 ; ++dy)
			for (int dx = -3 ; dx < n->Size().x + 3 ; ++dx)
			{
				TilePosition t = n->TopLeft() + TilePosition(dx, dy);
				if (pMap->Valid(t))
					ai()->GetVMap().SetCommandCenterImpossible(pMap->GetTile(t, no_check));
			}
	}
}


WalkPosition VMap::GetIncreasingAltitudeDirection(WalkPosition pos) const
{
	assert_throw(GetMap()->Valid(pos));

	altitude_t maxAltitude = -1;
	WalkPosition bestDir;

	for (WalkPosition delta : { WalkPosition(-1, -1), WalkPosition(0, -1), WalkPosition(+1, -1),
								WalkPosition(-1,  0),                      WalkPosition(+1,  0),
								WalkPosition(-1, +1), WalkPosition(0, +1), WalkPosition(+1, +1)})
	{
		WalkPosition w = pos + delta;
		if (GetMap()->Valid(w))
		{
			altitude_t altitude = GetMap()->GetMiniTile(w, no_check).Altitude();
			if (altitude > maxAltitude)
			{
				maxAltitude = altitude;
				bestDir = delta;
			}
		}
	}

	return bestDir;
}


Position VMap::RandomPosition(Position center, int radius) const
{
	return GetMap()->Crop(center + Position(radius - rand()%(2*radius), radius - rand()%(2*radius)));
}


Position VMap::RandomSeaPosition() const
{
	for (int tries = 0 ; tries < 1000 ; ++tries)
	{
		Position p = GetMap()->RandomPosition();
		if (GetMap()->GetMiniTile(WalkPosition(p), no_check).Sea())
			return p;
	}

	return GetMap()->RandomPosition();
}

	
} // namespace stone



