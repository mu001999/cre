#include <iostream>
#include <cstdio>
#include <cassert>
#include "cre.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	assert(cre::match("a", "a") == 0);

	assert(cre::match("ab", "ab") == 0);

	assert(cre::match("a|b", "a") == 0);
	assert(cre::match("a|b", "b") == 0);

	assert(cre::match("ab|c", "ab") == 0);
	assert(cre::match("ab|c", "c") == 0);

	assert(cre::match("a(b|c)*", "abbbbbc") == 0);
	assert(cre::match("a(b|c)*", "a") == 0);

	assert(cre::match("ab|c*", "ccc") == 0);

	assert(cre::match("abb*", "ab") == 0);
	assert(cre::match("abb*", "a") == 1);

	auto pattern = cre::Pattern("(abcdefg|123456789)*|cyyzerono1|suchangdashabi|chaoqunlaogenb|(ab*c)");
	assert(pattern.match("abcdefgabcdefg") == 0);
	assert(pattern.match("12345668912345") == 1);
	assert(pattern.match("cyyzerono1") == 0);
	assert(pattern.match("cvvzerono1") == 1);
	assert(pattern.match("abbbbbbbbc") == 0);
	assert(pattern.match("ac") == 0);

	std::cout << "test pass" << std::endl;

	// system("pause"); // test on win.
	return 0;
}