//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "examples.h"
#include "map.h"
#include "base.h"
#include "neutral.h"
#include "mapDrawer.h"
#include "bwapiExt.h"

using namespace BWAPI;
using namespace BWAPI::UnitTypes::Enum;
namespace { auto & bw = Broodwar; }

using namespace std;


namespace BWEM {

using namespace utils;
using namespace BWAPI_ext;

namespace utils
{




void drawMap(const Map & theMap)
{
	if (MapDrawer::showFrontier)
		for (auto f : theMap.RawFrontier())
			bw->drawBoxMap(Position(f.second), Position(f.second + 1), MapDrawer::Color::frontier, bool("isSolid"));

	for (int y = 0 ; y < theMap.Size().y ; ++y)
	for (int x = 0 ; x < theMap.Size().x ; ++x)
	{
		TilePosition t(x, y);
		const Tile & tile = theMap.GetTile(t, no_check);
		
		if (MapDrawer::showUnbuildable && !tile.Buildable())
			drawDiagonalCrossMap(Position(t), Position(t + 1), MapDrawer::Color::unbuildable);
	}

	for (int y = 0 ; y < theMap.WalkSize().y ; ++y)
	for (int x = 0 ; x < theMap.WalkSize().x ; ++x)
	{
		WalkPosition w(x, y);
		const MiniTile & miniTile = theMap.GetMiniTile(w, no_check);
			
		if (MapDrawer::showSeas && miniTile.Sea())
			drawDiagonalCrossMap(Position(w), Position(w + 1), MapDrawer::Color::sea);
		
		if (MapDrawer::showLakes && miniTile.Lake())
			drawDiagonalCrossMap(Position(w), Position(w + 1), MapDrawer::Color::lakes);
	}

	if (MapDrawer::showCP)
		for (const Area & area : theMap.Areas())
			for (const ChokePoint * cp : area.ChokePoints())
				for (ChokePoint::node end : {ChokePoint::end1, ChokePoint::end2})
					bw->drawLineMap(Position(cp->Pos(ChokePoint::middle)), Position(cp->Pos(end)), MapDrawer::Color::cp);

	if (MapDrawer::showMinerals)
		for (auto & m : theMap.Minerals())
		{
			bw->drawBoxMap(Position(m->TopLeft()), Position(m->TopLeft() + m->Size()) , MapDrawer::Color::minerals);
			if (m->Blocking())
				drawDiagonalCrossMap(Position(m->TopLeft()), Position(m->TopLeft() + m->Size()), MapDrawer::Color::minerals);
		}

	if (MapDrawer::showGeysers)
		for (auto & g : theMap.Geysers())
			bw->drawBoxMap(Position(g->TopLeft()), Position(g->TopLeft() + g->Size()) , MapDrawer::Color::geysers);

	if (MapDrawer::showStaticBuildings)
		for (auto & s : theMap.StaticBuildings())
		{
			bw->drawBoxMap(Position(s->TopLeft()), Position(s->TopLeft() + s->Size()) , MapDrawer::Color::staticBuildings);
			if (s->Blocking())
				drawDiagonalCrossMap(Position(s->TopLeft()), Position(s->TopLeft() + s->Size()), MapDrawer::Color::staticBuildings);
		}

	for (const Area & area : theMap.Areas())
		for (const Base & base : area.Bases())
		{
			if (MapDrawer::showBases)
				bw->drawBoxMap(Position(base.Location()), Position(base.Location() + UnitType(Terran_Command_Center).tileSize()) , MapDrawer::Color::bases);

			if (MapDrawer::showAssignedRessources)
			{
				vector<Ressource *> AssignedRessources(base.Minerals().begin(), base.Minerals().end());
				AssignedRessources.insert(AssignedRessources.end(), base.Geysers().begin(), base.Geysers().end());
				for (const Ressource * r : AssignedRessources)
					bw->drawLineMap(base.Center(), r->Pos(), MapDrawer::Color::assignedRessources);
			}
		}
}



}} // namespace BWEM::utils



