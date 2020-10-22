//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "debug.h"
#include "Stone.h"

namespace { auto & bw = Broodwar; }




namespace stone
{

/*
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class TimerStats
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

void TimerStats::Finishing()
{
	++m_count;
	m_time = m_Timer.ElapsedMilliseconds();
	m_total += m_time;
	m_max = max(m_max, m_time);
}
*/

void reportCommandError(const string & command)
{
	::unused(command);
//2	bw << "BWAPI ERROR: " << bw->getLastError() << " on: " << command << endl;
//2	bw->setLastError();
//2	ai()->SetDelay(2000);
}


void drawLineMap(Position a, Position b, Color color, bool crop)
{
	if (crop)
	{
		a = ai()->GetMap().Crop(a);
		b = ai()->GetMap().Crop(b);
	}

	assert_throw(ai()->GetMap().Valid(TilePosition(a)));
	assert_throw(ai()->GetMap().Valid(TilePosition(b)));
	bw->drawLineMap(a, b, color);
}
	
} // namespace stone


