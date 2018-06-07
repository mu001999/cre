#ifndef _CRE_H_
#define _CRE_H_

namespace cre
{
	class Pattern
	{
	private:

	public:

		Pattern(
			char *pattern, int flags) 
		{
			
		}
		~Pattern() {}

		int match(
			char *str)
		{

		}
	};

	class Cre
	{
	private:

	public:

		Cre() {}
		~Cre() {}

		static int match(
			char *pattern, char *str, int flags)
		{
			Pattern _pattern(pattern, flags);
			return _pattern.match(str);
		}
	};
}

#endif // _CRE_H_
