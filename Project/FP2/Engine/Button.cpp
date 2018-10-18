#include "Button.h"

Button::Button(Vec2D _TL, Vec2D _BR, Graphics &gfx, Mouse& m) :
	R(_TL, _BR),
	gfx(gfx), m(m) {
}
Button::~Button(){}
void Button::Press(Button * b, void(Button::* funcp)(void)) {
	(b->*funcp)();
}
void Button::Draw(Graphics &gfx) {
	if (changeDraw) {
		R.Draw(gfx, false);
		changeDraw = false;
	}
	else {
		R.Draw(gfx, true);
	}
	if(txt.getlen()>0) {
		txt.Draw(gfx, true);
	}
}
void Button::disp() {
	OutputDebugStringA("Test\n");
}

int Button::CheckPressed() {
	Vec2D xy = Vec2D(m.GetPosX(), m.GetPosY());
	if (R.TestCol(xy)) {
		if (m.LeftIsPressed()) {
			isPressed = true;
			changeDraw = true;
			R.c = Colors::Gray;
		}
		if (!isPressed) {
			isHigh = true; R.c = Colors::Yellow; changeDraw = true;
		}
	}
	else if(isPressed) {
		R.c = Colors::Gray;
	}
	else {
		isHigh = false; R.c = Colors::White;
	}
	return changeDraw+isPressed;
}
void Button::setTXT(std::string s) {
	clrTXT();
	txt.setTXT(s);
	int a = .5*R.TL.x + R.BR.x / 2 - s.size() * 6;
	int b = .5*R.TL.y + R.BR.y / 2 - 10;
	assert(a+s.length()*13 < gfx.ScreenWidth);
	txt.setPos(Vec2D(a,b));
}
void Button::setTXT(std::vector<std::string> s) {
	clrTXT();
	txt.setTXT(s);
	int maxlen = 0;
	for (int i = 0; i < s.size(); i++) {
		if (s.at(i).length() > maxlen) {
			maxlen = s.at(i).length();
		}
	}
	int a = .5*R.TL.x + R.BR.x / 2 - maxlen * 6;
	int b = .5*R.TL.y + R.BR.y / 2 - 10;
	assert(a + maxlen * 13 < gfx.ScreenWidth);
	txt.setPos(Vec2D(a, b));
}
void Button::clrTXT() {
	txt.setTXT("");
}

void Button::setFunc(void(*f)()) {
	fp = f;
}
