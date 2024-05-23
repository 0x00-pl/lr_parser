#include "parser.h"
#include "config.h"

using namespace parser;

parser::node* parser::node::visit(parser::node_visitor* v)
{
  return v->visit(this);
}

node* char_range::visit(node_visitor* v)
{
  return v->visit(this);
}

node* seq::visit(node_visitor* v)
{
  return v->visit(this);
}

node* capture::visit(node_visitor* v)
{
  return v->visit(this);
}

node* loop::visit(node_visitor* v)
{
  return v->visit(this);
}

size_t char2size_t(char ch){
  return size_t(ch)&((1<<8)-1);
}

node* parser_item(string& s, size_t& i);

char_range* parse_char(string& s, size_t& i){
  if(s[i]=='\\'){i++;}
  if(s[i]=='.'){i++; return new char_range(0,255);}
  if(s[i+1]=='-'){
    char lo= s[i];
    i+=2;
    char_range* hi= parse_char(s,i);
    hi->low=lo;
    return hi;
  }
  return new char_range(s[i++]);
}

capture* parser_capture(string& s, size_t& i){
  capture* ret= new capture;
  if(s[i]!='(') throw "unknow";
  i++;
  while(s[i]!=')'){
    if(s[i]=='\0') throw "unknow";
    if(s[i]=='|'){ret->add_seq();i++;continue;}
    ret->add_node(parser_item(s,i));
  }
  i++;
  return ret;
}

node* parser_item(string& s, size_t& i){
  node* ret=nullptr;
  switch(s[i]){
    case '(':
      ret= parser_capture(s,i);
      break;
    default:
      ret= parse_char(s,i);
  }
  if(s[i]=='*'){
    ret= new loop(ret);
    i++;
  }
  return ret;
}

class build_sub_set_tab: public node_visitor{
public:
  bool char_sub_set[256+1];
  build_sub_set_tab(){
    for(size_t i=0; i<257; i++){
      char_sub_set[i]=false;
    }
  }
  virtual node* visit(node* n){throw "unknow"; return n;}
  virtual node* visit(char_range* n){
    char_sub_set[char2size_t(n->low)]=true;
    char_sub_set[char2size_t(n->heigh)+1]=true;
    return n;
  }
  ostream& print(ostream& o){
    for(size_t i=0; i<257; i++){
      if(i%16==0) o<<endl;
      o<<char_sub_set[i];
    }
    o<<endl;
    return o;
  }
};

class build_copy_of_sub_set_char: public node_visitor{
public:
  bool* char_sub_set;
  build_copy_of_sub_set_char(bool* _char_sub_set){char_sub_set=_char_sub_set;}
  virtual node* visit(node* n){throw "unknow";return n;}
  virtual node* visit(char_range* n){
    bool cut=false;
    for(size_t i=char2size_t(n->low)+1; i<=char2size_t(n->heigh); i++){
      if(char_sub_set[i]){cut=true; break;}
    }
    if(!cut){
      return new char_range(n->low,n->heigh);
    }
    capture* ret= new capture;
    size_t lo=char2size_t(n->low);
    for(size_t i=char2size_t(n->low)+1; i<=char2size_t(n->heigh); i++){
      if(char_sub_set[i]){
	ret->add_node(new char_range(lo,i-1));
	ret->add_seq();
	lo=i;
      }
    }
    ret->add_node(new char_range(lo,n->heigh));
    return ret;
  }
  virtual node* visit(seq* n){
    seq* ret= new seq;
    for(size_t i=0; i<n->items.size(); i++){
      ret->items.push_back( n->items[i]->visit(this) );
    }
    return ret;
  }
  virtual node* visit(capture* n){
    capture* ret= new capture;
    ret->clear();
    for(size_t i=0; i<n->seqs.size(); i++){
      ret->seqs.push_back( dynamic_cast<seq*>(n->seqs[i]->visit(this)) );
    }
    return ret;
  }
  virtual node* visit(loop* n){
    loop* ret= new loop( n->loop_body->visit(this) );
    return ret;
  }
};


node* parser::parser_str(string s){
  s= string("(")+s+")";
  size_t i=0;
  node* ast= parser_capture(s,i);
  ast->print(cout);
  cout<<endl;
  build_sub_set_tab sstab;
  ast->visit(&sstab);
  //sstab.print(cout);
  build_copy_of_sub_set_char fix_overlay(sstab.char_sub_set);
  node* ret= ast->visit(&fix_overlay);
  delete ast;
  return ret;
}
  
  