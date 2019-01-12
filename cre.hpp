#pragma once


#include <set>
#include <tuple>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <memory>
#include <bitset>
#include <algorithm>
#include <functional>
#include <unordered_map>


namespace cre
{

    ::std::bitset<128> SPACES(0X100003e00ULL);
    ::std::bitset<128> DIGITS(287948901175001088ULL);
    ::std::bitset<128> LWORDS("111111111111111111111111110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    ::std::bitset<128> UWORDS("1111111111111111111111111100000000000000000000000000000000000000000000000000000000000000000");
    ::std::bitset<128> WORD_S("111111111111111111111111110100001111111111111111111111111100000001111111111000000000000000000000000000000000000000000000000");
    ::std::bitset<128> NOT_SPACES = ~SPACES;
    ::std::bitset<128> NOT_DIGITS = ~DIGITS;
    ::std::bitset<128> NOT_LWORDS = ~LWORDS;
    ::std::bitset<128> NOT_UWORDS = ~UWORDS;
    ::std::bitset<128> NOT_WORD_S = ~WORD_S;

    ::std::unordered_map<char, ::std::bitset<128>> ECMAP =
    {
        {'s', SPACES}, {'S', NOT_SPACES},
        {'d', DIGITS}, {'D', NOT_DIGITS},
        {'l', LWORDS}, {'L', NOT_LWORDS},
        {'u', UWORDS}, {'U', NOT_UWORDS},
        {'w', WORD_S}, {'W', NOT_WORD_S}
    };


    class NFAState
    {
    public:

        enum class EdgeType
        {
            EPSILON,
            CCL,
            EMPTY
        } edge_type;

        ::std::bitset<128> input_set;
        ::std::shared_ptr<NFAState> next, next2;

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

        ::std::unordered_map<char, ::std::shared_ptr<DFAState>> to;

        DFAState() : state_type(StateType::NORMAL) {}
        DFAState(StateType type) : state_type(type) {}

    };


    class NFAPair
    {
    private:

        ::std::set<::std::shared_ptr<NFAState>> eps_closure(::std::set<::std::shared_ptr<NFAState>> S)
        {
            ::std::function<void(::std::set<::std::shared_ptr<NFAState>>&, const ::std::shared_ptr<NFAState>&)> add2rS;
            add2rS = [&](::std::set<::std::shared_ptr<NFAState>> &S, const ::std::shared_ptr<NFAState> &s)
            {
                S.insert(s);

                if (s->edge_type == NFAState::EdgeType::EPSILON)
                {
                    if (!S.count(s->next)) add2rS(S, s->next);
                    if (s->next2 && !S.count(s->next2)) add2rS(S, s->next2);
                }
            };

            ::std::set<::std::shared_ptr<NFAState>> rS;

            for (auto &s: S) add2rS(rS, s);

            return rS;
        }

        ::std::set<::std::shared_ptr<NFAState>> delta(::std::set<::std::shared_ptr<NFAState>> &q, char c)
        {
            ::std::set<::std::shared_ptr<NFAState>> rq;

            for (auto &s: q) if (s->edge_type == NFAState::EdgeType::CCL && s->input_set[c]) rq.insert(s->next);

            return rq;
        }

        ::std::shared_ptr<DFAState> dfa_minimization(::std::vector<::std::shared_ptr<DFAState>> &mp)
        {
            ::std::set<::std::set<int>> T, P;

            auto indexof_inmp = [&](::std::shared_ptr<DFAState> state)
            {
                for (int i = 0; i < mp.size(); ++i) if (mp[i] == state) return i;
                return -1;
            };

            {
                ::std::vector<::std::set<int>> _T = {{}, {}};

                for (int i = 0; i < mp.size(); ++i) _T[mp[i]->state_type == DFAState::StateType::END].insert(i);

                T.insert(_T[0]); T.insert(_T[1]);
            }

            auto split = [&](const ::std::set<int> &S)
            {
                ::std::vector<::std::set<int>> res = {S};

                for (char c = static_cast<char>(0); c >= 0; ++c)
                {
                    ::std::set<int> s1, s2;

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
                                    else if (flag_it == P.end())
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

            ::std::vector<::std::shared_ptr<DFAState>> states;
            for (auto &_: T) states.push_back(::std::make_shared<DFAState>());
            ::std::shared_ptr<DFAState> start = nullptr;

            {
                ::std::vector<::std::set<int>> P(T.begin(), T.end());

                auto indexof_inp = [&](::std::shared_ptr<DFAState> state)
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
            }
            return start;
        }

    public:

        ::std::shared_ptr<NFAState> start, end;

        NFAPair() : start(::std::make_shared<NFAState>()), end(::std::make_shared<NFAState>()) {}
        NFAPair(::std::shared_ptr<NFAState> start, ::std::shared_ptr<NFAState> end) : start(start), end(end) {}

        ::std::shared_ptr<DFAState> to_dfa()
        {
            auto q0 = eps_closure({start});
            ::std::vector<::std::set<::std::shared_ptr<NFAState>>> Q = {q0}, work_list = {q0};
            ::std::vector<::std::shared_ptr<DFAState>> mp = {::std::make_shared<DFAState>((::std::find(q0.begin(), q0.end(), end) != q0.end()) ? DFAState::StateType::END : DFAState::StateType::NORMAL)};

            while (work_list.size())
            {
                auto q = work_list.back();
                work_list.pop_back();
                for (char c = static_cast<char>(0); c >= 0; ++c)
                {
                    auto t = eps_closure(delta(q, c));
                    if (t.empty()) continue;
                    for (int i = 0, j = -1; i < Q.size() && j == -1; ++i) if (Q[i] == q)
                    {
                        while (++j < Q.size()) if (Q[j] == t && (mp[i]->to[c] = mp[j]) == mp[j]) break;

                        if (j == Q.size())
                        {
                            Q.push_back(t);
                            work_list.push_back(t);
                            mp.push_back(::std::make_shared<DFAState>((::std::find(t.begin(), t.end(), end) != t.end()) ? DFAState::StateType::END : DFAState::StateType::NORMAL));
                            mp[i]->to[c] = mp.back();
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
        virtual ::std::shared_ptr<NFAPair> compile() = 0;

    };

    class LeafNode : public Node
    {
    private:

        char leaf;

    public:

        LeafNode(char c) : leaf(c) {}
        virtual ::std::shared_ptr<NFAPair> compile()
        {
            auto ptr = ::std::make_shared<NFAPair>();

            ptr->start->edge_type = NFAState::EdgeType::CCL;
            ptr->end->edge_type = NFAState::EdgeType::EMPTY;
            ptr->start->next = ptr->end;
            ptr->start->input_set.set(leaf);

            return ptr;
        }

    };

    class CatNode : public Node
    {
    private:

        ::std::shared_ptr<Node> left, right;

    public:

        CatNode(::std::shared_ptr<Node> left, ::std::shared_ptr<Node> right) : left(left), right(right) {}
        virtual ::std::shared_ptr<NFAPair> compile()
        {
            auto left = this->left->compile();
            auto right = this->right->compile();
            auto ptr = ::std::make_shared<NFAPair>(left->start, right->end);

            left->end->edge_type = NFAState::EdgeType::EPSILON;
            left->end->next = right->start;

            return ptr;
        }

    };

    class SelectNode : public Node
    {
    private:

        ::std::shared_ptr<Node> left, right;

    public:

        SelectNode(::std::shared_ptr<Node> left, ::std::shared_ptr<Node> right) : left(left), right(right) {}
        virtual ::std::shared_ptr<NFAPair> compile()
        {
            auto left = this->left->compile();
            auto right = this->right->compile();
            auto ptr = ::std::make_shared<NFAPair>();

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

        ::std::shared_ptr<Node> content;

    public:

        ClosureNode(::std::shared_ptr<Node> content) : content(content) {}
        virtual ::std::shared_ptr<NFAPair> compile()
        {
            auto content = this->content->compile();
            auto ptr = ::std::make_shared<NFAPair>();

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

    class QualifierNode : public Node
    {
    private:

        ::std::shared_ptr<Node> content;
        int n, m;

    public:

        QualifierNode(::std::shared_ptr<Node> content, int n, int m) : content(content), n(n), m(m) {}
        virtual ::std::shared_ptr<NFAPair> compile()
        {
            ::std::shared_ptr<NFAPair> ptr = ::std::make_shared<NFAPair>();
            ptr->end->edge_type = NFAState::EdgeType::EMPTY;

            // -2 means '{n}', -1 means '{n,}', >=0 means '{n,m}'
            if (m == -2) // for '{n}'
            {
                ::std::shared_ptr<Node> temp = (n > 0) ? content : nullptr;
                for (int i = 1; i < n; ++i) temp = ::std::make_shared<CatNode>(temp, content);
                if (temp) return temp->compile();
                else
                {
                    ptr->start->edge_type = NFAState::EdgeType::EPSILON;
                    ptr->start->next = ptr->end;
                }
            }
            else if (m == -1) // for '{n,}'
            {
                ::std::shared_ptr<Node> temp = (n > 0) ? content : nullptr;
                for (int i = 1; i < n; ++i) temp = ::std::make_shared<CatNode>(temp, content);
                return temp ? ::std::make_shared<CatNode>(temp, ::std::make_shared<ClosureNode>(content))->compile() : ::std::make_shared<ClosureNode>(content)->compile();
            }
            else if (n < m && n >= 0) // for '{n,m}'
            {
                auto first = content->compile();
                auto pre = first;
                ptr->start->edge_type = NFAState::EdgeType::EPSILON;
                ptr->start->next = first->start;
                if (n == 0) ptr->start->next2 = ptr->end;

                for (int i = 1; i < m; ++i)
                {
                    auto now = content->compile();
                    pre->end->edge_type = NFAState::EdgeType::EPSILON;
                    pre->end->next = now->start;
                    if (i > n - 2) pre->end->next2 = ptr->end;
                    pre = now;
                }

                pre->end->edge_type = NFAState::EdgeType::EPSILON;
                pre->end->next = ptr->end;
            }
            else
            {
                ptr->start->edge_type = NFAState::EdgeType::EPSILON;
                ptr->start->next = ptr->end;
            }

            return ptr;
        }
    };

    class DotNode : public Node
    {
    public:

        virtual ::std::shared_ptr<NFAPair> compile()
        {
            auto ptr = ::std::make_shared<NFAPair>();

            ptr->start->edge_type = NFAState::EdgeType::CCL;
            ptr->end->edge_type = NFAState::EdgeType::EMPTY;
            ptr->start->next = ptr->end;
            ptr->start->input_set = 1024ULL;
            ptr->start->input_set.flip();

            return ptr;
        }
    };

    class BracketNode : public Node
    {
    private:

        ::std::bitset<128> chrs;

    public:

        BracketNode(::std::bitset<128> chrs) : chrs(chrs) {}
        virtual ::std::shared_ptr<NFAPair> compile()
        {
            auto ptr = ::std::make_shared<NFAPair>();

            ptr->start->edge_type = NFAState::EdgeType::CCL;
            ptr->end->edge_type = NFAState::EdgeType::EMPTY;
            ptr->start->next = ptr->end;
            ptr->start->input_set = chrs;

            return ptr;
        }

    };


    inline ::std::tuple<::std::shared_ptr<DFAState>, bool, bool> gen_dfa(const char *reading)
    {
        ::std::unordered_map<::std::string, ::std::shared_ptr<Node>> ref_map;

        ::std::shared_ptr<DFAState> dfa;

        bool begin = false, end = false;
        auto begin_address = reading;

        ::std::function<char(const char *&)> translate_escape_chr;
        ::std::function<::std::bitset<128>(char &, const char *&, bool range)> translate_echr2bset;
        ::std::function<::std::shared_ptr<Node>(const char *&)> translate_echr2node, gen_bracket, gen_subexpr, gen_node;

        translate_escape_chr = [&](const char *&reading)
        {
            ++reading;
            if (*reading)
            {
                switch (*reading)
                {
                case '0': return '\0';
                case 'a': return '\a';
                case 'b': return '\b';
                case 't': return '\t';
                case 'n': return '\n';
                case 'v': return '\v';
                case 'f': return '\f';
                case 'r': return '\r';
                case 'e': return '\e';
                case 'c':
                    if (*(reading+1) && (isalpha(*(reading+1)) || (*(reading + 1) > 63 && *(reading + 1) < 94)))
                    {
                        ++reading;
                        return static_cast<char>(toupper(*(reading+1)) - 64);
                    }
                    else return 'c';
                default: return *reading;
                }
            }
            else
            {
                printf("cre syntax error at pos %d: only '\\'\n", reading - begin_address);
                return static_cast<char>(-1);
            }
        };

        translate_echr2bset = [&](char &left, const char *&reading, bool range)
        {
            char res = translate_escape_chr(reading);
            ::std::bitset<128> ret;
            if (ECMAP.count(res)) ret = ECMAP[res];
            else if (range) for (; left <= res; ++left) ret.set(left);
            else left = res;
            if (!range && ECMAP.count(res)) left = -1;
            return ret;
        };

        translate_echr2node = [&](const char *&reading)
        {
            ::std::shared_ptr<Node> node = nullptr;
            char left = -1;
            auto res = translate_echr2bset(left, reading, false);
            if (left == -1) node = ::std::make_shared<BracketNode>(res);
            else node = ::std::make_shared<LeafNode>(left);
            return node;
        };

        gen_bracket = [&](const char *&reading)
        {
            char left = -1;
            bool range = false, exclude = false;
            ::std::bitset<128> chrs;

            if (*reading == '^')
            {
                ++reading;
                exclude = true;
            }
            if (*reading == ']') return ::std::make_shared<BracketNode>(chrs);

            while (*reading && *reading != ']')
            {
                if (*reading == '-')
                {
                    if (range || left == -1) printf("cre syntax error at pos %d: incorrect position of '-'\n", reading - begin_address);
                    else range = true;
                }
                else if (range)
                {
                    if (*reading == '\\') chrs |= translate_echr2bset(left, reading, true);
                    else for (; left <= *reading; ++left) chrs.set(left);
                    left = -1;
                    range = false;
                }
                else
                {
                    if (left != -1)  chrs.set(left);
                    if (*reading == '\\') chrs |= translate_echr2bset(left, reading, false);
                    else left = *reading;
                }
                ++reading;
            }

            if (left != -1) chrs.set(left);

            if (exclude) chrs.flip();

            return ::std::make_shared<BracketNode>(chrs);
        };

        gen_subexpr = [&](const char *&reading)
        {
            ::std::shared_ptr<Node> node = nullptr;
            if (*reading == '?')
            {
                ++reading;
                if (*reading == ':') ++reading;
                else printf("cre syntax error at pos %d: missing ':'\n", reading - begin_address);
                if (*reading == '<') ++reading;
                else printf("cre syntax error at pos %d: missing '<'\n", reading - begin_address);
                ::std::string name;
                while (isalnum(*reading) || *reading == '_') name += *reading++;
                if (*reading == '>') ++reading;
                else printf("cre syntax error at pos %d: missing '>'\n", reading - begin_address);
                if (*reading == ')')
                {
                    if (ref_map.count(name)) return ref_map[name];
                    else printf("cre syntax error at pos %d: can't find ref to %s\n", reading - begin_address, name.c_str());
                }
                else
                {
                    node = gen_node(reading);
                    ref_map[name] = node;
                }
            }
            else node = gen_node(reading);
            return node;
        };

        gen_node = [&](const char *&reading)
        {
            ::std::shared_ptr<Node> node = nullptr, right = nullptr;

            if (*reading == '^')
            {
                ++reading;
                if (begin) printf("cre syntax error at pos %d: '^' should be the begin of pattern string\n", reading - begin_address);
                begin = true;
            }

            if (*reading == '(')
            {
                ++reading;
                node = gen_subexpr(reading);
                if (*reading != ')') printf("cre syntax error at pos %d: missing ')'\n", reading - begin_address);
            }
            else if (*reading == '[')
            {
                ++reading;
                node = gen_bracket(reading);
                if (*reading != ']') printf("cre syntax error at pos %d: missing ']'\n", reading - begin_address);
            }
            else if (*reading == '.') node = ::std::make_shared<DotNode>();
            else if (*reading && *reading != '|' && *reading != ')') node = *reading == '\\' ? translate_echr2node(reading) : ::std::make_shared<LeafNode>(*reading);

            if (!node) return node;
            ++reading;

            while (*reading && *reading != '|' && *reading != ')' && *reading != '$')
            {
                switch (*reading)
                {
                case '(':
                    ++reading;
                    if (right) node = ::std::make_shared<CatNode>(node, right);
                    right = gen_subexpr(reading);
                    if (*reading != ')') printf("cre syntax error at pos %d: missing ')'\n", reading - begin_address);
                    break;
                case '[':
                    ++reading;
                    if (right) node = ::std::make_shared<CatNode>(node, right);
                    right = gen_bracket(reading);
                    if (*reading != ']') printf("cre syntax error at pos %d: missing ']'\n", reading - begin_address);
                    break;
                case '{':
                    ++reading;
                    if (isdigit(*reading))
                    {
                        int n = *reading - '0', m = -2;
                        ++reading;
                        if (*reading == ',')
                        {
                            ++reading;
                            m = isdigit(*reading) ? *reading++ - '0' : -1;
                        }

                        if (right) right = ::std::make_shared<QualifierNode>(right, n, m);
                        else node = ::std::make_shared<QualifierNode>(node, n, m);
                        if (*reading != '}') printf("cre syntax error at pos %d: missing '}'\n", reading - begin_address);
                    }
                    else printf("cre syntax error at pos %d: only '{' & no number after '{'\n", reading - begin_address);
                    break;
                case '*':
                    if (right) right = ::std::make_shared<ClosureNode>(right);
                    else node = ::std::make_shared<ClosureNode>(node);
                    break;
                case '+':
                    if (right) right = ::std::make_shared<QualifierNode>(right, 1, -1);
                    else node = ::std::make_shared<QualifierNode>(node, 1, -1);
                    break;
                case '?':
                    if (right) right = ::std::make_shared<QualifierNode>(right, 0, 1);
                    else node = ::std::make_shared<QualifierNode>(node, 0, 1);
                    break;
                case '.':
                    if (right) node = ::std::make_shared<CatNode>(node, right);
                    right = ::std::make_shared<DotNode>();
                    break;
                default:
                    if (right) node = ::std::make_shared<CatNode>(node, right);
                    right = *reading == '\\' ? translate_echr2node(reading) : ::std::make_shared<LeafNode>(*reading);
                    break;
                }
                ++reading;
            }

            if (*reading == '|')
            {
                ++reading;
                if (right) node = ::std::make_shared<CatNode>(node, right);
                node = ::std::make_shared<SelectNode>(node, gen_node(reading));
            }
            else if (right) node = ::std::make_shared<CatNode>(node, right);

            if (*reading == '$')
            {
                ++reading;
                if (end) printf("cre syntax error at pos %d: '$' should be the end of the pattern string\n", reading - begin_address);
                end = true;
            }
            return node;
        };

        auto node = gen_node(reading);
        if (!node) dfa = ::std::make_shared<DFAState>(DFAState::StateType::END);
        else dfa = node->compile()->to_dfa();
        return ::std::make_tuple(dfa, begin, end);
    }


    class Pattern
    {
    private:

        void cal_next()
        {
            if (!dfa->to.size()) return;

            ::std::set<::std::shared_ptr<DFAState>> caled = {dfa};
            ::std::vector<::std::shared_ptr<DFAState>> states;

            for (auto it: dfa->to) if (!caled.count(it.second))
            {
                next[it.second] = dfa;
                caled.insert(it.second);
                states.push_back(it.second);
            }

            while (states.size())
            {
                for (auto state: states) for (auto it: state->to) if (!caled.count(it.second))
                {
                    auto _s = state;
                    while (_s != dfa)
                    {
                        if (next[_s]->to.count(it.first))
                        {
                            next[it.second] = _s;
                            break;
                        }
                        else _s = next[_s];
                    }
                    if (!next.count(it.second)) next[it.second] = dfa;
                }

                ::std::vector<::std::shared_ptr<DFAState>> _ss;
                for (auto state: states) for (auto it: state->to) if (!caled.count(it.second))
                {
                    _ss.push_back(it.second);
                    caled.insert(it.second);
                }
                states = _ss;
            }
        }

        ::std::shared_ptr<DFAState> dfa;
        ::std::unordered_map<::std::shared_ptr<DFAState>, ::std::shared_ptr<DFAState>> next;
        bool begin, end;

    public:

        Pattern(const ::std::string pattern)
        {
            ::std::tie(dfa, begin, end) = gen_dfa(pattern.c_str());
            cal_next();
        }


        ::std::string match(const ::std::string &str)
        {
            ::std::string res, temp;

            auto reading = str.c_str();
            auto state = dfa;

            while (*reading)
            {
                if (state->to.count(*reading)) state = state->to[*reading];
                else if (end) return "";
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

        ::std::string search(const ::std::string &str)
        {
            if (begin) return match(str);

            ::std::unordered_map<::std::shared_ptr<DFAState>, ::std::string> mapstr = {{dfa, ""}};
            ::std::string res, temp;

            auto reading = str.c_str();
            auto state = dfa;

            while (*reading)
            {
                if (state->to.count(*reading))
                {
                    state = state->to[*reading];
                    mapstr[state] = (temp += *reading);
                    if (state->state_type == DFAState::StateType::END) res = str.substr(reading - str.c_str() - temp.size() + 1, temp.size());
                }
                else if (!end && res.size()) return res;
                else if (next.count(state))
                {
                    state = next[state];
                    temp = mapstr[state];
                    continue;
                }
                else mapstr[state] = temp = "";
                ++reading;
            }
            return end ? temp : res;
        }

        ::std::string replace(const ::std::string &str, const ::std::string &target)
        {
            if (begin) return target + str.substr(match(str).size());

            ::std::unordered_map<::std::shared_ptr<DFAState>, ::std::string> mapstr = {{dfa, ""}};
            ::std::string ret, res, temp;

            auto reading = str.c_str();
            auto state = dfa;

            while (*reading)
            {
                if (state->to.count(*reading))
                {
                    state = state->to[*reading];
                    mapstr[state] = (temp += *reading);
                    if (state->state_type == DFAState::StateType::END) res = str.substr(reading - str.c_str() - temp.size() + 1, temp.size());
                }
                else if (!end && res.size())
                {
                    ret += target + temp.substr(res.size());
                    state = dfa;
                    res = temp = "";
                    continue;
                }
                else if (next.count(state))
                {
                    state = next[state];
                    if (mapstr[state].size() < temp.size()) ret += temp.substr(mapstr[state].size());
                    temp = mapstr[state];
                    continue;
                }
                else
                {
                    mapstr[state] = temp = "";
                    ret += *reading;
                }
                ++reading;
            }

            if (res.size()) ret += target + temp.substr(res.size());

            return ret;
        }

        ::std::vector<::std::string> matches(const ::std::string &str)
        {
            ::std::vector<::std::string> ret;

            ::std::string res, temp;

            auto reading = str.c_str();
            auto state = dfa;

            while (*reading)
            {
                if (state->to.count(*reading)) state = state->to[*reading];
                else if (end) return {};
                else
                {
                    ret.push_back(res);
                    res = temp = "";
                }

                temp += *reading;

                if (state->state_type == DFAState::StateType::END)
                {
                    res += temp;
                    temp = "";
                }

                ++reading;
            }

            return ret;
        }

    };


    inline ::std::string match(const ::std::string &pattern, const ::std::string &str)
    {
        return Pattern(pattern).match(str);
    }

    inline ::std::string search(const ::std::string &pattern, const ::std::string &str)
    {
        return Pattern(pattern).search(str);
    }

    inline ::std::string replace(const ::std::string &pattern, const ::std::string &str, const ::std::string &target)
    {
        return Pattern(pattern).replace(str, target);
    }

    inline ::std::vector<::std::string> matches(const ::std::string &str)
    {
        return Pattern(pattern).matches(str);
    }

}
