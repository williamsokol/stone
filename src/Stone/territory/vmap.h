//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef VMAP_H
#define VMAP_H

#include <BWAPI.h>
#include "vbase.h"
#include "varea.h"
#include "vcp.h"
#include "../utils.h"


namespace stone
{

class BWAPIUnit;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VMap
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class VMap
{
public:
									VMap(BWEM::Map * pMap);

	const BWEM::Map *				GetMap() const		{ return m_pMap; }

	const vector<VBase> &			Bases() const		{ return m_Bases; }
	vector<VBase> &					Bases()				{ return m_Bases; }

	const vector<VArea> &			Areas() const		{ return m_Areas; }
	vector<VArea> &					Areas()				{ return m_Areas; }

	const vector<VChokePoint> &		ChokePoints() const	{ return m_ChokePoints; }
	vector<VChokePoint> &			ChokePoints()		{ return m_ChokePoints; }

	BWAPIUnit *						GetBuildingOn(const Tile & tile) const			{ return static_cast<BWAPIUnit *>(tile.Ptr()); }
	void							SetBuildingOn(const Tile & tile, BWAPIUnit * b) { assert_throw(!b != !tile.Ptr()); tile.SetPtr(b); }

	bool							ImpasseOrNearBuildingOrNeutral(const Tile & tile) const	{ return (tile.Data() & 7) != 0; }
	
	bool							Impasse(const Tile & tile) const				{ return (tile.Data() & 4) != 0; }

	void							SetNearBuilding(const Tile & tile)				{ tile.SetData(tile.Data() | 1); }
	void							UnsetNearBuilding(const Tile & tile)			{ tile.SetData(tile.Data() & ~1); }

	void							SetNearNeutral(const Tile & tile)				{ tile.SetData(tile.Data() | 2); }
	void							UnsetNearNeutral(const Tile & tile)				{ tile.SetData(tile.Data() & ~2); }

	void							SetImpasse(const Tile & tile)					{ tile.SetData(tile.Data() | 4); }

	bool							CommandCenterImpossible(const Tile & tile) const{ return (tile.Data() & 8) != 0; }
	void							SetCommandCenterImpossible(const Tile & tile)	{ tile.SetData(tile.Data() | 8); }

	WalkPosition					GetIncreasingAltitudeDirection(WalkPosition pos) const;

	Position						RandomPosition(Position center, int radius) const;
	Position						RandomSeaPosition() const;


	VMap &							operator=(const VMap &) = delete;

private:
	BWEM::Map *						m_pMap;
	vector<VBase>					m_Bases;
	vector<VArea>					m_Areas;
	vector<VChokePoint>				m_ChokePoints;
};



} // namespace stone


#endif

