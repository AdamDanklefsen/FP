#include "Pipe.h"


Pipe::Pipe(State & S1, State & S2) {
	in.push_back(S1);
	out.push_back(S2);
}
Pipe::~Pipe() {}

void Pipe::Draw(Graphics & gfx, bool backg) {
	auto a = Vec2D(in.at(0).scrcoor.x, in.at(0).scrcoor.y);
	auto b = Vec2D(out.at(0).scrcoor.x, out.at(0).scrcoor.y);
	gfx.DrawLine(a, b, Colors::White, 5, backg);
}

double Pipe::ffactor(double a, double b, double guess) {
	long double f = guess; double h = .0001; // floating point tolerance
	long double x, xp;
	if (a < 2300) { return 64 / a; }
	do {
		x = 1.0 / sqrt(f) + 2.0*log10(b / 3.7 + 2.51 / a / sqrt(f));
		xp = (sqrt(f)*(-.5*a*b - 4.03329) - 4.6435) / (a*b*f*f + 9.287*f*sqrt(f));
		f -= x / xp;
		assert(f > 0); // Bad guess
	} while (abs(x) > h);
	return f;
}
// deltaH, Total Length, Flow Rate, Guess
double Pipe::DfromHL(double h, double L, double Q, double guess) {
	long double D = guess;
	long double x, xp; // def hl(D)==0
	long double C = Q * Q * 8 / pi / pi, a, ap, bp;
	long double aD = (rho*Q * 4) / (mu*pi); // def a*D, use aD/D to get a -> Re
	long double apDD = -aD; // def a'*D^2, use apDD/D/D to get a'

	do {
		a = aD / D; ap = -aD / D / D; bp = -e / D / D;

		long double ff = ffactor(a, e / D, .02);
		x = h - ff * L / pow(D, 5.0)*C;
		long double gdv = 2 / log(10) * (sqrt(ff)*a*a*bp - 9.287*ap) / (a*(sqrt(ff)*a*(e / D) + 9.287)); //gradient dot d(a,b)/dD
		xp = C * pow(D, -5.0)*L*(gdv - 5.0*ff / D);
		long double m = x / xp;

		assert(D > 0 && D < 10); //Bad Guess
		D -= x / xp;

	} while (abs(x) > .01);

	return D;
}

double Pipe::DfromHL_v(double h, double L, double Q, double guess) {
	long double D = guess;
	long double x, xp; // def hl(D)==0
	long double C = Q * Q * 8.0 / pi / pi, a, ap, bp;
	long double aD = (rho*Q * 4.0) / (mu*pi); // def a*D, use aD/D to get a -> Re
	long double apDD = -aD; // def a'*D^2, use apDD/D/D to get a'

	do {
		a = aD / D; ap = -aD / D / D; bp = -e / D / D;

		long double ff = ffactor(a, e / D, .02), v = 4.0 * Q / pi / D / D;
		x = h - ff * L / pow(D, 5.0)*C + .5*v*v;
		long double vp = -12.0*Q / pi / D / D / D;
		long double gdv = 2 / log(10) * (sqrt(ff)*a*a*bp - 9.287*ap) / (a*(sqrt(ff)*a*(e / D) + 9.287)); //gradient dot d(a,b)/dD
		xp = C * pow(D, -5.0)*L*(gdv - 5.0*ff / D) + v * vp;
		long double m = x / xp;

		assert(D > 0 && D < 10); //Bad Guess
		D -= x / xp;

	} while (abs(x) > .01);

	return D;
}

double Pipe::HL(double L, double Q, double D) {
	double ff = ffactor(rho*Q * 4 / mu / pi / D, e / D, .02);
	return ff * L / pow(D, 5.0)*Q*Q * 8 / pi / pi;
}

double Pipe::Pvmid(State &s, double h, double L, double Q, double D) {
	return(rho*(-s.Head() + h - HL(L, Q, D)));
}
// Fitting type, fitting param, Flow Rate, Controlling Diameter
double Pipe::mHLK(mHLKParam p) {
	double K;
	switch (p.f) {
	case Elbow90:
		K = .3f;
		return K;
	case Elbow45:
		K = .16f;
		return K;
	case Throttle:
		K = p.p / p.D; //Regressed from table
		K = 282.25*pow(K, 4.f) - 838.43*pow(K, 3.f) + 929.22*K*K - 462.3*K + 89.438;
		return K;
	case Compression:
		K = .42*(1 - p.p * p.p / p.D / p.D); // v in smaller pipe
		return K;
	case Expansion:
		K = (1 - p.p * p.p / p.D / p.D)*(1 - p.p * p.p / p.D / p.D); // v in smaller pipe
		return K;
	case BranchTh:
		K = .4;
		return K;
	case BranchOut:
		K = 1.5;
		return K;
	default:
		throw std::invalid_argument("Invalid Fitting");
	}
}
double Pipe::HLmin(double L, double Q, mHLKParam p, double D) {
	double K = 0;
		K = mHLK(p);
	double v = 4 * Q / pi / D / D;
	return(K * v * v / 2);
}
// Total Length in Seg. FlowRate in Seg. minorHLParam, D
double Pipe::HLtot(double L, double Q, std::vector<mHLKParam> p, double D) {
	double K = 0;
	for (int i = 0; i < p.size(); i++) {
		K += mHLK(p.at(i));
	}
	K += ffactor(rho*Q*4.f/D/pi/mu, e/D, .02) * L/D;
	double v = 4 * Q / pi / D / D;
	return(K * v * v / 2);
}
double Pipe::findThrot(double res, double L, double Q, double p, double D) {
	long double x, xp, h = .0001; double v = 4 * Q / pi / D / D;
	double z = p;
	do {
		x = (282.25*pow(z, 4.f) - 838.43*pow(z, 3.f) + 929.22*z*z - 462.3*z + 89.438)*v*v / 2 - res;
		xp = (1129.0*z*z*z - 2515.29*z*z+ 1858.44*z - 462.3)*v*v / 2;
		z -= x / xp;
		assert(z > 0); // Use another Throttle
	} while (abs(x) > h);
	return z;
}

std::string Pipe::getRes(State & s) {
	std::string out, tabn("     "), tab("      ");
	std::string H = std::to_string(round(s.Headv()*1000.0) / 1000.0).substr(0, 6);
	std::string z = std::to_string(s.Getz());
	int b = 0, a = tabn.size() - s.Getname().size(); if (a == 0) { b = 1; }
	out = tabn.substr(0, a / 2) + s.Getname() + tabn.substr(0, a + 2 + b) + H + tab + z;
	return out;
}

std::string Pipe::getRes2(Pipe & p) {
	std::string out, tabn("         "), tab("   ");
	std::string D = std::to_string(round(p.D * 12 * 1000.0) / 1000.0).substr(0, 5);
	std::string L = std::to_string(round(p.L * 100.0) / 100.0).substr(0, 6);
	std::string n = p.in.at(0).Getname() + "->" + p.out.at(0).Getname();
	int b = 0, a = tabn.size() - n.size(); if (a == 0) { b = 1; }

	out = tabn.substr(0, a / 2) + n + tabn.substr(0, a + 2 + b) + D + tab + L;
	return out;
}