#include "Noise.h"
#include <cmath>
#include <iostream>

Noise::Noise(int repeat) {
	this->repeat = repeat;
	for (int x = 0;x<512;x++) {
		p[x] = permutation[(int)std::fmod(x, 256)];
	}
}

Noise::Noise() {
	this->repeat = -1;
	for (int x = 0;x<512;x++) {
		p[x] = permutation[(int)std::fmod(x, 256)];
	}
}

double Noise::octave(double x, double y, double z, int octaves, double persistence) {
	double total = 0;
	double frequency = 1;
	double amplitude = 1;
	double maxValue = 0;
	for (int i = 0;i<octaves;i++) {
		total += noise(x * frequency, y * frequency, z * frequency) * amplitude;

		maxValue += amplitude;

		amplitude *= persistence;
		frequency *= 2;
	}

	return total / maxValue;
}

double Noise::noise(double x, double y, double z) {
	if (repeat > 0) {
		x = std::fmod(x, repeat);
		y = std::fmod(y, repeat);
		z = std::fmod(z, repeat);
	}

	//if (x < 0 || y < 0 || z < 0) std::cout << x << ", " << y << ", " <<  z << std::endl;

	int xi = (int)x & 255;
	int yi = (int)y & 255;
	int zi = (int)z & 255;
	double xf = x - (int)x;
	double yf = y - (int)y;
	double zf = z - (int)z;
	double u = fade(xf);
	double v = fade(yf);
	double w = fade(zf);

	int aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = p[p[p[xi] + yi] + zi];
	aba = p[p[p[xi] + inc(yi)] + zi];
	aab = p[p[p[xi] + yi] + inc(zi)];
	abb = p[p[p[xi] + inc(yi)] + inc(zi)];
	baa = p[p[p[inc(xi)] + yi] + zi];
	bba = p[p[p[inc(xi)] + inc(yi)] + zi];
	bab = p[p[p[inc(xi)] + yi] + inc(zi)];
	bbb = p[p[p[inc(xi)] + inc(yi)] + inc(zi)];

	double x1, x2, y1, y2;
	x1 = lerp(grad(aaa, xf, yf, zf),
		grad(baa, xf - 1, yf, zf),
		u);
	x2 = lerp(grad(aba, xf, yf - 1, zf),
		grad(bba, xf - 1, yf - 1, zf),
		u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1),
		grad(bab, xf - 1, yf, zf - 1),
		u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1),
		grad(bbb, xf - 1, yf - 1, zf - 1),
		u);
	y2 = lerp(x1, x2, v);

	return (lerp(y1, y2, w) + 1) / 2;
}

int Noise::inc(int num) {
	num++;
	if (repeat > 0) num = std::fmod(num, repeat);
	return num;
}

double Noise::grad(int hash, double x, double y, double z) {
	switch (hash & 0xF)
	{
	case 0x0: return  x + y;
	case 0x1: return -x + y;
	case 0x2: return  x - y;
	case 0x3: return -x - y;
	case 0x4: return  x + z;
	case 0x5: return -x + z;
	case 0x6: return  x - z;
	case 0x7: return -x - z;
	case 0x8: return  y + z;
	case 0x9: return -y + z;
	case 0xA: return  y - z;
	case 0xB: return -y - z;
	case 0xC: return  y + x;
	case 0xD: return -y + z;
	case 0xE: return  y - x;
	case 0xF: return -y - z;
	default: return 0;
	}
}

double Noise::fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);      // 6t^5 - 15t^4 + 10t^3
}

double Noise::lerp(double a, double b, double x) {
	return a + x * (b - a);
}