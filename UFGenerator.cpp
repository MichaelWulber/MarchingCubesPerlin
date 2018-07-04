#include "UFGenerator.h"


UFGenerator::UFGenerator() {
	this->name = "";
	this->ext = "";
	this->count = 0;
}

UFGenerator::UFGenerator(std::string name, std::string ext) {
	this->name = name;
	this->ext = ext;
	this->count = 0;
}

std::string UFGenerator::getUniqueName() {
	std::string fileName;
	if (count < 10) {
		fileName = name + "_000" + std::to_string(count) + "." + ext;
	}
	else if (count < 100) {
		fileName = name + "_00" + std::to_string(count) + "." + ext;
	}
	else if (count < 1000) {
		fileName = name + "_0" + std::to_string(count) + "." + ext;
	}
	else if (count < 10000) {
		fileName = name + "_" + std::to_string(count) + "." + ext;
	}
	else {
		count %= 10000;
		fileName = name + "_000" + std::to_string(count) + "." + ext;
	}
	count++;
	return fileName;
}