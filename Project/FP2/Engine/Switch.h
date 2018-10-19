#pragma once
#include "Rect.h"
#include <assert.h>
#include "Graphics.h"
#include "Mouse.h"
#include "Text.h"

typedef enum class SwitchState {
	ON = 0, // Day   // State Prop
	OFF = 1 // Night // Pipe Prop
};
typedef enum class SwitchType {
	DayNight = 0,
	Prop = 1
};

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
	SwitchState toggle = SwitchState::OFF;
	SwitchType type = SwitchType::Prop;
};

