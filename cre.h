#ifndef _CRE_H_
#define _CRE_H_

#include <string>
#include <cstring>

namespace cre
{
	enum class STATE
	{
		START,
		MID,
		END
	};

	class State
	{

	};

	class StarState : public State
	{

	};

	class PlusState : public State
	{

	};

	class DotState : public State
	{

	};

	class NFA
	{
	private:

	public:

		NFA(const char *pattern, const int &flags)
		{
			
		}
	};

	class Pattern
	{
	private:

		NFA nfa;

	public:

		Pattern(
			const char *pattern, const int &flags) : nfa(pattern, flags)
		{
			
		}
		Pattern(
			const std::string &pattern, const int &flags) : nfa(pattern.c_str(), flags)
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
