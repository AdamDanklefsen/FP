#pragma once
#include "State.h"
#include <string>
#include "Vec3D.h"
#include "Graphics.h"
#include <vector>
#include <assert.h>
#include <functional>

class Pipe {
public:
	Pipe(State &S1, State &S2); //Assume direstion is 1->2
	~Pipe();
	enum Fitting { // triple / means does not need additional parameters
		Elbow90, ///
		Elbow45, ///
		Throttle, 
		Compression,
		Expansion,
		BranchTh, ///
		BranchOut ///
	};
	struct mHLKParam {
		Fitting f; //Fitting Type
		double D; // Controlling Diameter
		double p;
		mHLKParam(Fitting _f, double _p = 0, double _D = 0) {
			f = _f; D = _D; p = _p;
		}
	};
	void Draw(Graphics &gfx, bool backg);
	static double ffactor(double a, double b, double guess);
	static double DfromHL(double h, double L, double Q, double guess);
	static double DfromHL_v(double h, double L, double Q, double guess); // lhs == Ha - Hb + KE Head
	static double HL(double L, double Q, double D);
	static double Pvmid(State &s, double h, double L, double Q, double D);
	static double mHLK(mHLKParam p);
	static double HLtot(double L, double Q, std::vector<mHLKParam> p, double D);

	static std::string getRes(State &s);
	static std::string getRes2(Pipe &p);
	double L, D = 0, ff = 0, v = 0, Q;
	static constexpr double e = .0018f/12.f; //ft //roughness of Steel
	static constexpr double rho = 1.94, mu = 1.791e-5;
	std::string name;
	std::vector<State> in, out; // vectors for branches, may not be needed
	private:
	static constexpr double pi = 3.14159269358979;
	bool isOpen = false, isPump = false, isWT = false; // 0,1,2 in Ctor
};