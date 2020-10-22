//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "my.h"
#include "../behavior/defaultBehavior.h"
#include "../behavior/mining.h"
#include "../behavior/harassing.h"
#include "../territory/stronghold.h"
#include "../strategy.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MyUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


unique_ptr<MyUnit> MyUnit::Make(BWAPI::Unit u)
{
	switch (u->getType())
	{
		#define MYBUILDING_MAKE_CASE(tid)   case tid: return make_unique<My<tid>>(u);
		MYBUILDING_MAKE_CASE(Terran_SCV)
	}

	assert_throw_plus(false, "could not make " + u->getType().getName());
	return nullptr;
}
	

MyUnit::MyUnit(BWAPI::Unit u, unique_ptr<IBehavior> pBehavior)
	: MyBWAPIUnit(u, move(pBehavior))
{
}


void MyUnit::Update()
{
	MyBWAPIUnit::Update();

	{
		vector<HisBWAPIUnit *> HisBWAPIUnits;
		for (auto & u : ai()->Him().Units())
			HisBWAPIUnits.push_back(u.get());

		for (auto & b : ai()->Him().Buildings())	
			if (!b->InFog())
				HisBWAPIUnits.push_back(b.get());

		const int minDistanceToRange = 10*32;
		m_FaceOffs.clear();
		for (HisBWAPIUnit * his : HisBWAPIUnits)
		{
			m_FaceOffs.emplace_back(this, his);
			if ((!m_FaceOffs.back().MyAttack() && !m_FaceOffs.back().HisAttack()) ||
				((m_FaceOffs.back().DistanceToMyRange() > minDistanceToRange) && (m_FaceOffs.back().DistanceToHisRange() > minDistanceToRange)))
				m_FaceOffs.pop_back();
		}
	}
/*
	{
		vector<MyBWAPIUnit *> MyBWAPIUnits;
		for (auto & u : ai()->Me().Units()) 	 MyBWAPIUnits.push_back(u);
		for (auto & b : ai()->Me().Buildings())	 MyBWAPIUnits.push_back(b);

		const int minDistanceToRange = 10*32;
		m_FaceOffs.clear();
		for (MyBWAPIUnit * my : MyBWAPIUnits) if (this != my)
		{
			m_FaceOffs.emplace_back(this, my);
			if ((!m_FaceOffs.back().MyAttack() && !m_FaceOffs.back().HisAttack()) ||
				((m_FaceOffs.back().DistanceToMyRange() > minDistanceToRange) && (m_FaceOffs.back().DistanceToHisRange() > minDistanceToRange)))
				m_FaceOffs.pop_back();
		}
	}
*/
}


void MyUnit::OnDangerHere()
{
	m_lastDangerReport = ai()->Frame();
}




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MyBuilding
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


unique_ptr<MyBuilding> MyBuilding::Make(BWAPI::Unit u)
{
	switch (u->getType())
	{
		#define MYBUILDING_MAKE_CASE(tid)   case tid: return make_unique<My<tid>>(u);
		MYBUILDING_MAKE_CASE(Terran_Command_Center)
		MYBUILDING_MAKE_CASE(Terran_Supply_Depot)
	}

	assert_throw_plus(false, "could not make " + u->getType().getName());
	return nullptr;
}


MyBuilding::MyBuilding(BWAPI::Unit u, unique_ptr<IBehavior> pBehavior)
	: MyBWAPIUnit(u, move(pBehavior)), m_size(u->getType().tileSize())
{
	if (!Flying()) PutBuildingOnTiles();
}


MyBuilding::~MyBuilding()
{
	try //3
	{
		if (!Flying()) RemoveBuildingFromTiles();
	}
	catch(...){}
}


void MyBuilding::Update()
{
	MyBWAPIUnit::Update();

	if (JustLifted()) RemoveBuildingFromTiles();
	if (JustLanded()) PutBuildingOnTiles();
}


void MyBuilding::Train(BWAPI::UnitType type, bool noCheck)
{
	bool ok = Unit()->train(type);
	if (!ok && !noCheck) reportCommandError(NameWithId() + " train " + type.getName());
}


void MyBuilding::Lift(bool noCheck)
{
	bool ok = Unit()->lift();
	if (!ok && !noCheck) reportCommandError(NameWithId() + " lift");
}


bool MyBuilding::Land(TilePosition location, bool noCheck)
{
	bool ok = Unit()->land(location);
	if (!ok && !noCheck) reportCommandError(NameWithId() + " land");
	return ok;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class My<Terran_Command_Center>
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

My<Terran_Command_Center>::My(BWAPI::Unit u)
	: MyBuilding(u, make_unique<DefaultBehavior>(this))
{
	assert_throw(u->getType() == Terran_Command_Center);
}



void My<Terran_Command_Center>::DefaultBehaviorOnFrame()
{
//	if (!Unit()->isInterruptible()) return;

	if (Life() < MaxLife()*2/3)
	{
		const bool underAttack = Life() < PrevLife(63);
		const Base * pMyStartingBase = ai()->Me().Bases().front()->BWEMPart();

		if (Flying())
		{
			if (underAttack)
			{
				if (!Unit()->isMoving())
					return Move(ai()->GetVMap().RandomSeaPosition());
			}
			else
				if (ai()->Frame() % 10 == 0)
					if (!Land(pMyStartingBase->Location(), no_check))
						for (MyUnit * u : ai()->Me().Units())
							if (u->Unit()->isInterruptible())
								if (u->Pos().getApproxDistance(pMyStartingBase->Center()) < 3*32)
									u->Move(ai()->GetVMap().RandomPosition(Position(pMyStartingBase->GetArea()->Top()), 5*32));

			return;
		}
		else
		{
			if (underAttack)
				if (TopLeft() == pMyStartingBase->Location())
					return Lift(no_check);
		}

	}

	if (ai()->Me().CanPay(Terran_SCV))
		return Train(Terran_SCV, no_check);

}



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class My<Terran_Supply_Depot>
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

My<Terran_Supply_Depot>::My(BWAPI::Unit u)
	: MyBuilding(u, make_unique<DefaultBehavior>(this))
{
	assert_throw(u->getType() == Terran_Supply_Depot);
}




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class My<Terran_SCV>
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

My<Terran_SCV>::My(BWAPI::Unit u)
	: MyUnit(u, make_unique<DefaultBehavior>(this))
{
	assert_throw(u->getType() == Terran_SCV);
}


void My<Terran_SCV>::DefaultBehaviorOnFrame()
{
	assert_throw(GetStronghold());

	if (ai()->Him().Accessible())
		if (!Mining::Instances().empty() && (GetArea() != ai()->Me().GetArea()))
			return ChangeBehavior<Harassing>(this);

	if (auto * base = GetStronghold()->HasBase())
		if (base->LackingMiners())
			return ChangeBehavior<Mining>(this);
}


void My<Terran_SCV>::Gather(Mineral * m, bool noCheck)
{
	bool ok = Unit()->gather(m->Unit());
	if (!ok && !noCheck) reportCommandError(NameWithId() + " gather " + "Mineral #" + to_string(m->Unit()->getID()));
}


void My<Terran_SCV>::ReturnCargo(bool noCheck)
{
	bool ok = Unit()->returnCargo();
	if (!ok && !noCheck) reportCommandError(NameWithId() + " returnCargo");
}


void My<Terran_SCV>::Repair(MyBWAPIUnit * target, bool noCheck)
{
///	bw << NameWithId() + " repair " + target->NameWithId() << endl;
	bool ok = Unit()->repair(target->Unit());
	if (!ok && !noCheck) reportCommandError(NameWithId() + " repair " + target->NameWithId());
}


bool My<Terran_SCV>::Build(BWAPI::UnitType type, TilePosition location, bool noCheck)
{
	bool ok = Unit()->build(type, location);
	if (!ok && !noCheck) reportCommandError(NameWithId() + " build " + type.getName());
	return ok;
}

	
} // namespace stone



