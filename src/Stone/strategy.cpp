//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "strategy.h"
#include "units/me.h"
#include "behavior/scouting.h"
#include "behavior/mining.h"
#include "behavior/constructing.h"
#include "behavior/harassing.h"
#include "behavior/fleeing.h"
#include "behavior/chasing.h"
#include "behavior/defaultBehavior.h"
#include "Stone.h"

namespace { auto & bw = Broodwar; }




namespace stone
{

static void drawChasingGroup(const vector<MyUnit *> & Group)
{
	assert_throw(Group.size() >= 2);
/*
	vector<const Chasing *> Chasers;
	for (MyUnit * u : Group)
		if (Chasing * pChaser = u->GetBehavior()->IsChasing())
			if (pChaser->Supervised())
				Chasers.push_back(pChaser);

	if (Chasers.empty()) return;
*/
	const auto & Targets = Group.front()->GetBehavior()->IsChasing()->GroupTargets();
	assert_throw(!Targets.empty());

	Position ChasingBoundingBox_TopLeft(1000000, 1000000);
	Position ChasingBoundingBox_BottomRight(-1, -1);

	for (FaceOff::His_t * target : Targets)
		makeBoundingBoxIncludePoint(ChasingBoundingBox_TopLeft, ChasingBoundingBox_BottomRight, target->Pos());

	bw->drawBoxMap(ChasingBoundingBox_TopLeft - 32, ChasingBoundingBox_BottomRight + 32, Group.front()->GetBehavior()->IsChasing()->GetColor());
	bw->drawTextMap(ChasingBoundingBox_TopLeft - 32 - Position(0, 15), "%cadvantage = %d", Group.front()->GetBehavior()->IsChasing()->GetTextColor(), Group.front()->GetBehavior()->IsChasing()->GroupAdvantage());
}


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Strategy
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


Strategy::Strategy()
: m_HisPossibleLocations(ai()->GetMap().StartingLocations())
{
	const Area * pMyStartingArea = ai()->GetMap().GetArea(ai()->Me().Player()->getStartLocation());
	really_remove(m_HisPossibleLocations, ai()->Me().Player()->getStartLocation());
	really_remove_if(m_HisPossibleLocations, [pMyStartingArea](TilePosition t)
		{ return !ai()->GetMap().GetArea(t)->AccessibleFrom(pMyStartingArea); });

//	assert_throw_plus(!m_HisPossibleLocations.empty(), "enemy must be accessible");
	if (m_HisPossibleLocations.empty()) ai()->Him().SetNotAccessible();
}


void Strategy::RemovePossibleLocation(TilePosition pos)
{
	assert_throw(contains(m_HisPossibleLocations, pos));
	really_remove(m_HisPossibleLocations, pos);
}


static vector<FaceOff::His_t *> localTargets(const Area * pArea, const MyUnit * u)
{
	vector<FaceOff::His_t *> Candidates;
	for (const FaceOff & faceOff : u->FaceOffs())
		if (faceOff.His()->GetArea() == pArea)
		//	if (faceOff.HisAttack())
			if (!((pArea == ai()->Me().GetArea()) && faceOff.His()->Type().isWorker()))
				Candidates.push_back(faceOff.His());

	return Candidates;
}


// TODO: move in chasing.cpp
void Strategy::CheckChasingGroup(const Area * pArea, const vector<MyUnit *> & Group)
{
	// Chasers alone will not be grouped:
	assert_throw(all_of(Group.begin(), Group.end(), [](MyUnit * u)
					{ return !u->GetBehavior()->IsChasing() || u->GetBehavior()->IsChasing()->PreviouslyInGroup(); }));

	vector<FaceOff::His_t *> Targets = localTargets(pArea, Group.front());

	if (Group.size() < 2) return;

	if (Targets.empty()) return;

	if (Group.size() < Targets.size()) return;

	for (MyUnit * u : Group)
		if (localTargets(pArea, u) != Targets) return;


	for (MyUnit * u : Group)
		for (const FaceOff & faceOff : u->FaceOffs())
			if (faceOff.His()->GetArea() == pArea)
				if (faceOff.HisAttack() * (2+(int)Targets.size()) >= u->Life())
					return;


	frame_t framesToKillThem, framesToKillUs;
	quickSimulation(Group, Targets, framesToKillThem, framesToKillUs);
	const frame_t advantage = framesToKillUs - framesToKillThem;
///	bw << advantage << " = " << framesToKillUs << " - " << framesToKillThem << endl;

	if (advantage < 20 * (int)Targets.size()) return;

	
	// Ok, now we (re)form the group:

	BWAPIUnit * pBestTarget = nullptr;	// initialized to clear warning
	int minTargetLife = numeric_limits<int>::max();
	for (FaceOff::His_t * target : Targets)
		if (target->LifeWithShields() < minTargetLife)
		{
			minTargetLife = target->LifeWithShields();
			pBestTarget = target;
		}

	for (MyUnit * u : Group)
	{
		Chasing * pChaser = u->GetBehavior()->IsChasing();
		if (!pChaser || (pChaser->Target() != pBestTarget))
		{
			u->ChangeBehavior<Chasing>(u, pBestTarget);
			pChaser = u->GetBehavior()->IsChasing();
		}

		pChaser->SetGroup(pArea, Targets, advantage);
	}

//1	drawChasingGroup(Group);
}


void Strategy::OnFrame()
{
	if (ai()->Him().Accessible())
		if (ai()->Frame() == 0)
		{
			for (int i = 0 ; i < 3 ; ++i)
			{
				auto * pScout = ai()->Me().Units(Terran_SCV)[i]->As<Terran_SCV>();
				pScout->ChangeBehavior<Scouting>(pScout);
			}
		}

	const auto countCC = ai()->Me().Buildings(Terran_Command_Center).size();
	const auto countDepots = ai()->Me().Buildings(Terran_Supply_Depot).size();

	if (My<Terran_SCV> * pMiner = (Mining::Instances().empty() ? nullptr : Mining::Instances().back()->Agent()))
	{
		{
			static int k0 = 1;
			static int k1 = 2;
			static int k2 = 1;
			static int k3 = 1;
			if (ai()->Him().StartingBase())
			{
				if (!ai()->Me().Bases().front()->LackingMiners() ||
					((Mining::Instances().size() > 2) && (countDepots == 0) && (--k0 >= 0)) ||
					((Mining::Instances().size() > 3) && (countDepots == 0)) ||
					((Mining::Instances().size() > 3) && (countDepots == 1) && (--k1 >= 0)) ||
					((Mining::Instances().size() > 5) && (countDepots == 1)) ||
					((Mining::Instances().size() > 5) && (countDepots == 2) && (--k2 >= 0)) ||
					((Mining::Instances().size() > 6) && (countDepots == 2)) ||
					((Mining::Instances().size() > 6) && (countDepots == 3) && (--k3 >= 0)) ||
					((Mining::Instances().size() > 7) && (countDepots >= 3) && (countCC == 1)) ||
					((Mining::Instances().size() > 6*countCC) && (countDepots >= 2) && (countCC > 1))
				   )
				return pMiner->ChangeBehavior<Harassing>(pMiner);
			}
		}
	
		if (ai()->Me().SupplyMax() < 200)
			if (ai()->Me().SupplyAvailable() <= ((countDepots <= 1) ? 1 : 2))
				if (ai()->Me().CanPay(Terran_Supply_Depot))
					if (!beingConstructed(Terran_Supply_Depot))
						return pMiner->ChangeBehavior<Constructing>(pMiner, Terran_Supply_Depot);
			
		if (ai()->Me().Buildings(Terran_Command_Center).size() < 4)
			if (ai()->Me().CanPay(Terran_Command_Center))
				if (!beingConstructed(Terran_Command_Center))
					return pMiner->ChangeBehavior<Constructing>(pMiner, Terran_Command_Center);
	}


	// group-Chasing management

	if (ai()->Him().StartingBase())
	{
		// 1) Clear all the groups:
		for (Chasing * pChaser : Chasing::Instances())
			pChaser->Ungroup();

		// 2) For each Area, check if a group of Chasing units can be formed:
		{
			map<const Area *, vector<MyUnit *>> CandidatesByArea;

			for (MyUnit * u : ai()->Me().Units())
				if (u->Unit()->isCompleted())
					if (u->Life() >= 20)
						if (!u->GetBehavior()->IsScouting())
						if (!u->GetBehavior()->IsConstructing())
						if (!u->GetBehavior()->IsChasing() || u->GetBehavior()->IsChasing()->PreviouslyInGroup())
							CandidatesByArea[u->GetArea()].push_back(u);

			for (auto a : CandidatesByArea)
				CheckChasingGroup(a.first, a.second);
		}

		// 3) Reset Chasing units that just leaved a group:
		for (MyUnit * u : ai()->Me().Units())
			if (Chasing * pChaser = u->GetBehavior()->IsChasing())
				if (pChaser->PreviouslyInGroup() && !pChaser->InGroup())
					u->ChangeBehavior<DefaultBehavior>(u);
	}

/*
	if (ai()->Frame() == 10)
		ai()->Me().Units()[0]->ChangeBehavior<Fleeing>(ai()->Me().Units()[0]);

	if (ai()->Frame() == 50)
		ai()->Me().Units()[1]->ChangeBehavior<Chasing>(ai()->Me().Units()[1], ai()->Me().Units()[0]);
*/

}




} // namespace stone



