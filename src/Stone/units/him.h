//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef HIM_H
#define HIM_H

#include <BWAPI.h>
#include "his.h"
#include <memory>


namespace stone
{

class VBase;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Him
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Him
{
public:

	void									Update();

	const BWAPI::Player						Player() const;
	BWAPI::Player							Player();

	VBase *									StartingBase() const		{ return m_pStartingBase; }

	BWAPI::Race								Race() const				{ return Player()->getRace(); }

	bool									Accessible() const			{ return m_accessible; }
	void									SetNotAccessible()			{ m_accessible = false; }

	void									AddUnit(BWAPI::Unit u);
	void									AddBuilding(BWAPI::Unit u);

	void									RemoveUnit(BWAPI::Unit u);
	void									RemoveBuilding(BWAPI::Unit u);

	void									OnShowBuilding(BWAPI::Unit u);
	void									OnHideBuilding(BWAPI::Unit u);

	const vector<unique_ptr<HisUnit>> &		Units() const				{ return m_Units; }
	vector<unique_ptr<HisUnit>> &			Units()						{ return m_Units; }

	const vector<unique_ptr<HisBuilding>> &	Buildings() const			{ return m_Buildings; }
	vector<unique_ptr<HisBuilding>> &		Buildings()					{ return m_Buildings; }

private:

	vector<unique_ptr<HisUnit>>				m_Units;
	vector<unique_ptr<HisBuilding>>			m_Buildings;
	VBase *									m_pStartingBase = nullptr;
	bool									m_accessible = true;
};


} // namespace stone


#endif

