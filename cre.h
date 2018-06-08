#ifndef _CRE_H_
#define _CRE_H_

#include <string>
#include <cstring>

namespace cre
{
	class Pattern
	{
	private:

	public:

		Pattern(
			const char *pattern, const int &flags) 
		{
			
		}
		Pattern(
			const std::string &pattern, const int &flags)
		{

		}
		~Pattern() {}

		int match(
			const char *str)
		{

		}

		int match(
			const std::string &pattern)
		{

		}
	};

	class Cre
	{
	private:

	public:

		Cre() {}
		~Cre() {}

		template <typename T>
		static int match(
			const T &pattern, const T &str, const int &flags)
		{
			Pattern _pattern(pattern, flags);
			return _pattern.match(str);
		}
	};
}

#endif // _CRE_H_
