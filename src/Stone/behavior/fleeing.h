//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef FLEEING_H
#define FLEEING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../units/my.h"
#include "../utils.h"


namespace stone
{

class HisUnit;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Fleeing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Fleeing : public Behavior<MyUnit>
{
public:
	static const vector<Fleeing *> &	Instances()					{ return m_Instances; }

	enum state_t {dragOut, insideArea, anywhere/*, takeAway*/};

								Fleeing(MyUnit * pAgent, frame_t noAlertFrames = 0);
								~Fleeing();

	const Fleeing *				IsFleeing() const override			{ return this; }
	Fleeing *					IsFleeing() override				{ return this; }

	string						Name() const override				{ return "fleeing"; }
	string						StateName() const override;

	BWAPI::Color				GetColor() const override			{ return Colors::Green; }
	Text::Enum					GetTextColor() const override		{ return Text::Green; }

	state_t						State() const						{ return m_state; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * ) const override;

	double						AvgPursuers() const					{ return m_avgPursuers; }
	bool						CanReadAvgPursuers() const			{ return m_canReadAvgPursuers; }

private:
	void						ChangeState(state_t st);
	void						OnFrame_dragOut();
	void						OnFrame_anywhere();
//	void						OnFrame_takeAway();

	Vect						CalcultateThreatVector(const vector<const FaceOff *> & Threats) const;
	Vect						CalcultateDivergingVector() const;
	Vect						CalcultateIncreasingAltitudeVector() const;
	void						AdjustTarget(Position & target, const int walkRadius) const;
	bool						AttackedAndSurrounded() const;
	void						Alert();
	
	state_t						m_state;
	Position					m_lastTarget;
	Position					m_lastPos;
	frame_t						m_inStateSince;
	frame_t						m_remainingFramesInFreeState;
	int							m_cumulativePursuers = 0;
	int							m_numberOf_pursuersCount = 0;
	bool						m_canReadAvgPursuers = false;
	double						m_avgPursuers = 0.0;
	const Area *				m_pFarArea = nullptr;
	frame_t						m_noAlertUntil;
	
	static vector<Fleeing *>	m_Instances;
};


vector<const FaceOff *> findPursuers(const MyUnit * pAgent);
vector<const FaceOff *> findThreats(const MyUnit * pAgent, int maxDistToHisRange, int * pDistanceToHisFire = nullptr);

} // namespace stone


#endif

