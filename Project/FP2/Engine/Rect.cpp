#include "Rect.h"

Rect::Rect() {

}

Rect::Rect(Vec2D _TL, Vec2D _BR) {
	TL = _TL; BR = _BR;
}

Rect::Rect(Vec2D _TL, Vec2D _BR, Color col) {
	TL = _TL; BR = _BR;
	c = col;
}

Rect::Rect(int a, int b, int c, int d) {
	TL = Vec2D(a, b); BR = Vec2D(c, d);
}

Rect::~Rect() {
}

void Rect::Draw(Graphics & gfx, bool backg) {
	for (int i = (int)round(TL.x); i < (int)round(BR.x); i++) {
		gfx.PutPixel(i, (int)round(TL.y), c, backg);
		gfx.PutPixel(i, (int)round(BR.y), c, backg);
	}
	for (int i = (int)round(TL.y); i < (int)round(BR.y); i++) {
		gfx.PutPixel((int)round(TL.x), i, c, backg);
		gfx.PutPixel((int)round(BR.x), i, c, backg);
	}
}

bool Rect::TestCol(Vec2D m) {
	return m.x > TL.x && m.x<BR.x
		&& m.y>TL.y && m.y < BR.y;
}
