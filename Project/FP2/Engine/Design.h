#pragma once
#include <string>
#include <vector>
#include "State.h"
#include "Graphics.h"
#include "Pipe.h"
#include <fstream>
#include <stdexcept>
#include "Text.h"
#include <functional>

typedef enum DesignNumber {
	Design_1,
	Design_2,
	Design_3
};
inline DesignNumber& operator++(DesignNumber &DS, int) {
	DS = DesignNumber((int(DS) + 1) % 3);
	return(DS);
}

class Design {
public:
	Design(Graphics &gfx, DesignNumber n);
	~Design();
	void Draw();
	void AddPipe(State &S1, State &S2);
	void defineStates();
	State& getState(std::string s);
	Pipe& getPipe(std::string s);
	void definePipes();
	void Solve1();
	void Solve2();
	void getRes();
	void getSProp();

	double pi = 3.14159269358979;
	std::vector<Pipe> Pipes;
	std::vector<State> States;
	double x = 1165.561*2; double y = 1061.7255*2; // also defined in State.h
	int borderx = 75; int bordery = 50;
	Graphics &gfx;
	Text Res_S, Res_P;
	Text Day_S, Day_P, Night_S, Night_P;
	DesignNumber desn;
};

