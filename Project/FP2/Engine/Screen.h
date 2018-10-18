#pragma once
#include "Graphics.h"
#include "Rect.h"
#include "Design.h"
#include "Button.h"
#include "Mouse.h"
#include "Vec2D.h"
#include "Text.h"
#include "Switch.h"

class Screen {
public:
	Screen(Graphics &gfx, Mouse &m_);
	~Screen();
	void DrawFrame();
	void DrawBackGround();
	void TestCol();
	void FlushButtonPress();
	static void Test(){}

	DesignNumber desn;
	std::vector<Design> des;
	std::vector<Button> B;
	bool DrawBackG = true;

private:
	Graphics &gfx;
	Mouse &m;
	Text Title;
	Switch DyNgt, SorP;
};

