//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef VBASE_H
#define VBASE_H

#include <BWAPI.h>
#include "../utils.h"
#include "../defs.h"


namespace stone
{

class Stronghold;
class MyUnit;
class MyBuilding;
class Mining;

FORWARD_DECLARE_MY(Terran_Command_Center)

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VBase
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class VBase
{
public:
	static VBase *					Get(const BWEM::Base * pBWEMPart);

									VBase(const BWEM::Base * pBWEMPart);
									~VBase();

	const BWEM::Base *				BWEMPart() const				{ return m_pBWEMPart; }

	Stronghold *					GetStronghold() const			{ return m_pStronghold; }
	void							SetStronghold(Stronghold * sh)	{ assert_throw(sh && !m_pStronghold); m_pStronghold = sh; }

	const vector<Mining *> &		Miners() const					{ return m_Miners; }
	int								MaxMiners() const				{ return 2 * BWEMPart()->Minerals().size(); }
	int								LackingMiners() const			{ return MaxMiners() - Miners().size(); }
	void							AddMiner(Mining * pMiner);
	void							RemoveMiner(Mining * pMiner);

	void							OnUnitIn(MyUnit * u);
	void							OnBuildingIn(MyBuilding * b);

	void							OnUnitOut(MyUnit * u);
	void							OnBuildingOut(MyBuilding * b);

	My<Terran_Command_Center> *		GetCC() const					{ return m_pCC; }
	void							SetCC(My<Terran_Command_Center> * pCC);

	Mineral *						FindVisibleMineral() const;

	VBase &							operator=(const VBase &) = delete;

private:
	const BWEM::Base *				m_pBWEMPart;
	Stronghold *					m_pStronghold = nullptr;
	My<Terran_Command_Center> *		m_pCC = nullptr;
	vector<Mining *>				m_Miners;
};



} // namespace stone


#endif

