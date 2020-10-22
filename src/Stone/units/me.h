//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef ME_H
#define ME_H

#include <BWAPI.h>
#include "my.h"
#include "../territory/vbase.h"
#include <memory>


namespace stone
{

class Stronghold;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Me
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Me
{
public:
											Me();
											~Me();

	void									Initialize();

	void									Update();
	void									RunBehaviors();

	const BWAPI::Player						Player() const;
	BWAPI::Player							Player();

	BWAPI::Race								Race() const							{ return Player()->getRace(); }


	const vector<unique_ptr<MyUnit>> &		Units(BWAPI::UnitType type) const		{ assert_throw(type <= max_utid_used); return m_Units[type]; }
	vector<unique_ptr<MyUnit>> &			Units(BWAPI::UnitType type)				{ assert_throw(type <= max_utid_used); return m_Units[type]; }
	const vector<MyUnit *> &				Units() const							{ return m_AllUnits; }
	void									AddUnit(BWAPI::Unit u);
	void									RemoveUnit(BWAPI::Unit u);

	const vector<unique_ptr<MyBuilding>> &	Buildings(BWAPI::UnitType type) const	{ assert_throw(type <= max_utid_used); return m_Buildings[type]; }
	vector<unique_ptr<MyBuilding>> &		Buildings(BWAPI::UnitType type)			{ assert_throw(type <= max_utid_used); return m_Buildings[type]; }
	const vector<MyBuilding *> &			Buildings() const						{ return m_AllBuildings; }
	void									AddBuilding(BWAPI::Unit u);
	void									RemoveBuilding(BWAPI::Unit u);

	const vector<VBase *> &					Bases() const							{ return m_Bases; }
	vector<VBase *> &						Bases()									{ return m_Bases; }
	void									AddBase(VBase * pBase);
	void									RemoveBase(VBase * pBase);

	const vector<unique_ptr<Stronghold>> &	StrongHolds() const						{ return m_Strongholds; }
	vector<unique_ptr<Stronghold>> &		StrongHolds()							{ return m_Strongholds; }
	void									AddBaseStronghold(VBase * b);

	int										MineralsAvailable() const				{ return m_mineralsAvailable; }
	int										GasAvailable() const					{ return m_gasAvailable; }
	int										SupplyUsed() const						{ return m_supplyUsed; }
	int										SupplyMax() const						{ return m_supplyMax; }
	int										SupplyAvailable() const					{ return m_supplyMax - m_supplyUsed; }

	bool									CanPay(BWAPI::UnitType type) const;

	void									OnBWAPIUnitDestroyed_InformOthers(BWAPIUnit * pBWAPIUnit);

	const Area *							GetArea() const							{ return Bases().front()->BWEMPart()->GetArea(); }

private:
	enum { max_utid_used = Terran_Bunker };

	vector<VBase *>							m_Bases;
	vector<unique_ptr<Stronghold>>			m_Strongholds;

	vector<unique_ptr<MyUnit>>				m_Units[max_utid_used + 1];
	vector<MyUnit *>						m_AllUnits;

	vector<unique_ptr<MyBuilding>>			m_Buildings[max_utid_used + 1];
	vector<MyBuilding *>					m_AllBuildings;

	int										m_mineralsAvailable;
	int										m_gasAvailable;
	int										m_supplyUsed;
	int										m_supplyMax;
};

	enum { o = sizeof(Me) };


} // namespace stone


#endif

