//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef MY_H
#define MY_H

#include <BWAPI.h>
#include "bwapiUnits.h"
#include "faceOff.h"
#include "../defs.h"
#include <memory>


namespace stone
{



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MyUnit
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class MyUnit : public MyBWAPIUnit
{
public:
	static unique_ptr<MyUnit>		Make(BWAPI::Unit u);

	virtual void					Update();

	const MyUnit *					IsMyUnit() const override			{ return this; }
	MyUnit *						IsMyUnit() override					{ return this; }

	const vector<FaceOff> &			FaceOffs() const					{ return m_FaceOffs; }

	void							OnDangerHere();
	frame_t							LastDangerReport() const			{ return m_lastDangerReport; }

protected:
									MyUnit(BWAPI::Unit u, unique_ptr<IBehavior> pBehavior);

private:
	vector<FaceOff>					m_FaceOffs;
	frame_t							m_lastDangerReport = 0;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MyBuilding
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class MyBuilding : public MyBWAPIUnit
{
public:
	static unique_ptr<MyBuilding>	Make(BWAPI::Unit u);

	using BWAPIUnit::JustLifted;
	using BWAPIUnit::JustLanded;

									~MyBuilding();

	virtual void					Update();

	const MyBuilding *				IsMyBuilding() const override			{ return this; }
	MyBuilding *					IsMyBuilding() override					{ return this; }

	TilePosition					Size() const							{ return m_size; }

	void							Train(BWAPI::UnitType type, bool noCheck = false);
	void							Lift(bool noCheck = false);
	bool							Land(TilePosition location, bool noCheck = false);

protected:
									MyBuilding(BWAPI::Unit u, unique_ptr<IBehavior> pBehavior);

private:
	const TilePosition				m_size;
};




template<tid_t> class My;



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class My<Terran_Command_Center>
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

template<>
class My<Terran_Command_Center> : public MyBuilding
{
public:
							My(BWAPI::Unit u);

private:
	void					DefaultBehaviorOnFrame() override;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class My<Terran_Supply_Depot>
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

template<>
class My<Terran_Supply_Depot> : public MyBuilding
{
public:
							My(BWAPI::Unit u);

private:
};



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class My<Terran_SCV>
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

template<>
class My<Terran_SCV> : public MyUnit
{
public:
							My(BWAPI::Unit u);

	void					Gather(Mineral * m, bool noCheck = false);
	void					ReturnCargo(bool noCheck = false);
	void					Repair(MyBWAPIUnit * target, bool noCheck = false);
	bool					Build(BWAPI::UnitType type, TilePosition location, bool noCheck = false);

private:
	void					DefaultBehaviorOnFrame() override;
};




} // namespace stone


#endif

