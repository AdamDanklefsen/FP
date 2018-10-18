#include "Switch.h"



Switch::Switch(Vec2D _TL, Vec2D _BR, Graphics &gfx, Mouse& m, SwitchType swty) :
	R(_TL, _BR),
	gfx(gfx), m(m) {

	R.c = Colors::White;
	RL = Rect(R.TL, R.BR - Vec2D((R.BR.x - R.TL.x) / 2.f, 0)); // TODO add in dx, dy for Rect
	RR = Rect(R.TL + Vec2D((R.BR.x - R.TL.x) / 2.f, 0), R.BR);
	type = swty;
	if (type == Prop) {
		TL.setTXT("State Prop."); TR.setTXT("Pipe Prop.");
		TL.setPos(RL.TL + Vec2D((RL.BR.x - RL.TL.x - TL.getSize().x) / 2.f, 10));
		TR.setPos(RR.TL + Vec2D((RR.BR.x - RR.TL.x - TR.getSize().x) / 2.f, 10));
	}
}


Switch::~Switch() {
}

void Switch::Draw(Graphics & gfx) {
	// Blank area before call
	R.Draw(gfx, true);
	Color c1=Colors::Gray, c2 = Colors::Gray;
	if (type == DayNight) {
		c1 = Color(245, 221, 11);
		c2 = Color(35, 47, 61);
	}
	switch (toggle) {
	case ON:
		for (int i = 0; i < (R.BR.x - R.TL.x) / 2.f - 2 * border; i++) {
			for (int j = 0; j < (R.BR.y - R.TL.y) - 2 * border; j++) {
				gfx.PutPixel(R.TL.x + border + i, R.TL.y + border + j, c1, true);
			}
		}
		break;
	case OFF:
		for (int i = 0; i < (R.BR.x - R.TL.x) / 2.f - 2 * border; i++) {
			for (int j = 0; j < (R.BR.y - R.TL.y) - 2 * border; j++) {
				gfx.PutPixel(R.BR.x - i - border, R.TL.y + border + j, c2, true);
			}
		}
		break;
	}
	if (type == Prop) {
		TL.Draw(gfx, false); TR.Draw(gfx, false);
	}
}

bool Switch::Press() {
	auto xy = Vec2D(m.GetPosX(), m.GetPosY());
	if (RL.TestCol(xy) && m.LeftIsPressed()) {
		toggle = ON; return true;
	} else if (RR.TestCol(xy) && m.LeftIsPressed()) {
		toggle = OFF; return true;
	}
}
