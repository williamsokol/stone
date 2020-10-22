//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "constructing.h"
#include "fleeing.h"
#include "walking.h"
#include "defaultBehavior.h"
#include "../units/my.h"
#include "../territory/stronghold.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{


bool beingConstructed(BWAPI::UnitType type)
{
	return any_of(Constructing::Instances().begin(), Constructing::Instances().end(), [type]
		(const Constructing * p){ return p->Type() == type; });
}



VBase * findNewBase()
{
	VBase * pMyStartingBase = ai()->Me().Bases().front();
	map<int, VBase *> Candidates;
	for (auto & base : ai()->GetVMap().Bases())
		if (&base != pMyStartingBase)
			if (!base.GetStronghold())
			{
				int length;
				ai()->GetMap().GetPath(Position(pMyStartingBase->BWEMPart()->GetArea()->Top()), base.BWEMPart()->Center(), &length);
				if (length > 0)
					Candidates[length] = &base;
			}

	return Candidates.empty() ? nullptr : Candidates.begin()->second;
}



static bool possibleLocation(BWAPI::UnitType type, TilePosition location)
{
	for (int dy = -1 ; dy <= type.tileSize().y ; ++dy)
	for (int dx = -1 ; dx <= type.tileSize().x ; ++dx)
	{
		const auto & tile = ai()->GetMap().GetTile(location + TilePosition(dx, dy));
		if (!tile.Buildable()) return false;
		if (tile.GetNeutral()) return false;
		if (ai()->GetVMap().GetBuildingOn(tile)) return false;
		if (ai()->GetVMap().CommandCenterImpossible(tile)) return false;
	}

	return true;
}


static const TilePosition findBuildingPlacement(BWAPI::UnitType type, TilePosition near)
{
/*
	if (type == Terran_Command_Center)
	{
		if (VBase * pNewBase = findNewBase())
			return pNewBase->BWEMPart()->Location();

		return TilePositions::None;
	}
*/

	map<int, TilePosition> Candidates;
	const int radius = 32;
	
	TilePosition SearchBoundingBox_TopLeft = ai()->GetMap().Crop(near - radius);
	TilePosition SearchBoundingBox_BottomRight = ai()->GetMap().Crop(near + radius + type.tileSize()-1);
	SearchBoundingBox_BottomRight -= type.tileSize()-1;

	SearchBoundingBox_TopLeft = SearchBoundingBox_TopLeft + 1;
	SearchBoundingBox_BottomRight = SearchBoundingBox_BottomRight - 1;

	for (int y = SearchBoundingBox_TopLeft.y ; y <= SearchBoundingBox_BottomRight.y ; ++y)
	for (int x = SearchBoundingBox_TopLeft.x ; x <= SearchBoundingBox_BottomRight.x ; ++x)
	{
		TilePosition target(x, y);
		if (ai()->GetMap().GetArea(target) == ai()->Me().Bases().front()->BWEMPart()->GetArea())
			if (possibleLocation(type, target)) Candidates[target.getApproxDistance(near)] = target;

	}

	return Candidates.empty() ? TilePositions::None : Candidates.begin()->second;
}



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Constructing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Constructing *> Constructing::m_Instances;


Constructing::Constructing(My<Terran_SCV> * pSCV, BWAPI::UnitType type)
	: Behavior(pSCV), m_type(type), m_pBase(pSCV->GetStronghold()->HasBase())
{
	assert_throw(pSCV->GetStronghold()->HasBase());

	push_back_assert_does_not_contain(m_Instances, this);
}


Constructing::~Constructing()
{
	try //3
	{
		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
	catch(...){}
}


string Constructing::StateName() const
{
	switch(State())
	{
	case reachingLocation:	return "reachingLocation";
	case waitingForMoney:	return "waitingForMoney";
	case constructing:		return "constructing";
	default:			return "?";
	}
}


void Constructing::OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other)
{
	if (other->TopLeft() == Location()) 
		return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}


void Constructing::OnFrame_v()
{
//1	if (Location() != TilePositions::None) bw->drawBoxMap(Position(Location()), Position(Location() + Type().tileSize()), GetColor());

	if (!Agent()->Unit()->isInterruptible()) return;

	if (Agent()->Life() < Agent()->PrevLife(30))
		if (Agent()->Life() < 20)
			return Agent()->ChangeBehavior<Fleeing>(Agent());

	switch (State())
	{
	case reachingLocation:		OnFrame_reachingLocation(); break;
	case waitingForMoney:		OnFrame_waitingForMoney(); break;
	case constructing:			OnFrame_constructing(); break;
	default: assert_throw(false);
	}
}



void Constructing::OnFrame_reachingLocation()
{
	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
		m_location = findBuildingPlacement(Type(), TilePosition(GetBase()->BWEMPart()->Center()));
		if (m_location == TilePositions::None)
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());

		SetSubBehavior<Walking>(Agent(), Position(Location()) + Position(Type().tileSize()) / 2);
	}

	if (Agent()->Life() < Agent()->PrevLife(30))
		return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (Agent()->Pos().getApproxDistance(GetSubBehavior()->IsWalking()->Target()) < 2*32)
		if (Agent()->Life() >= 30)
		{
			ResetSubBehavior();
			return ChangeState(waitingForMoney);
		}

	if (ai()->Frame() - m_inStateSince > 200)
		return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}



void Constructing::OnFrame_waitingForMoney()
{
	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
	}

	if (Agent()->Unit()->isConstructing())
		return ChangeState(constructing);

	if (!(ai()->Me().CanPay(Type()) && Agent()->Build(Type(), Location(), no_check)))
	{
		if (ai()->Frame() - m_inStateSince > 200)
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());

		return;
	}
}



void Constructing::OnFrame_constructing()
{
	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
	}

	if (!Agent()->Unit()->isConstructing())
		return Agent()->ChangeBehavior<DefaultBehavior>(Agent());

}






} // namespace stone



