//ここら辺ヘッダにしとく
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "optimize.h"
#define HEIGHT 5
#define WIDTH 6
#define MAX_MOVE 12 //最大で進める回数
enum color {
	RD, BL, GR, YE, PU, HT
};
typedef struct pair {
	int x;
	int y;
} Pair;
Pair set_pair(int x, int y) {
	Pair p;
	p.x = x;
	p.y = y;
	return p;
}
const Pair null_pos = {-1,-1};//盤面外を表す定数。（初期値）
int max_value = -1; //負の数で初期化
//ここまでmain.hに入れる
//ここからlist.c
struct node {
	Pair *pos;
	struct node *next;
};
typedef struct node Node;
Node *max_route; //探索した中で最も評価の高かったルート
Node *push_front(Node *begin, const Pair *pos) {
	Node *p = malloc(sizeof(Node));
	Pair *pos_ = malloc(sizeof(Pair));
	pos_->x = pos->x;
	pos_->y = pos->y;
	p->pos = pos_;
	p->next = begin;
	return p;
}
Node *pop_front(Node *begin) {
	Node *p = begin->next;
	free(begin->pos);
	free(begin);
	return p;
}
Node *pop_back(Node *begin) {
	if (begin == NULL) {
		return begin;     // 何もしない
	}
	Node *p = begin, *q;
	if (p->next == NULL) return pop_front(begin);
	while (p->next->next != NULL) {
		p = p->next;
	}
	q = p;
	q = q->next;
	free(q->pos);
	free(q);
	p->next = NULL;
	return begin;
}
Node *push_back(Node *begin, const Pair *pos) {
	if (begin == NULL) {
		return push_front(begin, pos);
	}
	Node *p = begin;
	while (p->next != NULL) {
		p = p->next;
	}
	Node *q = malloc(sizeof(Node));
	Pair *pos_ = malloc(sizeof(Pair));
	pos_->x = pos->x;
	pos_->y = pos->y;
	q->pos = pos_;
	q->next = NULL;
	p->next = q;
	return begin;
}
int lenlist(Node *p)     //リストの長さを返す関数
{
	int count = 0;
	for (; p != NULL; p = p->next) {     //次のノードがNULLに行き着くまでループ回す
		count++;
	}
	return count;
}
//ここまでlist
void init(int board_[][HEIGHT]) {
	//board_:盤面のデータ
	//与えられた盤面データを初期化する
	srand((unsigned) time(NULL));
	int x, y;
	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			board_[x][y] = rand() % 6;	//6色をランダムに入れる
		}
	}
}
void draw(int board_[][HEIGHT],Pair next_pos)
//board_:盤面のデータ next_pos:次動く位置（色を変えるため必要）
//現在のboardの中身を描く。
{
	int x, y;
	puts("\n  a b c d e f");
	for (y = 0; y < HEIGHT; y++) {
		printf("%d ", y + 1);
		for (x = 0; x < WIDTH; x++) {
			int d = board_[x][y];
			if (next_pos.x == x && next_pos.y == y) printf("\x1b[46m"); /* 背景色をシアンに */
			//Mac上では文字の色を変更することが出来る。
			//参照：http://www.serendip.ws/archives/4635
			char c = '-';
			if (d == RD) {
				printf("\x1b[31m");
				c = 'R';
			}
			if (d == BL) {
				printf("\x1b[34m");
				c = 'B';
			}
			if (d == GR) {
				printf("\x1b[32m");
				c = 'G';
			}
			if (d == YE) {
				printf("\x1b[33m");
				c = 'Y';
			}
			if (d == PU) {
				printf("\x1b[35m");
				c = 'P';
			}
			if (d == HT) c = 'H';
			printf("%c ", c);
			printf("\x1b[39m");
			printf("\x1b[49m");				//背景色をデフォルトに戻す

//			char c = '-';
//			if (d == 0) c = 'R';
//			if (d == 1) c = 'B';
//			if (d == 2) c = 'G';
//			if (d == 3) c = 'Y';
//			if (d == 4) c = 'P';
//			if (d == 5) c = 'H';
//			printf("%c ", c);
		}
		putchar('\n');
	}
	putchar('\n');
}
int is_legal_pos(const Pair pos) {
//posがボードの上を指しているかを判定。正しい位置の場合、真（１）を返す
	if (pos.x < 0 || pos.x >= WIDTH) return 0;
	if (pos.y < 0 || pos.y >= HEIGHT) return 0;
	return 1;
}
int is_legal_move(Pair now_pos, Pair next_pos) {
//posが進める位置を指しているかを判定。進める位置の場合、真（１）を返す。
	int i;
	if (!is_legal_pos(next_pos)) return 0; //ボード上にいない場合即座に０を返す
	for (i = 0; i < 8; i++) {
		if (now_pos.x + 1 == next_pos.x && now_pos.y == next_pos.y) return 1;
		if (now_pos.x - 1 == next_pos.x && now_pos.y == next_pos.y) return 1;
		if (now_pos.x == next_pos.x && now_pos.y + 1 == next_pos.y) return 1;
		if (now_pos.x == next_pos.x && now_pos.y - 1 == next_pos.y) return 1;
		//上下左右1マス分隣なら1を返す
	}
	return 0; //それ以外は正しい位置ではない
}
//ここからcombo.cとかでも作る？
int search(int *combo, int board_[][HEIGHT], int is_draw) { //この盤面配置において、
//消去出来るところ（縦or横に3つ以上連続するとき、消去。ぷよぷよ的な）を消去するための関数。
//消去したら、n combo!という表示とともに消された盤面も表示する
//board;思考の対象となる盤面　combo:この盤面での可能消去数。参照渡し。
//is_draw:盤面をいちいちprintするかを判断するための変数。
//この関数が異なるふたつの関数から呼び出されるため
//これが0の場合drawする。
	int i, x, y;
	int flag = 0;
	for (y = 0; y < HEIGHT; y++) {     //横探索
		for (x = 0; x < WIDTH - 2; x++) {     //探索のスタート点を指定
			int count = 1;     //(連続数のカウント用)
			if (board_[x][y] < 0) {
				continue;
			}
			int color = board_[x][y];     //色を記憶
			for (i = x + 1; i < WIDTH; i++) {
				if (board_[i][y] != color) {     //違う色なら
					break;
				}
				count++;
			}
			if (count >= 3) {     //消去の処理
				(*combo)++;
				for (i = 0; i < count; i++) {
					board_[x + i][y] = -1;
				}
				if (is_draw == 0) {
					draw(board_,null_pos);
					printf("%d combo!\n", (*combo));
				}
				flag = 1;
			}
		}
	}
	for (x = 0; x < WIDTH; x++) {     //縦探索
		for (y = 0; y < HEIGHT - 2; y++) {     //探索のスタート点を指定
			int count = 1;     //(連続数のカウント用)
			if (board_[x][y] < 0) {
				continue;
			}
			int color = board_[x][y];     //色を記憶
			for (i = y + 1; i < HEIGHT; i++) {
				if (board_[x][i] != color) {     //違う色なら
					break;
				}
				count++;
			}
			if (count >= 3) {     //消去の処理
				(*combo)++;
				for (i = 0; i < count; i++) {
					board_[x][y + i] = -1;
				}
				if (is_draw == 0) {
					draw(board_,null_pos);
					printf("%d combo!\n", (*combo));
				}
				flag = 1;
			}
		}
	}
	return flag;
}
void gravity(int board_[][HEIGHT]) {     //消去があった盤面を引数に取って、あたかも重力をかけるかのようにして、
//したへ詰める処理をする関数。
//連鎖完了後、下まで詰める。
	int x, y, z;     //
	int flag, count;
	for (x = 0; x < WIDTH; x++) {     //列の指定
		count = 0;     //その行の空なマスを数える
		for (y = 0; y < HEIGHT; y++) {
			if (board_[x][y] < 0) {
				count++;
			}
		}
		do {
			flag = 0;
			for (y = HEIGHT - 1; y > 0; y--) {     //y=1~4の各座標で、チェック
				if (board_[x][y] < 0) {     //無いとき
					for (z = y; z > 0; z--) {
						board_[x][z] = board_[x][z - 1];
					}
					board_[x][0] = -1;     //一番上は空白
				}
			}
			for (y = 0; y < count; y++) {
				if (board_[x][y] >= 0) {     //マスに駒があるなら
					flag = 1;     //
				}
			}
		} while (flag == 1);     //厳密な処理が思い浮かばなかったので。
	}
	return;     //何も動かなくなったら
}
void copy_board(int new[][HEIGHT], int old[][HEIGHT]) {
	//6*5のint型new配列にoldをコピーする。maxrouteを考えるときの盤面の保存用
	int x, y;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y < HEIGHT; y++) {
			new[x][y] = old[x][y];
		}
	}
}
void calc_combo(int board_[][HEIGHT]) {     //盤面を引数にとり、（参照渡しであることに注意）
//その盤面を実際に動かして連鎖数を計算する。
//内部では、searchにより連鎖発生の有無を考え、それがあるときには、盤面を下に詰めるgravityを行う。
//最後に、全操作の終了した盤面を再度表示して終わる。
	int combo = 0;     //総コンボ数
	int new = 0;     //新たな消去があったか
	int board_buf[WIDTH][HEIGHT];
	copy_board(board_buf, board_);
	do {
		gravity(board_buf);     //下まで詰める
		new = search(&combo, board_buf, 0);
	} while (new != 0);     //newcomboが出てこなくなるまで
	draw(board_buf,null_pos);
	return;
}
//ここまでコンボ
int main(void) {
	int board[WIDTH][HEIGHT];	//盤面を宣言
	init(board); //boardを初期化
	draw(board,null_pos); //初期化されたboardを描写
//ここからplayerの操作
	int len; //historyの長さを格納しておく
	int break_flag = 0; //最大回数まで操作する前に操作を中断する時1となるフラグ
	char buf[1000]; //入力文字を受け付けるバッファ
	Node *history = NULL; //プレイヤーが選択したルートを保持しておくリスト
	//history自身はそのリストの最初(一番最近の変更)を指す。
	int board_init[WIDTH][HEIGHT]; //プレイヤー操作前の盤面を入れておく
	copy_board(board_init, board);
	Pair init_pos; //初期位置の記憶
	Pair now_pos=null_pos;
	Pair next_pos=null_pos; //前回の位置と次進む位置を宣言かつ-1(盤面外）で初期化
	puts("開始する位置を入力してください 例：c2"); //開始位置を取得
	while (1) {
		scanf("%s", buf); //bufに文字列を取得して入れる
		next_pos.x = buf[0] - 'a'; //1文字目の英字に対応する数字を求める
		next_pos.y = buf[1] - '1'; //2文字目の数字の文字に対応する数字を求める
		if (is_legal_pos(next_pos)) break; //legalな位置だった場合は入力しなおす
		puts("そこは選択出来ません！");
	}
	init_pos.x = next_pos.x;
	init_pos.y = next_pos.y;
	while ((len = lenlist(history)) <= MAX_MOVE) { //MAX_MOVE回より多く動こうとするとループを抜ける
		draw(board,next_pos);
		history = push_front(history, &next_pos);		//選択したルートをリストを用いて記憶しておく
		len++;		//上の行でリストに１つ追加するので長さをインクリメントして修正
		now_pos = next_pos;	//現在の位置をnow_posに記憶しておく
		printf("次の位置を入力してください 現在地：%c%c 回数：%d\n", now_pos.x + 'a',
				now_pos.y + '1', len);
		puts("移動を終了する場合は0を、やり直す場合は1を入力してください");
//ここから文字入力受け付け
		while (1) {
			break_flag = 0;		//ループごとにflagを初期化
			scanf("%s", buf);		//bufに受け取った文字を入れる
			//「途中で終了する」が選択された場合
			if (buf[0] == '0') {
				break_flag = 1;		//flagを1にしてループ抜ける
				puts("移動を終了します");
				break;
			} else if (buf[0] == '1') {
				//やり直しが選択された場合
				if (history->next == NULL) { //一番最初の選択まで戻ることは出来ない。
					puts("これ以上は戻れません");
				} else {
					//nextに２回前に選択した位置を入れておくと、次のループの序盤にそれがnow_posに格納される
					now_pos = *(history->pos);
					history = pop_front(history);
					next_pos = *(history->pos);
					history = pop_front(history);	//これがないと2回前の選択肢が２重で格納される。
					puts("やり直します");
					break;
				}
			} else {
				//終了でもやり直しでもない場合（場所を入力する場合）
				next_pos.x = buf[0] - 'a';
				next_pos.y = buf[1] - '1';
				if (is_legal_move(now_pos, next_pos)) break;//選択可能な場所を選択していた場合ループ抜ける
				puts("そこは選択出来ません！");
			}
		}
//文字取得終了
		if (break_flag == 1)		//break_flagが立っていた場合プレイヤーの操作は終了
			break;
		int tmp;		//ここでtmpを介してnowとnextの中身を入れ替える
		tmp = board[next_pos.x][next_pos.y];
		board[next_pos.x][next_pos.y] = board[now_pos.x][now_pos.y];
		board[now_pos.x][now_pos.y] = tmp;
	}
//プレイヤーの操作終了
//ここからコンボの計算、結果表示に入る
	calc_combo(board);
	puts("プレイヤーの操作終了");
//ここから思考エンジン
	printf("-----------最適解探索---------\n\n");
	optimize(10, init_pos, board_init);//10回探索して結果を表示
	return 0;
}
//nowposがグローバルでかぶっている。
//戻るのを省く
//同じとこが出るのを省く
//assert入れる
//分割コンパイル
//posをポインタにする
//読みやすいソースとは
//freeは放置
