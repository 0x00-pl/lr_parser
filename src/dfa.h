#pragma once
#include <set>
#include <map>
#include <algorithm>
#include "config.h"
#include "graph.h"
using namespace std;

namespace dfa{
  
  void merge_next_move(graph::graph& nfa, set<size_t>& node_set, map<pair<char,char>, set<size_t> >& next_move){
    next_move.clear();
    for(size_t i : node_set){
      for(size_t j : nfa.adj(i)){
	graph::char_edge* e= dynamic_cast<graph::char_edge*>(nfa.edges[j]);
	next_move[pair<char,char>(e->low,e->heigh)].insert(e->to);
      }
    }
  }
  
  size_t find_set_index(vector<set<size_t> >& tab, set<size_t>& val){
    for(size_t i=0; i<tab.size(); i++){
      if(tab[i]==val){return i;}
    }
    return tab.size();
  }
  
  void build_dfa(graph::graph& nfa, set<size_t>& nfa_end_state_set, graph::graph& dfa, set<size_t>& dfa_end_state_set){
    vector<set<size_t> > tab;
    map<pair<char,char>, set<size_t> > next_move;
    set<size_t> begin_state;
    begin_state.insert(0);
    tab.push_back(begin_state);
    dfa.add_node();
    
    for(size_t p=0; p<tab.size(); p++){  
      merge_next_move(nfa, tab[p], next_move);
      // for each next_move
      for(auto next_set_i= next_move.begin(); next_set_i!=next_move.end(); ++next_set_i){
        char low= next_set_i->first.first;
        char heigh= next_set_i->first.second;
	set<size_t>& next_set= next_set_i->second;
	// if next_set has not in table insert to table
	size_t next_set_index_of_tab= find_set_index(tab, next_set);
	if(next_set_index_of_tab==tab.size()){
	  tab.push_back(next_set);
	  dfa.add_node();
	  
	  // test and insert to dfa_end_state_set
	  vector<size_t> tmp;
	  set_intersection(next_set.begin(),next_set.end()
	  , nfa_end_state_set.begin(),nfa_end_state_set.end()
	  , back_inserter(tmp));
	  if(!tmp.empty()){
	    dfa_end_state_set.insert(tab.size()-1);
	  }
	}
	dfa.add_edge(new graph::char_edge(p, next_set_index_of_tab, low, heigh));
      }
    }
#ifdef PL_DEBUG
    //print tab
    cout<<"*** tab ***"<<endl;
    for(size_t i=0; i<tab.size(); i++){
      cout<<i<<" {";
      for(auto j:tab[i]){
	cout<<j<<" ";
      }
      cout<<"}"<<endl;
    }
    cout<<"------"<<endl;
    dfa.print(cout);
    cout<<"dfa-end-state: ";
    for(size_t i : dfa_end_state_set){
      cout<<i<<" ";
    }
    cout<<endl;
#endif
    //all table done
    if(tab.size()!=dfa.sum_v()){
      throw "unknow";
    }
  }

  void dfa_state_merge_edge(graph::graph& dfa, size_t node_id){
    vector<size_t>& adj= dfa.adj(node_id);
    sort(adj.begin(), adj.end(), [&](size_t a, size_t b){
      graph::char_edge* ea= dynamic_cast<graph::char_edge*>(dfa.edges[a]);
      graph::char_edge* eb= dynamic_cast<graph::char_edge*>(dfa.edges[b]);
      return ea->low < eb->low;
    });
    
    for(size_t i=1; i<adj.size();){
      graph::char_edge* em1= dynamic_cast<graph::char_edge*>(dfa.edges[adj[i-1]]);
      graph::char_edge* ecur= dynamic_cast<graph::char_edge*>(dfa.edges[adj[i]]);
      if(em1->to==ecur->to && em1->heigh+1==ecur->low){
	em1->heigh= ecur->heigh;
	dfa.remove_edge_from_node(adj[i]);
      }else{
	i++;
      }
    }
  }
  
  void dfa_merge_edge(graph::graph& dfa){
    for(size_t i=0; i<dfa.sum_v(); i++){
      dfa_state_merge_edge(dfa,i);
    }
#ifdef PL_DEBUG
    dfa.print(cout);
#endif
  }
  
}