#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#define HEIGHT 5
#define WIDTH 6
#define MAX_MOVE 12 //最大で進める回数

Node *set_route(int board_[][HEIGHT], Node *route)
//board_：盤上のデータ  route:ルートのリスト
//盤上のデータにルートを反映させる
{
	Pair now, next;
	Node *buf = route;
	int tmp;
	if (route == NULL) {
		return buf;
	} else
		now = *(route->pos);
	route = route->next;
	while (route != NULL) { //ルートの要素通りに1マスずつすすんでいった際の盤面の変化を再現する
		next = now;
		now = *(route->pos);
		tmp = board_[now.x][now.y];
		board_[now.x][now.y] = board_[next.x][next.y];
		board_[next.x][next.y] = tmp;
		route = route->next;
	}
	return buf;
}
Node *set_list(Node *new, Node *old)
//new:コピー先のノード //old:コピー元のノード
//newにoldのデータをコピーする
{
	if (old == NULL) {     //もしoldがNULLならnewをそのまま返す
		return new;
	} else
		new = push_back(new, old->pos);
	old = old->next;
	while (old != NULL) {     //oldの先端から末尾までループして、oldの要素をnewにプッシュしていく
		new = push_back(new, old->pos);     //順番を合わせるために後ろからプッシュしていく
		old = old->next;
	}
	return new;
}
int eval(int board_[WIDTH][HEIGHT]) {     //最大コンボ数を考える関数recの中で用いる
//引数として盤面を参照渡しされ、それをいじりながら、総コンボ数を返す
	int combo = 0;     //総コンボ数
	int new = 0;     //新たな消去があったか
	int board_buf[WIDTH][HEIGHT];
	copy_board(board_buf, board_);
	do {
		gravity(board_buf);     //下まで詰める
		new = search(&combo, board_buf, 1);
	} while (new != 0);     //newcomboが出てこなくなるまで
	return combo;
}
void rec(int n, Pair pos, Node *route, int board_[][HEIGHT]) {
	//n:再帰の残り回数　pos:次動く位置　route:今までの軌跡　board_:プレーヤー操作前の盤面データ
	//上下左右のマスで動けるものがあれば再帰を用いて計n回探索する
	assert(n >= 0);
	int board_buf[WIDTH][HEIGHT];	//参照渡しの連続が怖いので複製しておく。
	copy_board(board_buf, board_);
	if (n == 0) {	//n回動いたらそのルートを評価する
		route = set_route(board_buf, route);	//操作前の盤面にrouteを反映させる
		int value = eval(board_buf);	//移動後の盤面の評価値をvalueに代入
		if (max_value < value) { //今回の評価値が今までの中で最大だった場合
			max_value = value; //その評価値とルートを保存する。
			max_route = route;
		}
		return;
	}
	route = push_back(route, &pos); //ルートに次動く位置を追加
	//ここで上下左右4パターンのルートに分岐させるため、route_right,route_left,route_down,route_upと
	//言う４つのポインタを新たに作り、今までのrouteを反映させておく。
	Node *route_right = NULL;
	route_right = set_list(route_right, route);	//route_rightにrouteをコピーする
	if (is_legal_pos(set_pair(pos.x + 1, pos.y)))	//右に１マス進んだところが盤上なら再帰して探索
		rec(n - 1, set_pair(pos.x + 1, pos.y), route_right, board_buf);
	Node *route_left = NULL;	//以下同じ
	route_left = set_list(route_left, route);
	if (is_legal_pos(set_pair(pos.x - 1, pos.y)))
		rec(n - 1, set_pair(pos.x - 1, pos.y), route_left, board_buf);
	Node *route_down = NULL;
	route_down = set_list(route_down, route);
	if (is_legal_pos(set_pair(pos.x, pos.y + 1)))
		rec(n - 1, set_pair(pos.x, pos.y + 1), route_down, board_buf);
	Node *route_up = NULL;
	route_up = set_list(route_up, route);
	if (is_legal_pos(set_pair(pos.x, pos.y - 1)))
		rec(n - 1, set_pair(pos.x, pos.y - 1), route_up, board_buf);
}
void optimize(int n, Pair pos, int board_[][HEIGHT])
//探索数n　pos:動き初めの位置　board_:プレイヤー操作前の盤面データ
//再帰により探索を行い、最適解を表示する。
{
	Node *route = NULL;
	puts("初期状態");
	draw(board_,null_pos);//初期状態表示
	rec(n, pos, route, board_);	//探索を行う。
	puts("最適解のルートが見つかりました！");
	max_route = set_route(board_, max_route);//board_には最適解のルートを反映させておく
	do {	//最適解のルートを表示
		printf("%c%c\n", max_route->pos->x + 'a', max_route->pos->y + '1');
	} while ((max_route = max_route->next) != NULL);
	printf("その際のコンボ数:%dcombo!\n", max_value);	//最大値を表示
	puts("最適解通り移動した場合");
	draw(board_,null_pos);
	puts("コンボ詳細");
	calc_combo(board_);
}
