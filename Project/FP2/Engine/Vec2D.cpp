#include "Vec2D.h"
#include <math.h>


Vec2D::Vec2D(double a, double b) {
	x = a; y = b;
}

Vec2D::Vec2D() {
	x = 0; y = 0;
}

Vec2D::~Vec2D() {
}

double Vec2D::magsqr() {
	return(x*x + y*y);
}
double Vec2D::mag() {
	return(sqrt(x*x + y*y));
}

Vec2D Vec2D::operator=(const Vec2D & a) {
	x = a.x; y = a.y;
	return a;
}

Vec2D Vec2D::operator-(const Vec2D & a) {
	return Vec2D(x - a.x, y - a.y);
}

Vec2D Vec2D::operator+(const Vec2D & a) {
	return Vec2D(x + a.x, y + a.y);
}

Vec2D Vec2D::operator*(double a) {
	return Vec2D(a*x, a*y);
}