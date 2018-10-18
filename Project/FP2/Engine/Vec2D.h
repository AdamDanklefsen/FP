#pragma once

class Vec2D {
public:
	double x, y;
public:
	Vec2D(double, double);
	Vec2D();
	~Vec2D();
	double magsqr();
	double mag();
	Vec2D operator=(const Vec2D &a);
	Vec2D operator-(const Vec2D &a);
	Vec2D operator+(const Vec2D &a);
	Vec2D operator*(double a);
	operator int() { return Vec2D((int)this->x, (int)this->y); }
};