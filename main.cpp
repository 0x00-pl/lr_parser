#include <iostream>
#include "src/lr.h"

using namespace lr;

int main(int argc, char **argv) {
    std::cout << "Hello, world!" << std::endl;
    
    token_type_set tks;
    token_type _a= tks.add_token_type("a");
    token_type _add= tks.add_token_type("+");
    token_type _mul= tks.add_token_type("*");
    token_type _l= tks.add_token_type("\\(");
    token_type _r= tks.add_token_type("\\)");
     
    token_type _S= tks.add_token_type();
    token_type _A= tks.add_token_type();
    token_type _M= tks.add_token_type();
    token_type _V= tks.add_token_type();
    
    lr::rule_set rst(tks);
//     rst.add_rule(lr::rule(_S,_a,_a,_a));
//     rst.add_rule(lr::rule(_S,_A));
//     rst.add_rule(lr::rule(_S,_A,_add,_S));
//     rst.add_rule(lr::rule(_A,_M));
//     rst.add_rule(lr::rule(_A,_M,_mul,_A));
    rst.add_rule(lr::rule(_M,_V));
    rst.add_rule(lr::rule(_V,_l,_M,_r));
    rst.add_rule(lr::rule(_V,_a));
    
    graph::graph g;
    vector<size_t> end_states;
    rst.gen_graph(_M, g, end_states);
    
    g.print(cout);
    
    vector<token> token_line;
    token_line.push_back(token(_l,"("));
    token_line.push_back(token(_a,"a"));
    token_line.push_back(token(_r,")"));
    
    lr::lr lr_parser(g, end_states, rst.rules);
    
    size_t res= lr_parser.parse(token_line);
    
    return 0;
}
