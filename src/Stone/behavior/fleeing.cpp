//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "fleeing.h"
#include "walking.h"
#include "repairing.h"
#include "chasing.h"
#include "defaultBehavior.h"
#include "harassing.h"
#include "../strategy.h"
#include "../Stone.h"
#include <queue>

namespace { auto & bw = Broodwar; }


namespace stone
{

static void markImpasse(const TilePosition & pos, int radius = 2)
{
	if (radius == 0) return;

	assert_throw(ai()->GetMap().Valid(pos));

	vector<TilePosition> FreeNeighbours;
	for (auto delta : {TilePosition(0, -1), TilePosition(-1, 0), TilePosition(+1, 0), TilePosition(0, +1)})
	{
		TilePosition t = pos + delta;
		if (ai()->GetMap().Valid(t))
		{
			const auto & tile = ai()->GetMap().GetTile(t, no_check);
			if (tile.Walkable())
			if (!ai()->GetVMap().ImpasseOrNearBuildingOrNeutral(tile))
				FreeNeighbours.push_back(t);
		}
	}

	if (FreeNeighbours.size() <= 2)
	{
		ai()->GetVMap().SetImpasse(ai()->GetMap().GetTile(pos, no_check));

		for (auto t : FreeNeighbours)
			markImpasse(t, radius-1);
	}
}



const TilePosition dimCC = UnitType(Terran_Command_Center).tileSize();

const double pursuer_angle = cos(pi/4);



const Area * findFarArea(const Area * pFrom)
{
	vector<const Area *> Candidates;

	for (int i = 8 ; i >= 4 ; i -= 2)
	{
		Candidates.clear();
		for (const Area & area : ai()->GetMap().Areas())
			if (area.MaxAltitude() > 32*i)
				if (&area != pFrom)
				if (&area != ai()->Him().StartingBase()->BWEMPart()->GetArea())
				if (&area != ai()->Me().GetArea())
				if (area.AccessibleFrom(pFrom))
					Candidates.push_back(&area);

		if (Candidates.size() >= 1)
		{
			return random_element(Candidates);
		}
	}

	return pFrom;
}


vector<const FaceOff *> findPursuers(const MyUnit * pAgent)
{
//1	bw->drawCircleMap(pAgent->Pos(), pAgent->Sight() + 7*32, Colors::Purple);
	vector<const FaceOff *> Pursuers;
	for (const auto & faceOff : pAgent->FaceOffs())
		if (faceOff.HisAttack())
		if (faceOff.Distance() < pAgent->Sight() + 5*32)
		{
			Vect PA = toVect(pAgent->Pos() - faceOff.His()->Pos());
			PA.Normalize();

			Vect Pacc = faceOff.His()->Acceleration();
			Pacc.Normalize();

			if (PA * Pacc > pursuer_angle)
			{
				Pursuers.push_back(&faceOff);
//1				bw->drawCircleMap(faceOff.His()->Pos(), 18, Colors::Purple);
//1				drawLineMap(faceOff.His()->Pos(), pAgent->Pos(), Colors::Purple);
			}
		}

	return Pursuers;
}


vector<const FaceOff *> findThreats(const MyUnit * pAgent, int maxDistToHisRange, int * pDistanceToHisFire)
{
	if (pDistanceToHisFire) *pDistanceToHisFire = numeric_limits<int>::max();

	vector<const FaceOff *> Threats;
	for (const auto & faceOff : pAgent->FaceOffs())
		if (faceOff.HisAttack())
		if (faceOff.DistanceToHisRange() < maxDistToHisRange)
		{
			Threats.push_back(&faceOff);
			if (pDistanceToHisFire)
				if (faceOff.DistanceToHisRange() < *pDistanceToHisFire)
					*pDistanceToHisFire = faceOff.DistanceToHisRange();
		}

	return Threats;
}




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Fleeing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Fleeing *> Fleeing::m_Instances;

Fleeing::Fleeing(MyUnit * pAgent, frame_t noAlertFrames)
	: Behavior(pAgent),
	m_state((Agent()->GetArea()->MaxAltitude() > 32*8) ? insideArea : anywhere),
	m_noAlertUntil(ai()->Frame() + noAlertFrames)
{
	push_back_assert_does_not_contain(m_Instances, this);
//	ai()->SetDelay(100);
}


Fleeing::~Fleeing()
{
	try //3
	{
		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
	catch(...){}
}


void Fleeing::ChangeState(state_t st)
{
	assert_throw(m_state != st);
	
	m_state = st; OnStateChanged();

	m_pFarArea = nullptr;
}


string Fleeing::StateName() const
{
	switch(State())
	{
	case dragOut:		return "dragOut";
	case insideArea:	return "insideArea";
	case anywhere:		return "anywhere";
//	case takeAway:		return "takeAway";
	default:			return "?";
	}
}


bool Fleeing::CanRepair(const MyBWAPIUnit * ) const
{
	if (State() == dragOut) return false;

	return CanReadAvgPursuers() && (AvgPursuers() < 0.5);
}


Vect Fleeing::CalcultateDivergingVector() const
{
	Fleeing * pCloser = nullptr;
	const int maxDist = 32*10;
	int minDist = maxDist;

	for (Fleeing * pFleeing : Instances())
		if (pFleeing != this)
		{
			int d = Agent()->Pos().getApproxDistance(pFleeing->Agent()->Pos());
			if (d < minDist) 
			{
				minDist = d;
				pCloser = pFleeing;
			}
		}

	Vect dir;
	if (pCloser)
	{
		dir = toVect(Agent()->Pos() - pCloser->Agent()->Pos());
		dir.Normalize();
		dir *= ((maxDist - minDist) / 2 / (double)maxDist);
	}

	return dir;
}


Vect Fleeing::CalcultateThreatVector(const vector<const FaceOff *> & Threats) const
{
	Vect dir;
	for (const auto * pFaceOff : Threats)
	{
		Vect d = toVect(Agent()->Pos() - pFaceOff->His()->Pos());
		d.Normalize();
		d *= pFaceOff->HisAttack();
		d *= double(pFaceOff->HisRange()) / (max(1, pFaceOff->Distance()));
		dir += d;
//1		drawLineMap(pFaceOff->His()->Pos(), pFaceOff->His()->Pos() + toPosition(32*d), Colors::White, crop);
	}

	dir.Normalize();
	return dir;
}


Vect Fleeing::CalcultateIncreasingAltitudeVector() const
{
	const auto w = WalkPosition(Agent()->Pos());
	const altitude_t altitude = ai()->GetMap().GetMiniTile(w).Altitude();
	const altitude_t maxAltitude = Agent()->GetArea()->MaxAltitude();

	Vect dir = toVect(Position(ai()->GetVMap().GetIncreasingAltitudeDirection(WalkPosition(Agent()->Pos()))));
	dir.Normalize();
	dir *= (maxAltitude - min(altitude * 2, (int)maxAltitude)) / 2 / (double)maxAltitude;
	return dir;
}


static map<int, vector<WalkPosition>> DeltasByRadius;


void Fleeing::AdjustTarget(Position & target, const int walkRadius) const
{
	vector<Area::id> AreasAllowed = {Agent()->GetArea()->Id()};

	if ((State() == anywhere) || (Agent()->Life() < Agent()->PrevLife(30)))
		for (const Area * area : Agent()->GetArea()->AccessibleNeighbours())
			if (area != ai()->Me().GetArea())
				AreasAllowed.push_back(area->Id());

	bool searchLastTarget = (m_lastPos.getApproxDistance(Agent()->Pos()) < 32);
	bool foundLastTarget = false;

	Position bestTarget = target;
	int minDistToTarget = numeric_limits<int>::max();

	vector<WalkPosition> & Deltas = DeltasByRadius[walkRadius];
	if (Deltas.empty())
		for (int dy = -walkRadius ; dy <= +walkRadius ; ++dy)
		for (int dx = -walkRadius ; dx <= +walkRadius ; ++dx)
			if (abs(norm(dx, dy) - walkRadius) < 0.35)
				Deltas.push_back(WalkPosition(dx, dy));

	int candidates = 0;
	for (const WalkPosition & delta : Deltas)
	{
		const WalkPosition w = WalkPosition(Agent()->Pos()) + delta;
		const Position p = center(w);
		if (ai()->GetMap().Valid(w))
		{
			const auto & miniTile = ai()->GetMap().GetMiniTile(w, no_check);
			if (!contains(AreasAllowed, miniTile.AreaId())) continue;
			const Area * area = ai()->GetMap().GetArea(miniTile.AreaId());
			if (Agent()->GetArea() != area)
				if (all_of(Agent()->GetArea()->ChokePoints(area).begin(), Agent()->GetArea()->ChokePoints(area).end(),
					[p, walkRadius](const ChokePoint & cp){ return VChokePoint::Get(&cp)->AirDistanceFrom(p) > walkRadius*8; }))
					continue;
			if (miniTile.Altitude() < 8) continue;

			bool blocked = false;
			for (int iwalkRadius = walkRadius ; iwalkRadius >= 4 ; iwalkRadius -= 4)
			{
				const WalkPosition iDelta = delta * iwalkRadius / walkRadius;
				const WalkPosition iw = WalkPosition(Agent()->Pos()) + iDelta;
				const auto & iTile = ai()->GetMap().GetTile(TilePosition(iw), no_check);

				if (ai()->GetVMap().ImpasseOrNearBuildingOrNeutral(iTile)) { blocked = true; break; }
				
				const Position ip = center(iw);
				if (any_of(ai()->Critters().Get().begin(), ai()->Critters().Get().end(),
					[ip](const unique_ptr<HisUnit> & critter){ return critter->Pos().getApproxDistance(ip) < 32; }))
					{ blocked = true; break; }
			}
			if (blocked) continue;


			int d = target.getApproxDistance(p);
			if (d < minDistToTarget)
			{
				minDistToTarget = d;
				bestTarget = p;
			}

			if (searchLastTarget && !foundLastTarget)
				if (m_lastTarget.getApproxDistance(p) < 32*3)
					foundLastTarget = true;

		///	drawLineMap(Agent()->Pos(), p, Colors::Grey);

			++candidates;
		}
	}

	if (searchLastTarget && !foundLastTarget && (candidates > 10))
		markImpasse(TilePosition(m_lastTarget));

	target = bestTarget;
}


void Fleeing::OnFrame_v()
{
//1	if (m_pFarArea) drawLineMap(Agent()->Pos(), Position(m_pFarArea->Top()), GetColor());

	if (!Agent()->Unit()->isInterruptible()) return;

	{
		++m_numberOf_pursuersCount;
		const vector<const FaceOff *> & Pursuers = findPursuers(Agent());
		for (const FaceOff * faceOff : Pursuers)
			if (HisUnit * pHisUnit = faceOff->His()->IsHisUnit())
				pHisUnit->SetPursuingTarget(Agent());
		
		if (CanReadAvgPursuers() && (AvgPursuers() >= 1.5)) Alert();

		m_cumulativePursuers += Pursuers.size();

//1		bw->drawTextMap(Agent()->Pos().x + Agent()->Type().width() + 3, Agent()->Pos().y - Agent()->Type().height() - 10, "%cpursuers: %.1f", Text::Purple, AvgPursuers());

		if (m_numberOf_pursuersCount > 30)
		{
			m_avgPursuers = m_cumulativePursuers / double(m_numberOf_pursuersCount);
			m_cumulativePursuers = 0;
			m_numberOf_pursuersCount = 0;
			m_canReadAvgPursuers = true;
		}
	}

	switch (State())
	{
	case dragOut:		OnFrame_dragOut(); break;
	case insideArea:
	case anywhere:		OnFrame_anywhere(); break;
//	case takeAway:		OnFrame_takeAway(); break;
	default: assert_throw(false);
	}
}




bool Fleeing::AttackedAndSurrounded() const
{
	assert_throw(Agent()->Is<Terran_SCV>());

	if (Agent()->Life() + 8 < Agent()->PrevLife(24)) return true;

	if (Agent()->Life() < Agent()->PrevLife(20))
		if (findThreats(Agent(), 2*32).size() >= 3)
			return true;

	return false;
}


void Fleeing::Alert()
{
	if (ai()->Frame() < m_noAlertUntil) return;

//1	const int alertRadius = 32*(6 + (CanReadAvgPursuers() ? int(AvgPursuers()+0.5) : 2));
//1	bw->drawCircleMap(Agent()->Pos(), alertRadius, GetColor());
	for (MyUnit * u : ai()->Me().Units()) if (u != Agent())
		if (u->Pos().getApproxDistance(Agent()->Pos()) < 32*8)
		{
			u->OnDangerHere();
//1			bw->drawCircleMap(u->Pos(), 10, GetColor());
//1			drawLineMap(Agent()->Pos(), u->Pos(), GetColor());
		}
}


void Fleeing::OnFrame_dragOut()
{
	if (JustArrivedInState())
	{
		assert_throw(Agent()->Is<Terran_SCV>());
		m_inStateSince = ai()->Frame();
		Alert();
	}

	if (ai()->Frame() - m_inStateSince > 50)
		if (findThreats(Agent(), 2*32).size() < 2)
			return ChangeState(insideArea);

	for (const auto * base : ai()->Me().Bases())
		if (Mineral * m = base->FindVisibleMineral())
			return Agent()->Is<Terran_SCV>()->Gather(m);

	return Agent()->Move(Position(ai()->Me().GetArea()->Top()));
}


void Fleeing::OnFrame_anywhere()	// also insideArea
{
	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
		m_lastPos = Positions::None;
	}

	if (My<Terran_SCV> * pSCV = Agent()->Is<Terran_SCV>())
		if (pSCV->GetArea() != ai()->Me().GetArea())
		{
			if (AttackedAndSurrounded())
				return ChangeState(dragOut);

			if (ai()->Frame() - m_inStateSince > 50)
				if (ai()->Frame() - Agent()->LastFrameMoving() > 50)
					return ChangeState(dragOut);
		}

	int distanceToHisFire;
	vector<const FaceOff *> Threats = findThreats(Agent(), 5*32, &distanceToHisFire);

	if (State() == insideArea)
	{
		if (distanceToHisFire < 3*32)
		{
			m_remainingFramesInFreeState = 80;
			return ChangeState(anywhere);
		}
		else m_remainingFramesInFreeState = 1000000;
	}
	else
	{
		assert_(State() == anywhere);
		if (--m_remainingFramesInFreeState == 0) return ChangeState(insideArea);
	}


	if ((m_numberOf_pursuersCount == 0) && CanReadAvgPursuers())
	{
		if (AvgPursuers() > 2.5)
		{
			if (State() == insideArea)
				return ChangeState(anywhere);
		}
		else if (AvgPursuers() < 0.5)
		{
			if (Agent()->Life() == Agent()->PrevLife(30))
			{
				return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
			}
		}
		else if (AvgPursuers() < 1.1)
		{
			auto Threats = findThreats(Agent(), 5*32);
			if (Threats.size() == 1)
				if (HisUnit * pHisUnit = Threats.front()->His()->IsHisUnit())
					if (pHisUnit->PursuingTarget() == Agent())
						if (Threats.front()->FramesToKillMe() > Threats.front()->FramesToKillHim() + 10)
							return Agent()->ChangeBehavior<Chasing>(Agent(), pHisUnit);
		}
	}


	if (!Threats.empty())
	{
		Vect threatForce = CalcultateThreatVector(Threats);
		Vect divergingForce = CalcultateDivergingVector();
		Vect altitudeForce = CalcultateIncreasingAltitudeVector();
		Vect dir = threatForce + divergingForce + altitudeForce;
		dir.Normalize();

		const int walkRadius = 24;
		Position delta = toPosition(dir * (8*walkRadius));
		Position target = Agent()->Pos() + delta;
		AdjustTarget(target, walkRadius);
		if (ai()->GetMap().Valid(target))
		{
			m_lastPos = Agent()->Pos();
			m_lastTarget = target;
//1			drawLineMap(Agent()->Pos(), Agent()->Pos() + toPosition(threatForce*8*walkRadius), Colors::Red, crop);
//1			drawLineMap(Agent()->Pos(), Agent()->Pos() + toPosition(divergingForce*8*walkRadius), Colors::Blue, crop);
//1			drawLineMap(Agent()->Pos(), Agent()->Pos() + toPosition(altitudeForce*8*walkRadius), Colors::Yellow, crop);
//1			drawLineMap(Agent()->Pos(), target, GetColor());
			return Agent()->Move(target);
		}
	}
	else
	{
		if (ai()->Frame() - Agent()->LastFrameMoving() > 20)
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());

		return Agent()->Stop();
	}

}


/*
void Fleeing::OnFrame_takeAway()
{
	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
		m_pFarArea = findFarArea(Agent()->GetArea());

		SetSubBehavior<Walking>(Agent(), Position(m_pFarArea->Top()));
	}

	if (My<Terran_SCV> * pSCV = Agent()->Is<Terran_SCV>())
	{
		if (AttackedAndSurrounded() || (ai()->Frame() - Agent()->LastFrameMoving() > 50))
		{
			ResetSubBehavior();
			return ChangeState(dragOut);
		}
	}

	if (Agent()->GetArea() == m_pFarArea)
		if (Agent()->Pos().getApproxDistance(Position(m_pFarArea->Top())) < 32*5)
		{
			ResetSubBehavior();
			return ChangeState(insideArea);
		}

	if (ai()->Frame() - Agent()->LastDangerReport() < 5)
		if (!findThreats(Agent(), 5*32).empty())
		{
			ResetSubBehavior();
			return ChangeState(anywhere);
		}

	if (findThreats(Agent(), 5*32).empty())
	{
		if (GetSubBehavior()) return ResetSubBehavior();

		if (ai()->Frame() - Agent()->LastFrameMoving() > 30)
			return Agent()->ChangeBehavior<DefaultBehavior>(Agent());
	}
	else if (!GetSubBehavior()) SetSubBehavior<Walking>(Agent(), Position(m_pFarArea->Top()));

	//if (ai()->Frame() - m_inStateSince > 20)
//		if (m_numberOf_pursuersCount == 0)
//			if (AvgPursuers() < 1.1)
//				return Agent()->ChangeBehavior<DefaultBehavior>(Agent());

}
*/

} // namespace stone



