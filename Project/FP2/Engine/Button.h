#pragma once
#include "Rect.h"
#include "Text.h"
#include <string>
#include <functional>
#include <assert.h>
#include "Graphics.h"
#include <algorithm>
#include "Mouse.h"

class Button : public Rect {
public:
	Button(Vec2D _TL, Vec2D _BR, Graphics &gfx, Mouse& m);
	~Button();
	bool isPressed = false;
	bool isHigh = false;
	bool changeDraw = false;

	void Press(Button* b, void(Button::*funcp)(void));
	void Draw(Graphics &gfx);
	void disp();
	int CheckPressed();
	void setTXT(std::string s);
	void setTXT(std::vector<std::string> s);
	void clrTXT();
	void setFunc(void(*f)());

private:
	Graphics &gfx;
	Mouse &m;
	Rect R;
	Text txt;
	void(*fp)();
	int border = 0;
};

