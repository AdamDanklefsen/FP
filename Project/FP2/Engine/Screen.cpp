#include "Screen.h"
#include <d2d1.h>

// TODO
// Sensitivity stuff
// Cost Analysis
// Draw Graphs 


Screen::Screen(Graphics &gfx, Mouse &m_) :
	gfx(gfx), m(m_), desn(Design_1), // Design_x is defualt design
	DyNgt(Vec2D(850, 50), Vec2D(925, 80), gfx, m_, SwitchType::DayNight),
	SorP({ 1012, 110 }, {1612, 150}, gfx, m_, SwitchType::Prop),
	SLabel({1300, 40}, {1600, 80}, gfx, m_) {

	//overhead
	ShowCursor(false);
	SorP.type = SwitchType::Prop; pTest = &SorP.toggle;
	des.push_back(Design(gfx,Design_1)); des.push_back(Design(gfx,Design_2)); 
	des.push_back(Design(gfx,Design_3));
	B.push_back(Button(Vec2D(10, 10), Vec2D(250, 100), gfx, m_));
	B.push_back(Button(Vec2D(260, 10), Vec2D(510, 100), gfx, m_));
	B.push_back(Button(Vec2D(520, 10), Vec2D(770, 100), gfx, m_));
	Title.setPos(Vec2D(1100, 40));
	std::vector<std::string> T; T.push_back("Adam Danklefsen"); T.push_back("MEE 308 Project");
	Title.setTXT(T); B.at(desn).isPressed = true;
	DyNgt.toggle = SwitchState::OFF; SorP.toggle = SwitchState::ON;

	// Calc properties and return
	for (int i = 0; i < 3; i++) {
		des.at(i).defineStates();
		des.at(i).definePipes();
		std::string s("Design " + std::to_string(i+1)); B.at(i).setTXT(s);
		des.at(i).Res_S.setPos(Vec2D(1000, 200)); // Default locations if need be 
		des.at(i).Res_P.setPos(Vec2D(1000, 200));
	}
	des.at(Design_1).Solve1();
	des.at(Design_2).Solve2();
	des.at(Design_3).Solve3();
	for (int i = 0; i < 3; i++) {
		des.at(i).getRes();
		Totalhl.push_back(des.at(i).Totalhl);
		Win.push_back(des.at(i).wp*0.7457);
		pCost.push_back(des.at(i).pCost);
		opCost.push_back(Win.at(i) * eleCost * 12); // [kW] * $/kWh * h -> $
	}
	Writeout();

	// Design_3 results
	{des.at(Design_3).Day_S.setPos({ 1000, 200 });
	des.at(Design_3).Day_P.setPos({ 1000, 200 });
	des.at(Design_3).Night_S.setPos({ 1000, 200 });
	des.at(Design_3).Night_P.setPos({ 1000, 200 });
	auto night = des.at(Design_3).Res_S.getTXT(0, 8, 9, 12); // (0,4)U(5,9) + (3,4)
	des.at(Design_3).Night_S.setTXT(night);
	auto day = des.at(Design_3).Res_S.getTXT(0, 8, 12, 19); // (0,4)U(9,15) + (3,4)
	des.at(Design_3).Day_S.setTXT(day);
	night = des.at(Design_3).Res_P.getTXT(0, 9); // (0,6) + (3,4)
	des.at(Design_3).Night_P.setTXT(night);
	day = des.at(Design_3).Res_P.getTXT(0, 3, 9, 23); // (7,19) + (3,4)
	des.at(Design_3).Day_P.setTXT(day); }


}

Screen::~Screen() {}

void Screen::DrawFrame() {
	// State HighLights
	TestCol();
	for (int i = 0; i < des.at(desn).States.size(); i++) {
		if (des.at(desn).States.at(i).isHigh) {
			des.at(desn).States.at(i).R.Draw(gfx, false);
			SLabel.setTXT(des.at(desn).States.at(i).Getname());
			SLabel.Draw(gfx,false);
		}
	}
	//ReDraw Buttons
	for (int i = 0; i < B.size(); i++) {
		B.at(i).Draw(gfx, true);
	}
	// Switch
	gfx.DrawBlank(DyNgt.R.TL, DyNgt.R.BR + Vec2D(1, 1), true);
	if (desn == Design_3) {
		auto Swtoggle = DyNgt.toggle;
		DyNgt.Press(); 
		if (DyNgt.toggle != Swtoggle) { DrawBackG = true; }
		DyNgt.Draw(gfx);
	} 
	SwitchState Swtoggle = SorP.toggle; SorP.type = SwitchType::Prop;
	SorP.Press(); gfx.DrawBlank(SorP.R.TL, SorP.R.BR + Vec2D(1, 1), true);
	if (SorP.toggle != Swtoggle) { DrawBackG = true; }
	SorP.Draw(gfx);

	// Mouse
	if (m.IsInWindow() && m.GetPosX() + 11 < gfx.ScreenWidth && m.GetPosY() + 15 < gfx.ScreenHeight) {
		m.Draw(gfx, false);
	}
}

void Screen::DrawBackGround() {
	assert(DrawBackG);
	// Structure
	gfx.DrawHLine(0, gfx.ScreenWidth, 100, Colors::White, true);
	gfx.DrawVLine(0, gfx.ScreenHeight, 961, Colors::White, true);
	// Pipes
	if (desn == Design_3) {
		switch (DyNgt.toggle) {
		case SwitchState::OFF:
			for (int i = 0; i < 6; i++) { des.at(desn).Pipes.at(i).Draw(gfx, true); }
			break;
		case SwitchState::ON:
			for (int i = 6; i < des.at(desn).Pipes.size(); i++) { des.at(desn).Pipes.at(i).Draw(gfx, true); }
			break;
		}
	}
	else {
		for (int i = 0; i < des.at(desn).Pipes.size(); i++) { des.at(desn).Pipes.at(i).Draw(gfx, true); }
	}
	
	// States
	for (int i = 0; i < des.at(desn).States.size(); i++) {
		int a = des.at(desn).States.at(i).scrcoor.x;
		int b = des.at(desn).States.at(i).scrcoor.y;
		Color c = des.at(desn).States.at(i).C;
		gfx.DrawCircle(a, b, 5, c, true);
	}
	// Buttons - needs to be after states
	for (int i = 0; i < B.size(); i++) {
		B.at(i).Draw(gfx, true);
	}

	//Text
	auto temp = des.at(desn).Res_S.getPos();
	gfx.DrawBlank(temp, Vec2D(gfx.ScreenWidth-1, gfx.ScreenHeight-1), true);
	temp = des.at(desn).Res_P.getPos();
	gfx.DrawBlank(temp, Vec2D(gfx.ScreenWidth-1, gfx.ScreenHeight-1), true);
	if (desn == Design_3) {
		if (DyNgt.toggle == SwitchState::ON) { //ON : Day,State
			if (SorP.toggle == SwitchState::ON) { des.at(desn).Day_S.Draw(gfx, true); }
			else { des.at(desn).Day_P.Draw(gfx, true); }
		}
		else { // OFF : Night,Pipe
			if (SorP.toggle == SwitchState::ON) { des.at(desn).Night_S.Draw(gfx, true); }
			else { des.at(desn).Night_P.Draw(gfx, true); }
		}
	}
	else {
		if (SorP.toggle == SwitchState::ON) { des.at(desn).Res_S.Draw(gfx, true); }
		else { des.at(desn).Res_P.Draw(gfx, true); }
	}
	Title.Draw(gfx, true);
	DrawBackG = false;
}

void Screen::TestCol() {
	Vec2D xy = Vec2D(m.GetPosX(), m.GetPosY());
	for (int i = 0; i < des.at(desn).States.size(); i++) {
		des.at(desn).States.at(i).isHigh = false;
		if (des.at(desn).States.at(i).R.TestCol(xy)) {
			des.at(desn).States.at(i).isHigh = true;
		}
	}
	for (int i = 0; i < B.size(); i++) {
		switch (B.at(i).CheckPressed()) {
		case 1: // Draw || Pressed -> Draw
			FlushButtonPress(); B.at(i).isPressed = true;
			B.at(i).Draw(gfx, true);
			break;
		case 2: // Draw && Pressed
			FlushButtonPress(); B.at(i).isPressed = true; desn = DesignNumber(i);
			B.at(i).Draw(gfx, true);
			DrawBackG = true; 
			break;
		default:// Neither
			break;
		}
	}
}

void Screen::FlushButtonPress() {
	for (int i = 0; i < B.size(); i++) {
		B.at(i).isPressed = false;
	}
}

void Screen::Writeout() {
	using namespace std;
	ofstream f("../../out.txt", fstream::out); assert(f);
	for (int i = 0; i < 3; i++) {
		f << "Design " << i + 1 << endl;
		string line = to_string(Totalhl.at(i)) + " ft^2/s^2" + to_string(Win.at(i)) + " kW" +
			to_string(pCost.at(i)) + " $" + to_string(opCost.at(i)) + " $";
		f << line << endl;
	}
}
