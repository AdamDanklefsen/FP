#pragma once
#include "Rect.h"
#include <assert.h>
#include "Graphics.h"
#include "Mouse.h"
#include "Text.h"

typedef enum SwitchState {
	ON, // Day   // State Prop
	OFF // Night // Pipe Prop
};
enum SwitchType {
	DayNight,
	Prop
};
inline SwitchState& operator!(SwitchState& s){
	s = SwitchState((int(s) + 1) % 2);
	return(s);
}

class Switch {
public:
	Switch(Vec2D _TL, Vec2D _BR, Graphics &gfx, Mouse& m, SwitchType swty);
	~Switch();
	void Draw(Graphics &gfx);
	bool Press();

	Graphics &gfx;
	Mouse &m;
	Rect R, RL, RR;
	Text TL, TR;
	int border = 2;
	SwitchState toggle = OFF;
	SwitchType type;
};

