#include "Design.h"

Design::Design(Graphics & gfx, DesignNumber n) : desn(n), gfx(gfx) {}
Design::~Design() {}
void Design::Draw() {}
void Design::AddPipe(State &S1, State &S2) {
	Pipes.push_back(Pipe(S1, S2));
}

void Design::defineStates() {
	//Reads Numerical Data from Google Earth -> Python -> CSV file
	States.resize(16);
	{
		using namespace std;
		ifstream f("../../res.csv"); assert(f);
		vector<string> line; line.resize(16);
		for (int i = 0; i < 16; i++) {
			f >> line.at(i);
			vector<string> subs;
			int pos = 0;
			for (int j = 0; j < 4; j++) {
				pos = line.at(i).find(",");
				string sub = line.at(i).substr(0, pos);
				subs.push_back(sub);
				line.at(i).erase(0, pos + 1);
			}
			States.at(i) = State(subs.at(0), Vec2D(borderx,bordery));
			States.at(i).Setr(Vec3D(subs.at(1), subs.at(2), subs.at(3)));
			if (!States.at(i).Getname().compare("KU")) {
				States.at(i).isOpen = true; States.at(i).SetP(25.0); States.at(i).C = Colors::Red;
			}
			if (!States.at(i).Getname().compare("AS") || !States.at(i).Getname().compare("STJ")) {
				States.at(i).isOpen = true; States.at(i).SetP(0.0); States.at(i).C = Colors::Red;
			}
			if (!States.at(i).Getname().compare("Inlet")) {
				States.at(i).isPump = true; States.at(i).SetP(5 * .006944); States.at(i).C = Colors::Green; // lbf/ft/ft -> psi
			}
			if (!States.at(i).Getname().compare("WT")) { States.at(i).isWT = true; States.at(i).C = Colors::Yellow; }
		}
		f.close();
	}
}
// must be called after defineStates()
State& Design::getState(std::string s) {
	for (int i = 0; i < States.size(); i++) {
		if (!States.at(i).Getname().compare(s)) {
			return(States.at(i));
		}
	}
	auto str = s;
	assert(false);
	throw std::invalid_argument("No such State exists");
}

Pipe& Design::getPipe(std::string s) {
	for (int i = 0; i < Pipes.size(); i++) {
		if (!Pipes.at(i).name.compare(s)) {
			return(Pipes.at(i));
		}
	}
	assert(false);
	throw std::invalid_argument("No such Pipe exists");
}

// Must be called after DefineStates()
void Design::definePipes() {
	int numlines[] = { 11,15,20 };
	using namespace std;
	std::string file("../../Conn"+ std::to_string(int(desn)+1) +".csv");
	ifstream f(file); assert(f);
	vector<string> line; line.resize(numlines[desn]);
	for (int i = 0; i < numlines[desn]; i++) {
		f >> line.at(i);
		vector<string> subs;
		int pos = 0;
		for (int j = 0; j < 2; j++) {
			pos = line.at(i).find(",");
			string sub = line.at(i).substr(0, pos);
			subs.push_back(sub);
			line.at(i).erase(0, pos + 1);
		}

		AddPipe(getState(subs.at(0)), getState(subs.at(1)));
		Pipes.at(i).name = std::string(subs.at(0) + "->" + subs.at(1));
		Pipes.at(i).L = (getState(subs.at(0)).Getr() - getState(subs.at(1)).Getr()).mag();
	}
}

// Solve Template
// Declare States/Pipes
// Declare L, Q, D
// Store D, v
// Declare wp (specific work of pump) at given flow rate
// Minor HL Cooef.
// Compute hl and H
// Solve for extra pressure

//Should be per design, at least for now. TODO add switch des#
void Design::Solve1() {
	//http://products.anssteel.com/viewitems/steel-pipes/sch-40-bare-black-standard-steel-pipe
	double rho = 1.94; //slugs/ft^3
	//Inlet->B2
	{
		State &Inlet = getState("Inlet"), &B2 = getState("B2");
		Pipe &InletB2 = getPipe("Inlet->B2");
		InletB2.Q = 150 * .002228; Qpump = 30 * .002228;
		InletB2.D = 3.068 / 12.0; // ft
		double D = InletB2.D; // Convinience
		double L = InletB2.L;
		B2.Setv(4*InletB2.Q/pi/D/D);
		pCost += L * Pipe::getprice(pPrice::D3);

		// Split flow into Qp and Qs || branches, Qp has pump
		// pump at halfway point, Qs has length increased to allign with flow ratio
		double Qp = 30.f * .0022288, Qs = InletB2.Q - Qp, r = Qs / Qp;
		wp = .75*550.0 / Qp / rho * .57; // ft^2/s^2 f(Qp) // Q = 30GPM
		//wp = .78*550.0 / Qp / rho * .58; // ft^2/s^2 f(Qp) // Q = 35GPM
		//wp = .80*550.0 / Qp / rho * .57; // ft^2/s^2 f(Qp) // Q = 40GPM
		// wp = HP * 550/Q/rho * eta
		double Lp = 32.f / 36.f; //ft //TODO put calc in final pres
		double Ls = r * Lp; L -= Lp; // account for differently allocated length

		double hl = 0; std::vector<Pipe::mHLKParam> FitParam;  {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchTh));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchTh));
			hl += Pipe::HLtot(Lp, Qp, FitParam, D); FitParam.clear();
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchOut));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchOut));
			hl += Pipe::HLtot(Ls, Qs, FitParam, D); FitParam.clear();
			hl += Pipe::HL(L, InletB2.Q, D); }
		double H = Inlet.Head() - B2.Head() + wp;
		double res = H - hl; B2.SetP(rho*res);
		Totalhl += hl;
	}
	//B2->B1->S1->AS
	{
		State &B2 = getState("B2"), &B1 = getState("B1"), &S1 = getState("S1"), &AS = getState("AS");
		Pipe &B2B1 = getPipe("B2->B1"), &B1S1 = getPipe("B1->S1"), &S1AS = getPipe("S1->AS");
		double L = B2B1.L + B1S1.L + S1AS.L;
		double Q = 50.0 * .002228; //ft^3/s
		double D = 2.067f / 12.f; // Chosen Value
		B2B1.D = D; B1S1.D = D; S1AS.D = D;
		AS.Setv(4*Q/pi/D/D);
		pCost += L * Pipe::getprice(pPrice::D2);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, D, getPipe("Inlet->B2").D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.1839*D, D));
		}
		double throt;
		Pipe::HLmin(L, Q, Pipe::mHLKParam(Pipe::Fitting::Throttle, .186*D, D), D);
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B2.Head() - AS.Head();
		double res = H - hl;
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		AS.SetP(rho*(H-hl)); // psig // extra head ~= 0
		Totalhl += hl;
	}
	//B2->S5->S6->B3
	{
		State &B2 = getState("B2"), &S5 = getState("S5"), &S6 = getState("S6"), &B3 = getState("B3");
		Pipe &B2S5 = getPipe("B2->S5"), &S5S6 = getPipe("S5->S6"), &S6B3 = getPipe("S6->B3");
		double L = B2S5.L + S5S6.L + S6B3.L; //ft
		double Q = 100 * .002228; //ft^3/s
		double D = 3.068f / 12.f;
		B2S5.D = D; S5S6.D = D; S6B3.D = D;
		B3.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D3);
		
		std::vector<Pipe::mHLKParam> FitParam;
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B2.Head() - B3.Head();
		double res = H - hl;
		B3.SetP(rho*res);
		Totalhl += hl;
	}
	//B3->KU
	{
		State &B3 = getState("B3"), &KU = getState("KU");
		Pipe &B3KU = getPipe("B3->KU");
		double L = B3KU.L + KU.Getz() - B3.Getz(); //ft // added in Z to take care of getting on the roof
		double Q = 80.0*.002228; //ft^3/s
		double D = 2.469f / 12.f;
		B3KU.D = D; KU.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D2p5);
		
		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.2616*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.Head() - KU.Head();
		double res = H - hl; // res*rho ~= 0
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		Totalhl += hl;
	}
	//B3->S7->S8->STJ
	{
		State &B3 = getState("B3"), &S7 = getState("S7"), &S8 = getState("S8"), &STJ = getState("STJ");
		Pipe &B3S7 = getPipe("B3->S7"), &S7S8 = getPipe("S7->S8"), &S8STJ = getPipe("S8->STJ");
		double L = B3S7.L + S7S8.L + S8STJ.L; //ft
		double Q = (20.0)*.002228; //ft^3/s
		double D = 1.610f / 12.f;
		B3S7.D = D; S7S8.D = D; S8STJ.D = D;
		STJ.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D1p5);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .1*D, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.2298*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.Head() - STJ.Head();
		double res = H - hl;
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		STJ.SetP(rho*(H - hl));
		Totalhl += hl;
	}
}
void Design::Solve2() {
	double rho = 1.94; //slugs/ft^3
	//I->WT
	{
		State &I = getState("Inlet"), &WT = getState("WT");
		Pipe &IWT = getPipe("Inlet->WT");
		double dz = I.Getz() - WT.Getz(); //ft
		double L = sqrt(IWT.L*IWT.L - dz * dz) + dz;
		double Q = 150 * .002228; Qpump = 150 * .002228;
		double D = 4.026f / 12.f; IWT.D = D;
		I.Setv(4 * Q / pi / D / D); WT.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D4);

		// Flow split into three branches with three pumps in ||
		double PumpL = 17.25, Lmid = 36.f - PumpL, Lout = Lmid + 24.f + 6.f; // in
		L = L - 3.f + (Lmid + 2 * Lout) / 12.f; // ft
		wp = 32.2f*80.f * 3; // Q = 150/3

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D)); // Lmid
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));

			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); // Lout
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));

			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); // Lout
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));

			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); }
		double hl = Pipe::HLtot(L, Q, FitParam, D) - wp;

		double H = I.Head() - WT.Head();
		double res = H - hl;
		WT.SetP(rho*(H - hl));
		Totalhl += hl;
	}
	// WT->S4->S3->S2->B1
	{
		State &WT = getState("WT"), &S4 = getState("S4"), &S3 = getState("S3"), &S2 = getState("S2"), &B1 = getState("B1");
		Pipe &WTS4 = getPipe("WT->S4"), &S4S3 = getPipe("S4->S3"), &S3S2 = getPipe("S3->S2"), &S2B1 = getPipe("S2->B1");
		double L = WTS4.L + S4S3.L + S3S2.L + S2B1.L;
		double Q = 150 * .002228;
		double D = 3.068f / 12.f; // ft
		WTS4.D = D; S4S3.D = D;  S3S2.D = D; S2B1.D = D;
		B1.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D3);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); }
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = WT.Head() - B1.Head();
		double res = H - hl;
		B1.SetP(rho*res);
		Totalhl += hl;
	}
	//B1->S1->AS
	{
		State &B1 = getState("B1"), &S1 = getState("S1"), &AS = getState("AS");
		Pipe &B1S1 = getPipe("B1->S1"), &S1AS = getPipe("S1->AS");
		double L = B1S1.L + S1AS.L;
		double Q = 50 * .002228;
		double D = 1.610f / 12.f;
		B1S1.D = D; S1AS.D = D;
		AS.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D1p5);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.24991*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B1.Head() - AS.Head();
		double res = H - hl;
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		AS.SetP(rho*res);
		Totalhl += hl;
	}
	//B1->B2->S5->S6->B3
	{
		State &B1 = getState("B1"), &B2 = getState("B2"), &S5 = getState("S5"), &S6 = getState("S6"), &B3 = getState("B3");
		Pipe &B1B2 = getPipe("B1->B2"), &B2S5 = getPipe("B2->S5"), &S5S6 = getPipe("S5->S6"), &S6B3 = getPipe("S6->B3");
		double L = B1B2.L + B2S5.L + S5S6.L + S6B3.L;
		double Q = 100 * .002228;
		double D = 2.469f / 12.f;
		B1B2.D = D; B2S5.D + D; S5S6.D = D; S6B3.D = D;
		B3.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D2p5);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchTh));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B1.Head() - B3.Head();
		double res = H - hl;
		double bleh = 0;
		B3.SetP(rho*res);
		Totalhl += hl;
	}
	//B3->KU
	{
		State &B3 = getState("B3"), &KU = getState("KU");
		Pipe &B3KU = getPipe("B3->KU");
		double L = B3KU.L + KU.Getz() - B3.Getz(); //ft // added in Z to take care of getting on the roof
		double Q = 80.0*.002228; //ft^3/s
		double D = 1.610f / 12.f;
		B3KU.D = D; KU.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D1p5);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.517394*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.Head() - KU.Head();
		double res = H - hl; // res*rho ~= 0
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		KU.SetP(KU.GetP() + rho * res);
		Totalhl += hl;
	}
	//B3->S7->S8->STJ
	{
		State &B3 = getState("B3"), &S7 = getState("S7"), &S8 = getState("S8"), &STJ = getState("STJ");
		Pipe &B3S7 = getPipe("B3->S7"), &S7S8 = getPipe("S7->S8"), &S8STJ = getPipe("S8->STJ");
		double L = B3S7.L + S7S8.L + S8STJ.L; //ft
		double Q = (20.0)*.002228; //ft^3/s
		double D = 1.380f / 12.f;
		B3S7.D = D; S7S8.D = D; S8STJ.D = D;
		STJ.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D1p25);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .1*D, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .1*D, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .1*D, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.2587*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.Head() - STJ.Head();
		double res = H - hl;
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		STJ.SetP(rho*(H - hl));
		Totalhl += hl;
	}
}
void Design::Solve3() {
	double rho = 1.94; //slugs/ft^3
	//I->B2->B1->S2->S3->S4->WT
	{
		State &I = getState("Inlet"), &B2 = getState("B2"), &B1 = getState("B1"), &S2 = getState("S2");
		State &S3 = getState("S3"), &S4 = getState("S4"), &WT = getState("WT");
		Pipe &IB2 = getPipe("Inlet->B2"), &B2B1 = getPipe("B2->B1"), &B1S2 = getPipe("B1->S2");
		Pipe &S2S3 = getPipe("S2->S3"), &S3S4 = getPipe("S3->S4"), &S4WT = getPipe("S4->WT");
		double dz = I.Getz() - WT.Getz(); //ft
		double L = IB2.L + B2B1.L + B1S2.L + S2S3.L + S3S4.L + S4WT.L;
		double Q = 150 * .002228; Qpump = 150 * .002228;
		double D = 4.026f / 12.f;
		IB2.D = D; B2B1.D = D; B1S2.D = D; S2S3.D = D; S3S4.D = D; S4WT.D = D;
		I.Setv(4 * Q / pi / D / D); WT.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D4);

		// Flow split into three branches with three pumps in ||
		double PumpL = 17.25, Lmid = 36.f - PumpL, Lout = Lmid + 24.f + 6.f; // in
		L = L - 3.f + (Lmid + 2 * Lout) / 12.f; // ft
		wp = 32.2f*80.f * 3; // Q = 150/3

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D)); // Lmid
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));

			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); // Lout
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));

			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); // Lout
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f / 12.f, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));

			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); }
		double hl = Pipe::HLtot(L, Q, FitParam, D) - wp;

		double H = I.Head() - WT.Head();
		double res = H - hl;
		WT.SetP(rho*(H - hl));
		Totalhl += hl;
	}
	// WT->S4->S3->S2->B1
	{
		State &WT = getState("WT"), &S4 = getState("S4"), &S3 = getState("S3"), &S2 = getState("S2"), &B1 = getState("B1");
		Pipe &WTS4 = getPipe("WT->S4"), &S4S3 = getPipe("S4->S3"), &S3S2 = getPipe("S3->S2"), &S2B1 = getPipe("S2->B1");
		double L = WTS4.L + S4S3.L + S3S2.L + S2B1.L;
		double Q = 150 * .002228;
		double D = 3.068f / 12.f; // ft
		WTS4.D = D; S4S3.D = D;  S3S2.D = D; S2B1.D = D;
		B1.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D3);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90)); }
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = WT.Head() - B1.Head();
		double res = H - hl;
		B1.SetP(rho*res);
		Totalhl += hl;
	}
	//B1->S1->AS
	{
		State &B1 = getState("B1"), &S1 = getState("S1"), &AS = getState("AS");
		Pipe &B1S1 = getPipe("B1->S1"), &S1AS = getPipe("S1->AS");
		double L = B1S1.L + S1AS.L;
		double Q = 50 * .002228;
		double D = 1.610f / 12.f;
		B1S1.D = D; S1AS.D = D;
		AS.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D1p5);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.33276*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B1.Head() - AS.Head();
		double res = H - hl;
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		AS.SetP(rho*res);
		Totalhl += hl;
	}
	//B1->B2->S5->S6->B3
	{
		State &B1 = getState("B1"), &B2 = getState("B2"), &S5 = getState("S5"), &S6 = getState("S6"), &B3 = getState("B3");
		Pipe &B1B2 = getPipe("B1->B2"), &B2S5 = getPipe("B2->S5"), &S5S6 = getPipe("S5->S6"), &S6B3 = getPipe("S6->B3");
		double L = B1B2.L + B2S5.L + S5S6.L + S6B3.L;
		double Q = 100 * .002228;
		double D = 2.469f / 12.f;
		B1B2.D = D; B2S5.D + D; S5S6.D = D; S6B3.D = D;
		B3.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D2p5);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchTh));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B1.Head() - B3.Head();
		double res = H - hl;
		double bleh = 0;
		B3.SetP(rho*res);
		Totalhl += hl;
	}
	//B3->KU
	{
		State &B3 = getState("B3"), &KU = getState("KU");
		Pipe &B3KU = getPipe("B3->KU");
		double L = B3KU.L + KU.Getz() - B3.Getz(); //ft // added in Z to take care of getting on the roof
		double Q = 80.0*.002228; //ft^3/s
		double D = 2.067f / 12.f;
		B3KU.D = D; KU.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D2);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.12527*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.Head() - KU.Head();
		double res = H - hl; // res*rho ~= 0
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		KU.SetP(KU.GetP() + rho * res);
		Totalhl += hl;
	}
	//B3->S7->S8->STJ
	{
		State &B3 = getState("B3"), &S7 = getState("S7"), &S8 = getState("S8"), &STJ = getState("STJ");
		Pipe &B3S7 = getPipe("B3->S7"), &S7S8 = getPipe("S7->S8"), &S8STJ = getPipe("S8->STJ");
		double L = B3S7.L + S7S8.L + S8STJ.L; //ft
		double Q = (20.0)*.002228; //ft^3/s
		double D = 1.380f / 12.f;
		B3S7.D = D; S7S8.D = D; S8STJ.D = D;
		STJ.Setv(4 * Q / pi / D / D);
		pCost += L * Pipe::getprice(pPrice::D1p25);

		std::vector<Pipe::mHLKParam> FitParam; {
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .1*D, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .1*D, D));
			FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, 0.141038*D, D));
		}
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.Head() - STJ.Head();
		double res = H - hl;
		//double z = Pipe::findThrot(res, L, Q, .2, D);
		STJ.SetP(rho*(H - hl));
		Totalhl += hl;
	}
}

void Design::getRes() {
	int range1 = States.size() + 3;
	int range2 = Pipes.size() + 3;
	// pHead and Elev.
	std::vector<std::string> out; out.resize(range1);
	for (int i = 3; i < range1; i++) { out.at(i) = Pipe::getRes(States.at(i-3)); }
	out.at(0) = std::string("State   pHead      Elev.");
	out.at(1) = std::string("       ft^2/s^2     ft");
	out.at(2) = std::string("_____  ________    _____");
	Res_S.setTXT(out);

	// Diam and Len
	std::vector<std::string> Lengths; Lengths.resize(range2);
	for (int i = 3; i < range2; i++) {
		Lengths.at(i) = Pipe::getRes2(Pipes.at(i-3));
	}
	Lengths.at(0) = std::string("  Pipe      Diam.    Len. ");
	Lengths.at(1) = std::string("             in       ft");
	Lengths.at(2) = std::string("_________  ______   ______");
	Res_P.setTXT(Lengths);
}
//Break after usage
//For Testing only
void Design::getSProp() {
	std::vector<double> prop; double max = 0;
	for (int i = 0; i < States.size(); i++) {
		prop.push_back(States.at(i).Getr().x);
		if (abs(States.at(i).Getr().x) > max) {
			max = abs(States.at(i).Getr().x);
		}
	}

	std::vector<double> prop2; double max2 = 0;
	for (int i = 0; i < States.size(); i++) {
		prop2.push_back(States.at(i).Getr().y);
		if (abs(States.at(i).Getr().y) > max2) {
			max2 = abs(States.at(i).Getr().y);
		}
	}
}
