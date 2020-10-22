//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "me.h"
#include "../Stone.h"
#include "../behavior/mining.h"
#include "../behavior/defaultBehavior.h"
#include "../territory/stronghold.h"

namespace { auto & bw = Broodwar; }

namespace stone
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Me
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

	
Me::Me()
{

}

	
Me::~Me()
{
}


const BWAPI::Player Me::Player() const
{
	return bw->self();
}


BWAPI::Player Me::Player()
{
	return bw->self();
}

	
void Me::Initialize()
{
	for (auto & base : ai()->GetVMap().Bases())
		if (base.BWEMPart()->Location() == Player()->getStartLocation())
			AddBase(&base);

	assert_throw(Bases().size() == 1);

	AddBaseStronghold(Bases().back());
}

	
void Me::Update()
{
	for (auto * u : Units())
		exceptionHandler("Me::Update(" + u->NameWithId() + ")", 5000, [u]()
		{
			u->Update();
		});

	for (auto * b : Buildings())
		exceptionHandler("Me::Update(" + b->NameWithId() + ")", 5000, [b]()
		{
			b->Update();
		});

	m_mineralsAvailable = Player()->minerals();
	m_gasAvailable = Player()->gas();
	m_supplyUsed = Player()->supplyUsed() / 2;
	m_supplyMax = Player()->supplyTotal() / 2;
}

	
void Me::RunBehaviors()
{
	for (auto * u : Units())
		exceptionHandler("Me::RunBehaviors(" + u->NameWithId() + ")", 5000, [u]()
		{
			u->GetBehavior()->OnFrame();
		});

	for (auto * b : Buildings())
		exceptionHandler("Me::RunBehaviors(" + b->NameWithId() + ")", 5000, [b]()
		{
			b->GetBehavior()->OnFrame();
		});
}


void Me::AddUnit(BWAPI::Unit u)
{
	assert_throw(u->getPlayer() == Player());
	assert_throw(!u->getType().isBuilding());

	vector<unique_ptr<MyUnit>> & List = Units(u->getType());
	assert_throw(none_of(List.begin(), List.end(), [u](const unique_ptr<MyUnit> & up){ return up->Unit() == u; }));

	List.push_back(MyUnit::Make(u));
	List.back()->InitializeStronghold();
	push_back_assert_does_not_contain(m_AllUnits, List.back().get());
}


void Me::OnBWAPIUnitDestroyed_InformOthers(BWAPIUnit * pBWAPIUnit)
{
	for (auto * u : Units())
		if (u != pBWAPIUnit)
			u->OnOtherBWAPIUnitDestroyed(pBWAPIUnit);

	for (auto * b : Buildings())
		if (b != pBWAPIUnit)
			b->OnOtherBWAPIUnitDestroyed(pBWAPIUnit);
}


void Me::RemoveUnit(BWAPI::Unit u)
{
	vector<unique_ptr<MyUnit>> & List = Units(u->getType());
	auto iMyUnit = find_if(List.begin(), List.end(), [u](const unique_ptr<MyUnit> & up){ return up->Unit() == u; });
	assert_throw(iMyUnit != List.end());

	OnBWAPIUnitDestroyed_InformOthers(iMyUnit->get());

	iMyUnit->get()->GetBehavior()->OnAgentBeingDestroyed();
	iMyUnit->get()->ChangeBehavior<DefaultBehavior>(iMyUnit->get());
	iMyUnit->get()->GetBehavior()->OnAgentBeingDestroyed();
	iMyUnit->get()->LeaveStronghold();

	{
		MyUnit * pMyUnit = iMyUnit->get();
		auto i = find(m_AllUnits.begin(), m_AllUnits.end(), pMyUnit);
		assert_throw(i != m_AllUnits.end());
		fast_erase(m_AllUnits, distance(m_AllUnits.begin(), i));
	}

	fast_erase(List, distance(List.begin(), iMyUnit));
}


void Me::AddBuilding(BWAPI::Unit u)
{
	assert_throw(u->getPlayer() == Player());
	assert_throw(u->getType().isBuilding());

	vector<unique_ptr<MyBuilding>> & List = Buildings(u->getType());
	assert_throw(none_of(List.begin(), List.end(), [u](const unique_ptr<MyBuilding> & up){ return up->Unit() == u; }));

	List.push_back(MyBuilding::Make(u));
	List.back()->InitializeStronghold();
	push_back_assert_does_not_contain(m_AllBuildings, List.back().get());
}


void Me::RemoveBuilding(BWAPI::Unit u)
{
	vector<unique_ptr<MyBuilding>> & List = Buildings(u->getType());
	auto iMyBuilding = find_if(List.begin(), List.end(), [u](const unique_ptr<MyBuilding> & up){ return up->Unit() == u; });
	assert_throw(iMyBuilding != List.end());

	OnBWAPIUnitDestroyed_InformOthers(iMyBuilding->get());

	iMyBuilding->get()->GetBehavior()->OnAgentBeingDestroyed();
	iMyBuilding->get()->ChangeBehavior<DefaultBehavior>(iMyBuilding->get());
	iMyBuilding->get()->GetBehavior()->OnAgentBeingDestroyed();
	iMyBuilding->get()->LeaveStronghold();

	{
		MyBuilding * pMyBuilding = iMyBuilding->get();
		auto i = find(m_AllBuildings.begin(), m_AllBuildings.end(), pMyBuilding);
		assert_throw(i != m_AllBuildings.end());
		fast_erase(m_AllBuildings, distance(m_AllBuildings.begin(), i));
	}

	fast_erase(List, distance(List.begin(), iMyBuilding));
}


void Me::AddBaseStronghold(VBase * b)
{
	assert_throw(!b->GetStronghold());

	m_Strongholds.push_back(make_unique<BaseStronghold>(b));
}


void Me::AddBase(VBase * b)
{
	push_back_assert_does_not_contain(m_Bases, b);
}


void Me::RemoveBase(VBase * b)
{
	auto iBase = find(m_Bases.begin(), m_Bases.end(), b);
	assert_throw(iBase != m_Bases.end());

	fast_erase(m_Bases, distance(m_Bases.begin(), iBase));
}


bool Me::CanPay(BWAPI::UnitType type) const
{
	if (BWAPI::UnitType(type).mineralPrice() > MineralsAvailable()) return false;
	if (BWAPI::UnitType(type).gasPrice() > GasAvailable()) return false;
	if (BWAPI::UnitType(type).supplyRequired()/2 > SupplyAvailable()) return false;
	return true;
}


	
} // namespace stone



