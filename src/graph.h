#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "config.h"
using namespace std;

namespace graph{
  
  class edge{
  public:
    size_t from;
    size_t to;
    edge(size_t _from, size_t _to){from=_from; to=_to;}
    virtual~edge(){}
    virtual edge* copy(){return new edge(from, to);}
    virtual bool match(string, size_t&){return true;}
    virtual bool is_e_edge(){return true;}
    virtual bool eq_cond(edge*){return false;}
    virtual ostream& print(ostream& o){
      o<<"edge "<<from<<" --> "<<to;
      o.flush();
      return o;
    }
  };
  
  class char_edge: public edge{
  public:
    char low;
    char heigh;
    char_edge(size_t from, size_t to, char _low, char _heigh):edge(from,to){
      low=_low;
      heigh=_heigh;
    }
    char_edge* copy(){return new char_edge(from, to, low, heigh);}
    bool match(string s, size_t& p){
      bool ret= ((unsigned char)low) <= ((unsigned char)s[p]) && ((unsigned char)s[p]) <= ((unsigned char)heigh);
      if(ret){p++;}
      return ret;
    }
    bool is_e_edge(){
      return false;
    }
    bool eq_cond(edge* e){
      char_edge* ce= dynamic_cast<char_edge*>(e);
      if(ce==nullptr){return false;}
      return ce->low==low && ce->heigh==heigh;
    }
    ostream& print(ostream& o){
      if(low=='\x00' && heigh=='\xff'){
	o<<"edge "<<from<<" -[.]> "<<to;
      }else{
	o<<"edge "<<from<<" -[";
	if(low=='\x00'){o<<"\\x00";}else{o<<low;}
	o<<"-";
	if(heigh=='\xff'){o<<"\\xff";}else{o<<heigh;}
	o<<"]> "<<to;
      }
      return o;
    }
  };
  
  class graph{
  public:
    vector<edge*> edges;
    vector<vector<size_t> > v_adj;
    graph(){}
    ~graph(){
      for(edge* i : edges){
	delete i;
      }
    }
    size_t add_node(){
      v_adj.push_back(vector<size_t>());
      return v_adj.size()-1;
    }
    void add_edge(edge* e){
      edges.push_back(e);
      v_adj[e->from].push_back(edges.size()-1);
    }
    void add_e_edge(size_t from, size_t to){
      add_edge(new edge(from,to));
    }
    void remove_edge_from_node(size_t edge_id){
      edge* e= edges[edge_id];
      vector<size_t>& old_node= v_adj[e->from];
      for(size_t i=0; i<old_node.size(); i++){
	if(old_node[i]==edge_id){
	  old_node.erase(old_node.begin()+i);
	  return;
	}
      }
    }
    void copy_edge_and_set_from(size_t edge_id, size_t from){
      edge* e= edges[edge_id]->copy();
      e->from= from;
      add_edge(e);
    }
    vector<size_t>& adj(size_t id){
      return v_adj[id];
    }
    size_t sum_v(){
      return v_adj.size();
    }
    ostream& print_node(ostream& o, size_t id){
      for(size_t i=0; i<adj(id).size(); i++){
	o<<" ";
	edges[adj(id)[i]]->print(o);
	o<<endl;
      }
      return o;
    }
    ostream& print(ostream& o){
      for(size_t i=0; i<sum_v(); i++){
	o<<"node "<<i<<endl;
	print_node(o,i);
	o<<endl;
      }
      return o;
    }
  };
  
}