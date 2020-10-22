//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef MINING_H
#define MINING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../units/my.h"
#include "../utils.h"


namespace stone
{

class VBase;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Mining
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//
//	Mineral::Data() is used to store the number of miners assigned to this Mineral.
//

class Mining : public Behavior<My<Terran_SCV>>
{
public:
	static const vector<Mining *> &	Instances()					{ return m_Instances; }

	enum state_t {gathering, returning};

								Mining(My<Terran_SCV> * pSCV);
								~Mining();

	const Mining *				IsMining() const override			{ return this; }
	Mining *					IsMining() override					{ return this; }

	string						Name() const override				{ return "mining"; }
	string						StateName() const override			{ return State() == gathering ? "gathering" : "returning"; }
	string						FullName() const override			{ return StateName(); }

	BWAPI::Color				GetColor() const override			{ return Colors::Cyan; }
	Text::Enum					GetTextColor() const override		{ return Text::Cyan; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * pTarget) const override;

	state_t						State() const						{ return m_state; }

	VBase *						GetBase() const						{ return m_pBase; }
	Mineral *					GetMineral() const					{ return m_pMineral; }

	void						OnMineralDestroyed(Mineral * m);

private:
	void						ChangeState(state_t st)				{ assert_throw(m_state != st); m_state = st; OnStateChanged(); }
	void						OnFrame_gathering();
	void						OnFrame_returning();

	Mineral *					FindFreeMineral() const;
	void						Assign(Mineral * m);
	void						Release();

	Mineral *					m_pMineral = nullptr;
	VBase *						m_pBase;
	state_t						m_state;

	static vector<Mining *>	m_Instances;
};



} // namespace stone


#endif

