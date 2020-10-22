//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef REPAIRING_H
#define REPAIRING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../units/my.h"
#include "../utils.h"


namespace stone
{

class VBase;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Repairing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Repairing : public Behavior<My<Terran_SCV>>
{
public:
	static const vector<Repairing *> &	Instances()					{ return m_Instances; }

	static Repairing *			GetRepairer(MyBWAPIUnit * pTarget);

	enum state_t {reachingTarget, repairing};

								Repairing(My<Terran_SCV> * pAgent, MyBWAPIUnit * pTarget);
								~Repairing();

	const Repairing *			IsRepairing() const override		{ return this; }
	Repairing *					IsRepairing() override				{ return this; }

	string						Name() const override				{ return "repairing"; }
	string						StateName() const override;

	BWAPI::Color				GetColor() const override			{ return Colors::Yellow; }
	Text::Enum					GetTextColor() const override		{ return Text::Yellow; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * ) const override			{ return false; }

	state_t						State() const						{ return m_state; }

	MyBWAPIUnit *				Target() const						{ return m_pTarget; }


private:
	void						OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other) override;
	void						ChangeState(state_t st);
	void						OnFrame_reachingTarget();
	void						OnFrame_repairing();

	static vector<Repairing *>	m_Instances;

	MyBWAPIUnit *				m_pTarget;
	frame_t						m_inStateSince;

	state_t						m_state = reachingTarget;
};




} // namespace stone


#endif

