#pragma once
#include "Vec3D.h"
#include <string>
#include "Rect.h"
#include "Graphics.h"
#include <vector>

class State {
public:
	State();
	State(std::string, Vec2D b);
	~State();

	bool isOpen = false, isPump = false, isWT = false;
	bool isHigh = false; Rect R;
	Vec2D scrcoor; Vec2D border;
	Color C = Colors::Cyan;
	
	double GetP();
	double Getrho();
	double Getv();
	int Getz();
	double Getmu();
	Vec3D Getr();
	std::string Getname();
	double Head();
	double Headv();

	void SetP(double);
	void Setrho(double);
	void Setv(double);
	void Setz(int);
	void Setmu(double);
	void Setr(Vec3D&);

private:
	double P=0, rho=1.940, v=0, mu=1.791e-5; // fluid prop.
	int z = 0;
	Vec3D r;
	int framex = 1400, framey = 836;
	double x = 2050; double y = 2000;
	std::string name;
};

