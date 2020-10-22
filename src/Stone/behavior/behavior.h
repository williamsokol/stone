//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <BWAPI.h>
#include "../defs.h"
#include "../utils.h"


namespace stone
{

class BWAPIUnit;
class MyBWAPIUnit;
class DefaultBehavior;
class Walking;
class Mining;
class Scouting;
class Harassing;
class Fleeing;
class Chasing;
class Repairing;
class Constructing;


bool canWalkOnTile(const TilePosition & t);

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class IBehavior
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class IBehavior
{
public:
	virtual							~IBehavior()					{ ResetSubBehavior(); }

	virtual string					Name() const = 0;
	virtual string					StateName() const = 0;
	virtual string					FullName() const				{ return Name() + ": " + StateName(); }

	virtual BWAPI::Color			GetColor() const = 0;
	virtual Text::Enum				GetTextColor() const = 0;

	virtual const DefaultBehavior *	IsDefaultBehavior() const		{ return nullptr; }
	virtual DefaultBehavior *		IsDefaultBehavior()				{ return nullptr; }

	virtual const Walking *			IsWalking() const				{ return nullptr; }
	virtual Walking *				IsWalking()						{ return nullptr; }

	virtual const Mining *			IsMining() const				{ return nullptr; }
	virtual Mining *				IsMining()						{ return nullptr; }

	virtual const Scouting *		IsScouting() const				{ return nullptr; }
	virtual Scouting *				IsScouting()					{ return nullptr; }

	virtual const Harassing *		IsHarassing() const				{ return nullptr; }
	virtual Harassing *				IsHarassing()					{ return nullptr; }

	virtual const Fleeing *			IsFleeing() const				{ return nullptr; }
	virtual Fleeing *				IsFleeing()						{ return nullptr; }
	
	virtual const Chasing *			IsChasing() const				{ return nullptr; }
	virtual Chasing *				IsChasing()						{ return nullptr; }
	
	virtual const Repairing *		IsRepairing() const				{ return nullptr; }
	virtual Repairing *				IsRepairing()					{ return nullptr; }
	
	virtual const Constructing *	IsConstructing() const			{ return nullptr; }
	virtual Constructing *			IsConstructing()				{ return nullptr; }

	virtual bool					CanRepair(const MyBWAPIUnit * pTarget) const = 0;

	IBehavior *						GetSubBehavior() const			{ return m_pSubBehavior.get(); }

	void							OnOtherBWAPIUnitDestroyed(BWAPIUnit * other)	{ if (m_pSubBehavior) m_pSubBehavior->OnOtherBWAPIUnitDestroyed(other); OnOtherBWAPIUnitDestroyed_v(other); }

	MyBWAPIUnit *					Agent() const	{ return m_pAgent; }

	void							OnFrame()						{ if (m_pSubBehavior) m_pSubBehavior->OnFrame(); OnFrame_v(); }
	void							OnAgentBeingDestroyed()			{ if (m_pSubBehavior) m_pSubBehavior->OnAgentBeingDestroyed(); m_agentBeingDestroyed = true; }

	IBehavior &						operator=(const IBehavior &) = delete;

protected:
									IBehavior(MyBWAPIUnit * pAgent);

	bool							JustArrivedInState() const		{ bool result = m_justArrivedInState; m_justArrivedInState = false; return result; }
	void							OnStateChanged() const			{ m_justArrivedInState = true; }

	template<class TBehavior, class... Args>
	void							SetSubBehavior(Args&&... args)	{ assert_throw(!m_pSubBehavior); m_pSubBehavior = make_unique<TBehavior>(std::forward<Args>(args)...); }

	void							ResetSubBehavior();

	bool							AgentBeingDestroyed() const		{ return m_agentBeingDestroyed; }

	virtual void					OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * )	{}

private:
	virtual void					OnFrame_v() = 0;

	unique_ptr<IBehavior>			m_pSubBehavior;
	MyBWAPIUnit *					m_pAgent;
	mutable bool					m_justArrivedInState;
	bool							m_agentBeingDestroyed = false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Behavior
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

template<class TAgent>
class Behavior : public IBehavior
{
public:
	TAgent *						Agent() const	{ return static_cast<TAgent *>(IBehavior::Agent()); }

protected:
									Behavior(TAgent * pAgent) : IBehavior(pAgent) {}


private:
};





} // namespace stone


#endif

