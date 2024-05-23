#pragma once
#include <vector>
#include <map>
#include <set>
#include "config.h"
#include "graph.h"
#include "parser.h"
using namespace std;
using namespace graph;
using namespace parser;

namespace nfa{
  void build_nfa_node(graph::graph& nfa, size_t begin_state, size_t end_state, parser::node* root_node);
  
  void build_nfa_char_range(graph::graph& nfa, size_t begin_state, size_t end_state, parser::char_range* _node){
    nfa.add_edge(new graph::char_edge(begin_state, end_state, _node->low, _node->heigh));
  }
  void build_nfa_seq(graph::graph& nfa, size_t begin_state, size_t end_state, parser::seq* _node){
    if(_node->items.size()==1){
      build_nfa_node(nfa, begin_state, end_state, _node->items[0]);
      return;
    }
    for(size_t i=0; i<_node->items.size(); i++){
      size_t t= nfa.add_node();
      build_nfa_node(nfa, begin_state, t, _node->items[i]);
      begin_state= t;
    }
    nfa.add_e_edge(begin_state, end_state);
  }
  void build_nfa_capture(graph::graph& nfa, size_t begin_state, size_t end_state, parser::capture* _node){
    for(size_t i=0; i<_node->seqs.size(); i++){
      build_nfa_seq(nfa, begin_state, end_state, _node->seqs[i]);
    }
  }
  
  void build_nfa_loop(graph::graph& nfa, size_t begin_state, size_t end_state, parser::loop* _node){
    build_nfa_node(nfa, begin_state, end_state, _node->loop_body);
    nfa.add_e_edge(end_state, begin_state);
  }
  
  void build_nfa_node(graph::graph& nfa, size_t begin_state, size_t end_state, parser::node* root_node){
    if(dynamic_cast<parser::char_range*>(root_node)!=nullptr){
      build_nfa_char_range(nfa, begin_state, end_state, dynamic_cast<parser::char_range*>(root_node));
    }else if(dynamic_cast<parser::seq*>(root_node)!=nullptr){
      build_nfa_seq(nfa, begin_state, end_state, dynamic_cast<parser::seq*>(root_node));
    }else if(dynamic_cast<parser::capture*>(root_node)!=nullptr){
      build_nfa_capture(nfa, begin_state, end_state, dynamic_cast<parser::capture*>(root_node));
    }else if(dynamic_cast<parser::loop*>(root_node)!=nullptr){
      build_nfa_loop(nfa, begin_state, end_state, dynamic_cast<parser::loop*>(root_node));
    }else{
      throw "unknow";
    }
  }
  
  bool nfa_node_has_e(vector<edge*>& edges, vector<size_t>&  node){
    for(size_t i=0; i<node.size(); i++){
      if(edges[node[i]]->is_e_edge()){
	return true;
      }
    }
    return false;
  }
  
  void nfa_node_copy_all_edge(graph::graph& nfa, size_t src, size_t dst){
    vector<size_t>& src_node= nfa.adj(src);
    for(size_t i=0; i<src_node.size(); i++){
      nfa.copy_edge_and_set_from(src_node[i],dst);
    }
  }
  
  bool is_end_state(size_t id, set<size_t>& end_state_set){
    return end_state_set.find(id) != end_state_set.end();
  }
  
  void nfa_node_remove_e(graph::graph& nfa, size_t id, set<size_t>& end_state_set){
    vector<size_t>& cur_node= nfa.adj(id);
    if(!nfa_node_has_e(nfa.edges, cur_node)){return;}
    
    for(size_t i=0; i<cur_node.size();){
      edge* cur_edge= nfa.edges[cur_node[i]];
      if(cur_edge->is_e_edge()){
	nfa_node_remove_e(nfa, cur_edge->to, end_state_set);
	if(is_end_state(cur_edge->to,end_state_set)){end_state_set.insert(id);}
	nfa.remove_edge_from_node(cur_node[i]); // remove e edge
	nfa_node_copy_all_edge(nfa, cur_edge->to, id);
      }else{
	i++; // nothing has removed
      }
    }
    
  }
  void nfa_remove_e(graph::graph& nfa, set<size_t>& end_state_set){
    for(size_t i=0; i<nfa.sum_v(); i++){
      nfa_node_remove_e(nfa, i, end_state_set);
    }
  }
  
  
  void build_nfa(graph::graph& nfa, parser::node* root_node, set<size_t>& end_state_set){
    size_t begin_state= nfa.add_node();
    size_t end_state= nfa.add_node();
    build_nfa_node(nfa, begin_state, end_state, root_node);
#ifdef PL_DEBUG
    cout<<"# nfa with e"<<endl;
    nfa.print(cout);
    cout<<"******"<<endl;
#endif
    end_state_set.insert(end_state);
    nfa_remove_e(nfa, end_state_set);
#ifdef PL_DEBUG
    cout<<"# nfa without e"<<endl;
    nfa.print(cout);
    cout<<"# nfa-end-state: ";
    for(size_t i : end_state_set){
      cout<<i<<" ";
    }
    cout<<endl;
    cout<<"******"<<endl;
#endif
  }
}