//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "chasing.h"
#include "walking.h"
#include "fleeing.h"
#include "defaultBehavior.h"
#include "../strategy.h"
#include "../vect.h"
#include "../Stone.h"
#include <queue>

namespace { auto & bw = Broodwar; }


namespace stone
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Chasing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Chasing *> Chasing::m_Instances;

Chasing::Chasing(MyUnit * pAgent, BWAPIUnit * pTarget, bool insist)
	: Behavior(pAgent), m_pTarget(pTarget), m_FaceOff(pAgent, pTarget->IsHisBWAPIUnit()), m_insist(insist)
{
	push_back_assert_does_not_contain(m_Instances, this);
	assert_throw(pTarget);

	m_pTarget->AddChaser(this);
}


Chasing::~Chasing()
{
	try //3
	{
		m_pTarget->RemoveChaser(this);

		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
	catch(...){}
}


void Chasing::ChangeState(state_t st)
{
	assert_throw(m_state != st);
	
	m_state = st; OnStateChanged();
}


string Chasing::StateName() const
{
	switch(State())
	{
	case chasing:			return InGroup() ? "chasing +" : m_insist ? "chasing !" : "chasing";
	case bypassing:			return "bypassing";
	default:				return "?";
	}
}


static void adjustDestWithAltitude(Position & dest)
{
	WalkPosition bestW = WalkPosition(dest);
	altitude_t maxAltitude = -1;
	for (int dy = -1 ; dy <= +1 ; ++dy)
	for (int dx = -1 ; dx <= +1 ; ++dx)
	{
		WalkPosition w (WalkPosition(dest) + WalkPosition(dx, dy));
		if (ai()->GetMap().Valid(w))
		{
			altitude_t altitude = ai()->GetMap().GetMiniTile(w, no_check).Altitude();
			if (altitude > maxAltitude)
			{
				maxAltitude = altitude;
				bestW = w;
			}
		}
	}

	dest = center(bestW);
}


static void adjustDestWithBlockedTiles(Position origin, Position & dest)
{
	const int margin = 5;
	const TilePosition originTile(origin);

	const bool vertical = abs(dest.y - origin.y) > abs(dest.x - origin.x);

	bool adjusted = false;
	if ((dest.y - origin.y < margin) && !canWalkOnTile(originTile + TilePosition(0, -1)))	{ dest.y = origin.y + margin; adjusted = true; }
	if ((dest.y - origin.y > margin) && !canWalkOnTile(originTile + TilePosition(0, 1)))		{ dest.y = origin.y - margin; adjusted = true; }
	if ((dest.x - origin.x < margin) && !canWalkOnTile(originTile + TilePosition(-1, 0)))	{ dest.x = origin.x + margin; adjusted = true; }
	if ((dest.x - origin.x > margin) && !canWalkOnTile(originTile + TilePosition(1, 0)))		{ dest.x = origin.x - margin; adjusted = true; }

	if (!adjusted)
	{
		if ((dest.x - origin.x < margin) && (dest.y - origin.y < margin) && !canWalkOnTile(originTile + TilePosition(-1, -1)))	
			if (vertical) dest.x = origin.x + margin;
			else          dest.y = origin.y + margin;

		if ((dest.x - origin.x > margin) && (dest.y - origin.y < margin) && !canWalkOnTile(originTile + TilePosition(1, -1)))	
			if (vertical) dest.x = origin.x - margin;
			else          dest.y = origin.y + margin;

		if ((dest.x - origin.x < margin) && (dest.y - origin.y > margin) && !canWalkOnTile(originTile + TilePosition(-1, 1)))	
			if (vertical) dest.x = origin.x + margin;
			else          dest.y = origin.y - margin;

		if ((dest.x - origin.x > margin) && (dest.y - origin.y > margin) && !canWalkOnTile(originTile + TilePosition(1, 1)))	
			if (vertical) dest.x = origin.x - margin;
			else          dest.y = origin.y - margin;
	}
}


void Chasing::SetGroup(const Area * pArea, const vector<FaceOff::His_t *> & Targets, frame_t advantage)
{
	assert_throw(contains(Targets, Target()));
	m_pGroupArea = pArea;
	m_GroupTargets = Targets;
	m_groupAdvantage = advantage;
	m_previouslyInGroup = true;
}



void Chasing::OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other)
{
	if (InGroup())
	{
		assert_throw(contains(GroupTargets(), Target()));
		if (contains(GroupTargets(), other))
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
	}
	else
	{
		if (Target() == other)
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
	}
}


void Chasing::OnFrame_v()
{
//1	drawLineMap(Agent()->Pos(), Target()->Pos(), GetColor());
//1	bw->drawBoxMap(Target()->Pos() - 25, Target()->Pos() + 25, GetColor());

	if (!Agent()->Unit()->isInterruptible()) return;

	m_FaceOff = FaceOff(Agent()->IsMyUnit(), Target()->IsHisBWAPIUnit());


	const frame_t noAlertTimeIfGoFleeing = InGroup() ? 50 : 0;

	if (Agent()->Life() <= 10)
		return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);

	if (InGroup())
	{
		for (FaceOff::His_t * target : GroupTargets())
			if (FaceOff(Agent()->IsMyUnit(), target).HisAttack() * (1+(int)GroupTargets().size()) >= Agent()->Life())
				return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);
	}
	else
	{
		if (GetFaceOff().HisAttack() * 2 >= Agent()->Life())
			return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);
	}

	if (!InGroup() && !m_insist)
		if (GetFaceOff().FramesToKillMe() < Target()->FramesToBeKilledByChasers() + 10*(int)Target()->Chasers().size())
			return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);


	if (InGroup())
	{
		if (ai()->Frame() - Agent()->IsMyUnit()->LastDangerReport() < 5)
			return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);
	}
	else if (m_insist)
	{
		// Cf AttackedAndSurrounded (TODO: refactor)

		if (Agent()->Life() + 8 < Agent()->PrevLife(24))
			return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);

		if (Agent()->Life() < Agent()->PrevLife(20))
			if (findThreats(Agent()->IsMyUnit(), 2*32).size() >= 3)
				return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);
	}
	else
	{
		if (findThreats(Agent()->IsMyUnit(), 5*32).size() > 1)
			return Agent()->ChangeBehavior<Fleeing>(Agent()->IsMyUnit(), noAlertTimeIfGoFleeing);
	}

	switch (State())
	{
	case chasing:		OnFrame_chasing(); break;
	case bypassing:		OnFrame_bypassing(); break;
	default: assert_throw(false);
	}
}


bool Chasing::CanRepair(const MyBWAPIUnit * ) const
{
	if (InGroup()) return false;

	if (GetFaceOff().FramesToKillHim() < 80) return false;

	return true;
}


void Chasing::OnFrame_chasing()
{
	if (JustArrivedInState())
	{
		m_lastFrameTouchedHim = ai()->Frame();
	}

	if (Agent()->Pos() == Agent()->PrevPos(5) && Agent()->Pos() == Agent()->PrevPos(10))
	if (ai()->Frame() - m_lastFrameTouchedHim > 20)
	{
		if (ai()->Frame() - m_lastFrameMovedToRandom > 10)
		{
		///	ai()->SetDelay(500); 	bw << "blocked: move to random position" << endl;
			m_lastFrameMovedToRandom = ai()->Frame();
			return Agent()->Move(ai()->GetVMap().RandomPosition(Agent()->Pos(), 5*32));
		}
		else return;
	}

	Vect Aacc(Agent()->Acceleration());
	Vect Tacc(Target()->Acceleration());
	Vect AT = toVect(Target()->Pos() - Agent()->Pos());

	double normAacc = Aacc.Norm();
	double normTacc = Tacc.Norm();
	double normAT = AT.Norm();

	if (!Agent()->CoolDown())
	{
		if ( (normAT < 32) ||
			((normAT < 80) && ((normTacc < 1.5) ||
							  ((normAacc > 0.1) && ((Aacc * Tacc) / (normAacc * normTacc) < -0.7)))))
		{
		///	bw << ai()->Frame() << " : attack!" << endl;
		///	bw << normAT << endl;
//1			bw->drawCircleMap(Target()->Pos(), 15, Colors::Yellow);
			if (ai()->Frame() - m_lastFrameAttack > 10)
			{
				m_lastFrameAttack = ai()->Frame();
				return Agent()->Attack(Target());
			}
			return;
		}
	}
	else
	{
		m_lastFrameTouchedHim = ai()->Frame();
		if (normTacc < 0.1) return;
	}

	Vect uAT = AT; uAT.Normalize();
	Vect uDir = uAT;
	if (normTacc > 0.1)
	{
		Vect uTacc = Tacc; uTacc.Normalize();

//1		drawLineMap(Target()->Pos(), Target()->Pos() + toPosition(uTacc*75), Colors::Red, crop);

		double angle_TA_Tacc = angle(-uAT, uTacc);
	
		if (abs(angle_TA_Tacc) < pi/4)
		{
			uDir = rot(uAT, -angle_TA_Tacc);
		}
		else
		{
			uDir = uAT*normAacc + uTacc*normTacc;
			uDir.Normalize();
		}
	}
	
	Position delta = toPosition(uDir * 75);
	Position dest = ai()->GetMap().Crop(Agent()->Pos() + delta);

//1	drawLineMap(Agent()->Pos(), dest, Colors::Grey, crop);

	if (normAT > 3*32)
		for (int i = 0 ; (i < 3) && (dist(Agent()->Pos(), dest) > 64) ; ++i)
			adjustDestWithAltitude(dest);

	assert_throw(ai()->GetMap().Valid(TilePosition(dest)));
	adjustDestWithBlockedTiles(Agent()->Pos(), dest);
	assert_throw(ai()->GetMap().Valid(TilePosition(dest)));

	if (queenWiseDist(Agent()->Pos(), dest) < 32)
	{
		uDir = toVect(dest) - toVect(Agent()->Pos());
		uDir.Normalize();
		m_ByPassingDir = toPosition(uDir * 75);
		return ChangeState(bypassing);
	}

//1	drawLineMap(Agent()->Pos(), dest, Colors::Green);
	Agent()->Move(dest);
}


void Chasing::OnFrame_bypassing()
{
	if (JustArrivedInState())
		m_bypassingStartFrame = ai()->Frame();

	if (ai()->Frame() - m_bypassingStartFrame > 20)
		return ChangeState(chasing);

	Position dest = ai()->GetMap().Crop(Agent()->Pos() + m_ByPassingDir);

//1	drawLineMap(Agent()->Pos(), dest, Colors::Green);
	return Agent()->Move(dest);

}


} // namespace stone



