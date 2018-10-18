#pragma once

#include "Colors.h"
#include <string>
#include <vector>
#include "Rect.h"
#include <fstream>
#include <assert.h>
#include "Graphics.h"

class Picture {
public:
	Picture(const std::string& filename, Graphics & gfx);
	Picture(int width, int height, Graphics &gfx);
	void PutPixel(int x, int y, Color c);
	Color GetPixel(int x, int y) const;
	int GetWidth() const;
	int GetHeight() const;
	Rect GetRect() const;
	void DrawPic(int x, int y, Vec2D a, Vec2D b);
	void Fill(Color c);
	const Color* Data() const;
	void resize(double r);
	std::vector<Color> pixels;
	int width = 0;
	int height = 0;
	Graphics &gfx;
};