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
//Should be per design, at least for now. TODO add switch des#
void Design::Solve1() {
	//http://products.anssteel.com/viewitems/steel-pipes/sch-40-bare-black-standard-steel-pipe
	double rho = 1.94; //slugs/ft^3
	//Inlet->B2
	{
		State &Inlet = getState("Inlet"), &B2 = getState("B2");
		Pipe &InletB2 = getPipe("Inlet->B2");
		InletB2.Q = 150 * .002228;
		InletB2.D = 3.068 / 12.0; // ft
		double D = InletB2.D; // Convinience
		double L = InletB2.L;
		B2.Setv(4*InletB2.Q/pi/D/D);
		// Split flow into Qp and Qs || branches, Qp has pump
		// pump at halfway point, Qs has length increased to allign with flow ratio
		double Qp = 30.f * .0022288, Qs = InletB2.Q - Qp, r = Qs / Qp;
		double wp = .75*550.0 / Qp / rho * .57; // ft^2/s^2 f(Qp) // Q = 30GPM
		//double wp = .78*550.0 / Qp / rho * .58; // ft^2/s^2 f(Qp) // Q = 35GPM
		//double wp = .80*550.0 / Qp / rho * .57; // ft^2/s^2 f(Qp) // Q = 40GPM
		double Lp = 32.f / 36.f; //ft //TODO put calc in final pres
		double Ls = r * Lp; L -= Lp; // account for differently allocated length

		std::vector<Pipe::mHLKParam> FitParam;
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchTh));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, 2.5f/12.f, D));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Expansion, 2.f / 12.f, D));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchTh));
		double hl = Pipe::HLtot(Lp, Qp, FitParam, D); FitParam.clear();
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchOut));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::BranchOut));
		hl += Pipe::HLtot(Ls, Qs, FitParam, D); FitParam.clear();
		hl += Pipe::HL(L, InletB2.Q, D);
		B2.SetP(rho*(Inlet.GetP()/rho + 32.2*(Inlet.Getz()-B2.Getz()) - hl + wp));

	}
	//B2->B1->S1->AS
	{
		State &B2 = getState("B2"), &B1 = getState("B1"), &S1 = getState("S1"), &AS = getState("AS");
		Pipe &B2B1 = getPipe("B2->B1"), &B1S1 = getPipe("B1->S1"), &S1AS = getPipe("S1->AS");
		double L = B2B1.L + B1S1.L + S1AS.L;
		double Q = 50.0 * .002228; //ft^3/s
		double D = 2.067f / 12.f; // Chosen Value
		B2B1.D = D; B1S1.D = D; S1AS.D = D;
		double dztot = B2.Getz() - AS.Getz();
		AS.Setv(4*Q/pi/D/D);

		std::vector<Pipe::mHLKParam> FitParam;
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Compression, D, getPipe("Inlet->B2").D));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .435*D,D)); 
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double Hdiff = B2.GetP() / rho + .5f*(B2.Getv()*B2.Getv() - AS.Getv()*AS.Getv()) + 32.2*dztot;

		double res = Hdiff - hl;
		AS.SetP(rho*(Hdiff-hl)); // psig // extra head ~= 0


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
		
		std::vector<Pipe::mHLKParam> FitParam;
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B2.GetP() / rho + .5f*(B2.Getv()*B2.Getv() - B3.Getv()*B3.Getv()) + 32.2*(B2.Getz() - B3.Getz());
		B3.SetP(rho*(H - hl));
	}
	//B3->KU
	{
		State &B3 = getState("B3"), &KU = getState("KU");
		Pipe &B3KU = getPipe("B3->KU");
		double L = B3KU.L + KU.Getz() - B3.Getz(); //ft // added in Z to take care of getting on the roof
		double Q = 80.0*.002228; //ft^3/s
		double D = 2.067f / 12.f;
		B3KU.D = D;
		
		std::vector<Pipe::mHLKParam> FitParam;
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = (B3.GetP() - KU.GetP()) / rho + .5f*(B3.Getv()*B3.Getv() - KU.Getv()*KU.Getv()) + 32.2*(B3.Getz() - KU.Getz());
		double res = H - hl;
	}

	//B3->S7->S8->STJ
	{
		State &B3 = getState("B3"), &S7 = getState("S7"), &S8 = getState("S8"), &STJ = getState("STJ");
		Pipe &B3S7 = getPipe("B3->S7"), &S7S8 = getPipe("S7->S8"), &S8STJ = getPipe("S8->STJ");
		double L = B3S7.L + S7S8.L + S8STJ.L; //ft
		double Q = (20.0)*.002228; //ft^3/s
		double D = 1.610f / 12.f;
		B3S7.D = D; S7S8.D = D; S8STJ.D = D;

		std::vector<Pipe::mHLKParam> FitParam;
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow90));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Elbow45));
		FitParam.push_back(Pipe::mHLKParam(Pipe::Fitting::Throttle, .13485*D, D));
		double hl = Pipe::HLtot(L, Q, FitParam, D);
		double H = B3.GetP() / rho + 32.2*(B3.Getz() - STJ.Getz());
		double res = H - hl;
		STJ.SetP(rho*(H - hl));
	}
}

void Design::Solve2() {
	double rho = 1.94; //slugs/ft^3

	//I->WT
	{
		State &I = getState("Inlet"), &WT = getState("WT");
		Pipe &IWT = getPipe("Inlet->WT");
		double dz = I.Getz() - WT.Getz();
		double L = sqrt(IWT.L*IWT.L - dz * dz) + dz;

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
