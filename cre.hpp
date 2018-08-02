#ifndef _CRE_H_
#define _CRE_H_

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cstring>


/* grammar
expr
	:
	| character (expr|selecttail|closuretail)
	| ( expr ) (expr|selecttail|closuretail)
	;

character
	: ASCII CODE
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

	class NFAState
	{
	public:
		
		enum class EdgeType
		{
			EPSILON,
			CCL,
			EMPTY
        } edge_type;
        
		std::vector<int> input_set;
		std::shared_ptr<NFAState> next;
		std::shared_ptr<NFAState> next2;

		NFAState() : next(nullptr), next2(nullptr) {}

	};
    

    class DFAState
    {
    public:
        
        enum class StateType
        {
            NORMAL,
            END
        } state_type;
        
        std::unordered_map<char, std::shared_ptr<DFAState>> to;

        DFAState() : state_type(StateType::NORMAL) {}
        DFAState(StateType type) : state_type(type) {}

    };


	class NFAPair
	{
    private:

        std::vector<std::shared_ptr<NFAState>> eps_closure(std::vector<std::shared_ptr<NFAState>> S)
        {
            std::function<void(std::vector<std::shared_ptr<NFAState>>&, std::shared_ptr<NFAState>)> add2rS;
            add2rS = [&](std::vector<std::shared_ptr<NFAState>> &S, std::shared_ptr<NFAState> s) 
            {
                S.push_back(s);
                if (s->edge_type == NFAState::EdgeType::EPSILON) 
                {
                    add2rS(S, s->next);
                    if (s->next2 != nullptr) add2rS(S, s->next2);
                }
            };

            std::vector<std::shared_ptr<NFAState>> rS;
            for (auto s: S) 
            {
                add2rS(rS, s);
            }
            return rS;
        }

        std::vector<std::shared_ptr<NFAState>> delta(std::vector<std::shared_ptr<NFAState>> q, char c)
        {
            std::vector<std::shared_ptr<NFAState>> rq;
            for (auto s: q)
            {
                if (s->edge_type == NFAState::EdgeType::CCL && std::find(s->input_set.begin(), s->input_set.end(), c) != s->input_set.end())
                {
                    rq.push_back(s->next);
                }
            }
            return rq;
        }

	public:

		std::shared_ptr<NFAState> start;
		std::shared_ptr<NFAState> end;

		NFAPair() : start(std::make_shared<NFAState>()), end(std::make_shared<NFAState>()) {}
		NFAPair(std::shared_ptr<NFAState> start, std::shared_ptr<NFAState> end) : start(start), end(end) {}

		std::shared_ptr<DFAState> to_dfa()
		{
            std::unordered_map<int, std::shared_ptr<DFAState>> mp;

            auto q0 = eps_closure({start});
            auto ptr = std::make_shared<DFAState>((std::find(q0.begin(), q0.end(), end) != q0.end()) ? DFAState::StateType::END : DFAState::StateType::NORMAL);

            std::vector<std::vector<std::shared_ptr<NFAState>>> Q = {q0};
            auto work_list = Q;
            mp[0] = ptr;

            while (!work_list.empty()) 
            {
                auto q = work_list.back();
                work_list.pop_back();
                for (char c = static_cast<char>(0); c >= 0; ++c) 
                {
                    auto t = eps_closure(delta(q, c));
                    if (t.empty()) continue;
                    for (int i = 0; i < Q.size(); ++i) 
                    {
                        if (Q[i] == q) 
                        {
                            int j = -1;

                            while (++j < Q.size())
                            {
                                if (Q[j] == t) 
                                {
                                    mp[i]->to[c] = mp[j];
                                    break;
                                }
                            }

                            if (j == Q.size()) 
                            {
                                Q.push_back(t); work_list.push_back(t);
                                mp[i]->to[c] = mp[mp.size()] = std::make_shared<DFAState>((std::find(t.begin(), t.end(), end) != t.end()) ? DFAState::StateType::END : DFAState::StateType::NORMAL);
                            }

                            break;
                        }
                    }
                }
            }

            return ptr;
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

			ptr->start->edge_type = NFAState::EdgeType::CCL;
			ptr->end->edge_type = NFAState::EdgeType::EMPTY;
			ptr->start->input_set.push_back(leaf);
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

			left->end->edge_type = NFAState::EdgeType::EPSILON;
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

			ptr->start->edge_type = NFAState::EdgeType::EPSILON;
			ptr->end->edge_type = NFAState::EdgeType::EMPTY;
			ptr->start->next = left->start;
			ptr->start->next2 = right->start;
			
			left->end->edge_type = NFAState::EdgeType::EPSILON;
			right->end->edge_type = NFAState::EdgeType::EPSILON;
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

			ptr->start->edge_type = NFAState::EdgeType::EPSILON;
			ptr->start->next = content->start;
			ptr->start->next2 = ptr->end;
			ptr->end->edge_type = NFAState::EdgeType::EMPTY;
			
			content->end->edge_type = NFAState::EdgeType::EPSILON;
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
				++reading;
				node = gen_node(reading);
				if (*(reading + 1) != ')') std::cout << "missing )" << std::endl;
				++reading;
			}
			else if (isalnum(*reading))
			{
				node = std::make_shared<LeafNode>(*reading++);
			}

			if (node == nullptr) 
			{
				std::cout << "syntax error" << std::endl;
				exit(0);
			}

			while (*reading == '(' || isalnum(*reading))
			{
				if (*reading == '(')
				{
					++reading;
					if (right != nullptr) 
					{
						node = std::make_shared<CatNode>(node, right);
					}
					right = gen_node(reading);
					if (*(reading + 1) != ')') std::cout << "missing )" << std::endl;
					++reading;
				}
				else
				{
					if (right != nullptr)
					{
						node = std::make_shared<CatNode>(node, right);
					}
					right = std::make_shared<LeafNode>(*reading++);
				}
			}

			switch (*reading)
			{
			case '|':
				++reading;
				if (right != nullptr) node = std::make_shared<CatNode>(node, right);
				node = std::make_shared<SelectNode>(node, gen_node(reading));
				break;
			case '*':
				++reading;
				if (right != nullptr) node = std::make_shared<CatNode>(node, std::make_shared<ClosureNode>(right));
				else node = std::make_shared<ClosureNode>(node);
				break;
			default:
				break;
			}
			
			return node;
		}

		std::shared_ptr<DFAState> dfa;

	public:

        Pattern(const char *pattern) : dfa(gen_node(pattern)->compile()->to_dfa()) {}
		Pattern(const std::string &pattern) 
		{
			auto reading = pattern.c_str();
			dfa = gen_node(reading)->compile()->to_dfa();
		}

		int match(const char *str)
		{
            auto reading = str;
            auto state = dfa;
            while (*reading) 
            {
                if (state->to.find(*reading) != state->to.end()) state = state->to[*reading];
                else break;
                if (state->state_type == DFAState::StateType::END) return 0;
                ++reading;
            }
			return state->state_type != DFAState::StateType::END;
		}

		int match(const std::string &str)
		{
			auto reading = str.c_str();
            auto state = dfa;
            while (*reading) 
            {
                if (state->to.find(*reading) != state->to.end()) state = state->to[*reading];
                else break;
                if (state->state_type == DFAState::StateType::END) return 0;
                ++reading;
            }
			return state->state_type != DFAState::StateType::END;
		}

    };
    

    int match(const char *pattern, const char *str)
    {
        return Pattern(pattern).match(str);
    }

    int match(const std::string &pattern, const char *str)
    {
        return Pattern(pattern).match(str);
    }

    int match(const char *pattern, const std::string &str)
    {
        return Pattern(pattern).match(str);
    }

    int match(const std::string &pattern, const std::string &str)
    {
        return Pattern(pattern).match(str);
    }

}

#endif // _CRE_H_
