//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "faceOff.h"
#include "my.h"
#include "his.h"
#include "../Stone.h"

namespace { auto & bw = Broodwar; }

namespace stone
{


void quickSimulation(const vector<MyUnit *> & MyUnits, const vector<FaceOff::His_t *> & HisUnits, frame_t & framesToKillThem, frame_t & framesToKillUs)
{
	assert_throw(MyUnits.size() >= HisUnits.size());

	framesToKillThem = 0;
	framesToKillUs = 0;

	double hisRate = 0;
	double myRate = 0;

	for (int i = 0 ; i < (int)MyUnits.size() ; ++i)
	{
		MyUnit * mine = MyUnits[i];
		FaceOff::His_t * his = HisUnits[i % HisUnits.size()];

		FaceOff faceOff(mine, his);

		myRate += 1.0 / faceOff.FramesToKillHim();
		if (i < (int)HisUnits.size()) hisRate += 1.0 / faceOff.FramesToKillMe();
	}

	framesToKillThem = frame_t(0.5 + 1.0 / myRate);
	framesToKillUs = frame_t(0.5 + 1.0 / hisRate);
}



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class FaceOff
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


FaceOff::FaceOff(MyUnit * mine, His_t * his)
	:m_mine(mine), m_his(his),
	m_distance(mine->Pos().getApproxDistance(his->Pos())),
	m_myAttack(GetAttack(mine, his)),
	m_hisAttack(GetAttack(his, mine)),
	m_myRange(GetRange(mine, his)),
	m_hisRange(GetRange(his, mine)),
	m_distanceToMyRange(m_distance - m_myRange),
	m_distanceToHisRange(m_distance - m_hisRange),
	m_framesToKillMe(GetKillTime(his, mine, m_hisAttack, m_distanceToHisRange)),
	m_framesToKillHim(GetKillTime(mine, his, m_myAttack, m_distanceToMyRange))
{
}


int FaceOff::GetAttack(const BWAPIUnit * attacker, const BWAPIUnit * defender) const
{
	if (!defender->Unit()->isVisible()) return 0;

	int damage = defender->Flying() ? attacker->AirAttack() : attacker->GroundAttack();
	damage -= defender->Armor();
	if (damage <= 0) return 0;

	const auto damageType = defender->Flying() ? attacker->Type().airWeapon().damageType() : attacker->Type().groundWeapon().damageType();
	const auto defenderSize = defender->Type().size();

	if (damageType == DamageTypes::Concussive) // 100% to small units, 50% damage to medium units, and 25% damage to large units
	{
		if		(defenderSize == UnitSizeTypes::Medium) damage /= 2;
		else if (defenderSize == UnitSizeTypes::Large) damage /= 4;
	}
	else if (damageType == DamageTypes::Explosive) // 100% to large units, 75% to medium units, and 50% to small units
	{
		if		(defenderSize == UnitSizeTypes::Medium) damage = damage*3/4;
		else if (defenderSize == UnitSizeTypes::Small) damage /= 2;
	}

	return damage;
}


int FaceOff::GetRange(const BWAPIUnit * attacker, const BWAPIUnit * defender) const
{
	return defender->Flying() ? attacker->AirRange() : attacker->GroundRange();
}


frame_t FaceOff::GetKillTime(const BWAPIUnit * attacker, const BWAPIUnit * defender, int damagePerAttack, int distanceToAttackerRange) const
{
	if (damagePerAttack == 0) return 1000000;

	const int regenTime = defender->RegenerationTime();
	
	frame_t frames = 0;	// avoid warning
	int maxLoop = 2;
	for (int life = defender->LifeWithShields() ; maxLoop ; life += frames / regenTime, --maxLoop)
	{
		int attackCount_minus1 = (life-1) / damagePerAttack;
		frames = attackCount_minus1 * attacker->AvgCoolDown();
		
		if (!frames || !regenTime) break;
	}

	return max(attacker->CoolDown(), distanceToAttackerRange/8) + frames;
}

	
} // namespace stone



