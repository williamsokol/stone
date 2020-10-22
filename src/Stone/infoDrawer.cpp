//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "infoDrawer.h"
#include "units/my.h"
#include "behavior/mining.h"
#include "Stone.h"

namespace { auto & bw = Broodwar; }




namespace stone
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class InfoDrawer
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

bool InfoDrawer::showTime						= true;
bool InfoDrawer::showTimes						= true;
bool InfoDrawer::showGame						= true;
bool InfoDrawer::showUnit						= true;
bool InfoDrawer::showBuilding					= true;
bool InfoDrawer::showCritters					= true;

const BWAPI::Color InfoDrawer::Color::time				= Colors::White;
const Text::Enum InfoDrawer::Color::timeText			= Text::White;

const BWAPI::Color InfoDrawer::Color::times				= Colors::Grey;
const Text::Enum InfoDrawer::Color::timesText			= Text::Grey;

const BWAPI::Color InfoDrawer::Color::game				= Colors::White;
const Text::Enum InfoDrawer::Color::gameText			= Text::White;

const BWAPI::Color InfoDrawer::Color::building			= Colors::Green;
const Text::Enum InfoDrawer::Color::buildingText		= Text::Green;

const BWAPI::Color InfoDrawer::Color::unit				= Colors::Green;
const Text::Enum InfoDrawer::Color::unitText			= Text::Green;

const BWAPI::Color InfoDrawer::Color::enemy				= Colors::Red;
const Text::Enum InfoDrawer::Color::enemyText			= Text::Red;

const BWAPI::Color InfoDrawer::Color::enemyInFog		= Colors::Grey;
const Text::Enum InfoDrawer::Color::enemyInFogText		= Text::Grey;

const BWAPI::Color InfoDrawer::Color::critter			= Colors::Grey;
const Text::Enum InfoDrawer::Color::critterText			= Text::Grey;


bool InfoDrawer::ProcessCommandVariants(const string & command, const string & attributName, bool & attribut)
{
	if (command == "show " + attributName) { attribut = true; return true; }
	if (command == "hide " + attributName) { attribut = false; return true; }
	if (command == attributName)		   { attribut = !attribut; return true; }
	return false;
}


bool InfoDrawer::ProcessCommand(const string & command)
{
	if (ProcessCommandVariants(command, "time", showTime))			return true;
	if (ProcessCommandVariants(command, "times", showTimes))		return true;
	if (ProcessCommandVariants(command, "game", showGame))			return true;
	if (ProcessCommandVariants(command, "unit", showUnit))			return true;
	if (ProcessCommandVariants(command, "building", showBuilding))	return true;
	if (ProcessCommandVariants(command, "critter", showCritters))	return true;
	
	if (ProcessCommandVariants(command, "all info", showTime))
	if (ProcessCommandVariants(command, "all info", showTimes))
	if (ProcessCommandVariants(command, "all info", showGame))
	if (ProcessCommandVariants(command, "all info", showUnit))
	if (ProcessCommandVariants(command, "all info", showBuilding))
	if (ProcessCommandVariants(command, "all info", showCritters))
		return true;

	return false;
}



void drawTime()
{
	int x = 3;
	int y = 0;
	bw->drawTextScreen(x, y, "%cF = %d", InfoDrawer::Color::timeText, ai()->Frame()); x += 55;
	bw->drawTextScreen(x, y, "%cT = %.2d:%.2d", InfoDrawer::Color::timeText, ai()->Time()/60, ai()->Time()%60); x += 60;
	bw->drawTextScreen(x, y, "%cFPS = %d", InfoDrawer::Color::timeText, (int)(0.5 + bw->getAverageFPS())); x += 55;
	bw->drawTextScreen(x, y, "%cdelay = %d", InfoDrawer::Color::timeText, ai()->Delay()); x += 50;
}

/* //4
void drawTimes()
{
	int x = 3;
	int y = 15;
	for (auto it : ai()->OnFrameTimes())
	{
		bw->drawTextScreen(x, y, "%c%s: avg = %.4f ; max = %.4f ; this = %.4f", InfoDrawer::Color::timesText, it.first.c_str(), it.second.Avg(), it.second.Max(), it.second.Time()); y += 10;
	}
}
*/

void drawGame()
{
	int x = 400;
	int y = 15;

	bw->drawTextScreen(x, y, "%cme:", InfoDrawer::Color::gameText); y += 10;
	bw->drawTextScreen(x, y, "%cMinerals = %d    Gas = %d    Supply = %d/%d", InfoDrawer::Color::gameText, ai()->Me().MineralsAvailable(), ai()->Me().GasAvailable(), ai()->Me().SupplyUsed(), ai()->Me().SupplyMax()); y += 10;
	bw->drawTextScreen(x, y, "%cu = %d    b = %d", InfoDrawer::Color::gameText, ai()->Me().Units().size(), ai()->Me().Buildings().size()); y += 10;

	y += 10;

	bw->drawTextScreen(x, y, "%chim:", InfoDrawer::Color::gameText); y += 10;
	bw->drawTextScreen(x, y, "%cu = %d    b = %d (%d in fog)", InfoDrawer::Color::gameText, ai()->Him().Units().size(), ai()->Him().Buildings().size(), count_if(ai()->Him().Buildings().begin(), ai()->Him().Buildings().end(), ([](const unique_ptr<HisBuilding> & up){ return up->InFog(); }))); y += 10;

	bw->drawTextScreen(x, y, "%crace = %s", InfoDrawer::Color::gameText, ai()->Him().Race().getName().c_str()); y += 10;

	if (ai()->Him().StartingBase())
		{ bw->drawTextScreen(x, y, "%cstarting Base = (%d, %d)", InfoDrawer::Color::gameText, ai()->Him().StartingBase()->BWEMPart()->Location().x, ai()->Him().StartingBase()->BWEMPart()->Location().y); y += 10; }
	else
		{ bw->drawTextScreen(x, y, "%cstarting Base = ?", InfoDrawer::Color::gameText); y += 10; }
}


void drawShieldsAndLife(const BWAPIUnit * u, int x, int y, Text::Enum colText)
{
	const char * lifeFormat = (u->Life() < u->MaxLife()) ? "%s/%s" : "%s%s";
	const char * shieldsFormat = (u->Shields() < u->MaxShields()) ? "(%s/%s)" : "(%s%s)";
	const string format = "%c" + (u->MaxShields() ? string(shieldsFormat) + lifeFormat : string("%s%s") + lifeFormat);

	const string life = to_string(u->Life());
	const string maxLife = (u->Life() < u->MaxLife()) ? to_string(u->MaxLife()) : "";
	const string shields = u->MaxShields() ? to_string(u->Shields()) : "";
	const string maxShields = (u->Shields() < u->MaxShields()) ? to_string(u->MaxShields()) : "";
		
	bw->drawTextMap(x, y, format.c_str(), colText, shields.c_str(), maxShields.c_str(), life.c_str(), maxLife.c_str());
}


void drawEnergy(const BWAPIUnit * u, int x, int y, Text::Enum colText)
{
	if (u->Energy() != u->MaxEnergy())	bw->drawTextMap(x, y, "%cenergy: %d / %d", colText, u->Energy(), u->MaxEnergy());
	else								bw->drawTextMap(x, y, "%cenergy: %d", colText, u->Energy());
}


void drawUnit()
{
	for (auto * u : ai()->Me().Units())
	{
		const Color col = u->GetBehavior()->GetColor();
		const Text::Enum colText = u->GetBehavior()->GetTextColor();

		bw->drawEllipseMap(u->Pos(), u->Type().width(), u->Type().height(), col);
		if (u->CoolDown()) bw->drawEllipseMap(u->Pos(), u->Type().width()-2, u->Type().height()-2, col);
		
		int x = u->Pos().x + u->Type().width() + 3;
		int y = u->Pos().y - u->Type().height();

		bw->drawTextMap(x, y, "%c#%d %s", colText, u->Unit()->getID(), ("<" + u->GetBehavior()->FullName() + ">" + (u->GetBehavior()->GetSubBehavior() ? " *" : "")).c_str()); y += 10;
		drawShieldsAndLife(u, x, y, colText); y += 10;
//		bw->drawTextMap(x, y, "%carmor: %d", colText, u->Armor()); y += 10;
//		bw->drawTextMap(x, y, "%cattack: %d / %d", colText, u->GroundAttack(), u->AirAttack()); y += 10;
//		bw->drawTextMap(x, y, "%crange: %d / %d", colText, u->GroundRange(), u->AirRange()); y += 10;
//		bw->drawTextMap(x, y, "%ccool: %d", colText, u->CoolDown()); y += 10;
//		bw->drawTextMap(x, y, "%cavg cool: %d", colText, u->AvgCoolDown()); y += 10;
//		bw->drawTextMap(x, y, "%csight: %d", colText, u->Sight()); y += 10;
//		if (u->MaxEnergy()) { drawEnergy(u, x, y, colText); y += 10; }
	//	bw->drawTextMap(x, y, "%cacc = %s -- %f, %f, %f", colText, my_to_string(u->Pos() - u->PrevPos(1)).c_str(), u->Unit()->getVelocityX(), u->Unit()->getVelocityY(), u->Unit()->getAngle()); y += 10;
	//	bw->drawTextMap(x, y, "%c%d", colText, distToRectangle(u->Pos(), u->GetBehavior()->IsMining()->GetMineral()->TopLeft(), u->GetBehavior()->IsMining()->GetMineral()->Size())); y += 10;

//		for (const auto & faceOff : u->FaceOffs())
//		{
//			drawLineMap(faceOff.Mine()->Pos(), faceOff.His()->Pos(), Colors::Grey);
//			bw->drawTextMap((faceOff.Mine()->Pos() + faceOff.His()->Pos())/2, "%c                  %d / %d", Text::Enum::Grey, faceOff.FramesToKillHim(), faceOff.FramesToKillMe());
//		}

	}

	for (auto & u : ai()->Him().Units())
	{
		const Color col = InfoDrawer::Color::enemy;
		const Text::Enum colText = InfoDrawer::Color::enemyText;

		bw->drawEllipseMap(u->Pos(), u->Type().width(), u->Type().height(), col);
		if (u->CoolDown()) bw->drawEllipseMap(u->Pos(), u->Type().width()-2, u->Type().height()-2, col);

		int x = u->Pos().x + u->Type().width() + 3;
		int y = u->Pos().y - u->Type().height();

		bw->drawTextMap(x, y, "%c#%d", colText, u->Unit()->getID()); y += 10;
		drawShieldsAndLife(u.get(), x, y, colText); y += 10;
		if (!u->Chasers().empty()) bw->drawTextMap(x, y, "%ckilled in: %d", colText, u->FramesToBeKilledByChasers()); y += 10;
//		bw->drawTextMap(x, y, "%carmor: %d", colText, u->Armor()); y += 10;
//		bw->drawTextMap(x, y, "%cattack: %d / %d", colText, u->GroundAttack(), u->AirAttack()); y += 10;
//		bw->drawTextMap(x, y, "%crange: %d / %d", colText, u->GroundRange(), u->AirRange()); y += 10;
//		bw->drawTextMap(x, y, "%ccool: %d", colText, u->CoolDown()); y += 10;
//		bw->drawTextMap(x, y, "%cavg cool: %d", colText, u->AvgCoolDown()); y += 10;
//		bw->drawTextMap(x, y, "%csight: %d", colText, u->Sight()); y += 10;
//		if (u->MaxEnergy()) { drawEnergy(u.get(), x, y, colText); y += 10; }
	}
}


void drawBuilding()
{
	for (auto * b : ai()->Me().Buildings())
	{
		const Color col = InfoDrawer::Color::building;
		const Text::Enum colText = InfoDrawer::Color::buildingText;

		bw->drawBoxMap(b->Pos() - Position(b->Size())/2, b->Pos() + Position(b->Size())/2, col);
		
		int x = (b->Pos() + Position(b->Size())/2).x + 3;
		int y = (b->Pos() - Position(b->Size())/2).y;

		bw->drawTextMap(x, y, "%c#%d %s", colText, b->Unit()->getID(), b->GetBehavior()->IsDefaultBehavior() ? "" : ("<" + b->GetBehavior()->FullName() + ">" + (b->GetBehavior()->GetSubBehavior() ? " *" : "")).c_str()); y += 10;
		drawShieldsAndLife(b, x, y, colText); y += 10;
//		bw->drawTextMap(x, y, "%carmor: %d", colText, b->Armor()); y += 10;
//		bw->drawTextMap(x, y, "%cattack: %d / %d", colText, b->GroundAttack(), b->AirAttack()); y += 10;
//		bw->drawTextMap(x, y, "%crange: %d / %d", colText, b->GroundRange(), b->AirRange()); y += 10;
//		bw->drawTextMap(x, y, "%csight: %d", colText, b->Sight()); y += 10;
//		if (b->MaxEnergy()) { drawEnergy(b, x, y, colText); y += 10; }
	}

	for (auto & b : ai()->Him().Buildings())
	{
		const Color col = b->InFog() ? InfoDrawer::Color::enemyInFog : InfoDrawer::Color::enemy;
		const Text::Enum colText = b->InFog() ? InfoDrawer::Color::enemyInFogText : InfoDrawer::Color::enemyText;

		bw->drawBoxMap(b->Pos() - Position(b->Size())/2, b->Pos() + Position(b->Size())/2, col);

		int x = (b->Pos() + Position(b->Size())/2).x + 3;
		int y = (b->Pos() - Position(b->Size())/2).y;

		bw->drawTextMap(x, y, "%c#%d", colText, b->Unit()->getID()); y += 10;
		drawShieldsAndLife(b.get(), x, y, colText); y += 10;
		if (!b->Chasers().empty()) bw->drawTextMap(x, y, "%ckilled in: %d", colText, b->FramesToBeKilledByChasers()); y += 10;
//		bw->drawTextMap(x, y, "%carmor: %d", colText, b->Armor()); y += 10;
//		bw->drawTextMap(x, y, "%cattack: %d / %d", colText, b->GroundAttack(), b->AirAttack()); y += 10;
//		bw->drawTextMap(x, y, "%crange: %d / %d", colText, b->GroundRange(), b->AirRange()); y += 10;
//		bw->drawTextMap(x, y, "%csight: %d", colText, b->Sight()); y += 10;
//		if (b->MaxEnergy()) { drawEnergy(b.get(), x, y, colText); y += 10; }
	}
}


void drawCritters()
{
	for (auto & u : ai()->Critters().Get())
	{
		const Color col = InfoDrawer::Color::critter;
	//	const Text::Enum colText = InfoDrawer::Color::critterText;

		bw->drawEllipseMap(u->Pos(), u->Type().width(), u->Type().height(), col);
	}
}


void drawInfos()
{
	if (InfoDrawer::showTime) drawTime();
//4	if (InfoDrawer::showTimes) drawTimes();
	if (InfoDrawer::showGame) drawGame();
	if (InfoDrawer::showUnit) drawUnit();
	if (InfoDrawer::showBuilding) drawBuilding();
	if (InfoDrawer::showCritters) drawCritters();

/*
	for (int y = 0 ; y < ai()->GetMap().Size().y ; ++y)
	for (int x = 0 ; x < ai()->GetMap().Size().x ; ++x)
	{
		TilePosition t(x, y);
		if (ai()->GetMap().GetTile(t).Ptr())
			drawDiagonalCrossMap(Position(t), Position(t + 1), Colors::White);
	}
*/

	for (int y = 0 ; y < ai()->GetMap().Size().y ; ++y)
	for (int x = 0 ; x < ai()->GetMap().Size().x ; ++x)
	{
		TilePosition t(x, y);
		if (ai()->GetVMap().ImpasseOrNearBuildingOrNeutral(ai()->GetMap().GetTile(t)))
		{
			auto col = ai()->GetVMap().Impasse(ai()->GetMap().GetTile(t)) ? Colors::White : Colors::Grey;
			drawDiagonalCrossMap(Position(t)+8, Position(t + 1)-8, col);
		}
	}

}


} // namespace stone



