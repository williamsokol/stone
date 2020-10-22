//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef FACE_OFF_H
#define FACE_OFF_H

#include "../defs.h"


namespace stone
{

class BWAPIUnit;
class MyUnit;
class HisBWAPIUnit;
class MyBWAPIUnit;


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class FaceOff
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class FaceOff
{
public:
//	typedef MyBWAPIUnit		His_t;
	typedef HisBWAPIUnit	His_t;

						FaceOff(MyUnit * mine, His_t * his);

	MyUnit *			Mine() const				{ return m_mine; }
	His_t *				His() const					{ return m_his; }

	// Euclidean distance
	int					Distance() const			{ return m_distance; }

	// Damage inflicted per attack
	int					MyAttack() const			{ return m_myAttack; }
	int					HisAttack() const			{ return m_hisAttack; }

	int					MyRange() const				{ return m_myRange; }
	int					HisRange() const			{ return m_hisRange; }

	// A positive value indicates an out-of-range distance in pixels
	// A negative value indicates an in-range distance in pixels
	int					DistanceToMyRange() const	{ return m_distanceToMyRange; }
	int					DistanceToHisRange() const	{ return m_distanceToHisRange; }

	frame_t				FramesToKillMe() const		{ return m_framesToKillMe; }
	frame_t				FramesToKillHim() const		{ return m_framesToKillHim; }

private:
	int					GetAttack(const BWAPIUnit * attacker, const BWAPIUnit * defender) const;
	int					GetRange(const BWAPIUnit * attacker, const BWAPIUnit * defender) const;
	frame_t				GetKillTime(const BWAPIUnit * attacker, const BWAPIUnit * defender, int damagePerAttack, int distanceToAttackerRange) const;

	MyUnit *			m_mine;
	His_t *		m_his;
	int					m_distance;
	int					m_myAttack;
	int					m_hisAttack;
	int					m_myRange;
	int					m_hisRange;
	int					m_distanceToMyRange;
	int					m_distanceToHisRange;
	frame_t				m_framesToKillMe;
	frame_t				m_framesToKillHim;
};


void quickSimulation(const vector<MyUnit *> & MyUnits, const vector<FaceOff::His_t *> & HisUnits, frame_t & framesToKillThem, frame_t & framesToKillUs);


} // namespace stone


#endif

