#include "Text.h"


Text::Text(std::string s) {
	line.resize(1);
	for (int i = 0; i < s.length(); i++) {
		line.at(1).push_back(Letter(s.at(i)));
	}
}

Text::Text(std::vector<std::string> s) {
	line.resize(s.size());
	for (int l = 0; l < s.size(); l++) {
		for (int i = 0; i < s.at(l).length(); i++) {
			line.at(1).push_back(Letter(s.at(l).at(i)));
		}
	}
}

Text::Text(Vec2D p, std::string s) : Text(s) { pos = p; }
Text::Text(int x, int y, std::string s) : Text(Vec2D(x,y), s  ){ }
Text::Text(){}
Text::~Text(){}

void Text::out() {
	char debug[2];
	for (int i = 0; i < line.size(); i++) {
		//sprintf_s(debug, "%c", line.at(i).let);
		OutputDebugStringA(debug);
	}
	sprintf_s(debug, "\n");
	OutputDebugStringA(debug);
}

void Text::Draw(Graphics & gfx, bool backg) {
	for (int i = 0; i < line.size(); i++) {
		assert(line.at(i).size()*13+pos.x < gfx.ScreenWidth);
		for (int cnum = 0; cnum < line.at(i).size(); cnum++) { // Loop through letters
			auto a = pos + Vec2D(0, 20 * i);
			line.at(i).at(cnum).Draw(gfx, pos + Vec2D(0, 20*i), cnum, backg); // Displays each letter
		}
	}
}

void Text::setPos(Vec2D p) { pos = p; }

Vec2D Text::getPos() {
	return pos;
}
Vec2D Text::getSize() {
	int max = 0;
	for (int i = 0; i < line.size(); i++) {
		if (line.at(i).size() > max) { max = line.at(i).size(); }
	}
	return Vec2D(13 * max, 20 * line.size());
}
Text::vvLetter Text::getTXT() { return line; }

// Returns TXT lines in range [a,b)
Text::vString Text::getTXT(int a, int b) {
	assert(a >= 0, a < b, b < line.size());
	Text::vString ret;
	for (int i = a; i < b; i++) {
		ret.push_back(vLetvStr(i));
	} return ret;
}
// getTXT() + getTXT
Text::vString Text::getTXT(int a, int b, int c, int d) {
	assert(a >= 0, a < b, b < line.size());
	Text::vString ret;
	for (int i = a; i < b; i++) {
		ret.push_back(vLetvStr(i));
	}
	assert(c >= 0, c < d, d < line.size());
	for (int i = c; i < d; i++) {
		ret.push_back(vLetvStr(i));
	} return ret;
}

// Converts vLetter to vString at line(a)
std::string Text::vLetvStr(int a) {
	std::string ret; assert(a < line.size());
	for (int i = 0; i < line.at(a).size(); i++) {
		ret.push_back(line.at(a).at(i).let);
	}
	return ret;
}

void Text::setTXT(std::string s) {
	line.resize(1); line.at(0).clear();
	for (int i = 0; i < s.length(); i++) {
		Letter a = Letter(s.at(i));
		line.at(0).push_back(a);
	}
}

void Text::setTXT(std::vector<std::string> s) {
	line.clear(); line.resize(s.size());
	for (int l = 0; l < s.size(); l++) {
		for (int i = 0; i < s.at(l).length(); i++) {
			line.at(l).push_back(Letter(s.at(l).at(i)));
		}
	}
}

int Text::getlen() {
	return line.size();
}

Text::Letter::Letter(char a) {
	let = a;
	x = 12 * (let - 32) % 384;
	y = 16 * (let / 32 - 1);
}

Text::Letter::~Letter() {
}

void Text::Letter::Draw(Graphics & gfx, Vec2D pos, int cnum, bool backg) {
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 16; j++) {
			int bleh = x + 384 * y;
			int a = 13 * cnum + pos.x + i;
			int b = pos.y + j;
			if (Text::sfont[x+i+384*(y+j)]) {
				gfx.PutPixel(a, b, Colors::White, backg);
			}
		}
	}
}