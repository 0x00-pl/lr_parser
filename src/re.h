#pragma once
#include "config.h"
#include "parser.h"
#include "nfa.h"
#include "dfa.h"
#include "graph.h"
#include <string>
using namespace std;

namespace dfa_re{
  
  class regex{
  public:
    graph::graph dfa;
    set<size_t> dfa_end_state_set;
    regex(string s){
      node* parsed= parser::parser_str(s);
      
      graph::graph nfa;
      set<size_t> nfa_end_state_set;
      nfa::build_nfa(nfa, parsed, nfa_end_state_set);
      
      dfa::build_dfa(nfa,nfa_end_state_set, dfa,dfa_end_state_set);
      
      dfa::dfa_merge_edge(dfa);
      
      delete parsed;
    }
    size_t match_step(size_t state, string& s, size_t& pos){
#ifdef PL_DEBUG
      cout<<"step "<<state<<" --> ";
#endif
      auto adj= dfa.adj(state);
      for(size_t i=0; i<adj.size(); i++){
	graph::edge* e= dfa.edges[adj[i]];
	if(dynamic_cast<graph::char_edge*>(e)!=nullptr){
	  graph::char_edge* ce= dynamic_cast<graph::char_edge*>(e);
	  if(ce->match(s,pos)){
#ifdef PL_DEBUG
      cout<<ce->to<<endl;
#endif
	    return ce->to;
	  }
	}else{
	  throw "unknow";
	}
      }
#ifdef PL_DEBUG
      cout<<"fail"<<endl;
#endif
      return dfa.sum_v();
    }
    bool match_to_end_state(string s, size_t& pos){
      size_t state=0;
      while(pos<s.size() && dfa_end_state_set.find(state)==dfa_end_state_set.end()){
	state= match_step(state, s, pos);
	if(state==dfa.sum_v()){return false;}
      }
      return dfa_end_state_set.find(state) != dfa_end_state_set.end();
    }
    size_t match_to_pos(string s, size_t& pos, size_t end_pos){
      size_t state=0;
      while(pos<s.size() && pos!=end_pos){
	state= match_step(state, s, pos);
	if(state==dfa.sum_v()){return false;}
      }
      return state;
    }
  };
}