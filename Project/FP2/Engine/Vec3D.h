#pragma once
#include <string>

class Vec3D {
public:
	double x, y, z;
	Vec3D(double, double, double);
	Vec3D(std::string, std::string, std::string);
	Vec3D();
	~Vec3D();
	double magsqr();
	double mag();
	Vec3D operator=(const Vec3D &a);
	Vec3D operator-(const Vec3D &a);
};