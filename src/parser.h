#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "config.h"
using namespace std;

namespace parser{
  
  class node_visitor;
  
  class node{
  public:
    virtual ~node(){}
    virtual ostream& print(ostream& o){o<<"<node>";return o;}
    virtual node* visit(node_visitor*);
  };

  class char_range: public node{
  public:
    char low;
    char heigh;
    char_range(char c){low=heigh=c;}
    char_range(char lo, char hi){low=lo;heigh=hi;}
    ostream& print(ostream& o){
      if(low==heigh){
	o<<low;
      }else if(low=='\0' && heigh==char(255)){
	o<<".";
      }else{
	if(low=='\0'){o<<"\\x00";}else{o<<low;}
	o<<"-";
	if(heigh=='\xff'){o<<"\\xff";}else{o<<heigh;}
      }
      return o;
    }
    node* visit(node_visitor*);
  };

  class seq: public node{
  public:
    vector<node*> items;
    ~seq(){
      for(auto i : items){
	delete i;
      }
    }
    ostream& print(ostream& o){
      for(size_t i=0; i<items.size(); i++){
	items[i]->print(o);
      }
      return o;
    }
    node* visit(node_visitor*);
  };
  
  class capture: public node{
  public:
    vector<seq*> seqs;
    string name;
    capture(){add_seq();}
    ~capture(){
      clear();
    }
    void clear(){
      for(auto i : seqs){
	delete i;
      }
      seqs.clear();
    }
    void add_node(node*n){seqs.back()->items.push_back(n);}
    void add_seq(){
      if(seqs.empty()){
	seqs.push_back(new seq);
      }else if(!seqs.back()->items.empty()){
	seqs.push_back(new seq);
      }
    }
    ostream& print(ostream& o){
      o<<"(";
      for(size_t i=0; i<seqs.size(); i++){
	if(i!=0){o<<"|";}
	seqs[i]->print(o);
      }
      o<<")";
      return o;
    }
    node* visit(node_visitor*);
  };
  
  class loop: public node{
  public:
    node* loop_body;
    loop(node* _loop_body){
      loop_body=_loop_body;
    }
    ~loop(){delete loop_body;}
    ostream& print(ostream& o){
      loop_body->print(o);
      o<<"*";
      return o;
    }
    node* visit(node_visitor*);
  };
  
  
  class node_visitor{
  public:
    virtual~node_visitor(){}
    virtual node* visit(node* n){return n;}
    virtual node* visit(char_range* n){return n;}
    virtual node* visit(seq* n){
      for(size_t i=0; i<n->items.size(); i++){
	n->items[i]->visit(this);
      }
      return n;
    }
    virtual node* visit(capture* n){
      for(size_t i=0; i<n->seqs.size(); i++){
	n->seqs[i]->visit(this);
      }
      return n;
    }
    virtual node* visit(loop* n){
      n->loop_body->visit(this);
      return n;
    }
  };
  
  node* parser_str(string s);
  
}