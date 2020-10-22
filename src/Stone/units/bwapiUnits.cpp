//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "bwapiUnits.h"
#include "../behavior/behavior.h"
#include "../behavior/chasing.h"
#include "../territory/stronghold.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class BWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

BWAPIUnit::BWAPIUnit(BWAPI::Unit u)
	: m_bwapiUnit(u), m_bwapiType(u->getType()),
	m_maxLife(u->getType().maxHitPoints()),
	m_maxShields(u->getType().maxShields()),
	m_regenerationTime(u->getType().getRace() == Races::Zerg ? 64 : u->getType().getRace() == Races::Protoss ? 37 : 0)
{
	m_flying = Unit()->isFlying();
	m_topLeft = Unit()->getTilePosition();
	fill(m_PrevPos, m_PrevPos + sizePrevPos, Unit()->getPosition());
	fill(m_PrevLife, m_PrevLife + sizePrevLife, Unit()->getHitPoints());
}


BWAPIUnit::~BWAPIUnit()
{

}


void BWAPIUnit::Update()
{
	m_justLifted = m_justLanded = false;
	if (m_flying != Unit()->isFlying())
	{
		if (m_flying)	m_justLanded = true;
		else			m_justLifted = true;

		m_flying = Unit()->isFlying();
	}

	m_topLeft = Unit()->getTilePosition();

	m_acceleration = Vect(Unit()->getVelocityX(), Unit()->getVelocityY());

	{
		for (int i = sizePrevPos-1 ; i > 0 ; --i)
			m_PrevPos[i] = m_PrevPos[i-1];
	
		m_PrevPos[0] = Unit()->getPosition();
	}

	{
		for (int i = sizePrevLife-1 ; i > 0 ; --i)
			m_PrevLife[i] = m_PrevLife[i-1];
	
		m_PrevLife[0] = Unit()->getHitPoints();
	}

	m_shields = Unit()->getShields();
	m_coolDown = Unit()->getGroundWeaponCooldown();
	m_avgCoolDown = Unit()->getPlayer()->weaponDamageCooldown(Type());
	m_sight = Unit()->getPlayer()->sightRange(Type());
	m_groundAttack = Unit()->getPlayer()->damage(Type().groundWeapon());
	m_airAttack = Unit()->getPlayer()->damage(Type().airWeapon());
	m_groundRange = Unit()->getPlayer()->weaponMaxRange(Type().groundWeapon());
	m_airRange = Unit()->getPlayer()->weaponMaxRange(Type().airWeapon());
	m_armor = Unit()->getPlayer()->armor(Type());
	m_maxEnergy = Unit()->getPlayer()->maxEnergy(Type());
	m_energy = Unit()->getEnergy();

	if (Unit()->isMoving()) m_lastFrameMoving = ai()->Frame();

	m_pArea = /*Flying() ? nullptr :*/ ai()->GetMap().GetNearestArea(WalkPosition(Pos()));
}



void BWAPIUnit::PutBuildingOnTiles()
{
	assert_throw(Type().isBuilding());
	if (Type().isRefinery()) return;

	for (int dy = 0 ; dy < Type().tileSize().y ; ++dy)
	for (int dx = 0 ; dx < Type().tileSize().x ; ++dx)
	{
		auto & tile = ai()->GetMap().GetTile(TopLeft() + TilePosition(dx, dy));
		assert_throw(!tile.GetNeutral());
		assert_throw(!ai()->GetVMap().GetBuildingOn(tile));
		ai()->GetVMap().SetBuildingOn(tile, this);
		ai()->GetVMap().SetNearBuilding(tile);
	}

	vector<TilePosition> Border = outerBorder(TopLeft(), Type().tileSize(), bool("noCorner"));
	for (auto t : Border)
		if (ai()->GetMap().Valid(t))
			ai()->GetVMap().SetNearBuilding(ai()->GetMap().GetTile(t, no_check));
}


void BWAPIUnit::RemoveBuildingFromTiles()
{
	assert_throw(Type().isBuilding());
	if (Type().isRefinery()) return;

	for (int dy = 0 ; dy < Type().tileSize().y ; ++dy)
	for (int dx = 0 ; dx < Type().tileSize().x ; ++dx)
	{
		auto & tile = ai()->GetMap().GetTile(TopLeft() + TilePosition(dx, dy));
		assert_throw(ai()->GetVMap().GetBuildingOn(tile) == this);
		ai()->GetVMap().SetBuildingOn(tile, nullptr);
		ai()->GetVMap().UnsetNearBuilding(tile);
	}

	vector<TilePosition> Border = outerBorder(TopLeft(), Type().tileSize(), bool("noCorner"));
	for (auto t : Border)
		if (ai()->GetMap().Valid(t))
			ai()->GetVMap().UnsetNearBuilding(ai()->GetMap().GetTile(t, no_check));
}


void BWAPIUnit::AddChaser(Chasing * pChaser)
{
	push_back_assert_does_not_contain(m_Chasers, pChaser);
}


void BWAPIUnit::RemoveChaser(Chasing * pChaser)
{
	assert_throw(contains(m_Chasers, pChaser));
	really_remove(m_Chasers, pChaser);
}


frame_t BWAPIUnit::FramesToBeKilledByChasers() const
{
	double rate = 0;
	for (Chasing * pChaser : m_Chasers)
		rate += 1.0 / pChaser->GetFaceOff().FramesToKillHim();

	return frame_t(0.5 + 1.0/rate);
}



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MyBWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


MyBWAPIUnit::MyBWAPIUnit(BWAPI::Unit u, unique_ptr<IBehavior> pBehavior)
  : BWAPIUnit(u),
	m_pBehavior(move(pBehavior))
{
}


MyBWAPIUnit::~MyBWAPIUnit()
{
	try //3
	{
		if (GetBehavior()) GetBehavior()->OnAgentBeingDestroyed();
	}
	catch(...){}
}


void MyBWAPIUnit::InitializeStronghold()
{
	if (ai()->Frame() == 0)
		SetStronghold(ai()->Me().Bases().front()->GetStronghold());
	else
	{
		// TODO
		SetStronghold(ai()->Me().Bases().front()->GetStronghold());
	}
}


void MyBWAPIUnit::LeaveStronghold()
{
	SetStronghold(nullptr);
}


void MyBWAPIUnit::Update()
{
	BWAPIUnit::Update();

	if (ai()->Him().StartingBase())
	{
		ai()->GetMap().GetPath(Pos(), ai()->Him().StartingBase()->BWEMPart()->Center(), &m_distToHisStartingBase);
		if (m_distToHisStartingBase < 0) m_distToHisStartingBase = 1000000;
	}
}


void MyBWAPIUnit::OnOtherBWAPIUnitDestroyed(BWAPIUnit * other)
{
	GetBehavior()->OnOtherBWAPIUnitDestroyed(other);
	OnOtherBWAPIUnitDestroyed_v(other);
}


void MyBWAPIUnit::SetStronghold(Stronghold * sh)
{
	assert_throw(!sh != !m_pStronghold);
	
	if (sh)
	{
		m_pStronghold = sh;
		if (MyUnit * u = IsMyUnit()) m_pStronghold->OnUnitIn(u);
		else                         m_pStronghold->OnBuildingIn(IsMyBuilding());
	}
	else
	{
		if (MyUnit * u = IsMyUnit()) m_pStronghold->OnUnitOut(u);
		else                         m_pStronghold->OnBuildingOut(IsMyBuilding());
		m_pStronghold = sh;
	}
}


void MyBWAPIUnit::Move(Position pos, bool noCheck)
{
///	if (GetBehavior()->IsChasing()) bw << NameWithId() << " move!" << endl;
	bool ok = Unit()->move(pos);
	if (!ok && !noCheck) reportCommandError(NameWithId() + " move to " + my_to_string(pos));
}


void MyBWAPIUnit::Attack(BWAPIUnit * u, bool noCheck)
{
//	if (GetBehavior()->IsChasing()) bw << NameWithId() << " attack! " << GetBehavior()->IsChasing() << endl;
	bool ok = Unit()->attack(u->Unit());
	if (!ok && !noCheck) reportCommandError(NameWithId() + " attack " + u->NameWithId());
}


void MyBWAPIUnit::Stop(bool noCheck)
{
	bool ok = Unit()->stop();
	if (!ok && !noCheck) reportCommandError(NameWithId() + " stop ");
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class HisBWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


void HisBWAPIUnit::Update()
{
	BWAPIUnit::Update();

}



	

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class NeutralBWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////



	
} // namespace stone



