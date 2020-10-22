//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "vect.h"



Vect rot(const Vect & a, double angle)
{
	const double cosAngle = cos(angle);
	const double sinAngle = sin(angle);
	return Vect(a.x*cosAngle - a.y*sinAngle, a.x*sinAngle + a.y*cosAngle);
}


double angle(const Vect & from, const Vect & to)
{
	return atan2(to.y, to.x) - atan2(from.y, from.x);
}