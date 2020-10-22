//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef BWEM_EXAMPLES_H
#define BWEM_EXAMPLES_H

#include <BWAPI.h>


namespace BWEM
{

class Map;

namespace utils
{

void drawMap(const Map & theMap);




// Prints information about theMap onto the game screen.
// The printed information is highly customizable (Cf. mapDrawer.cpp).
void pathExample(const Map & theMap);




}} // namespace BWEM::utils


#endif

