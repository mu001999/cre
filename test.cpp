#include <iostream>
#include <cstdio>
#include <cassert>
#include "cre.hpp"


using namespace std;


void test_blank()
{
	assert(cre::match("", "") == "");
	assert(cre::match("", "abcdefg") == "");
}

void test_single_character()
{
	assert(cre::match("a", "a") == "a");
	assert(cre::match("a", "b") == "");
	assert(cre::match("z", "z") == "z");
	assert(cre::match("z", "y") == "");
	assert(cre::match("A", "A") == "A");
	assert(cre::match("A", "B") == "");
	assert(cre::match("Z", "Z") == "Z");
	assert(cre::match("Z", "Y") == "");
	assert(cre::match("0", "0") == "0");
	assert(cre::match("0", "1") == "");
	assert(cre::match("9", "9") == "9");
	assert(cre::match("9", "8") == "");
}

void test_concatenate()
{
	assert(cre::match("ab", "ab") == "ab");
	assert(cre::match("ab", "ac") == "");
}

void test_select()
{
	assert(cre::match("a|b", "a") == "a");
	assert(cre::match("a|b", "b") == "b");
	assert(cre::match("ab|c", "ab") == "ab");
	assert(cre::match("ab|c", "c") == "c");
}

void test_closure()
{
	assert(cre::match("a(b|c)*", "abbbbbc") == "abbbbbc");
	assert(cre::match("a(b|c)*", "a") == "a");

	assert(cre::match("ab|c*", "ccc") == "ccc");

	assert(cre::match("abb*", "ab") == "ab");
	assert(cre::match("abb*", "a") == "");

	assert(cre::match("233+", "233") == "233");
	assert(cre::match("233+", "23") == "");
}

void test_questionmark()
{
	assert(cre::match("233?", "233") == "233");
	assert(cre::match("233?", "23") == "23");
	assert(cre::match("233?", "2333") == "233");
	assert(cre::match("233?", "2233") == "");
}

void test_complex()
{
	auto pattern = cre::Pattern("(abcdefg|123456789)*|cyyzerono1|suchangdashabi|chaoqunlaogenb|(ab*c)");
	assert(pattern.match("abcdefgabcdefg") == "abcdefgabcdefg");
	assert(pattern.match("12345668912345") == "");
	assert(pattern.match("cyyzerono1") == "cyyzerono1");
	assert(pattern.match("cvvzerono1") == "");
	assert(pattern.match("abbbbbbbbc") == "abbbbbbbbc");
	assert(pattern.match("ac") == "ac");
}


int main(int argc, char *argv[])
{
	test_blank();
	test_single_character();
	test_concatenate();
	test_select();
	test_closure();
	test_complex();

	std::cout << "test pass" << std::endl;

	// system("pause"); // test on win.
	return 0;
}