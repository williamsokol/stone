//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "varea.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }

namespace stone
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class VArea
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


VArea * VArea::Get(const BWEM::Area * pBWEMPart)
{
	return static_cast<VArea *>(pBWEMPart->Ext());
}


VArea::VArea(const BWEM::Area * pBWEMPart)
	: m_pBWEMPart(pBWEMPart)
{
	assert_throw(!VArea::Get(m_pBWEMPart));
	m_pBWEMPart->SetExt(this);

}


VArea::~VArea()
{
	m_pBWEMPart->SetExt(nullptr);

}

	
} // namespace stone



