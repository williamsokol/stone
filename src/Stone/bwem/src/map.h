//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef BWEM_MAP_H
#define BWEM_MAP_H

#include <BWAPI.h>
#include <vector>
#include <memory>
#include "tiles.h"
#include "area.h"
#include "cp.h"
#include "utils.h"
#include "defs.h"


namespace BWEM
{

class Tile;
class MiniTile;
class Area;
class Mineral;
class Geyser;
class StaticBuilding;
class ChokePoint;


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Map
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Map is the entry point:
//	- to access general information on the Map
//	- to access the Tiles and the MiniTiles
//	- to access the Areas
//	- to access the StartingLocations
//	- to access the Minerals, the Geysers and the StaticBuildings
//	- to parametrize the analysis process
//	- to update the information
// Map also provides some useful tools such as Paths between ChokePoints and generic algorithms like BreadthFirstSearch
//
// Map functionnality is provided through its singleton Map::Instance only.

class Map
{
public:
	// Returns the unique instance (singleton).
	// It is equal to use Map::Instance() each time, or store its result as use it instead.
	static Map &						Instance();

	// This has to be called before any other function is called.
	// A good place to call it is ExampleAIModule::onStart()
	virtual void						Initialize() = 0;

	// Returns the status of the automatic path update (off by default)
	// When on, each time a blocking Neutral (either Mineral or StaticBuilding) is destroyed,
	// any information relative to the paths through the Areas is updated accordingly.
	// For this to function, the Map needs to be informed of such destructions (call OnMineralDestroyed and OnStaticBuildingDestroyed)
	virtual bool						AutomaticPathUpdate() const = 0;

	// Enables the automatic path update (Cf. AutomaticPathUpdate())
	virtual void						EnableAutomaticPathAnalysis() const = 0;

	// Tries to assign one Base for each starting Location in StartingLocations().
	// Returns whether the function succeeded (a fail may indicate a bug in BWEM or a suboptimal placement in one of the starting Locations.
	// You normally should call this function.
	virtual bool						FindBasesForStartingLocations() = 0;

	// Returns the size of the Map in Tiles
	const BWAPI::TilePosition &			Size() const								{ return m_Size; }

	// Returns the size of the Map in MiniTiles
	const BWAPI::WalkPosition &			WalkSize() const							{ return m_WalkSize; }

	// Returns the center of the Map in pixels
	BWAPI::Position						Center() const								{ return m_center; }

	// Returns a random position in the Map in pixels
	BWAPI::Position						RandomPosition() const;

	// Returns the maximum altitude in the whole Map (Cf. MiniTile::Altitude())
	virtual altitude_t					MaxAltitude() const = 0;

	// Returns the number of Bases
	virtual int							BaseCount() const = 0;

	// Returns the number of ChokePoints
	virtual int							ChokePointCount() const = 0;

	// Returns a Tile, given its position
	const Tile &						GetTile(const BWAPI::TilePosition & p, bool noCheck = false) const 			{ bwem_assert(noCheck || Valid(p)); utils::unused(noCheck); return m_Tiles[Size().x * p.y + p.x]; }

	// Returns a MiniTile, given its position
	const MiniTile &					GetMiniTile(const BWAPI::WalkPosition & p, bool noCheck = false) const 		{ bwem_assert(noCheck || Valid(p)); utils::unused(noCheck); return m_MiniTiles[WalkSize().x * p.y + p.x]; }

	// Returns a Tile or a MiniTile, given its position
	// Provided for support of generic algorithms.
	template<class TPosition>
	typename const utils::TileOfPosition<TPosition>::type & GetTTile(const TPosition & p, bool noCheck = false) const;

	// Provides access to the internal array of Tiles 
	const std::vector<Tile> &			Tiles() const									{ return m_Tiles; }

	// Provides access to the internal array of MiniTiles 
	const std::vector<MiniTile> &		MiniTiles() const								{ return m_MiniTiles; }

	// Returns wehther the Tile position p is valid
	bool								Valid(const BWAPI::TilePosition & p) const		{ return (0 <= p.x) && (p.x < Size().x) && (0 <= p.y) && (p.y < Size().y); }

	// Returns wehther the MiniTile position p is valid
	bool								Valid(const BWAPI::WalkPosition & p) const		{ return (0 <= p.x) && (p.x < WalkSize().x) && (0 <= p.y) && (p.y < WalkSize().y); }

	bool								Valid(const BWAPI::Position & p) const			{ return Valid(BWAPI::WalkPosition(p)); }

	BWAPI::WalkPosition					Crop(const BWAPI::WalkPosition & p) const;
	BWAPI::TilePosition					Crop(const BWAPI::TilePosition & p) const		{ return BWAPI::TilePosition(Crop(BWAPI::WalkPosition(p))); }
	BWAPI::Position						Crop(const BWAPI::Position & p) const;

	// Returns a reference to the starting Locations.
	// Note: these are the ones provided by BWAPI::getStartLocations().
	virtual const std::vector<BWAPI::TilePosition> &				StartingLocations() const = 0;

	// Returns a reference to the Minerals (Cf. Mineral)
	virtual const std::vector<std::unique_ptr<Mineral>> &			Minerals() const = 0;

	// Returns a reference to the Geysers (Cf. Geyser)
	virtual const std::vector<std::unique_ptr<Geyser>> &			Geysers() const = 0;

	// Returns a reference to the StaticBuildings (Cf. StaticBuilding)
	virtual const std::vector<std::unique_ptr<StaticBuilding>> &	StaticBuildings() const = 0;

	// If a Mineral wrappers the given BWAPI unit, returns a pointer to it.
	// Otherwise, returns nullptr.
	virtual Mineral *					GetMineral(BWAPI::Unit u) const = 0;

	// If a Geyser wrappers the given BWAPI unit, returns a pointer to it.
	// Otherwise, returns nullptr.
	virtual Geyser *					GetGeyser(BWAPI::Unit g) const = 0;

	// Should be called for each destroyed BWAPI unit u having u->getType().isMineralField() == true
	virtual void						OnMineralDestroyed(BWAPI::Unit u) = 0;

	// Should be called for each destroyed BWAPI unit u having u->getType().isSpecialBuilding() == true
	virtual void						OnStaticBuildingDestroyed(BWAPI::Unit u) = 0;

	// Returns a reference to the Areas
	virtual const std::vector<Area> &	Areas() const = 0;

	// Returns an Area given its id.
	virtual const Area *				GetArea(Area::id id) const = 0;

	// If the MiniTile at w is walkable and is part of an Area, returns that Area.
	// Otherwise, returns nullptr;
	virtual const Area *				GetArea(BWAPI::WalkPosition w) const = 0;

	// If the Tile at t contains walkable sub-MiniTiles which are all part of the same Area, returns that Area.
	// Otherwise, returns nullptr;
	virtual const Area *				GetArea(BWAPI::TilePosition t) const = 0;

	// Returns the nearest Area from w.
	// Returns nullptr only if Areas().empty()
	// Note: Uses a breadth first search.
	virtual const Area *				GetNearestArea(BWAPI::WalkPosition w) const = 0;

	// Returns the nearest Area from t.
	// Returns nullptr only if Areas().empty()
	// Note: Uses a breadth first search.
	virtual const Area *				GetNearestArea(BWAPI::TilePosition t) const = 0;


	// Returns a list of ChokePoints, which is intended to be the shortest walking path from a to b.
	// Furthermore, if pLength != nullptr, the integer at its address is set to the corresponding length in pixels.
	// If a is not accessible from b, the empty Path is returned, and -1 is put in *pLength (if pLength != nullptr).
	// If a and b are in the same Area, the empty Path is returned, and a.getApproxDistance(b) is put in *pLength (if pLength != nullptr).
	// Otherwise, the function relies on ChokePoint::GetPathTo.
	// Note: in order to retrieve the Areas of a and b, the function starts by calling
	//       GetNearestArea(TilePosition(a)) and GetNearestArea(TilePosition(b)).
	//       While this brings robustness, this could yield surprising results in the case where a and/or b are in the Water.
	//       To avoid this and the potential performance penalty, just make sure GetArea(a) != nullptr and GetArea(b) != nullptr.
	//       Then GetPath should perform very quick.
	// Cf. ChokePoint::GetPathTo for more information
	virtual const CPPath &				GetPath(const BWAPI::Position & a, const BWAPI::Position & b, int * pLength = nullptr) const = 0;

	// Generic algorithm for breadth first search in the Map.
	// See the several use cases in BWEM source files.
	template<class TPosition, class Pred1, class Pred2>
	TPosition							BreadthFirstSearch(TPosition start, Pred1 findCond, Pred2 visitCond) const;

	virtual const std::vector<std::pair<std::pair<Area::id, Area::id>, BWAPI::WalkPosition>> & RawFrontier() const = 0;

	virtual								~Map() = default;


protected:
										Map() = default;

	Tile &								GetTile_(const BWAPI::TilePosition & p, bool noCheck = false)				{ return const_cast<Tile &>(static_cast<const Map &>(*this).GetTile(p, noCheck)); }
	MiniTile &							GetMiniTile_(const BWAPI::WalkPosition & p, bool noCheck = false)			{ return const_cast<MiniTile &>(static_cast<const Map &>(*this).GetMiniTile(p, noCheck)); }

	int							m_size;
	BWAPI::TilePosition			m_Size;

	int							m_walkSize;
	BWAPI::WalkPosition			m_WalkSize;

	BWAPI::Position				m_center;
	std::vector<Tile>			m_Tiles;
	std::vector<MiniTile>		m_MiniTiles;

private:
	static std::unique_ptr<Map>	m_gInstance;

};


template<>
inline typename const Tile & Map::GetTTile<BWAPI::TilePosition>(const BWAPI::TilePosition & t, bool noCheck) const
{
	return GetTile(t, noCheck);
}

template<>
inline typename const MiniTile & Map::GetTTile<BWAPI::WalkPosition>(const BWAPI::WalkPosition & w, bool noCheck) const
{
	return GetMiniTile(w, noCheck);
}


template<class TPosition, class Pred1, class Pred2>
inline TPosition Map::BreadthFirstSearch(TPosition start, Pred1 findCond, Pred2 visitCond) const
{
	typedef typename utils::TileOfPosition<TPosition>::type Tile_t;
	if (findCond(GetTTile(start), start)) return start;

	vector<TPosition> Visited;
	queue<TPosition> ToVisit;

	ToVisit.push(start);
	Visited.push_back(start);

	while (!ToVisit.empty())
	{
		TPosition current = ToVisit.front();
		ToVisit.pop();
		for (TPosition delta : {	TPosition(-1, -1), TPosition(0, -1), TPosition(+1, -1),
									TPosition(-1,  0),                   TPosition(+1,  0),
									TPosition(-1, +1), TPosition(0, +1), TPosition(+1, +1)})
		{
			TPosition next = current + delta;
			if (Valid(next))
			{
				const Tile_t & Next = GetTTile(next, no_check); 
				if (findCond(Next, next)) return next;

				if (visitCond(Next, next) && !contains(Visited, next))
				{
					ToVisit.push(next);
					Visited.push_back(next);
				}
			}
		}
	}

	bwem_assert(false);
	return start;
}



namespace utils {

bool seaSide(BWAPI::WalkPosition p, const Map * pMap);

} // namespace utils


} // namespace BWEM


#endif

