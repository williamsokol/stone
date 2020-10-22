//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "repairing.h"
#include "fleeing.h"
#include "walking.h"
#include "defaultBehavior.h"
#include "../strategy.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Repairing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Repairing *> Repairing::m_Instances;


Repairing * Repairing::GetRepairer(MyBWAPIUnit * pTarget)
{
	const int minDistToHisStartingBase = 32*50;

	int minDist = max(minDistToHisStartingBase, pTarget->DistToHisStartingBase());
	My<Terran_SCV> * pBestCandidate = nullptr;

	for (const auto & u : ai()->Me().Units(Terran_SCV))
		if (u.get() != pTarget)
		{
			int length;
			ai()->GetMap().GetPath(pTarget->Pos(), u->Pos(), &length);
			if ((length >= 0) && (length < min(minDist, max(minDistToHisStartingBase, u->DistToHisStartingBase()))))
				if (u->GetBehavior()->CanRepair(pTarget))
				{
					minDist = length;
					pBestCandidate = u->As<Terran_SCV>();
				}
		}

	if (!pBestCandidate) return nullptr;
	
	pBestCandidate->ChangeBehavior<Repairing>(pBestCandidate, pTarget);
	return pBestCandidate->GetBehavior()->IsRepairing();
}


Repairing::Repairing(My<Terran_SCV> * pAgent, MyBWAPIUnit * pTarget)
	: Behavior(pAgent), m_pTarget(pTarget)
{
	assert_throw(pAgent != pTarget);
	push_back_assert_does_not_contain(m_Instances, this);
//	ai()->SetDelay(100);
}


Repairing::~Repairing()
{
	try //3
	{
		if (Target()->GetBehavior()->IsRepairing())
		{
			assert_throw(Target()->GetBehavior()->IsRepairing()->Target() == Agent());
			Target()->ChangeBehavior<DefaultBehavior>(Target());
		}

		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
	catch(...){}
}


void Repairing::OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other)
{
	if (other == Target()) Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}


void Repairing::ChangeState(state_t st)
{
	assert_throw(m_state != st);
	
	m_state = st; OnStateChanged();
}


string Repairing::StateName() const
{
	switch(State())
	{
	case reachingTarget:	return "reachingTarget";
	case repairing:			return  (Target()->Life() < Target()->MaxLife()) ? "repairing" : "being repaired";
	default:				return "?";
	}
}


void Repairing::OnFrame_v()
{
//1	if (Target()) drawLineMap(Agent()->Pos(), Target()->Pos(), GetColor());

	if (!Agent()->Unit()->isInterruptible()) return;

	switch (State())
	{
	case reachingTarget:	OnFrame_reachingTarget(); break;
	case repairing:			OnFrame_repairing(); break;
	default: assert_throw(false);
	}
}


void Repairing::OnFrame_reachingTarget()
{
	if (JustArrivedInState())
	{
		SetSubBehavior<Walking>(Agent(), Target()->Pos());
	}

//	if (!findPursuers(Agent()).empty())	return Agent()->ChangeBehavior<Fleeing>(Agent());
	if (Agent()->Life() < Agent()->PrevLife(30)) return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (ai()->Frame() - Agent()->LastDangerReport() < 5)
		if (!findThreats(Agent(), 5*32).empty())
			return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (!findThreats(Agent(), 2*32).empty())
		return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (Agent()->Pos().getApproxDistance(Target()->Pos()) < 32* 2)
	{
		ResetSubBehavior();
		return ChangeState(repairing);
	}

	// This will check if there is a better repairer:
	if (ai()->Frame() % 32 == 0)
		return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}


void Repairing::OnFrame_repairing()
{
	if (JustArrivedInState())// || Agent()->Unit()->isIdle())
	{
		m_inStateSince = ai()->Frame();

		if (Target()->Life() < Target()->MaxLife())
			return Agent()->Repair(Target());
	}

	if (ai()->Frame() - m_inStateSince > 30)
		if (Target()->Life() < Target()->MaxLife())
			if (Target()->Life() == Target()->PrevLife(25)) 
			{
			///	ai()->SetDelay(500);
				return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
			}

	if (Agent()->Life() < Agent()->PrevLife(30)) return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (ai()->Frame() - Agent()->LastDangerReport() < 5)
		if (!findThreats(Agent(), 5*32).empty())
			return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (!findThreats(Agent(), 2*32).empty())
		return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (Target()->Life() == Target()->MaxLife())
		if (!Target()->GetBehavior()->IsRepairing() || (Agent()->Life() == Agent()->MaxLife()))
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());

//	if (Target()->Life() < Target()->MaxLife())
//		return Agent()->Repair(Target());
}


} // namespace stone



