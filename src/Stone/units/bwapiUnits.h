//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef BWAPI_UNITS_H
#define BWAPI_UNITS_H

#include <BWAPI.h>
#include "../defs.h"
#include "../vect.h"
#include "../utils.h"
#include <memory>


namespace stone
{

class IBehavior;
class Stronghold;
class MyUnit;
class MyBuilding;
class HisBWAPIUnit;
class HisUnit;
class HisBuilding;
class Chasing;

template<tid_t> class My;


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class BWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class BWAPIUnit
{
public:
	enum {sizePrevLife = 64};
	enum {sizePrevPos = 64};

									BWAPIUnit(BWAPI::Unit u);
	virtual							~BWAPIUnit();

	virtual string					NameWithId() const			{ return Type().getName() + " #" + to_string(Unit()->getID()); }

	virtual void					Update();

	// Returns the BWAPI::Unit this BWAPIUnit is wrapping around.
	BWAPI::Unit						Unit() const				{ return m_bwapiUnit; }

	// Returns the BWAPI::UnitType of the BWAPI::Unit this BWAPIUnit is wrapping around.
	BWAPI::UnitType					Type() const				{ return m_bwapiType; }

	const Area *					GetArea() const				{ return m_pArea; }

	bool							Flying() const				{ return m_flying; }

	// Returns the center of this BWAPIUnit, in pixels (same as Unit()->getPosition()).
	Position						Pos() const					{ return m_PrevPos[0]; }
	Position						PrevPos(int i) const		{ assert_throw((0 <= i) && (i < sizePrevPos)); return m_PrevPos[i]; }

	const Vect &					Acceleration() const		{ return m_acceleration; }

	frame_t							LastFrameMoving() const		{ return m_lastFrameMoving; }

	// Returns the topLeft tile of this BWAPIUnit (same as Unit()->getTilePosition()).
	BWAPI::TilePosition				TopLeft() const				{ return m_topLeft; }

	int								LifeWithShields() const		{ return Life() + Shields(); }

	int								Life() const				{ return m_PrevLife[0]; }
	int								MaxLife() const				{ return m_maxLife; }
	int								PrevLife(int i) const		{ assert_throw((0 <= i) && (i < sizePrevLife)); return m_PrevLife[i]; }

	int								Shields() const				{ return m_shields; }
	int								MaxShields() const			{ return m_maxShields; }

	// Number of frames to recover 1 pt of life / shields
	// 0 means no regeneration
	frame_t							RegenerationTime() const	{ return m_regenerationTime; }

	int								Energy() const				{ return m_energy; }
	int								MaxEnergy() const			{ return m_maxEnergy; }

	int								GroundAttack() const		{ return m_groundAttack; }
	int								AirAttack() const			{ return m_airAttack; }

	int								GroundRange() const			{ return m_groundRange; }
	int								AirRange() const			{ return m_airRange; }

	int								Armor() const				{ return m_armor; }

	int								CoolDown() const			{ return m_coolDown; }
	int								AvgCoolDown() const			{ return m_avgCoolDown; }

	int								Sight() const				{ return m_sight; }

	// The Chasing units that are targeting BWAPIUnit.
	const vector<Chasing *> &		Chasers() const				{ return m_Chasers; }
	void							AddChaser(Chasing * pChaser);
	void							RemoveChaser(Chasing * pChaser);
	frame_t							FramesToBeKilledByChasers() const;


	virtual const MyUnit *			IsMyUnit() const			{ return nullptr; }
	virtual MyUnit *				IsMyUnit()					{ return nullptr; }

	virtual const MyBuilding *		IsMyBuilding() const		{ return nullptr; }
	virtual MyBuilding *			IsMyBuilding()				{ return nullptr; }

	virtual const HisBWAPIUnit *	IsHisBWAPIUnit() const		{ return nullptr; }
	virtual HisBWAPIUnit *			IsHisBWAPIUnit()			{ return nullptr; }

	virtual const HisUnit *			IsHisUnit() const			{ return nullptr; }
	virtual HisUnit *				IsHisUnit()					{ return nullptr; }

	virtual const HisBuilding *		IsHisBuilding() const		{ return nullptr; }
	virtual HisBuilding *			IsHisBuilding()				{ return nullptr; }

	BWAPIUnit &						operator=(const BWAPIUnit &) = delete;

protected:
	void							PutBuildingOnTiles();
	void							RemoveBuildingFromTiles();

	bool							JustLifted() const			{ return m_justLifted; }
	bool							JustLanded() const			{ return m_justLanded; }

private:
	const BWAPI::Unit				m_bwapiUnit;
	const BWAPI::UnitType			m_bwapiType;

	bool							m_flying;
	bool							m_justLifted;
	bool							m_justLanded;
	TilePosition					m_topLeft;
	int								m_life;
	const int						m_maxLife;
	int								m_shields;
	const int						m_maxShields;
	const frame_t					m_regenerationTime;
	int								m_energy;
	int								m_maxEnergy;
	int								m_groundAttack;
	int								m_airAttack;
	int								m_groundRange;
	int								m_airRange;
	int								m_coolDown;
	int								m_avgCoolDown;
	int								m_armor;
	int								m_sight;
	frame_t							m_lastFrameMoving;
	Vect							m_acceleration;
	const Area *					m_pArea;
	vector<Chasing *>				m_Chasers;

	int								m_PrevLife[sizePrevLife];

	Position						m_PrevPos[sizePrevPos];
};




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MyBWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class MyBWAPIUnit : public BWAPIUnit
{
public:
									MyBWAPIUnit(BWAPI::Unit u, unique_ptr<IBehavior> pBehavior);
	virtual							~MyBWAPIUnit();

	virtual void					Update();

	template<tid_t tid> My<tid> *	As() { assert_throw(Type() == tid); return Is<tid>(); }
	template<tid_t tid> My<tid> *	Is() { return static_cast<My<tid> *>(this); }

	IBehavior *						GetBehavior() const		{ return m_pBehavior.get(); }

	template<class TBehavior, class... Args>
	void							ChangeBehavior(Args&&... args)	{ m_pBehavior = make_unique<TBehavior>(std::forward<Args>(args)...); }

	virtual void					DefaultBehaviorOnFrame() {}

	void							OnOtherBWAPIUnitDestroyed(BWAPIUnit * other);

	Stronghold *					GetStronghold() const			{ return m_pStronghold; }
	void							InitializeStronghold();
	void							LeaveStronghold();

	int								DistToHisStartingBase() const		{ return m_distToHisStartingBase; }

	void							Move(Position pos, bool noCheck = false);
	void							Attack(BWAPIUnit * u, bool noCheck = false);
	void							Stop(bool noCheck = false);

private:
	virtual void					OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * )	{}
	void							SetStronghold(Stronghold * sh);

	unique_ptr<IBehavior>			m_pBehavior;
	Stronghold *					m_pStronghold = nullptr;
	int								m_distToHisStartingBase;
};




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class HisBWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class HisBWAPIUnit : public BWAPIUnit
{
public:
									HisBWAPIUnit(BWAPI::Unit u) : BWAPIUnit(u) {}

	const HisBWAPIUnit *			IsHisBWAPIUnit() const override		{ return this; }
	HisBWAPIUnit *					IsHisBWAPIUnit() override			{ return this; }

	virtual void					Update();

private:
};




//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class NeutralBWAPIUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class NeutralBWAPIUnit : public BWAPIUnit
{
public:
									NeutralBWAPIUnit(BWAPI::Unit u) : BWAPIUnit(u) {}

private:
};





} // namespace stone


#endif

