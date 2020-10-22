//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef CHASING_H
#define CHASING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../units/my.h"
#include "../utils.h"


namespace stone
{

class VBase;
class HisUnit;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Chasing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Chasing : public Behavior<MyBWAPIUnit>
{
public:
	static const vector<Chasing *> &	Instances()					{ return m_Instances; }

	enum state_t {chasing, bypassing};

								Chasing(MyUnit * pAgent, BWAPIUnit * pTarget, bool insist = false);
								~Chasing();

	const Chasing *				IsChasing() const override			{ return this; }
	Chasing *					IsChasing() override				{ return this; }

	string						Name() const override				{ return "chasing"; }
	string						StateName() const override;

	BWAPI::Color				GetColor() const override			{ return Colors::Cyan; }
	Text::Enum					GetTextColor() const override		{ return Text::Cyan; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * pTarget) const override;

	state_t						State() const						{ return m_state; }

	BWAPIUnit *					Target() const						{ return m_pTarget; }
	const FaceOff &				GetFaceOff() const					{ return m_FaceOff; }

	void						SetGroup(const Area * pArea, const vector<FaceOff::His_t *> & Targets, frame_t advantage);
	void						Ungroup()							{ m_pGroupArea = nullptr; m_GroupTargets.clear(); }
	bool						InGroup() const						{ return m_pGroupArea != nullptr; }
	bool						PreviouslyInGroup() const			{ return m_previouslyInGroup; }
	const Area *				GroupArea() const					{ return m_pGroupArea; }
	const vector<FaceOff::His_t *> &GroupTargets() const			{ return m_GroupTargets; }
	frame_t						GroupAdvantage() const				{ assert_throw(InGroup()); return m_groupAdvantage; }

private:
	void						OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other) override;
	void						ChangeState(state_t st);
	void						OnFrame_chasing();
	void						OnFrame_bypassing();

	state_t						m_state = chasing;
	BWAPIUnit *					m_pTarget;
	FaceOff						m_FaceOff;
	frame_t						m_bypassingStartFrame;
	frame_t						m_lastFrameTouchedHim;
	frame_t						m_lastFrameMovedToRandom = 0;
	frame_t						m_lastFrameAttack = 0;
	Position					m_ByPassingDir;

	const Area *				m_pGroupArea = nullptr;
	frame_t						m_groupAdvantage = 0;
	vector<FaceOff::His_t *>	m_GroupTargets;
	bool						m_previouslyInGroup = false;
	bool						m_insist;

	static vector<Chasing *>	m_Instances;
};



} // namespace stone


#endif

