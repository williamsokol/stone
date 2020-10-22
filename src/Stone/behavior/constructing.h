//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef CONSTRUCTING_H
#define CONSTRUCTING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../units/my.h"
#include "../utils.h"


namespace stone
{

class VBase;


bool beingConstructed(BWAPI::UnitType type);


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Constructing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//
//	Mineral::Data() is used to store the number of miners assigned to this Mineral.
//

class Constructing : public Behavior<My<Terran_SCV>>
{
public:
	static const vector<Constructing *> &	Instances()					{ return m_Instances; }

	enum state_t {reachingLocation, waitingForMoney, constructing};

								Constructing(My<Terran_SCV> * pSCV, BWAPI::UnitType type);
								~Constructing();

	const Constructing *		IsConstructing() const override		{ return this; }
	Constructing *				IsConstructing() override			{ return this; }

	string						Name() const override				{ return "constructing"; }
	string						StateName() const override;

	BWAPI::Color				GetColor() const override			{ return Colors::Yellow; }
	Text::Enum					GetTextColor() const override		{ return Text::Yellow; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * ) const override { return false; }

	state_t						State() const						{ return m_state; }

	VBase *						GetBase() const						{ return m_pBase; }

	BWAPI::UnitType				Type() const						{ return m_type; }
	TilePosition				Location() const					{ return m_location;; }

	void						OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other) override;

private:
	void						ChangeState(state_t st)				{ assert_throw(m_state != st); m_state = st; OnStateChanged(); }
	void						OnFrame_reachingLocation();
	void						OnFrame_waitingForMoney();
	void						OnFrame_constructing();


	BWAPI::UnitType				m_type;
	VBase *						m_pBase;
	state_t						m_state = reachingLocation;
	TilePosition				m_location = TilePositions::None;
	frame_t						m_inStateSince;

	static vector<Constructing *>	m_Instances;
};







} // namespace stone


#endif

