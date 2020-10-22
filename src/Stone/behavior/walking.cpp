//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "walking.h"
#include "defaultBehavior.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Walking
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


Walking::Walking(MyBWAPIUnit * pAgent, Position target)
	: Behavior(pAgent), m_target(target), m_Path(ai()->GetMap().GetPath(pAgent->Pos(), target, &m_length))
{
	assert_throw(m_length >= 0);

}


Walking::~Walking()
{
}


string Walking::StateName() const
{
	switch(State())
	{
	case intermediate:	return "intermediate";
	case final:			return "final";
	case failed:		return "failed";
	case succeeded:		return "succeeded";
	default:			return "?";
	}
}


Position Walking::CurrentWayPoint() const
{
	if (m_index == (int)m_Path.size()) return Target();
	
	return Position(m_Path[m_index]->Center());
}


void Walking::OnFrame_v()
{
//1	drawLineMap(Agent()->Pos(), Target(), GetColor());
//1	drawLineMap(Agent()->Pos(), CurrentWayPoint(), Colors::Grey);

	if (!Agent()->Unit()->isInterruptible()) return;

	switch (State())
	{
	case intermediate:		OnFrame_intermediate(); break;
	case final:				OnFrame_final(); break;
	case failed:			break;
	case succeeded:			break;
	default: assert_throw(false);
	}
}


void Walking::OnFrame_intermediate()
{
	Position wp = CurrentWayPoint();

	if (wp == Target()) return ChangeState(final);

	if (Agent()->Pos().getApproxDistance(wp) < 32*10)
	{
		++m_index;
		return Agent()->Move(CurrentWayPoint());
	}

	if (JustArrivedInState()) return Agent()->Move(wp);

	if (Agent()->Unit()->isIdle())
	{
		if (++m_tries >= 10)
			return ChangeState(failed);
		
	///	bw << Agent()->NameWithId() << ", move!" << endl;
		return Agent()->Move(wp);
	}

}


void Walking::OnFrame_final()
{
	if (Agent()->Pos().getApproxDistance(Target()) < m_targetRadius)
		return ChangeState(succeeded);

	if (JustArrivedInState())
	{
		m_tries = 0;
		return Agent()->Move(Target());
	}

	if (m_tries >= 3)
	{
//1		bw->drawCircleMap(Agent()->Pos(), m_targetRadius, Colors::Grey);
		m_targetRadius += 5;
	}

	if (Agent()->Unit()->isIdle())
	{
		++m_tries;
	///	bw << Agent()->NameWithId() << ", move!" << endl;
		return Agent()->Move(Target());
	}

}



} // namespace stone



