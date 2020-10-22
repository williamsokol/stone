//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef STRATEGY_H
#define STRATEGY_H

#include <BWAPI.h>
#include "defs.h"
#include "utils.h"


namespace stone
{

class MyUnit;
	
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Strategy
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Strategy
{
public:
									Strategy();

	void							OnFrame();

	const vector<TilePosition> &	HisPossibleLocations() const	{ return m_HisPossibleLocations; }
	void							RemovePossibleLocation(TilePosition pos);

private:
	void							CheckChasingGroup(const Area * pArea, const vector<MyUnit *> & Group);

	vector<TilePosition>			m_HisPossibleLocations;

};


} // namespace stone


#endif

