//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef CRITTERS_H
#define CRITTERS_H

#include <BWAPI.h>
#include "his.h"
#include <memory>


namespace stone
{



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Critters
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Critters
{
public:

	void									Update();


	void									AddUnit(BWAPI::Unit u);
	void									RemoveUnit(BWAPI::Unit u);

	const vector<unique_ptr<HisUnit>> &		Get() const			{ return m_Units; }

private:
	vector<unique_ptr<HisUnit>>				m_Units;
};


} // namespace stone


#endif

