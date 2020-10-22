//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "mining.h"
#include "fleeing.h"
#include "repairing.h"
#include "chasing.h"
#include "defaultBehavior.h"
#include "../units/my.h"
#include "../territory/stronghold.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Mining
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Mining *> Mining::m_Instances;


Mining::Mining(My<Terran_SCV> * pSCV)
	: Behavior(pSCV), m_pBase(pSCV->GetStronghold()->HasBase())
{
	assert_throw(pSCV->GetStronghold()->HasBase());
	pSCV->GetStronghold()->HasBase()->AddMiner(this);

	Mineral * m = FindFreeMineral();
	assert_throw(m);
	Assign(m);

	m_state = Agent()->Unit()->isCarryingMinerals() ? returning : gathering;
	push_back_assert_does_not_contain(m_Instances, this);
}


Mining::~Mining()
{
	try //3
	{
		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);

		assert_throw(Agent()->GetStronghold()->HasBase());
		Agent()->GetStronghold()->HasBase()->RemoveMiner(this);

		Release();
	}
	catch(...){}
}


// Finds a free Mineral to gather.
// Minerals not assigned yet are prefered first, then the closest ones (to the Command Center).
Mineral * Mining::FindFreeMineral() const
{
	map<int, Mineral *> MineralsByDist;
	for (Mineral * m : GetBase()->BWEMPart()->Minerals())
		if (m->Data() < 2)	// Cf. class Mineral comments
			MineralsByDist[10000*m->Data() + distToRectangle(GetBase()->BWEMPart()->Center(), m->TopLeft(), m->Size())] = m;

	return MineralsByDist.empty() ? nullptr : MineralsByDist.begin()->second;
}


void Mining::Assign(Mineral * m)
{
	assert_throw(m && !m_pMineral);
	assert_throw(m->Data() < 2);		// Cf. class Mining comments
	m_pMineral = m;
	m->SetData(m->Data() + 1);
}


void Mining::Release()
{
	assert_throw(m_pMineral);
	assert_throw(m_pMineral->Data() >= 1);		// Cf. class Mining comments
	m_pMineral->SetData(m_pMineral->Data() - 1);
	m_pMineral = nullptr;
}


void Mining::OnMineralDestroyed(Mineral *)
{
	Agent()->ChangeBehavior<DefaultBehavior>(Agent());
}


bool Mining::CanRepair(const MyBWAPIUnit * pTarget) const
{
	return ((Me().MineralsAvailable() > 100) || (Mining::Instances().size() >= 3))
		&& (pTarget->GetArea() == m_pBase->BWEMPart()->GetArea());
}


void Mining::OnFrame_v()
{
//1	drawLineMap(Agent()->Pos(), GetMineral()->Pos(), GetColor());

	if (!Agent()->Unit()->isInterruptible()) return;

	if (Agent()->Life() < 10)
		if (!findThreats(Agent(), 5*32).empty())
			return Agent()->ChangeBehavior<Fleeing>(Agent());

	switch (State())
	{
	case gathering:		OnFrame_gathering(); break;
	case returning:		OnFrame_returning(); break;
	default: assert_throw(false);
	}
}



void Mining::OnFrame_gathering()
{
	if (Agent()->Unit()->isCarryingMinerals()) return ChangeState(returning);

	if (Agent()->Life() < Agent()->PrevLife(30)) return Agent()->ChangeBehavior<Fleeing>(Agent());

	if ((Me().MineralsAvailable() > 100) || (Mining::Instances().size() >= 2))
		if (Agent()->Life() < Agent()->MaxLife())
			if (Repairing * pRepairer = Repairing::GetRepairer(Agent()))
				return Agent()->ChangeBehavior<Repairing>(Agent(), pRepairer->Agent());


	if (Agent()->Unit()->isMoving())
	{
		auto Threats10 = findThreats(Agent(), 10*32);
		if (!Threats10.empty())
		{
			if (Agent()->GetArea() == ai()->Me().GetArea())
				for (const FaceOff * faceOff : Threats10)
					if (faceOff->His()->Type().isWorker())
						if (faceOff->His()->GetArea() == ai()->Me().GetArea())
							if (faceOff->His()->Chasers().empty())
								if (Agent()->Life() > 30)
									return Agent()->ChangeBehavior<Chasing>(Agent(), faceOff->His(), bool("insist"));
			
			if (!findThreats(Agent(), 3*32).empty())
				return Agent()->ChangeBehavior<Fleeing>(Agent());
		}
	}

	if ((distToRectangle(Agent()->Pos(), GetMineral()->TopLeft(), GetMineral()->Size()) > 18) ||
		Agent()->Unit()->isMoving() ||
		Agent()->Unit()->isIdle())
		return Agent()->Gather(GetMineral());
}


void Mining::OnFrame_returning()
{
	if (!Agent()->Unit()->isCarryingMinerals()) return ChangeState(gathering);

	if (JustArrivedInState() || Agent()->Unit()->isIdle())
		return Agent()->ReturnCargo();

}





} // namespace stone



