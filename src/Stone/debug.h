//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef DEBUG_H
#define DEBUG_H

#include <BWAPI.h>
#include "BWEM/src/winutils.h"
#include "defs.h"
#include "utils.h"



namespace stone
{



template<class Code>
void exceptionHandler(const string & functionName, int delay, Code code)
{
	::unused(functionName);
	::unused(delay);

	const char * exceptionType = "?";
	const char * message = nullptr;

	try
	{
		code();
		return;
	}
	catch (const ::Exception & e)
	{
		exceptionType = "Exception";
		message = e.what();
	}
	catch (const std::exception & e)
	{
		exceptionType = "std::exception";
		message = e.what();
	}
	catch (...)
	{
		exceptionType = "unexpected exception";
	}

//2	bw << exceptionType << " in " << functionName;
//2	if (message) bw << ": " << message;
//2	bw << endl;

//2	ai()->SetDelay(delay);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class TimerStats
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class TimerStats
{
public:

	void				Starting()			{ m_Timer.Reset(); }
	void				Finishing();

	double				Time() const		{ return m_time; }
	double				Total() const		{ return m_total; }
	double				Avg() const			{ return Total() / Count(); }
	double				Max() const			{ return m_max; }
	int					Count() const		{ return m_count; }

private:
	Timer	m_Timer;
	double	m_time = 0;
	double	m_max = 0;
	double	m_total = 0;
	int		m_count = 0;
};
*/


void reportCommandError(const string & command);


const bool crop = true;
void drawLineMap(Position a, Position b, Color color, bool crop = false);
	
} // namespace stone




#endif

