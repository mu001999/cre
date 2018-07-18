#ifndef _CRE_H_
#define _CRE_H_

#include <iostream>
#include <string>
#include <cstring>
#include <memory>

/* grammar
expr
	:
	| character (expr|selecttail|closuretail)
	| ( expr ) (expr|selecttail|closuretail)
	;

character
	: [a-zA-Z0-9]
	;

selecttail
	: | ( expr ) (expr|selecttail|closuretail)
	| | characters (expr|selecttail|closuretail)
	;

closuretail
	: * expr
	;
*/

namespace cre
{

	class NFA
	{
	public:
		// node 0 defaults to the start node
		char edges[100][100];
		int end;

		NFA() : end(0) 
		{
			memset(edges, 0, sizeof(edges));
		}

		int transfer(int s, char c)
		{
			for (int i = 0; i < 100; ++i)
			{
				if (edges[s][i] == c) return i;
			}
			std::cout << "can't find the valid edge" << std::endl;
			exit(0);
			return s;
		}

		friend std::shared_ptr<NFA> cat(std::shared_ptr<const NFA>, std::shared_ptr<const NFA>);
		friend std::shared_ptr<NFA> select(std::shared_ptr<const NFA>, std::shared_ptr<const NFA>);
		friend std::shared_ptr<NFA> closure(std::shared_ptr<const NFA>);
	};


	static std::shared_ptr<NFA> cat(std::shared_ptr<const NFA> left, std::shared_ptr<const NFA> right)
	{
		auto nfa = std::make_shared<NFA>();
		return nfa;
	}

	static std::shared_ptr<NFA> select(std::shared_ptr<const NFA> left, std::shared_ptr<const NFA> right)
	{
		return std::make_shared<NFA>();
	}

	static std::shared_ptr<NFA> closure(std::shared_ptr<const NFA> nfa)
	{
		return std::make_shared<NFA>();
	}


	class Node
	{
	private:

	public:

		virtual ~Node() {}
		virtual std::shared_ptr<NFA> compile() = 0;

	};

	class LeafNode : public Node
	{
	private:

		char leaf;

	public:

		LeafNode(char c) : leaf(c) {}
		virtual std::shared_ptr<NFA> compile()
		{
			auto ptr = std::make_shared<NFA>();
			ptr->edges[0][++(ptr->end)] = leaf;
			return ptr;
		}
	};

	class CatNode : public Node
	{
	private:

		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;

	public:

		CatNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) : left(left), right(right) {}
		virtual std::shared_ptr<NFA> compile()
		{
			return cat(left->compile(), right->compile());
		}
	};

	class SelectNode : public Node
	{
	private:

		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;

	public:

		SelectNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) : left(left), right(right) {}
		virtual std::shared_ptr<NFA> compile()
		{
			return std::make_shared<NFA>();
		}
	};

	class ClosureNode : public Node
	{
	private:

		std::shared_ptr<Node> content;

	public:

		ClosureNode(std::shared_ptr<Node> content) : content(content) {}
		virtual std::shared_ptr<NFA> compile()
		{
			return std::make_shared<NFA>();
		}
	};


	static std::shared_ptr<Node> gen_expr(const char *&reading)
	{
		std::shared_ptr<Node> node = nullptr, right = nullptr;

		if (*reading == '(')
		{
			node = gen_expr(reading);
			if (*(reading + 1) != ')') std::cout << "missing )" << std::endl;
			++reading;
		}
		else if (isalnum(*reading))
		{
			node = std::make_shared<LeafNode>(*reading++);
		}

		switch (*reading)
		{
		case '|':
			++reading;
			node = std::make_shared<SelectNode>(node, gen_expr(reading));
			break;
		case '*':
			++reading;
			node = std::make_shared<ClosureNode>(node);
			break;
		default:
			if (isalnum(*reading) || *reading == '(') right = gen_expr(reading);
			break;
		}

		if (right != nullptr) return std::make_shared<CatNode>(node, right);
		return node;
	}

	static std::shared_ptr<Node> get_node(const char *pattern)
	{
		return gen_expr(pattern);
	}


	class Pattern
	{
	private:
		std::shared_ptr<NFA> nfa_ptr;

	public:

		Pattern(const char *pattern)
		{
			nfa_ptr = get_node(pattern)->compile();
		}
		Pattern(const std::string &pattern)
		{
			nfa_ptr = get_node(pattern.c_str())->compile();
		}
		~Pattern() {}


		int match(const char *str)
		{
			int state = 0;
			auto reading = str;
			while (*reading)
			{
				state = nfa_ptr->transfer(state, *reading);
				++reading;
			}
			return state == nfa_ptr->end;
		}

		int match(const std::string &str)
		{
			int state = 0;
			for (auto &c : str) state = nfa_ptr->transfer(state, c);
			return state == nfa_ptr->end;
		}
	};


	class Cre
	{
	public:

		static int match(const char *pattern, const char *str)
		{
			return Pattern(pattern).match(str);
		}

		static int match(const std::string &pattern, const char *str)
		{
			return Pattern(pattern).match(str);
		}

		static int match(const char *pattern, const std::string &str)
		{
			return Pattern(pattern).match(str);
		}

		static int match(const std::string &pattern, const std::string &str)
		{
			return Pattern(pattern).match(str);
		}
	};
}

#endif // _CRE_H_
