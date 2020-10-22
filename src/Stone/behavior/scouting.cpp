//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "scouting.h"
#include "walking.h"
#include "harassing.h"
#include "defaultBehavior.h"
#include "../strategy.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Scouting
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Scouting *> Scouting::m_Instances;

Scouting::Scouting(My<Terran_SCV> * pAgent)
	: Behavior(pAgent)
{
	const auto & HisPossibleLocations = ai()->GetStrategy()->HisPossibleLocations();
	assert_throw(!HisPossibleLocations.empty());

	int index = min(2, (int)Instances().size());
	if (index < (int)HisPossibleLocations.size())
	{
	///	Log << Agent()->NameWithId() << ": unassigned target = " << HisPossibleLocations[index] << endl;
	}
	else
	{
		index %= HisPossibleLocations.size();	// TODO find best target
	///	Log << Agent()->NameWithId() << ": assigned target = " << HisPossibleLocations[index] << endl;
	}
	m_Target = HisPossibleLocations[index];

	SetSubBehavior<Walking>(pAgent, Position(Target()));

	push_back_assert_does_not_contain(m_Instances, this);
}


Scouting::~Scouting()
{
	try //3
	{
		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
	catch(...){}
}


void Scouting::OnFrame_v()
{
	if (!Agent()->Unit()->isInterruptible()) return;

	if (ai()->Him().StartingBase())
		return Agent()->ChangeBehavior<Harassing>(Agent());

	const bool targetAlreadyReached = !contains(ai()->GetStrategy()->HisPossibleLocations(), Target());
	if (targetAlreadyReached || Agent()->Pos().getApproxDistance(Position(Target()) + Position(UnitType(Terran_Command_Center).tileSize())/2) < 32*6)
	{
	///	if (targetAlreadyReached) Log << Agent()->NameWithId() << ": target already reached" << endl;
		if (!targetAlreadyReached)
			ai()->GetStrategy()->RemovePossibleLocation(Target());

		return Agent()->ChangeBehavior<Scouting>(Agent());
	}
}



} // namespace stone



