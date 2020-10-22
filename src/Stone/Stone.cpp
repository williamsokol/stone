//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "Stone.h"
#include "strategy.h"
#include "behavior/harassing.h"
#include "behavior/mining.h"
#include "infoDrawer.h"
#include "debug.h"
#include <iostream>

using namespace Filter;
namespace { auto & bw = Broodwar; }


namespace stone
{


Stone * Stone::m_pInstance = nullptr;


Stone::Stone()
  :m_Map(BWEM::Map::Instance())
{
	m_pInstance = this;
	SetDelay(0);
}

Stone::~Stone()
{

}


void Stone::SetDelay(delay_t delay)
{
	m_delay = delay;
	bw->setLocalSpeed(delay);
}


void Stone::Update()
{
	m_frame = bw->getFrameCount();
	m_time = bw->elapsedTime();

	exceptionHandler("Critters().Update()", 5000, [this]()
	{
		Critters().Update();
	});

	Him().Update();
	Me().Update();	// keep this call after Him().Update(), as Me().Update() depends on Him().Update()
}


void Stone::RunBehaviors()
{
	Me().RunBehaviors();
}


void Stone::onStart()
{
	exceptionHandler("onStart", 5000, [this]()
	{
	///	Log << Frame() << " onStart : " << endl;

	//	bw << "The map is " << bw->mapName() << "!" << endl;

		bw->enableFlag(Flag::UserInput);
	//	bw->enableFlag(Flag::CompleteMapInformation);

		bw->setCommandOptimizationLevel(0);

		if (bw->isReplay())
		{
			bw << "The following players are in this replay:" << endl;

			for (auto p : bw->getPlayers())
				if (!p->isObserver())
					bw << p->getName() << ", playing as " << p->getRace() << endl;
		}
		else
		{
		//	if (bw->enemy()) bw << "The matchup is " << bw->self()->getRace() << " vs " << bw->enemy()->getRace() << endl;


			GetMap().Initialize();
			GetMap().FindBasesForStartingLocations();

			m_pVMap = make_unique<VMap>(&GetMap());

			Me().Initialize();

			m_pStrategy = make_unique<Strategy>();

			MapDrawer::ProcessCommand("hide all");

			bw->setLocalSpeed(Delay());
			bw->sendText("be ready!");
		}
	});
}


void Stone::onEnd(bool isWinner)
{
	exceptionHandler("onEnd", 5000, [this, isWinner]()
	{
	///	Log << Frame() << " onEnd : " << endl;

		if (bw->isReplay())
		{
		}
		else
		{
			if (isWinner)
			{
				bw->sendText("that was interesting!");
			}
			else
			{
				bw->sendText("you surprised me!");
			}

			while (!Me().Units().empty())
				Me().RemoveUnit(Me().Units().back()->Unit());

			while (!Me().Buildings().empty())
				Me().RemoveBuilding(Me().Buildings().back()->Unit());
		}
	});
}


void Stone::onFrame()
{
	exceptionHandler("onFrame", 5000, [this]()
	{
		if (bw->isReplay())
		{
		}
		else
		{
			if (bw->isReplay() || bw->isPaused() || !bw->self() || HeHasLeft())
				return;

//4			m_OnFrameTimes["Update"].Starting();
			Update();
//4			m_OnFrameTimes["Update"].Finishing();

//4			m_OnFrameTimes["RunBehaviors"].Starting();
			RunBehaviors();
//4			m_OnFrameTimes["RunBehaviors"].Finishing();

//4			m_OnFrameTimes["drawMap"].Starting();
			//drawMap(GetMap());
//4			m_OnFrameTimes["drawMap"].Finishing();

//4			m_OnFrameTimes["strategy"].Starting();
			GetStrategy()->OnFrame();
//4			m_OnFrameTimes["strategy"].Finishing();


//1			drawInfos();
		}
	});
}


void Stone::onSendText(string text)
{
	exceptionHandler("onSendText", 5000, [this, text]()
	{
	///	Log << Frame() << " onSendText : " << endl;

		if (!MapDrawer::ProcessCommand(text))
		if (!InfoDrawer::ProcessCommand(text))
		{
			int delay;
			if (from_string(text, delay))	SetDelay(delay);
			else							bw->sendText("%s", text.c_str());
		}
	});
}


void Stone::onReceiveText(BWAPI::Player player, string text)
{
	exceptionHandler("onReceiveText", 5000, [this, player, text]()
	{
	///	Log << Frame() << " onReceiveText : " << endl;

		// Parse the received text
	//	bw << player->getName() << " said \"" << text << "\"" << endl;
	});
}


void Stone::onPlayerLeft(BWAPI::Player player)
{
	exceptionHandler("onPlayerLeft", 5000, [this, player]()
	{
	///	Log << Frame() << " onPlayerLeft : " << endl;

		// Interact verbally with the other players in the game by
		// announcing that the other player has left.
	//	bw->sendText("Goodbye %s!", player->getName().c_str());

		m_heHasLeft = true;
	});
}


void Stone::onNukeDetect(BWAPI::Position target)
{
	exceptionHandler("onNukeDetect", 5000, [this, target]()
	{
	///	Log << Frame() << " onNukeDetect : " << endl;

	});
}


void Stone::onUnitDiscover(BWAPI::Unit unit)
{
	exceptionHandler("onUnitDiscover", 5000, [this, unit]()
	{
	///	Log << Frame() << " onUnitDiscover : " << endl;

	});
}


void Stone::onUnitEvade(BWAPI::Unit unit)
{
	exceptionHandler("onUnitEvade", 5000, [this, unit]()
	{
	///	Log << Frame() << " onUnitEvade : " << endl;

	});
}


void Stone::onUnitShow(BWAPI::Unit unit)
{
	exceptionHandler("onUnitShow", 5000, [this, unit]()
	{
		if (bw->isReplay())
		{
		}
		else if (!HeHasLeft())
		{
		//	bw << Frame() << " onUnitShow : " << unit->getType() << " #" << unit->getID() << endl;
			
			if (unit->getPlayer() != bw->self())
			{
				if (unit->getType().isNeutral())
				{
					if (unit->getType().isCritter())		Critters().AddUnit(unit);
				}
				else if (!unit->getType().isSpecialBuilding())
				{
				//	bw << Frame() << " onUnitShow : " << unit->getType() << " #" << unit->getID() << endl;
					if (unit->getType().isBuilding())	Him().OnShowBuilding(unit);
					else								Him().AddUnit(unit);
				}
			}
		}
	});
}


void Stone::onUnitHide(BWAPI::Unit unit)
{
	exceptionHandler("onUnitHide", 5000, [this, unit]()
	{
		if (bw->isReplay())
		{
		}
		else if (!HeHasLeft())
		{
		///	bw << Frame() << " onUnitHide : " << unit->getType() << " #" << unit->getID() << endl;

			if (unit->getPlayer() != bw->self())
			{
				if (!unit->getInitialType().isMineralField())
				if (!unit->getType().isSpecialBuilding())
				{
				///	bw << Frame() << " onUnitHide : " << unit->getType() << " #" << unit->getID() << endl;
					if (unit->getInitialType().isCritter()) Critters().RemoveUnit(unit);

					Him().OnHideBuilding(unit);
					Him().RemoveUnit(unit);
				}
			}
		}
	});
}


void Stone::onUnitCreate(BWAPI::Unit unit)
{
	exceptionHandler("onUnitCreate", 5000, [this, unit]()
	{
		if (bw->isReplay())
		{
			// if we are in a replay, then we will print out the build order of the structures
			if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
			{
				int seconds = Frame() / 24;
				int minutes = seconds / 60;
				seconds %= 60;
				bw->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
			}
		}
		else
		{
		///	bw << Frame() << " onUnitCreate : " << unit->getType() << " #" << unit->getID() << endl;

			if (unit->getPlayer() == bw->self())
			{
				if (unit->getType().isBuilding())	Me().AddBuilding(unit);
				else								Me().AddUnit(unit);
			}
		}
	});
}


void Stone::onUnitDestroy(BWAPI::Unit unit)
{
	exceptionHandler("onUnitDestroy", 5000, [this, unit]()
	{
		if (bw->isReplay())
		{
		}
		else
		{
		///	bw << Frame() << " onUnitDestroy : " << unit->getType() << " #" << unit->getID() << endl;

			if (unit->getPlayer() == bw->self())
			{
				if (unit->getType().isBuilding())	Me().RemoveBuilding(unit);
				else								Me().RemoveUnit(unit);
			}
			else
				if (unit->getType().isMineralField())			OnMineralDestroyed(unit);
				else if (unit->getType().isSpecialBuilding())	OnStaticBuildingDestroyed(unit);
		}
	});
}


void Stone::onUnitMorph(BWAPI::Unit unit)
{
	exceptionHandler("onUnitMorph", 5000, [this, unit]()
	{
		if (bw->isReplay())
		{
			// if we are in a replay, then we will print out the build order of the structures
			if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
			{
				int seconds = Frame() / 24;
				int minutes = seconds / 60;
				seconds %= 60;
				bw->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
			}
		}
		else
		{
		//	bw << Frame() << " onUnitMorph : " << unit->getType().getName() << " #" << unit->getID() << endl;

			if (unit->getPlayer() == bw->self())
			{
				assert_throw(unit->getType().isRefinery());
				Me().AddBuilding(unit);
			}
			else if (!HeHasLeft())
				if (!unit->getType().isSpecialBuilding())
				{
					Him().RemoveUnit(unit);
					Him().RemoveBuilding(unit);
					if (unit->getType().isBuilding())	Him().AddBuilding(unit);
					else								Him().AddUnit(unit);
				}
		}
	});
}


void Stone::onUnitRenegade(BWAPI::Unit unit)
{
	exceptionHandler("onUnitRenegade", 5000, [this, unit]()
	{
	///	Log << Frame() << " onUnitRenegade : " << endl;

	});
}


void Stone::onSaveGame(string gameName)
{
	exceptionHandler("onSaveGame", 5000, [this, gameName]()
	{
	///	Log << Frame() << " onSaveGame : " << endl;

		bw << "The game was saved to \"" << gameName << "\"" << endl;
	});
}


void Stone::onUnitComplete(BWAPI::Unit unit)
{
	exceptionHandler("onUnitComplete", 5000, [this, unit]()
	{
	///	Log << Frame() << " onUnitComplete : " << endl;

	});
}


void Stone::OnMineralDestroyed(BWAPI::Unit u)
{
	Mineral * m = GetMap().GetMineral(u);
	assert_throw(m);

	{	// TODO: like OnOtherBWAPIUnitDestroyed

		for (auto * harasser : Harassing::Instances())
			harasser->OnMineralDestroyed(m);

		auto Miners = Mining::Instances();	// local copy as the call to OnMineralDestroyed(m) will modify Mining::Instances()
		for (auto * miner : Miners)
			miner->OnMineralDestroyed(m);
	}

	GetMap().OnMineralDestroyed(u);
}


void Stone::OnStaticBuildingDestroyed(BWAPI::Unit u)
{
	GetMap().OnStaticBuildingDestroyed(u);
}



	
} // namespace stone


