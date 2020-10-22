//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "harassing.h"
#include "fleeing.h"
#include "walking.h"
#include "mining.h"
#include "chasing.h"
#include "repairing.h"
#include "defaultBehavior.h"
#include "../strategy.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

const TilePosition dimCC = UnitType(Terran_Command_Center).tileSize();

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Harassing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Harassing *> Harassing::m_Instances;

Harassing::Harassing(My<Terran_SCV> * pAgent)
	: Behavior(pAgent), m_pHisBase(ai()->Him().StartingBase())
{
	assert_throw(m_pHisBase);
	push_back_assert_does_not_contain(m_Instances, this);
}


Harassing::~Harassing()
{
	try //3
	{
		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
	catch(...){}
}


void Harassing::ChangeState(state_t st)
{
	assert_throw(m_state != st);
	
	m_state = st; OnStateChanged();
	m_pCurrentTarget = nullptr;
	m_pCurrentMineralTarget = nullptr;
}


string Harassing::StateName() const
{
	switch(State())
	{
	case reachingHisBase:	return "reachingHisBase";
	case attackNearest:		return "attackNearest";
	case mineAndAttack:		return "mineAndAttack";
	default:				return "?";
	}
}


void Harassing::OnMineralDestroyed(Mineral * m)
{
	if (m_pCurrentMineralTarget == m) m_pCurrentMineralTarget = nullptr;
}


void Harassing::OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other)
{
	if (m_pCurrentTarget == other) 
		return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}


bool Harassing::CanRepair(const MyBWAPIUnit * ) const
{
	if (State() == reachingHisBase) return true;
	if (!Agent()->CoolDown() && Agent()->Life() == Agent()->MaxLife()) return true;
	return false;
}


void Harassing::OnFrame_v()
{
//1	if (m_pCurrentTarget) drawLineMap(Agent()->Pos(), m_pCurrentTarget->Pos(), GetColor());
//1	if (m_pCurrentMineralTarget) drawLineMap(Agent()->Pos(), m_pCurrentMineralTarget->Pos(), GetColor());


	if (!Agent()->Unit()->isInterruptible()) return;

	switch (State())
	{
	case reachingHisBase:	OnFrame_reachingHisBase(); break;
	case attackNearest:		OnFrame_attackNearest(); break;
	case mineAndAttack:		OnFrame_mineAndAttack(); break;
	default: assert_throw(false);
	}
}


void Harassing::OnFrame_reachingHisBase()
{
	const bool inHisArea = (Agent()->GetArea() == HisBase()->BWEMPart()->GetArea());

	if (Agent()->Life() < Agent()->PrevLife(30))
		return Agent()->ChangeBehavior<Fleeing>(Agent());

	auto Threats5 = findThreats(Agent(), 5*32);

	for (auto * faceOff : Threats5)
		if (faceOff->His()->Unit()->isConstructing())
			if (Agent()->Life() > 15)
				return Agent()->ChangeBehavior<Chasing>(Agent(), faceOff->His(), bool("insist"));

	if ((ai()->Frame() - Agent()->LastDangerReport() < 5) || (Agent()->Life() < 30))
		if (!Threats5.empty())
			return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (!inHisArea)
		if (!findThreats(Agent(), 2*32).empty())
			return Agent()->ChangeBehavior<Fleeing>(Agent());

	if ((Me().MineralsAvailable() > 100) || !Mining::Instances().empty())
		if (Agent()->Life() < Agent()->MaxLife())
			if (Repairing * pRepairer = Repairing::GetRepairer(Agent()))
				return Agent()->ChangeBehavior<Repairing>(Agent(), pRepairer->Agent());

	if (Agent()->DistToHisStartingBase() < 32*4)
		if (Agent()->Life() >= 30)
		{
			ResetSubBehavior();
			return ChangeState(attackNearest);
		}

	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
		SetSubBehavior<Walking>(Agent(), ai()->Him().StartingBase()->BWEMPart()->Center());
	}

	if (ai()->Frame() - m_inStateSince > 5)
		if (ai()->Frame() - Agent()->LastFrameMoving() > 50)
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}


Mineral * Harassing::FindHisFarestVisibleMineral() const
{
	map<int, Mineral *> HisMineralsByDist;
	for (Mineral * m : HisBase()->BWEMPart()->Minerals())
		if (m->Unit()->isVisible())
			HisMineralsByDist[distToRectangle(Agent()->Pos(), m->TopLeft(), m->Size())] = m;

	return HisMineralsByDist.empty() ? nullptr : HisMineralsByDist.rbegin()->second;
}


void Harassing::OnFrame_attackNearest()
{
	if (JustArrivedInState())
	{
	///	ai()->SetDelay(100);
		m_lifeBeforeAttack = Agent()->Life();
		m_timesCouldChaseRatherThanAttackNearest = 0;
	}

	if (Agent()->Life() < Agent()->PrevLife(30)) return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (Agent()->CoolDown() >= Agent()->AvgCoolDown()-3)
	{
		m_lastAttack = ai()->Frame();
	//	return Agent()->ChangeBehavior<Fleeing>(Agent());
	}

	auto Threats5 = findThreats(Agent(), 5*32);

	if (ai()->Frame() - m_lastAttack > Agent()->AvgCoolDown() + 20)
		if (ai()->Frame() - Agent()->LastDangerReport() < 5)
			if (!Threats5.empty())
				return Agent()->ChangeBehavior<Fleeing>(Agent());


	if ((Me().MineralsAvailable() > 100) || !Mining::Instances().empty())
		if ((Agent()->Life() < Agent()->MaxLife()) && (Agent()->Life() == m_lifeBeforeAttack))
			if (Repairing * pRepairer = Repairing::GetRepairer(Agent()))
				return Agent()->ChangeBehavior<Repairing>(Agent(), pRepairer->Agent());


	map<int, HisUnit *> Candidates;
	for (const auto & faceOff : Agent()->FaceOffs())
		if (HisUnit * pHisUnit = faceOff.His()->IsHisUnit())
		{
			if (ai()->Frame() - pHisUnit->PursuingTargetLastFrame() < 30) continue;
			if (pHisUnit->GetArea() != HisBase()->BWEMPart()->GetArea()) continue;

			if (pHisUnit->Type().isWorker())
			{
				int & prevDist = m_prevDistances[pHisUnit];
				int dist = faceOff.Distance();
				int malus = dist;

				if (toVect(pHisUnit->Pos() - Agent()->Pos()) * Agent()->Acceleration() < 0)
					malus *= 2;

				if (pHisUnit->Unit()->isMoving())
				{
					if (prevDist && (dist >= prevDist) && (dist > 40)) malus += 100000;
				}
				else malus /= 2;

				Candidates[malus] = pHisUnit;
				prevDist = dist;
			}
		}

	m_pCurrentTarget = Candidates.empty() ? nullptr : Candidates.begin()->second;
	if (m_pCurrentTarget)
	{
		if (Threats5.size() == 1)
			if (Threats5.front()->His() == m_pCurrentTarget)
				if (Threats5.front()->FramesToKillMe() > Threats5.front()->FramesToKillHim() + 10)
					if (++m_timesCouldChaseRatherThanAttackNearest == 12)
						return Agent()->ChangeBehavior<Chasing>(Agent(), m_pCurrentTarget);

		return Agent()->Attack(m_pCurrentTarget);
	}
	else
	{
		if (ai()->Frame() % 50 == 0)
			return Agent()->Move(ai()->GetVMap().RandomPosition(	ai()->Him().StartingBase()->BWEMPart()->Center(), 6*32));
	}


}


void Harassing::OnFrame_mineAndAttack()
{
	if (JustArrivedInState())
	{
	///	ai()->SetDelay(300);
		m_lifeBeforeAttack = Agent()->Life();

		if (HisBase()->BWEMPart()->Minerals().size() < 3) return ChangeState(attackNearest);

		// First move towards his Minerals so that they become visible
		Agent()->Move(HisBase()->BWEMPart()->Minerals().front()->Pos());
	}

	if (m_pCurrentMineralTarget)
		if (!m_pCurrentMineralTarget->Unit()->isVisible())
			m_pCurrentMineralTarget = nullptr;

	if (!m_pCurrentMineralTarget)
	{
		if (Mineral * m = FindHisFarestVisibleMineral())
			Agent()->Gather(m_pCurrentMineralTarget = m);

		return;
	}

	if (m_pCurrentMineralTarget)
		if ((distToRectangle(Agent()->Pos(), m_pCurrentMineralTarget->TopLeft(), m_pCurrentMineralTarget->Size()) < 40) ||
			(!Agent()->CoolDown() && !Agent()->Unit()->isMoving()))
			if (Mineral * m = FindHisFarestVisibleMineral())
				if (dist(m->Pos(), m_pCurrentMineralTarget->Pos()) < 32*3)
					return ChangeState(attackNearest);
				else
					return Agent()->Gather(m_pCurrentMineralTarget = m);


	if ((Agent()->Life() < 40) || (Agent()->Life() + 8 < Agent()->PrevLife(24)))
		return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (!Agent()->CoolDown())
	{
		if (Agent()->Acceleration().Norm() > 4)
			for (const auto & faceOff : Agent()->FaceOffs())
				if (faceOff.His()->Type().isWorker())
				{
					if (faceOff.Distance() < 30)
					if (!faceOff.His()->Unit()->isMoving() || faceOff.His()->Unit()->isCarryingMinerals() || faceOff.His()->Unit()->isCarryingGas())
					if (toVect(faceOff.His()->Pos() - Agent()->Pos()) * Agent()->Acceleration() > 0)
					{
//1						bw->drawCircleMap(faceOff.His()->Pos(), 15, Colors::Yellow);
						return Agent()->Attack(faceOff.His());
					}
				}
	}
	else
		if (m_pCurrentMineralTarget)
			return Agent()->Gather(m_pCurrentMineralTarget);
}


} // namespace stone



