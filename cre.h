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

		NFA() : end(0) {}
	};

	class LeafNode;
	class CatNode;
	class SelectNode;
	class ClosureNode;

	class Node
	{
	private:

		static std::shared_ptr<Node> gen_expr(const char *&reading)
		{
			std::shared_ptr<Node> node = nullptr, right = nullptr;

			if (*reading == '(')
			{
				node = gen_expr(reading);
				if (*(reading+1) != ')') std::cout << "missing )" << std::endl;
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

			if ((right = gen_expr(reading)) != nullptr) return std::make_shared<CatNode>(node, right);
			return node;
		}

	public:

		Node() {}
		virtual ~Node() {}
		virtual std::shared_ptr<NFA> compile() {}

		static std::shared_ptr<Node> get_node(const char *pattern, const int &flags)
		{
			return gen_expr(pattern);
		}
	};

	class LeafNode : public Node
	{
	private:

		char leaf;

	public:

		LeafNode(char c) : leaf(c) {}
		virtual ~LeafNode() {}
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
		virtual ~CatNode() {}
		virtual std::shared_ptr<NFA> compile()
		{
			auto left_ptr = left->compile();
			auto right_ptr = right->compile();

		}
	};

	class SelectNode : public Node
	{
	private:

		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;

	public:

		SelectNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) : left(left), right(right) {}
		virtual ~SelectNode() {}
		virtual std::shared_ptr<NFA> compile()
		{

		}
	};

	class ClosureNode : public Node
	{
	private:

		std::shared_ptr<Node> content;

	public:

		ClosureNode(std::shared_ptr<Node> content) : content(content) {}
		virtual ~ClosureNode() {}
		virtual std::shared_ptr<NFA> compile()
		{

		}
	};


	class Pattern
	{
	private:
		std::shared_ptr<NFA> nfa_ptr;

	public:

		Pattern(const char *pattern, const int &flags)
		{
			nfa_ptr = Node::get_node(pattern, flags)->compile();
		}
		Pattern(const std::string &pattern, const int &flags)
		{
			nfa_ptr = Node::get_node(pattern.c_str(), flags)->compile();
		}
		~Pattern() {}


		int match(const char *str)
		{

		}

		int match(const std::string &pattern)
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
		static int match(const T &pattern, const T &str, const int &flags)
		{
			return Pattern(pattern, flags).match(str);
		}
	};
}

#endif // _CRE_H_
