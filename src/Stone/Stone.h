//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef STONE_H
#define STONE_H

#include <BWAPI.h>
#include "territory/vmap.h"
#include "units/me.h"
#include "units/him.h"
#include "units/critters.h"
#include "utils.h"
#include "debug.h"
#include "defs.h"
#include <memory>

// Remember not to use "Broodwar" in any global class constructor!


namespace stone
{

class Strategy;

class Stone : public BWAPI::AIModule
{
public:
	static Stone *		Instance()	{ return m_pInstance; }

	// Virtual functions for callbacks, leave these as they are.
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(string text);
	virtual void onReceiveText(BWAPI::Player player, string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);
	// Everything below this line is safe to modify.

									Stone();
									~Stone();

	BWEM::Map &						GetMap() const				{ return m_Map; }
	VMap &							GetVMap() const				{ return *m_pVMap.get(); }

	const class Me &				Me() const					{ return m_Me; }
	class Me &						Me()						{ return m_Me; }

	const class Him &				Him() const					{ return m_Him; }
	class Him &						Him()						{ return m_Him; }

	const class Critters &			Critters() const			{ return m_Critters; }
	class Critters &				Critters()					{ return m_Critters; }

	Strategy *						GetStrategy() const			{ return m_pStrategy.get(); }

	frame_t							Frame() const				{ return m_frame; }
	gameTime_t						Time() const				{ return m_time; }
	delay_t							Delay() const				{ return m_delay; }
	void							SetDelay(delay_t delay);

//4	const map<string, TimerStats> &	OnFrameTimes() const	{ return m_OnFrameTimes; }


	Stone &							operator=(const Stone &) = delete;
private:

	void							OnMineralDestroyed(BWAPI::Unit u);
	void							OnStaticBuildingDestroyed(BWAPI::Unit u);
	void							Update();
	void							RunBehaviors();
	bool							HeHasLeft() const					{ return m_heHasLeft; }

	BWEM::Map &				m_Map;
	unique_ptr<VMap>		m_pVMap;
	class Me				m_Me;
	class Him				m_Him;
	class Critters			m_Critters;
	unique_ptr<Strategy>	m_pStrategy;

	frame_t					m_frame = 0;
	gameTime_t				m_time = 0;
	delay_t					m_delay;
	bool					m_heHasLeft = false;

//4	map<string, TimerStats>	m_OnFrameTimes;

	static Stone *			m_pInstance;
};


inline Stone * ai() { return Stone::Instance(); }

	
} // namespace stone




#endif

