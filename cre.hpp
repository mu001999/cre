#ifndef _CRE_H_
#define _CRE_H_

#include <set>
#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>


/* grammar
expr
	:
	| character (expr|selecttail|closuretail)
	| ( expr ) (expr|selecttail|closuretail)
	;

character
	: [0-9a-zA-Z]
	;

selecttail
	: | ( expr ) (expr|selecttail|closuretail)
	| | characters (expr|selecttail|closuretail)
	;

closuretail
	: * expr
	: + expr
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

        std::set<std::shared_ptr<NFAState>> eps_closure(std::set<std::shared_ptr<NFAState>> S)
        {
            std::function<void(std::set<std::shared_ptr<NFAState>>&, std::shared_ptr<NFAState>)> add2rS;
            add2rS = [&](std::set<std::shared_ptr<NFAState>> &S, std::shared_ptr<NFAState> s) 
            {
                S.insert(s);
                if (s->edge_type == NFAState::EdgeType::EPSILON) 
                {
                    add2rS(S, s->next);
                    if (s->next2) add2rS(S, s->next2);
                }
            };

            std::set<std::shared_ptr<NFAState>> rS;
            for (auto s: S) add2rS(rS, s);
            return rS;
        }

        std::set<std::shared_ptr<NFAState>> delta(std::set<std::shared_ptr<NFAState>> q, char c)
        {
            std::set<std::shared_ptr<NFAState>> rq;
            for (auto s: q) if (s->edge_type == NFAState::EdgeType::CCL && std::find(s->input_set.begin(), s->input_set.end(), c) != s->input_set.end()) rq.insert(s->next);
            return rq;
		}
		
		std::shared_ptr<DFAState> dfa_minimization(std::vector<std::shared_ptr<DFAState>> &mp)
		{
			std::set<std::set<int>> T;
			auto P = T;

			auto indexof_inmp = [&](std::shared_ptr<DFAState> state)
			{
				for (int i = 0; i < mp.size(); ++i) if (mp[i] == state) return i;
				return -1;
			};

			{
				std::vector<std::set<int>> _T = {{}, {}};
				for (int i = 0; i < mp.size(); ++i) _T[mp[i]->state_type == DFAState::StateType::END].insert(i);
				T.insert(_T[0]); T.insert(_T[1]);
			}

			auto split = [&](const std::set<int> &S)
			{
				std::vector<std::set<int>> res = {S};
				for (char c = static_cast<char>(0); c >= 0; ++c) 
				{
					std::set<int> s1, s2;
					auto flag_it = P.end();
					for (auto i: S)
					{
						if (mp[i]->to.count(c))
						{
							int k = indexof_inmp(mp[i]->to[c]);
							for (auto it = P.begin(); it != P.end(); ++it)
							{
								if (it->count(k))
								{
									if (it == flag_it) s1.insert(i);
									else if (flag_it == P.end() && it->count(i) == 0)
									{
										flag_it = it;
										s1.insert(i);
									}
									else s2.insert(i);
									break;
								}
							}
						}
						else s2.insert(i);
					}

					if (s1.size() && s2.size()) 
					{
						res = {s1, s2};
						return res;
					}
				}
				return res;
			};

			while (P != T)
			{
				P = T; T.clear();
				for (auto &p: P) for (auto &_p: split(p)) T.insert(_p);
			}

			std::vector<std::shared_ptr<DFAState>> states(T.size(), nullptr);
			for (auto &state: states) state = std::make_shared<DFAState>();
			std::shared_ptr<DFAState> start = nullptr;

			{
				std::vector<std::set<int>> P;
				for (auto &t: T) P.push_back(t);

				auto indexof_inp = [&](std::shared_ptr<DFAState> state)
				{
					for (int i = 0, k = indexof_inmp(state); i < P.size(); ++i) if (P[i].count(k)) return i;
					return -1;
				};

				for (int i = 0; i < P.size(); ++i)
				{
					for (auto &k: P[i])
					{
						if (mp[k]->state_type == DFAState::StateType::END) states[i]->state_type = DFAState::StateType::END;
						if (k == 0) start = states[i];
						for (auto it: mp[k]->to) states[i]->to[it.first] = states[indexof_inp(it.second)];
					}
				}

				// show the final dfa
				/*
				auto indexof_instates = [&](std::shared_ptr<DFAState> state)
				{
					for (int i = 0; i < states.size(); ++i) if (states[i] == state) return i;
					return -1;
				};
				for (int i = 0; i < states.size(); ++i)
				{
					printf("d%d\t", i);
					for (auto it: states[i]->to)
					{
						printf("%c->d%d\t", it.first, indexof_instates(it.second));
					}
					printf("\n");
				}
				*/

			}
			return start;
		}

	public:

		std::shared_ptr<NFAState> start;
		std::shared_ptr<NFAState> end;

		NFAPair() : start(std::make_shared<NFAState>()), end(std::make_shared<NFAState>()) {}
		NFAPair(std::shared_ptr<NFAState> start, std::shared_ptr<NFAState> end) : start(start), end(end) {}

		std::shared_ptr<DFAState> to_dfa()
		{
			auto q0 = eps_closure({start});
			std::vector<std::set<std::shared_ptr<NFAState>>> Q = {q0};
			auto work_list = Q;
            std::vector<std::shared_ptr<DFAState>> mp = { std::make_shared<DFAState>((std::find(q0.begin(), q0.end(), end) != q0.end()) ? DFAState::StateType::END : DFAState::StateType::NORMAL)};

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
								Q.push_back(t); 
								work_list.push_back(t); 
								mp.push_back(std::make_shared<DFAState>((std::find(t.begin(), t.end(), end) != t.end()) ? DFAState::StateType::END : DFAState::StateType::NORMAL));
                                mp[i]->to[c] = mp.back();
                            }

                            break;
                        }
                    }
                }
			}
			
            return dfa_minimization(mp);
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
            ptr->start->next = ptr->end;

			if (leaf == '.') for (char c = static_cast<char>(0); c >= 0; ++c) ptr->start->input_set.push_back(c);
			else ptr->start->input_set.push_back(leaf);
            
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

	class QuestionMarkNode : public Node
	{
	private:

		std::shared_ptr<Node> content;

	public:

		QuestionMarkNode(std::shared_ptr<Node> content) : content(content) {}
		virtual std::shared_ptr<NFAPair> compile()
		{
			auto content = this->content->compile();
			auto ptr = std::make_shared<NFAPair>();

			ptr->start->edge_type = NFAState::EdgeType::EPSILON;
			ptr->start->next = content->start;
			ptr->start->next2 = ptr->end;
			ptr->end->edge_type = NFAState::EdgeType::EMPTY;
			
			content->end->edge_type = NFAState::EdgeType::EPSILON;
			content->end->next = ptr->end;

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
				if (*reading != ')') std::cout << "missing )" << std::endl;
				++reading;
			}
			else if (*reading && *reading != '|' && *reading != ')') node = std::make_shared<LeafNode>(*reading++);

			if (!node) return node;

			while (*reading && *reading != '|' && *reading != ')')
			{
				switch (*reading)
				{
				case '(':
					++reading;
					if (right) node = std::make_shared<CatNode>(node, right);
					right = gen_node(reading);
					if (*reading != ')') std::cout << "missing )" << std::endl;
					break;
				case '*':
					if (right) 
					{
						node = std::make_shared<CatNode>(node, std::make_shared<ClosureNode>(right));
						right = nullptr;
					}
					else node = std::make_shared<ClosureNode>(node);
					break;
				case '+':
					if (right)
					{
						node = std::make_shared<CatNode>(node, std::make_shared<CatNode>(right, std::make_shared<ClosureNode>(right)));
						right = nullptr;
					}
					else node = std::make_shared<CatNode>(node, std::make_shared<ClosureNode>(node));
					break;
				case '?':
					if (right)
					{
						node = std::make_shared<CatNode>(node, std::make_shared<QuestionMarkNode>(right));
						right = nullptr;
					}
					else node = std::make_shared<QuestionMarkNode>(node);
					break;
				default:
					if (right) node = std::make_shared<CatNode>(node, right);
					right = std::make_shared<LeafNode>(*reading);
					break;
				}
				++reading;
			}

			if (*reading == '|')
			{
				++reading;
				if (right) node = std::make_shared<CatNode>(node, right);
				node = std::make_shared<SelectNode>(node, gen_node(reading));
			}
			else if (right) node = std::make_shared<CatNode>(node, right);

			return node;
		}

		std::shared_ptr<DFAState> dfa;

	public:

		Pattern(const char *pattern)
		{
			auto node = gen_node(pattern);
			if (!node) dfa = std::make_shared<DFAState>(DFAState::StateType::END);
			else dfa = node->compile()->to_dfa();
		}

		Pattern(const std::string &pattern) 
		{
			auto reading = pattern.c_str();
			auto node = gen_node(reading);
			if (!node) dfa = std::make_shared<DFAState>(DFAState::StateType::END);
			else dfa = node->compile()->to_dfa();
		}


		std::string match(const char *str)
		{
			std::string res, temp;
            auto reading = str;
            auto state = dfa;
            while (*reading) 
            {
                if (state->to.count(*reading)) state = state->to[*reading];
				else break;
				temp += *reading;
				if (state->state_type == DFAState::StateType::END) 
				{
					res += temp;
					temp = "";
				}
                ++reading;
			}
			return res;
		}

		std::string match(const std::string &str)
		{
			std::string res, temp;
			auto reading = str.c_str();
            auto state = dfa;
            while (*reading) 
            {
                if (state->to.count(*reading)) state = state->to[*reading];
				else break;
				temp += *reading;
				if (state->state_type == DFAState::StateType::END) 
				{
					res += temp;
					temp = "";
				}
                ++reading;
			}
			return res;
		}

    };
    

    std::string match(const char *pattern, const char *str)
    {
        return Pattern(pattern).match(str);
    }

    std::string match(const std::string &pattern, const char *str)
    {
        return Pattern(pattern).match(str);
    }

    std::string match(const char *pattern, const std::string &str)
    {
        return Pattern(pattern).match(str);
    }

    std::string match(const std::string &pattern, const std::string &str)
    {
        return Pattern(pattern).match(str);
    }

}

#endif // _CRE_H_
