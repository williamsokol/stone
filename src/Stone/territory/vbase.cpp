//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "vbase.h"
#include "../behavior/behavior.h"
#include "../behavior/mining.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }

namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VBase
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


VBase * VBase::Get(const BWEM::Base * pBWEMPart)
{
	return static_cast<VBase *>(pBWEMPart->Ext());
}


VBase::VBase(const BWEM::Base * pBWEMPart)
	: m_pBWEMPart(pBWEMPart)
{
	assert_throw(pBWEMPart);
	assert_throw(!VBase::Get(m_pBWEMPart));
	m_pBWEMPart->SetExt(this);

}


VBase::~VBase()
{
	m_pBWEMPart->SetExt(nullptr);

}


void VBase::SetCC(My<Terran_Command_Center> * pCC)
{
	assert_throw(!pCC != !m_pCC);
	
	m_pCC = pCC;
}


void VBase::OnUnitIn(MyUnit * u)
{
	assert_throw(u);
	assert_throw(GetStronghold());
	assert_throw(u->GetStronghold() == GetStronghold());

	if (Mining * pMiner = u->GetBehavior()->IsMining()) push_back_assert_does_not_contain(m_Miners, pMiner);
}


void VBase::OnBuildingIn(MyBuilding * b)
{
	assert_throw(b);
	assert_throw(GetStronghold());
	assert_throw(b->GetStronghold() == GetStronghold());
	
	if (auto * pCC = b->Is<Terran_Command_Center>())
		if (b->TopLeft() == BWEMPart()->Location())
			SetCC(pCC);
}


void VBase::OnUnitOut(MyUnit * u)
{
	assert_throw(u);
	assert_throw(GetStronghold());
	assert_throw(u->GetStronghold() == GetStronghold());

	if (Mining * pMiner = u->GetBehavior()->IsMining())
	{
		auto i = find(m_Miners.begin(), m_Miners.end(), pMiner);
		assert_throw(i != m_Miners.end());
		fast_erase(m_Miners, distance(m_Miners.begin(), i));
	}
}


void VBase::OnBuildingOut(MyBuilding * b)
{
	assert_throw(b);
	assert_throw(GetStronghold());
	assert_throw(b->GetStronghold() == GetStronghold());

	if (auto * pCC = b->Is<Terran_Command_Center>())
		if (b->TopLeft() == BWEMPart()->Location())
			SetCC(nullptr);
}


void VBase::AddMiner(Mining * pMiner)
{
	assert_throw(pMiner->Agent()->GetStronghold() == GetStronghold());

	push_back_assert_does_not_contain(m_Miners, pMiner);
}


void VBase::RemoveMiner(Mining * pMiner)
{
	assert_throw(pMiner->Agent()->GetStronghold() == GetStronghold());

	auto it = find(m_Miners.begin(), m_Miners.end(), pMiner);
	assert_throw(it != m_Miners.end());
	fast_erase(m_Miners, distance(m_Miners.begin(), it));
}


Mineral * VBase::FindVisibleMineral() const
{
	for (Mineral * m : BWEMPart()->Minerals())
		if (m->Unit()->isVisible())
			return m;

	return nullptr;
}




} // namespace stone



