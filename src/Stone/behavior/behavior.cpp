//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "behavior.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{



bool canWalkOnTile(const TilePosition & t)
{
	if (ai()->GetMap().Valid(t))
	{
		const auto & tile = ai()->GetMap().GetTile(t, no_check);
		if (tile.Walkable())
		if (!tile.GetNeutral())
		if (!tile.Ptr())
			return true;
	}

	return false;
}




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class IBehavior
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

IBehavior::IBehavior(MyBWAPIUnit * pAgent)
	: m_pAgent(pAgent), m_justArrivedInState(true)
{
}


void IBehavior::ResetSubBehavior()
{
	if (m_pSubBehavior) m_pSubBehavior.reset();
	
	if (!AgentBeingDestroyed() && Agent()->Unit()->isInterruptible())
		Agent()->Stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Behavior
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////



	
} // namespace stone



