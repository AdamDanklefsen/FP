#include "State.h"


State::State() {}
State::State(std::string s, Vec2D b) {
	name = s;
	border = b;
}


State::~State() {}

double State::GetP() { return P; }
double State::Getrho() { return rho; }
double State::Getv() { return v; }
int    State::Getz() { return z; }
double State::Getmu() { return mu; }
Vec3D State::Getr() { return r; }
std::string State::Getname() { return name; }

double State::Head() {
	double Hp = P / rho;
	double Hv = v*v / 2;
	double Hz = 32.1*z;
	return Hp + Hv + Hz;
}

double State::Headv() {
	double Hp = P / rho;
	double Hz = 32.1*z;
	return Hp + Hz;
}

void State::SetP(double _P) { P = _P; }
void State::Setrho(double _rho) { rho = _rho; }
void State::Setv(double _v) { v = _v; }
void State::Setz(int _z) { z = _z; }
void State::Setmu(double _mu) { mu = _mu; }
void State::Setr(Vec3D &_r) {
	r = _r; z = _r.z;
	int a = framey - ((r.x / x + .5) * (framey - border.y) + border.y/2) + 25;
	int b = (r.y / y + .5) * (framey - border.y) + border.y/2 + 35;
	scrcoor = Vec2D(a,b);
	R = Rect(Vec2D(a - 10, b - 10), Vec2D(a + 10, b + 10), Colors::Yellow);
}