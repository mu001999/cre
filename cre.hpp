#ifndef _CRE_H_
#define _CRE_H_

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>


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

	class DFA
	{
	public:


	};


	class NFANode
	{
	public:
		
		enum class EdgeType
		{
			EPSILON,
			CCL,
			EMPTY
		} edge;
		std::vector<int> inputSet;
		std::shared_ptr<NFANode> next;
		std::shared_ptr<NFANode> next2;

		NFANode() : next(nullptr), next2(nullptr) {}

	};

	class NFAPair
	{
	public:

		std::shared_ptr<NFANode> start;
		std::shared_ptr<NFANode> end;

		NFAPair() : start(std::make_shared<NFANode>()), end(std::make_shared<NFANode>()) {}
		NFAPair(std::shared_ptr<NFANode> start, std::shared_ptr<NFANode> end) : start(start), end(end) {}

		std::shared_ptr<DFA> to_dfa()
		{
			return nullptr;
		}

	};


	class Node
	{
	public:

		virtual ~Node() {}
		virtual std::shared_ptr<NFAPair> compile() = 0;

	};

	class LeafNode : public Node
	{
	private:

		char leaf;

	public:

		LeafNode(char c) : leaf(c) {}
		virtual std::shared_ptr<NFAPair> compile()
		{
			auto ptr = std::make_shared<NFAPair>();

			ptr->start->edge = NFANode::EdgeType::CCL;
			ptr->end->edge = NFANode::EdgeType::EMPTY;
			ptr->start->inputSet.push_back(leaf);
			ptr->start->next = ptr->end;

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
		virtual std::shared_ptr<NFAPair> compile()
		{
			auto left = this->left->compile();
			auto right = this->right->compile();
			auto ptr = std::make_shared<NFAPair>(left->start, right->end);

			left->end->edge = NFANode::EdgeType::EPSILON;
			left->end->next = right->start;

			return ptr;
		}

	};

	class SelectNode : public Node
	{
	private:

		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;

	public:

		SelectNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) : left(left), right(right) {}
		virtual std::shared_ptr<NFAPair> compile()
		{
			auto left = this->left->compile();
			auto right = this->right->compile();
			auto ptr = std::make_shared<NFAPair>();

			ptr->start->edge = NFANode::EdgeType::EPSILON;
			ptr->end->edge = NFANode::EdgeType::EMPTY;
			ptr->start->next = left->start;
			ptr->start->next2 = right->start;
			
			left->end->edge = NFANode::EdgeType::EPSILON;
			right->end->edge = NFANode::EdgeType::EPSILON;
			left->end->next = ptr->end;
			right->end->next = ptr->end;

			return ptr;
		}

	};

	class ClosureNode : public Node
	{
	private:

		std::shared_ptr<Node> content;

	public:

		ClosureNode(std::shared_ptr<Node> content) : content(content) {}
		virtual std::shared_ptr<NFAPair> compile()
		{
			auto content = this->content->compile();
			auto ptr = std::make_shared<NFAPair>();

			ptr->start->edge = NFANode::EdgeType::EPSILON;
			ptr->start->next = content->start;
			ptr->start->next2 = ptr->end;
			ptr->end->edge = NFANode::EdgeType::EMPTY;
			
			content->end->edge = NFANode::EdgeType::EPSILON;
			content->end->next = content->start;
			content->end->next2 = ptr->end;

			return ptr;
		}

	};


	class Pattern
	{
	private:

		std::shared_ptr<Node> gen_node(const char *&reading)
		{
			std::shared_ptr<Node> node = nullptr, right = nullptr;

			if (*reading == '(')
			{
				node = gen_node(reading);
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
				node = std::make_shared<SelectNode>(node, gen_node(reading));
				break;
			case '*':
				++reading;
				node = std::make_shared<ClosureNode>(node);
				break;
			default:
				if (isalnum(*reading) || *reading == '(') right = gen_node(reading);
				break;
			}

			if (right != nullptr) return std::make_shared<CatNode>(node, right);
			return node;
		}

		std::shared_ptr<DFA> dfa;

	public:

		Pattern(const char *pattern) : dfa(gen_node(pattern)->compile()->to_dfa()) {}
		Pattern(const std::string &pattern) 
		{
			auto reading = pattern.c_str();
			dfa = gen_node(reading)->compile()->to_dfa();
		}

		int match(const char *str)
		{
			/*
			auto reading = str;
			auto state = nfa->start;
			while (*reading) 
			{
				if (state == nfa->end) break;
				else if (std::find(state->inputSet.begin(), state->inputSet.end(), *reading) == state->inputSet.end()) break;
				else state = state->next;
				++reading;
			}
			if (state == nfa->end) return 0;
			*/
			return 1;
		}

		int match(const std::string &str)
		{
			/*
			auto reading = str.c_str();
			auto state = nfa->start;
			while (*reading)
			{
				if (state == nfa->end) break;
				else if (std::find(state->inputSet.begin(), state->inputSet.end(), *reading) == state->inputSet.end()) break;
				else state = state->next;
				++reading;
			}
			if (state == nfa->end) return 0;
			*/
			return 1;
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
