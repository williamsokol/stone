//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "defaultBehavior.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }


namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class DefaultBehavior
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


DefaultBehavior::DefaultBehavior(MyBWAPIUnit * pAgent)
	: Behavior(pAgent)
{

}


DefaultBehavior::~DefaultBehavior()
{

}


void DefaultBehavior::OnFrame_v()
{
	if (!Agent()->Unit()->isInterruptible()) return;
	if (!Agent()->Unit()->isCompleted()) return;

	if (Agent()->IsMyBuilding())
	{
		if (Agent()->Unit()->isTraining()) return;
	}
	else
	{
		if (!Agent()->Unit()->isIdle())
		{
			Agent()->Stop();
		}

		assert_throw(Agent()->Unit()->isIdle());
	}

	Agent()->DefaultBehaviorOnFrame();
}



} // namespace stone



