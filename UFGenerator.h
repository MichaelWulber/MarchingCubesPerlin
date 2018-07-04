#include <string>

#ifndef UFGENERATOR_H
#define UFGENERATOR_H

class UFGenerator {
public:
	UFGenerator();
	UFGenerator(std::string name, std::string ext);
	std::string getUniqueName();

private:
	std::string name;
	int count;
	std::string ext;
};

#endif