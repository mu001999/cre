#include <iostream>
#include <cstdio>
#include <cassert>
#include "cre.hpp"


int main(int argc, char *argv[])
{
	assert(cre::match("a", "a") == 0);
	assert(cre::match("ab", "ab") == 0);
	assert(cre::match("a(b|c)*", "abbbbc"));

	std::cout << "success" << std::endl;
	system("pause");
	return 0;
}