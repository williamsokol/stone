//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "his.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class HisUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


void HisUnit::SetPursuingTarget(MyUnit * pAgent)
{
	m_pPursuingTarget = pAgent;
	m_pPursuingTargetLastFrame = ai()->Frame();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class HisBuilding
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


HisBuilding::HisBuilding(BWAPI::Unit u)
	: HisBWAPIUnit(u), m_size(u->getType().tileSize())
{
	if (!Flying()) PutBuildingOnTiles();
}


HisBuilding::~HisBuilding()
{
	try	//3
	{
		if (!InFog() && !Flying()) RemoveBuildingFromTiles();
	}
	catch(...){}
}


void HisBuilding::SetInFog()
{
	assert_throw(!InFog());
	
	m_inFog = true;
	m_lastFrameNoVisibleTile = ai()->Frame();

	if (!Flying()) RemoveBuildingFromTiles();
}


bool HisBuilding::AtLeastOneTileIsVisible() const
{
	for (auto t : {TopLeft(), TopLeft() + TilePosition(Size().x-1, 0), TopLeft() + TilePosition(0, Size().y-1), TopLeft() + Size()-1})
		if (bw->isVisible(t))
			return true;

	return false;
}


void HisBuilding::Update()
{
	if (InFog())
	{
		if (!AtLeastOneTileIsVisible())
			m_lastFrameNoVisibleTile = ai()->Frame();
	}
	else
	{
		HisBWAPIUnit::Update();

		if (JustLifted()) RemoveBuildingFromTiles();
		if (JustLanded()) PutBuildingOnTiles();
	}
}


	
} // namespace stone



