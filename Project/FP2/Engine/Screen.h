#pragma once
#include "Graphics.h"
#include "Rect.h"
#include "Design.h"
#include "Button.h"
#include "Mouse.h"
#include "Vec2D.h"
#include "Text.h"
#include "Switch.h"
#include <fstream>

class Screen {
public:
	Screen(Graphics &gfx, Mouse &m_);
	~Screen();
	void DrawFrame();
	void DrawBackGround();
	void TestCol();
	void FlushButtonPress();
	static void Test(){}
	void Writeout();

	DesignNumber desn;
	std::vector<Design> des;
	std::vector<Button> B;
	Button SLabel;
	bool DrawBackG = true;

	std::vector<double> Totalhl, Win, pCost, opCost;
	double eleCost = 1.96/100; // $/kWh 
	SwitchState* pTest;

private:
	Graphics &gfx;
	Mouse &m;
	Text Title;
	Switch DyNgt, SorP;
};

