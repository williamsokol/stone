//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef INFO_DRAWER_H
#define INFO_DRAWER_H

#include <BWAPI.h>
#include "defs.h"
#include "utils.h"



namespace stone
{

class Stone;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class InfoDrawer
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//


class InfoDrawer
{
public:

	static bool ProcessCommand(const std::string & command);


	static bool		showTime;
	static bool		showTimes;
	static bool		showGame;
	static bool		showUnit;
	static bool		showBuilding;
	static bool		showCritters;

	struct Color
	{
		static const BWAPI::Color	time;
		static const Text::Enum		timeText;

		static const BWAPI::Color	times;
		static const Text::Enum		timesText;

		static const BWAPI::Color	game;
		static const Text::Enum		gameText;

		static const BWAPI::Color	unit;
		static const Text::Enum		unitText;

		static const BWAPI::Color	building;
		static const Text::Enum		buildingText;

		static const BWAPI::Color	enemy;
		static const Text::Enum		enemyText;

		static const BWAPI::Color	enemyInFog;
		static const Text::Enum		enemyInFogText;

		static const BWAPI::Color	critter;
		static const Text::Enum		critterText;
	};

private:
	static bool ProcessCommandVariants(const std::string & command, const std::string & attributName, bool & attribut);
};


void drawInfos();


} // namespace stone


#endif

