//-------------------------------------------------
//  ヘッダーファイル挿入
//-------------------------------------------------
#include <math.h>
#include "DxLib.h"

#define WINDOW_WIDTH (1600)
#define WINDOW_HEIGHT (900)

//パッドの情報を列挙型で取得
enum e_But
{
	But_Dow,		//1bit
	But_Lef,		//2bit
	But_Rig,		//3bit
	But_Up,		//4bit
	But_A,		//5bit
	But_B,		//6bit
	But_X,		//7bit
	But_Y,		//8bit
	But_L,		//9bit
	But_R,		//10bit
	But_Sel,		//11bit
	But_Sta,		//12bit
	But_L3,		//13bit
	But_R3,		//14bit
	But_Total
};

//パッドのビット情報
int Pad_Inp;

//パッドの押したカウント数
int Pad_Cou[e_But::But_Total];

//パッドの入力情報を取得する関数
bool Pad_Read()
{
	Pad_Inp = GetJoypadInputState(DX_INPUT_PAD1);

	for (int b = 0; b < e_But::But_Total; b = b + 1)
	{
		if (Pad_Inp & (1 << b))
		{
			Pad_Cou[b] = Pad_Cou[b] + 1;
		}
		else Pad_Cou[b] = 0;
	}
	return FALSE;
}


//カメラ座標
int Camera_x;

//ブロックタイプを列挙体で定義
enum e_Type
{
	e_空,
	e_地面,
	e_壊れる,
	e_固い,
	e_土管,
	e_ハテナ,
	e_ゴール,
	e_クリボー,
	e_ノコノコ,
	e_マリオ,
};

//写真の構造体定義
struct Pic
{
	int p;			//写真ハンドル
	int w;			//写真幅
	int t;			//写真厚み
};

//画像の状態列挙体定義
enum e_Ani
{
	e_Normal,
	e_Walk1,
	e_Walk2,
	e_Jump,
	e_End,
	e_Ani_Total,
};

//長方形の構造体定義
struct Ract
{
	int x;										//x座標
	int y;										//y座標
	int w;										//幅
	int t;										//厚み
	int k;										//種類
	double r;									//回転角度
	Pic p[e_Ani::e_Ani_Total];						//写真
	int a;										//アニメーション
	bool d;										//向き
	int j;										//ジャンプカウンタ
	bool c;										//キャラクターか否か（動くか否か）
};

//テキストファイルからブロック情報を読み取るための構造体定義
struct Text
{
	int k;			//種類
	int x;			//x座標
	int y;			//y座標
};

//テキストファイルからブロック情報を読み取るための変数定義
//（2次元配列）
Text** Stage;

//ステージの最大値
int STAGE_TOTAL;
int STAGE_WIDTH;
int STAGE_HEIGHT;

//背景色の構造体定義
struct Color
{
	int r;		//緑成分
	int g;		//赤成分
	int b;		//青成分
};

//背景色定義
Color Col;

//ブロック定義
//（1次元配列）
Ract* Block;

//マリオのアドレス
Ract* Mar;

//ブロック初期化
void Block_Ini(Ract* b)
{
	b->x = 0;							//x座標
	b->y = 0;							//y座標
	b->w = 0;							//幅
	b->t = e_Type::e_空;					//厚み
	b->k = 0;							//種類
	b->r = 0;							//回転角度

	for (int i = 0; i < e_Ani::e_Ani_Total; i = i + 1)
	{
		b->p[i].p = 0;				//写真ハンドル
		b->p[i].w = 0;				//写真の幅
		b->p[i].t = 0;				//写真の厚み
	}

	b->a = e_Ani::e_Normal;			//アニメーション
	b->d = FALSE;						//向き
	b->j = 0;						//ジャンプカウンタ
	b->c = FALSE;						//キャラクターか否か(動くか否か)
}

//ジャンプ状態の列挙体定義
enum e_Jump
{
	地面に着地,
	ジャンプ中,
	落下中,
};

//ジャンプフラグ
int Jump;

//ジャンプ開始時のy座標記録
int Jump_Ymem;

//カウンタ
int Cou;

//ゲーム開始時の処理を行う関数
void Game_Ini()
{
	//.txtからステージ情報を取得
	char name[] = { "1-1.txt" };
	FILE* fp = NULL;
	errno_t error = fopen_s(&fp, name, "r");

	//ステージ情報を取得する
	if (error == NULL &&
		fp != NULL)
	{
		//背景色取得
		fscanf_s(fp, "%d\n", &Col.r);
		fscanf_s(fp, "%d\n", &Col.g);
		fscanf_s(fp, "%d\n", &Col.b);
		SetBackgroundColor(Col.r, Col.g, Col.b);

		//ステージの幅×厚み・幅・厚み取得
		fscanf_s(fp, "%d\n", &STAGE_TOTAL);
		fscanf_s(fp, "%d\n", &STAGE_WIDTH);
		fscanf_s(fp, "%d\n", &STAGE_HEIGHT);

		//メモリ確保（動的配列）
		Stage = new Text * [STAGE_WIDTH];
		for (int i = 0; i < STAGE_WIDTH; i = i + 1)
		{
			Stage[i] = new Text[STAGE_HEIGHT];
		}

		//ブロックのx番号・y番号・種類読み込み
		for (int y = 0; y < STAGE_HEIGHT; y = y + 1)
		{
			for (int x = 0; x < STAGE_WIDTH; x = x + 1)
			{
				fscanf_s(fp, "%d", &Stage[x][y].k);
				Stage[x][y].x = x;
				Stage[x][y].y = y;
			}
		}
		//ファイル閉じる
		fclose(fp);
	}

	//メモリ確保
	Block = new Ract[STAGE_TOTAL];

	//Text型からRact型に変換し、txtの情報をBlock[]に格納
	int i = 0;
	for (int y = 0; y < STAGE_HEIGHT; y = y + 1)
	{
		for (int x = 0; x < STAGE_WIDTH; x = x + 1)
		{
			if (Stage[x][y].k == e_Type::e_空)		//ブロックなしの場合
			{
				//ブロック初期化
				Block_Ini(&Block[i]);
			}
			else														//ブロックのみの場合
			{
				//ブロックの幅と厚みをブロックの種類から判断
				switch (Stage[x][y].k)
				{
				case e_Type::e_地面:

					//ブロックの写真代入
					Block[i].p[0].p = 
					LoadGraph("picture/jimen_block.png");
						
					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = FALSE;

					break;

				case e_Type::e_壊れる:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/renga_block.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = FALSE;

					break;

				case e_Type::e_固い:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = FALSE;

					break;

				case e_Type::e_土管:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/dokan.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = FALSE;

					break;

				case e_Type::e_ハテナ:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = FALSE;

					break;

				case e_Type::e_ゴール:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = FALSE;

					break;

				case e_Type::e_クリボー:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/kuribo.png");

					//向き
					Block[i].d = TRUE;

					//キャラクターか否か
					Block[i].c = TRUE;

					break;
//#ifdef false
				case e_Type::e_ノコノコ:
					//ブロックの写真代入
					Block[i].p[0].p = LoadGraph("picture/hatena_block.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = TRUE;

					break;
//#endif
				case e_Type::e_マリオ:
					//ブロックの写真代入
					Block[i].p[e_Ani::e_Normal].p = LoadGraph("picture/neutral_mario.png");
					Block[i].p[e_Ani::e_Walk1].p = LoadGraph("picture/aruku_mario.png");
					Block[i].p[e_Ani::e_Walk2 ].p = LoadGraph("picture/aruku2_mario.png");
					Block[i].p[e_Ani::e_Jump].p = LoadGraph("picture/jump_mario.png");
					Block[i].p[e_Ani::e_End].p = LoadGraph("picture/miss_mario.png");

					//向き
					Block[i].d = FALSE;

					//キャラクターか否か
					Block[i].c = TRUE;

					break;
				}

				//ブロック写真から厚みと幅取得
				GetGraphSize(
					Block[i].p[0].p,
					&Block[i].p[0].w,
					&Block[i].p[0].t);

				//ブロックの幅と厚みを代入
				Block[i].w = Block[i].p[0].w;
				Block[i].t = Block[i].p[0].t;

				//ブロックの座標・種類・回転角度・ジャンプカウンタを代入
				Block[i].x = Stage[x][y].x * 50;
				Block[i].y = Stage[x][y].y * 50;
				Block[i].k = Stage[x][y].k;
				Block[i].r = 0.;
				Block[i].j = 0;
				Block[i].a = e_Ani::e_Normal;
			}

			//ループカウンタ
			i = i + 1;
		}
	}

	//メモリ解放（Text型 Stage）
	for (int i = 0; i < STAGE_WIDTH; i = i + 1)
	{
		delete[] Stage[i];
	}
	delete[] Stage;

	//初期化
	Camera_x = 0;

	//ジャンプ時の変数初期化
	Jump = e_Jump::地面に着地;

	//カウンタ
	Cou = 0;
}

//ゲーム終了時の処理を行う関数
void Game_End() {}

//マリオの移動量
#define Move_X (3)
#define Move_Max_Y (300)

//--------------------------------
//概　要：接触判定を行う関数
//引　数：キャラ、キャラの因子、接触時に倒すか否か
//戻り値：接触判定結果
//--------------------------------
bool HitJudg(Ract c, int i_c, bool d)
{
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		if (i != i_c)	//同じもの同士の比較を避けるため
		{
			if (((Block[i].x <= c.x &&
				c.x < (Block[i].x + Block[i].w)) ||

				(Block[i].x <= c.x + c.w &&
				c.x + c.w < Block[i].x + Block[i].w)) &&

				((Block[i].y <= c.y &&
				c.y < Block[i].y + Block[i].t) ||

				(Block[i].y <= c.y + c.t &&
				c.y + c.t < Block[i].y + Block[i].t)))
			{
#ifdef false
				//マリオが敵キャラと接触した場合
				if ((c.k == e_Type::e_マリオ || Block[i].k == e_Type::e_マリオ) &&
					(Block[i].c == TRUE))
#endif
				//マリオが敵キャラと接触した場合（引数の構造体がキャラクターとして設定されている場合のみ使用可）
				if ((c.k == e_Type::e_マリオ || Block[i].k == e_Type::e_マリオ) &&
					(c.c == TRUE) &&
					(Block[i].c == TRUE))

				{
					//接触時に敵キャラを倒すか否か
					switch (d)
					{
					case FALSE:
						Game_End();			//ゲーム終了時の処理(ゲームオーバー)
						break;

					case TRUE:
						Block_Ini(&Block[i]);	//ブロック初期化(敵キャラ倒す)
						break;
					}
				}
				//ゴールに接触した場合
				else if (c.k == e_Type::e_マリオ &&
					Block[i].k == e_Type::e_ゴール)
				{
					Game_End();				//ゲーム終了時の処理(ゲームクリア)
				}

				return TRUE;	//接触
			}
		}
	}
	return FALSE;	//未接触
}

//ゲーム計算処理を行う関数
void Game_Cal()
{
	//マリオy方向移動量定義
	int y_add = 0;

	//ジャンプ時の比例定数
	const int a = 25;

	//マリオの因数
	int c_i = 0;

	//敵キャラクターの移動
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		//敵キャラ探索
		if (Block[i].k == e_Type::e_クリボー ||
			Block[i].k == e_Type::e_ノコノコ)
		{
			//printf("%b\n", &Block[i].d);
			//敵キャラクターx方向移動計算
			for (int x = 0; x < Move_X; x = x + 1)
			{
				//敵キャラクター移動
				switch (Block[i].d)
				{
				case FALSE:	//右方向
					Block[i].x = Block[i].x + 1;
					break;
				case TRUE:	//左方向
					Block[i].x = Block[i].x - 1;
					break;
				}
//#ifdef false
				//x方向の敵キャラとブロックの接触判定
				if (HitJudg(Block[i], i, FALSE) == TRUE)
				{
					//敵キャラクター反転と移動
					switch (Block[i].d)
					{
					case FALSE:	//右方向
						Block[i].d = TRUE;
						Block[i].x = Block[i].x + 1;
						break;

					case TRUE:	//左方向
						Block[i].d = FALSE;
						Block[i].x = Block[i].x - 1;
						break;
					}

					//ループ解除
					break;
				}
//#endif
			}


			Block[i].j = Block[i].j + 1;

			//敵キャラクターy方向移動量計算
			y_add = (Block[i].j * Block[i].j) / a;

			//敵キャラクター自由落下計算
			for (int y = 0; y < y_add; y = y + 1)
			{
				//敵キャラクターy座標計算
				Block[i].y = Block[i].y + 1;

				//y方向の敵キャラとブロックの接触判定
				if (Block[i].y > WINDOW_HEIGHT ||
					HitJudg(Block[i], i, FALSE) == TRUE)
				{
					//敵キャラクターy座標計算
					Block[i].y = Block[i].y - 1;

					//ジャンプカウンタ初期化
					Block[i].j = 0;

					//ループ解除
					break;
				}
			}
		}
	}

	//マリオをブロックから探索してコピー
	int m_x = 0;
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		if (Block[i].k == e_Type::e_マリオ)
		{
			//マリオコピー
			Mar = &Block[i];

			//マリオ座標コピー
			m_x = Block[i].x;

			//マリオの因数
			c_i = i;

			//ループ解除
			break;
		}
	}

	//マリオ右移動(長押し対応)
	if (Pad_Cou[e_But::But_Rig] != 0)
	{
		//マリオ右方向を向く
		Mar->d = FALSE;

		for (int i = 0; i < Move_X; i = i + 1)
		{
			//マリオx座標計算
			Mar->x = Mar->x + 1;

			//カメラx座標計算
			if (Mar->x > WINDOW_WIDTH / 2 &&
				Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
			{
				Camera_x = Camera_x - 1;
			}

			//x方向のマリオとブロックの接触判定
			if (Mar->x > STAGE_WIDTH * 50 - Mar->w ||
				HitJudg(*Mar, c_i, FALSE) == TRUE)
			{
				//マリオx座標計算
				Mar->x = Mar->x - 1;

				//カメラx座標計算
				if (Mar->x > WINDOW_WIDTH / 2 &&
					Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
				{
					Camera_x = Camera_x + 1;
				}

				//ループ解除
				break;
			}
		}
	}

	//マリオ左移動(長押し対応)
	if (Pad_Cou[e_But::But_Lef] != 0)
	{
		//マリオ左方向を向く
		Mar->d = TRUE;

		for (int i = 0; i < Move_X; i = i + 1)
		{
			//マリオx座標計算
			Mar->x = Mar->x - 1;

			//カメラx座標計算
			if (Mar->x > WINDOW_WIDTH / 2 &&
				Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
			{
				Camera_x = Camera_x + 1;
			}

			//x方向のマリオとブロックの接触判定
			if (Mar->x < 0 ||
				HitJudg(*Mar, c_i, FALSE) == TRUE)
			{
				//マリオx座標計算
				Mar->x = Mar->x + 1;

				//カメラx座標計算
				if (Mar->x > WINDOW_WIDTH / 2 &&
					Mar->x < STAGE_WIDTH * 50 - WINDOW_WIDTH / 2)
				{
					Camera_x = Camera_x - 1;
				}

				//ループ解除
				break;
			}
		}
	}

	//マリオジャンプ
	if (Pad_Cou[e_But::But_Up] == 1 &&
		Jump == e_Jump::地面に着地)
	{
		//ジャンプフラグ変更
		Jump = e_Jump::ジャンプ中;

		//ジャンプ時のマリオ座標記録
		Jump_Ymem = Mar->y;
	}

	//ジャンプ時の座標計算
	if (Jump == e_Jump::ジャンプ中)
	{
		//ジャンプカウンタ
		Mar->j = Mar->j + 1;

		//マリオy方向移動量計算
		y_add = (Mar->j * Mar->j) / a;

		for (int i = 0; i < y_add; i = i + 1)
		{
			//マリオのy座標計算
			Mar->y = Mar->y - 1;

			//y方向のマリオとブロックの接触判定
			if (Mar->y < 0 ||
				HitJudg(*Mar, c_i, FALSE) == TRUE ||
				Move_Max_Y == Jump_Ymem - Mar->y)
			{
				//ジャンプフラグ変更
				Jump = e_Jump::落下中;

				//マリオy座標計算
				Mar->y = Mar->y + 1;

				//ジャンプカウンタ初期化
				Mar->j = 0;

				//ループ解除
				break;
			}
		}
	}

	//マリオ自由落下
	if (Jump == e_Jump::落下中 ||
		Jump == e_Jump::地面に着地)
	{
		//ジャンプカウンタ
		Mar->j = Mar->j + 1;

		//マリオy方向移動量計算
		y_add = (Mar->j * Mar->j) / a-3;

		for (int i = 0; i < y_add; i = i + 1)
		{
			//マリオy座標計算
			Mar->y = Mar->y + 1;

			//y方向のマリオとブロックの接触判定
			if (Mar->y > WINDOW_HEIGHT ||
				HitJudg(*Mar, c_i, TRUE) == TRUE)
			{
				//ジャンプフラグ変更
				Jump = e_Jump::地面に着地;

				//マリオy座標計算
				Mar->y = Mar->y - 1;

				//ジャンプカウンタ初期化
				Mar->j = 0;

				//ループ解除
				break;
			}
		}
	}

	Cou = Cou + 1;
	if (Cou == 60) Cou = 0;

	//マリオアニメーション
	if (Jump == e_Jump::ジャンプ中 ||
		Jump == e_Jump::落下中)
	{
		Mar->a = e_Ani::e_Jump;
	}
	else if (m_x != Mar->x)
	{
		if (0 <= Cou && Cou < 10)Mar->a = e_Ani::e_Normal;
		else if (10 <= Cou && Cou < 20)Mar->a = e_Ani::e_Walk1;
		else if (20 <= Cou && Cou < 30)Mar->a = e_Ani::e_Walk2;
		else if (30 <= Cou && Cou < 40)Mar->a = e_Ani::e_Normal;
		else if (40 <= Cou && Cou < 50)Mar->a = e_Ani::e_Walk1;
		else if (50 <= Cou && Cou < 26)Mar->a = e_Ani::e_Walk2;
	}
	else Mar->a = e_Ani::e_Normal;
}

//ゲーム描画処理を行う関数
void Game_Draw()
{
	//ブロック描画
	for (int i = 0; i < STAGE_TOTAL; i = i + 1)
	{
		//マリオ描画
		if (Block[i].k == e_Type::e_マリオ)
		{
			switch (Mar->d)
			{
			case FALSE:		//右方向向く
				DrawGraph(
					Mar->x + Camera_x,
					Mar->y,
					Mar->p[Mar->a].p,
					TRUE);

				break;

			case TRUE:		//左方向向く
				DrawTurnGraph(
					Mar->x + Camera_x,
					Mar->y,
					Mar->p[Mar->a].p,
					TRUE);

				break;
			}
		}

		//マリオ以外描画
		else if(Block[i].k != e_Type::e_空)
		{
			switch (Block[i].d)
			{
			case FALSE:		//右方向向く
				DrawTurnGraph(
					Block[i].x + Camera_x,
					Block[i].y,
					Block[i].p[0].p,
					TRUE);

				break;

			case TRUE:		//左方向向く
				DrawGraph(
					Block[i].x + Camera_x,
					Block[i].y,
					Block[i].p[0].p,
					TRUE);

				break;
			}
		}

	}
}

//キーの情報を列挙型で取得
enum e_Key
{
	key_Dow,	//1bit
	Key_Lef,	//2bit
	Key_Rig,	//3bit
	Key_Up,		//4bit
	Key_A,		//5bit
	Key_D,		//6bit
	Key_W,		//7bit
	Key_Spa,	//8bit
	Key_Total
};

//キーのビット情報
int Key_Inp;

//キーの押したカウント数
int Key_Cou[e_Key::Key_Total];

#ifdef false
//キーの入力情報を取得する関数
bool Key_Read()
{
		BYTE key[256];
		GetKeyboardState(key);

		for (int k = 0; k < e_Key::Key_Total; k = k + 1)
		{
			if (Key_Inp & (1 << k))
			{
				Key_Cou[k] = Key_Cou[k] + 1;
			}
			else Key_Cou[k] = 0;
	}
	return FALSE;
}
#endif


//-------------------------------------------------
//  概要：メイン関数
//  引数：-
//  戻り値：正常/異常
//-------------------------------------------------
static int mStartTime;		//測定開始時刻
static int mCount;			//カウンタ
static float mFps;			//fps
static const int N = 60;		//平均を取るサンプル数
static const int FPS = 60;		//設定したFPS

void Update()
{
	if (mCount == 0)
	{
		mStartTime = GetNowCount();
	}
	if (mCount == N)
	{
		int t = GetNowCount();
		mFps = 1000.f / ((t - mStartTime) / (float)N);
		mCount = 0;
		mStartTime = t;
	}
	mCount++;
}

void Wait()
{
	int tookTime = GetNowCount() - mStartTime;			//かかった時間
	int waitTime = mCount * 1000 / FPS - tookTime;		//待つべき時間
	if (waitTime > 0)
	{
		Sleep(waitTime);		//待機
	}
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{

	ChangeWindowMode(TRUE);
	DxLib_Init();

	//ウィンドウ初期化
	SetMainWindowText("スーパーマリオ");
	SetWindowInitPosition(0, 0);						//背景画面の初期設定
	SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);		//背景画面の幅と高さ
	SetDrawScreen(DX_SCREEN_BACK);

	Game_Ini();

	//１秒間に６０回（ＦＰＳ回）無限ループ
	while (ScreenFlip() == 0 &&
		ClearDrawScreen() == 0 &&
		ProcessMessage() == 0 &&	//上三つは絶対に書くやつ
		Pad_Read() == FALSE)
	{
		Update();
		Game_Cal();
		Game_Draw();
		Wait();
	}

	DxLib_End();
	return 0;
}


