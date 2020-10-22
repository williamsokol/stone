//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "him.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }

namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Him
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


const BWAPI::Player Him::Player() const
{
	return bw->enemy();
}


BWAPI::Player Him::Player()
{
	return bw->enemy();
}


void Him::Update()
{
	for (auto & u : Units())
		exceptionHandler("Him::Update(" + u->NameWithId() + ")", 5000, [&u]()
		{
			u->Update();
		});

	for (auto & b : Buildings())
		exceptionHandler("Him::Update(" + b->NameWithId() + ")", 5000, [&b]()
		{
			b->Update();
		});

	{	// Remove any InFog-HisBuilding having at least one tile visible since 10 frames. See also Him::OnShowBuilding
		vector<BWAPI::Unit> ObsoleteInFogBuildings;
		for (auto & b : Buildings())
			if (b->InFog())
				if (ai()->Frame() - b->LastFrameNoVisibleTile() > 10)
					ObsoleteInFogBuildings.push_back(b->Unit());

		for (BWAPI::Unit b : ObsoleteInFogBuildings)
			RemoveBuilding(b);
	}

	if (!m_pStartingBase)
	{
		auto it = find_if(Buildings().begin(), Buildings().end(), [](const unique_ptr<HisBuilding> & up){ return up->Type().isResourceDepot(); });
		if (it != Buildings().end())
			for (auto & base : ai()->GetVMap().Bases())
				if (base.BWEMPart()->Location() == (*it)->TopLeft())
					m_pStartingBase = &base;
	}
}


void Him::AddUnit(BWAPI::Unit u)
{
	assert_throw_plus(u->getPlayer() == Player(), u->getType().getName());
	assert_throw(!u->getType().isBuilding());

	assert_throw_plus(none_of(m_Units.begin(), m_Units.end(), [u](const unique_ptr<HisUnit> & up){ return up->Unit() == u; }), to_string(u->getID()));

	m_Units.push_back(make_unique<HisUnit>(u));
}


void Him::RemoveUnit(BWAPI::Unit u)
{
	auto iHisUnit = find_if(m_Units.begin(), m_Units.end(), [u](const unique_ptr<HisUnit> & up){ return up->Unit() == u; });
	if (iHisUnit != m_Units.end())
	{
		ai()->Me().OnBWAPIUnitDestroyed_InformOthers(iHisUnit->get());
		fast_erase(m_Units, distance(m_Units.begin(), iHisUnit));
	}
}


void Him::AddBuilding(BWAPI::Unit u)
{
	assert_throw_plus(u->getPlayer() == Player(), u->getType().getName());
	assert_throw(u->getType().isBuilding());

	assert_throw(none_of(m_Buildings.begin(), m_Buildings.end(), [u](const unique_ptr<HisBuilding> & up){ return up->Unit() == u; }));

	m_Buildings.push_back(make_unique<HisBuilding>(u));
}


void Him::RemoveBuilding(BWAPI::Unit u)
{
	auto iHisBuilding = find_if(m_Buildings.begin(), m_Buildings.end(), [u](const unique_ptr<HisBuilding> & up){ return up->Unit() == u; });
	if (iHisBuilding != m_Buildings.end())
	{
	//	bw << "Him::RemoveBuilding(" << iHisBuilding->get()->NameWithId() << ")" << endl;
		fast_erase(m_Buildings, distance(m_Buildings.begin(), iHisBuilding));
	}
}


void Him::OnShowBuilding(BWAPI::Unit u)
{
	{	// First, remove any corresponding InFog-HisBuilding. See also Him::Update
		vector<BWAPI::Unit> ObsoleteInFogBuildings;
		for (auto & b : m_Buildings)
			if (b->Unit() == u)
			{
				assert_throw(b->InFog());
				ObsoleteInFogBuildings.push_back(b->Unit());
			}

		for (BWAPI::Unit b : ObsoleteInFogBuildings)
			RemoveBuilding(b);
	}

	AddBuilding(u);
}


void Him::OnHideBuilding(BWAPI::Unit u)
{
	auto iHisBuilding = find_if(m_Buildings.begin(), m_Buildings.end(), [u](const unique_ptr<HisBuilding> & up){ return up->Unit() == u; });
	if (iHisBuilding != m_Buildings.end())
	{
		ai()->Me().OnBWAPIUnitDestroyed_InformOthers(iHisBuilding->get());
		if (iHisBuilding->get()->Flying())
			RemoveBuilding(u);
		else
			iHisBuilding->get()->SetInFog();
	}
}

	
} // namespace stone



