//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef SCOUTING_H
#define SCOUTING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../units/my.h"
#include "../utils.h"


namespace stone
{

class VBase;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Scouting
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Scouting : public Behavior<My<Terran_SCV>>
{
public:
	static const vector<Scouting *> &	Instances()					{ return m_Instances; }

								Scouting(My<Terran_SCV> * pAgent);
								~Scouting();

	const Scouting *			IsScouting() const override			{ return this; }
	Scouting *					IsScouting() override				{ return this; }

	string						Name() const override				{ return "scouting"; }
	string						StateName() const override			{ return "-"; }


	BWAPI::Color				GetColor() const override			{ return Colors::Green; }
	Text::Enum					GetTextColor() const override		{ return Text::Green; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * ) const override			{ return false; }

	TilePosition				Target() const						{ return m_Target; }

private:
	TilePosition				m_Target;

	static vector<Scouting *>	m_Instances;
};



} // namespace stone


#endif

