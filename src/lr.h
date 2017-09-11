#pragma once
#include <vector>
#include <set>
#include <queue>
using namespace std;
#include "re.h"

namespace lr{
  typedef size_t token_type;
  class graph_token_edge: public graph::edge{
  public:
    token_type token;
    graph_token_edge(size_t from, size_t to, token_type _tk):edge(from,to),token(_tk){}
    virtual edge* copy(){return new graph_token_edge(from, to, token);}
    virtual bool match(string, size_t&){return false;}
    virtual bool is_e_edge(){return false;}
    virtual bool eq_cond(edge*){return false;}
    virtual ostream& print(ostream& o){
      o<<"graph_token_edge "<<from<<" -["<<token<<"]-> "<<to;
      o.flush();
      return o;
    }
  };
  class token{
  public:
    token_type type;
    string text;
    token(token_type _type, string _text){ type=_type; text=_text;}
    token(){ type= numeric_limits<token_type>::max(); }
  };
  
  class token_type_set{
  public:
    vector<dfa_re::regex*> token_regexs;
    token_type next_type(){
      return token_regexs.size();
    }
    token_type add_token_type(){
      token_type ret= next_type();
      token_regexs.push_back(nullptr);
      return ret;
    }
    token_type add_token_type(string _regex){
      token_type ret= next_type();
      token_regexs.push_back(new dfa_re::regex(_regex));
      return ret;
    }
    bool is_terminal(token_type tk){
      return token_regexs[tk]!=nullptr;
    }
    size_t size(){return token_regexs.size();}
  };
  
  class rule{
  public:
    vector<token_type> cur_rule;
    rule(token_type l, token_type r1){
      cur_rule.push_back(l);
      cur_rule.push_back(r1);
    }
    rule(token_type l, token_type r1, token_type r2){
      cur_rule.push_back(l);
      cur_rule.push_back(r1);
      cur_rule.push_back(r2);
    }
    rule(token_type l, token_type r1, token_type r2, token_type r3){
      cur_rule.push_back(l);
      cur_rule.push_back(r1);
      cur_rule.push_back(r2);
      cur_rule.push_back(r3);
    }
    rule(token_type l, token_type r1, token_type r2, token_type r3, token_type r4){
      cur_rule.push_back(l);
      cur_rule.push_back(r1);
      cur_rule.push_back(r2);
      cur_rule.push_back(r3);
      cur_rule.push_back(r4);
    }
    token_type left(){return cur_rule[0];}
    token_type at(size_t i){return cur_rule[i+1];}
    size_t right_size(){return cur_rule.size()-1;}
  };
  
  class rule_set{
  public:
    vector<rule> rules;
    token_type_set& token_manager;
    rule_set(token_type_set& tokens):token_manager(tokens){}
    void add_rule(rule r){
      rules.push_back(r);
    }
    class rule_prefix{
    public:
      size_t rule_id;
      size_t token_id;
      rule_prefix(size_t _rid, size_t _tknid):rule_id(_rid),token_id(_tknid){}
      token_type cur_token_type(vector<rule>& rules){
	return rules[rule_id].at(token_id);
      }
      bool rule_end(vector<rule>& rules){
	return token_id >= rules[rule_id].right_size();
      }
      bool operator<(const rule_prefix& o)const{
	if(rule_id==o.rule_id) { return token_id < o.token_id; }
	return rule_id < o.rule_id;
      }
      bool operator==(const rule_prefix& o)const{
	return rule_id==o.rule_id && token_id==o.token_id;
      }
    };
    
    void rule_left_is(token_type tk, set<size_t>& ret){
      for(size_t i=0; i<rules.size(); i++){
	if(rules[i].left()==tk){
	  ret.insert(i);
	}
      }
    }

    void item_set_expand(set<rule_prefix>& s_io, token_type_set& tk_set){
      queue<token_type> non_terminal_q;
      for(rule_prefix i : s_io){
	if( !tk_set.is_terminal( i.cur_token_type(rules) ) ){
	  non_terminal_q.push(i.cur_token_type(rules));
	}
      }
      set<token_type> marked;
      while(!non_terminal_q.empty()){
	token_type cur_tk= non_terminal_q.front(); non_terminal_q.pop();
	
	if(marked.find(cur_tk)!=marked.end()){ continue; }
	marked.insert(cur_tk);
	
	set<size_t> r;
	rule_left_is(cur_tk, r);
	for(size_t i : r){
	  if(!tk_set.is_terminal(rules[i].at(0))){
	    non_terminal_q.push(rules[i].at(0));
	  }
	}
      }
      
      for(token_type i : marked){
	set<size_t> r;
	rule_left_is(i, r);
	
	for(size_t rule_id : r){
	  s_io.insert(rule_prefix(rule_id,0));
	}
      }
    }
    
    void merge_item_set(set<rule_prefix>& item_set, map<token_type, set<rule_prefix> >& ret){
      for(rule_prefix i : item_set){
	ret[i.cur_token_type(rules)].insert(rule_prefix(i.rule_id, i.token_id+1));
      }
    }

    size_t get_node(vector<set<rule_prefix> >& node_to_item_set, set<rule_prefix>& x){
      for(size_t i=0; i<node_to_item_set.size(); i++){
	if(node_to_item_set[i] == x){
	  return i;
	}
      }
      return node_to_item_set.size();
    }
    
    set<rule_prefix> s_set(token_type s){
      set<rule_prefix> ret;
      for(size_t i=0; i<rules.size(); i++){
	if(rules[i].left()==s){
	  ret.insert(rule_prefix(i,0));
	}
      }
      item_set_expand(ret, token_manager);
      return ret;
    }
    void gen_graph_line(vector<set<rule_prefix> >& node_to_item_set, size_t cur_line_index, graph::graph& g){
      set<rule_prefix> cur_line= node_to_item_set[cur_line_index];
      item_set_expand(cur_line, token_manager);
      map<token_type, set<rule_prefix> > next_tab;
      merge_item_set(cur_line, next_tab);
      for(auto& i : next_tab){
	item_set_expand(i.second, token_manager);
	size_t nid= get_node(node_to_item_set, i.second);
	if(nid==node_to_item_set.size()){
	  g.add_node();
	  
	  node_to_item_set.push_back(i.second); // node_to_item_set
	}
	g.add_edge(new graph_token_edge(cur_line_index, nid, i.first));
      }
    }
    bool has_rule_end(set<rule_prefix>& _sub_rules, size_t& rule_id){
      for(rule_prefix i : _sub_rules){
	if(i.rule_end(rules)){
	  rule_id= i.rule_id;
	  return true;
	}
      }
      return false;
    }
    void gen_graph(token_type s, graph::graph& g, vector<size_t>& rule_of_end_state){
      // merge_next() and test if end of rule
      rule_of_end_state.clear();
      vector<set<rule_prefix> > node_to_item_set;
      node_to_item_set.push_back(s_set(s));
      g.add_node();
      for(size_t i=0; i<node_to_item_set.size(); i++){
	size_t rule_end_id= numeric_limits<size_t>::max();
	if(has_rule_end(node_to_item_set[i], rule_end_id)){
	  rule_of_end_state.push_back(rule_end_id);
	}else{
	  rule_of_end_state.push_back(numeric_limits<size_t>::max());
	  gen_graph_line(node_to_item_set, i, g);
	}
      }
      (void)node_to_item_set;
    }
  };
  
  
  class lr{
  public:
    graph::graph& g;
    token_type begin_state;
    vector<size_t>& rule_of_end_state;
    vector<rule>& rules;
    lr(graph::graph& _g, vector<size_t>& _rule_of_end_state, vector<rule>& _rules)
    :g(_g), rule_of_end_state(_rule_of_end_state), rules(_rules){
      begin_state= 0;
    }
    void push_hook(token& _cur_token){
      cout<<"push: "<< _cur_token.type<<" "<<_cur_token.text<<endl;
    }
    void reduce_hook(vector<rule>& _rules, size_t rule_index){
      cout<<"reduce: rule"<<rule_index<<endl;
    }
    
    void parse_step_jmp(vector<token>& token_stack, vector<size_t>& state_stack){
      size_t cur_state= state_stack.back();
      vector<size_t>& tab= g.adj(cur_state);
      for(size_t i=0; i<tab.size(); i++){
	edge* cur_edge_= g.edges[tab[i]];
	graph_token_edge* cur_edge= dynamic_cast<graph_token_edge*>(cur_edge_);
	if(cur_edge->token==token_stack.back().type){
	  state_stack.push_back(cur_edge->to);
	  return;
	}
      }
      throw "unknow";
    }
    void prase_reduce(vector<token>& token_stack, vector<size_t>& state_stack){
      size_t cur_state= state_stack.back();
      size_t rule_index= rule_of_end_state[cur_state];
      rule& cur_rule= rules[rule_index];
      size_t right_size= cur_rule.right_size();
      reduce_hook(rules, rule_index);
      while(right_size-->0){
	state_stack.pop_back();
	token_stack.pop_back();
      }
      token_stack.push_back(token(cur_rule.left(),""));
      parse_step_jmp(token_stack, state_stack);
    }
    void parse_step_push(vector<token>& token_line, size_t& pos
    , vector<token>& token_stack, vector<size_t>& state_stack)
    {
      size_t cur_state= state_stack.back();
      if(rule_of_end_state[cur_state]!=numeric_limits<size_t>::max()){
	prase_reduce(token_stack, state_stack);
	return;
      }
      token& cur_token= token_line[pos++];
      push_hook(cur_token);
      token_stack.push_back(cur_token);
      parse_step_jmp(token_stack, state_stack);
    }
    size_t parse(vector<token>& token_line){
      size_t pos=0;
      size_t cur_state= begin_state;
      vector<token> token_stack;
      vector<size_t> state_stack;
      
      token_stack.push_back(token());//fake token
      state_stack.push_back(cur_state);
      
      while(!token_stack.empty()){
	if(pos==token_line.size()){
	  if(rule_of_end_state[state_stack.back()]==0){
	    return pos;
	  }
	}
	parse_step_push(token_line, pos, token_stack, state_stack);
#ifdef PL_DEBUG
	cout<<"state: "<< state_stack.back()<<endl;
#endif
      }
    }
  };
}