//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef HIS_H
#define HIS_H

#include <BWAPI.h>
#include "bwapiUnits.h"
#include <memory>


namespace stone
{



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class HisUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class HisUnit : public HisBWAPIUnit
{
public:
									HisUnit(BWAPI::Unit u) : HisBWAPIUnit(u) {}

	const HisUnit *					IsHisUnit() const override			{ return this; }
	HisUnit *						IsHisUnit() override				{ return this; }

	MyUnit *						PursuingTarget() const				{ return m_pPursuingTarget; }
	frame_t							PursuingTargetLastFrame() const		{ return m_pPursuingTargetLastFrame; }
	void							SetPursuingTarget(MyUnit * pAgent);

private:
	MyUnit *						m_pPursuingTarget = nullptr;
	frame_t							m_pPursuingTargetLastFrame = 0;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class HisBuilding
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class HisBuilding : public HisBWAPIUnit
{
public:
	using BWAPIUnit::JustLifted;
	using BWAPIUnit::JustLanded;

									HisBuilding(BWAPI::Unit u);
									~HisBuilding();

	virtual void					Update();

	const HisBuilding *				IsHisBuilding() const override	{ return this; }
	HisBuilding *					IsHisBuilding() override		{ return this; }

	TilePosition					Size() const					{ return m_size; }

	bool							InFog() const					{ return m_inFog; }
	void							SetInFog();

	frame_t							LastFrameNoVisibleTile() const	{ assert_throw(m_inFog); return m_lastFrameNoVisibleTile; }

private:
	bool							AtLeastOneTileIsVisible() const;

	const TilePosition				m_size;
	frame_t							m_lastFrameNoVisibleTile;
	bool							m_inFog = false;
};



} // namespace stone


#endif

