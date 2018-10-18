#include "Picture.h"



Picture::Picture(const std::string & filename, Graphics &gfx) : gfx(gfx) {

	std::ifstream file(filename, std::ios::binary);
	assert(file);

	BITMAPFILEHEADER bmFileHeader;
	file.read(reinterpret_cast<char*>(&bmFileHeader), sizeof(bmFileHeader));

	BITMAPINFOHEADER bmInfoHeader;
	file.read(reinterpret_cast<char*>(&bmInfoHeader), sizeof(bmInfoHeader));

	assert(bmInfoHeader.biBitCount == 24 || bmInfoHeader.biBitCount == 32);
	assert(bmInfoHeader.biCompression == BI_RGB);

	const bool is32b = bmInfoHeader.biBitCount == 32;

	width = bmInfoHeader.biWidth;

	// test for reverse row order and control
	// y loop accordingly
	int yStart;
	int yEnd;
	int dy;
	if (bmInfoHeader.biHeight < 0) {
		height = -bmInfoHeader.biHeight;
		yStart = 0;
		yEnd = height;
		dy = 1;
	}
	else {
		height = bmInfoHeader.biHeight;
		yStart = height - 1;
		yEnd = -1;
		dy = -1;
	}

	pixels.resize(width * height);

	file.seekg(bmFileHeader.bfOffBits);
	// padding is for the case of of 24 bit depth only
	const int padding = (4 - (width * 3) % 4) % 4;

	for (int y = yStart; y != yEnd; y += dy) {
		for (int x = 0; x < width; x++) {
			PutPixel(x, y, Color(file.get(), file.get(), file.get()));
			if (is32b) {
				file.seekg(1, std::ios::cur);
			}
		}
		if (!is32b) {
			file.seekg(padding, std::ios::cur);
		}
	}
}

Picture::Picture(int width, int height, Graphics &gfx) :
	width(width),
	height(height),
	pixels(width * height),
	gfx(gfx) {

}

void Picture::PutPixel(int x, int y, Color c) {
	assert(x >= 0);
	assert(x < width);
	assert(y >= 0);
	assert(y < height);
	pixels.data()[y * width + x] = c;
}

Color Picture::GetPixel(int x, int y) const {
	assert(x >= 0);
	assert(x < width);
	assert(y >= 0);
	assert(y < height);
	return pixels.data()[y * width + x];
}

int Picture::GetWidth() const { return width; }

int Picture::GetHeight() const { return height; }

Rect Picture::GetRect() const {	return Rect(0,height,0,width); }

void Picture::DrawPic(int x, int y, Vec2D a, Vec2D b) {
	if (b.x > width) {
		b.x = width - 1;
	}
	if (b.y > height) {
		b.y = height - 1;
	}
	int scx = gfx.ScreenWidth, scy = gfx.ScreenHeight;
	const size_t rowBytes = 961 * sizeof(Color);
	for (size_t y = 100u; y < 936; y++) {
		memcpy(&gfx.BackGround[y * scx], &pixels[(y-100u+128) * width + 455], rowBytes); \
	}
}

void Picture::Fill(Color c) {
}

const Color * Picture::Data() const { return pixels.data(); }

void Picture::resize(double r) {

}