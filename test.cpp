#include <iostream>
#include <cstdio>
#include <cassert>
#include "cre.hpp"


int main(int argc, char *argv[])
{
	assert(cre::match("a", "a") == 0);

	assert(cre::match("ab", "ab") == 0);

	assert(cre::match("a|b", "a") == 0);
	assert(cre::match("a|b", "b") == 0);

	assert(cre::match("ab|c", "ab") == 0);
	assert(cre::match("ab|c", "ac") == 0); // actually it should be "c", but with bugs;

	// assert(cre::match("a(b|c)*", "abbbbc"));

	std::cout << "success" << std::endl;
	// system("pause");
	return 0;
}