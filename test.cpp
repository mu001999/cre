#include <iostream>
#include <cstdio>
#include <cassert>
#include "cre.hpp"

using namespace cre;

int main(int argc, char *argv[])
{
	assert(Cre::match("a", "a") == 0);
	assert(Cre::match("ab", "ab") == 0);
	std::cout << "susscss" << std::endl;
	// assert(Cre::match("a(b|c)*", "abbbbc"));
	system("pause");
	return 0;
}