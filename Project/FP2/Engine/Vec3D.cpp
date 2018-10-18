#include "Vec3D.h"
#include <math.h>


Vec3D::Vec3D(double a, double b, double c) {
	x = a; y = b; z = c;
}

Vec3D::Vec3D(std::string a, std::string b, std::string c) {
	Vec3D obj = Vec3D(std::stod(a), std::stod(b), std::stod(c));
	x = obj.x; y = obj.y; z = obj.z;
}

Vec3D::Vec3D() {
	x = 0; y = 0; z = 0;
}

Vec3D::~Vec3D() {

}

double Vec3D::magsqr() {
	return(x*x + y*y + z*z);
}
double Vec3D::mag() {
	return(sqrt(x*x + y*y + z*z));
}

Vec3D Vec3D::operator=(const Vec3D & a) {
	x = a.x; y = a.y; z = a.z;
	return a;
}

Vec3D Vec3D::operator-(const Vec3D & a) {
	return Vec3D(x-a.x, y-a.y, z-a.z);
}