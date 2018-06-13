#ifndef _CRE_H_
#define _CRE_H_

#include <string>
#include <cstring>
#include <memory>

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
	public:

		STATE state;
		std::shared_ptr<State> next;
		State(STATE state, std::shared_ptr<State> next = nullptr) : state(state), next(next) {}
		virtual ~State() = 0 {};
	};

	class SingleCharState : public State
	{
		SingleCharState(char chr) : State(STATE::MID) {}
	};

	class StarState : public State
	{
		StarState(std::shared_ptr<State>) : State(STATE::MID) {}
	};

	class PlusState : public State
	{
		PlusState() : State(STATE::MID) {}
	};

	class DotState : public State
	{
		DotState() : State(STATE::MID) {}
	};

	class NFA
	{
	private:

	public:

		NFA(const char *pattern, const int &flags)
		{
			while (*pattern)
			{
				++pattern;
			}
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
