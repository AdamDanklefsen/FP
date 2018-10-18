#pragma once
#include "Vec2D.h"
#include "Graphics.h"

class Rect  {
public:
	Rect();
	Rect(Vec2D _TL, Vec2D _BR);
	Rect(Vec2D _TL, Vec2D _BR, Color col);
	Rect(int a, int b, int c, int d);
	~Rect();

	void Draw(Graphics &gfx, bool backg);
	bool TestCol(Vec2D m);
	Vec2D TL, BR;
	bool draw = false;
	Color c;
};