#ifndef _OPTIMIZE_H_
#define _OPTIMIZE_H_
Node *set_route(int board_[][HEIGHT], Node *route);
Node *set_list(Node *new, Node *old);
int eval(int board_[WIDTH][HEIGHT]);
void rec(int n, Pair pos, Node *route, int board_[][HEIGHT]);
void optimize(int n, Pair pos, int board_[][HEIGHT]);


#endif

