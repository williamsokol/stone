//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "map.h"
#include "mapImpl.h"
#include "bwapiExt.h"

using namespace BWAPI;
using namespace BWAPI::UnitTypes::Enum;
namespace { auto & bw = Broodwar; }

using namespace std;


namespace BWEM {

using namespace detail;
using namespace BWAPI_ext;

namespace utils {

bool seaSide(WalkPosition p, const Map * pMap)
{
	if (!pMap->GetMiniTile(p).Sea()) return false;

	for (WalkPosition delta : {WalkPosition(0, -1), WalkPosition(-1, 0), WalkPosition(+1, 0), WalkPosition(0, +1)})
		if (pMap->Valid(p + delta))
			if (!pMap->GetMiniTile(p + delta, no_check).Sea())
				return true;

	return false;
}

} // namespace utils


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Map
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

unique_ptr<Map> Map::m_gInstance = nullptr;


Map & Map::Instance()
{
	if (!m_gInstance) m_gInstance = make_unique<MapImpl>();

	return *m_gInstance.get();
}


Position Map::RandomPosition() const
{
	const auto PixelSize = Position(Size());
	return Position(rand() % PixelSize.x, rand() % PixelSize.y);
}


WalkPosition Map::Crop(const WalkPosition & p) const
{
	WalkPosition res = p;

	if		(res.x < 0)				res.x = 0;
	else if (res.x >= WalkSize().x)	res.x = WalkSize().x-1;

	if		(res.y < 0)				res.y = 0;
	else if (res.y >= WalkSize().y)	res.y = WalkSize().y-1;

	return res;
}


Position Map::Crop(const Position & p) const
{
	Position res(Crop(WalkPosition(p)));

	res.x += p.x % 8;
	res.y += p.y % 8;

	return res;
}


} // namespace BWEM



