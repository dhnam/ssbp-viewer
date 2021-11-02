#include "DxLib.h"
#include "SSPlayer/SS5Player.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <cassert>

static int previousTime;
static int waitTime;
int mGameExec;

#define WAIT_FRAME (16)

void init(void);
void update(float dt);
void draw(void);
void relese(void);

/// SS5プレイヤー
ss::Player* ssplayer;
ss::ResourceManager* resman;
std::string filename;
std::vector<std::string> animeNames;
int animeNum;
float x_pos;
float y_pos;

/**
* メイン関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//DXライブラリの初期化
	ChangeWindowMode(true);	//ウインドウモード
	SetGraphMode(1280, 720, GetColorBitDepth());
	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}
	SetDrawScreen(DX_SCREEN_BACK);

	//メインループ
	mGameExec = 1;
	previousTime = GetNowCount();

	// プレイヤー初期化
	init();

	while (mGameExec == 1) {
		ClearDrawScreen();
		update((float)waitTime / 1000.0f);		//ゲームの更新
		draw();									//ゲームの描画
		ScreenFlip();							//描画結果を画面に反映

		//次のフレームまでの時間待ち
		waitTime = GetNowCount() - previousTime;
		previousTime = GetNowCount();

		if (waitTime < WAIT_FRAME) {
			WaitTimer((WAIT_FRAME - waitTime));
		}
		else {
			if (ProcessMessage() == -1) mGameExec = 0;
		}
	}

	/// プレイヤー終了処理
	relese();


	DxLib_End();			// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了 
}

void init(void)
{
	/**********************************************************************************

	SSアニメ表示のサンプルコード
	Visual Studio Community、DXライブラリで動作を確認しています。
	ssbpとpngがあれば再生する事ができますが、Resourcesフォルダにsspjも含まれています。

	**********************************************************************************/

	//リソースマネージャの作成
	resman = ss::ResourceManager::getInstance();
	//プレイヤーの作成
	ssplayer = ss::Player::create();

	//アニメデータをリソースに追加

	// File dialogue start
	char fileNameChar[512] = "\0";

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFilter = "SSBP files(*.ssbp)\0*.ssbp\0All files(*.*)\0*.*\0";
	ofn.lpstrFile = fileNameChar;
	ofn.nMaxFile = 512;
	std::string file_s;
	if (0 != GetOpenFileName(&ofn)) {
		file_s = std::string(ofn.lpstrFile);
	}
	else {
		exit(1);
	}
	// File dialogue end

	filename = file_s.substr(file_s.find_last_of("/\\") + 1);
	std::string::size_type const p(filename.find_last_of('.'));
	filename = filename.substr(0, p);
	//それぞれのプラットフォームに合わせたパスへ変更してください。
	resman->addData(file_s);
	//プレイヤーにリソースを割り当て
	ssplayer->setData(filename);        // ssbpファイル名（拡張子不要）

	animeNames = resman->getAnimeName(filename);
	animeNum = 0;
	ofstream fout;
	fout.open("./animeNames.txt");
	int i = 0;
	for (auto next_str : animeNames) {
		fout << i << ". ";
		fout << next_str << std::endl;
		i += 1;
	}

	//再生するモーションを設定

	ssplayer->play(animeNames[animeNum]);				 // アニメーション名を指定(ssae名/アニメーション名も可能、詳しくは後述)

	//表示位置を設定
	x_pos = 1280 / 2;
	y_pos = 300;
	ssplayer->setPosition(x_pos, y_pos);
	//スケール設定
	ssplayer->setScale(0.5f, 0.5f);
	//回転を設定
	ssplayer->setRotation(0.0f, 0.0f, 0.0f);
	//透明度を設定
	ssplayer->setAlpha(255);
	//反転を設定
	ssplayer->setFlip(false, false);

}

//メインループ
//Zボタンでアニメをポーズ、再開を切り替えできます。
//ポーズ中は左右キーで再生するフレームを変更できます。
static bool sstest_push = false;
static int sstest_count = 0;
static bool sstest_pause = false;
static float size_factor = 0.5;
bool hide = false;
void update(float dt)
{
	char str[256];
	//キー入力操作
	int animax = ssplayer->getMaxFrame();
	if (CheckHitKey(KEY_INPUT_ESCAPE))
	{
		mGameExec = 0;
	}

	if (CheckHitKey(KEY_INPUT_Z))
	{
		if (sstest_push == false)
		{
			if (sstest_pause == false)
			{
				sstest_pause = true;
				sstest_count = ssplayer->getFrameNo();;
				ssplayer->animePause();
			}
			else
			{
				sstest_pause = false;
				ssplayer->animeResume();
			}
		}
		sstest_push = true;

	}
	else if (CheckHitKey(KEY_INPUT_UP))
	{
		if (sstest_push == false)
		{
			sstest_count += 20;
			if (sstest_count >= animax)
			{
				sstest_count = 0;
			}
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_DOWN))
	{
		if (sstest_push == false)
		{
			sstest_count -= 20;
			if (sstest_count < 0)
			{
				sstest_count = animax - 1;
			}
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_LEFT))
	{
		if (sstest_push == false)
		{
			sstest_count--;
			if (sstest_count < 0)
			{
				sstest_count = animax - 1;
			}
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_RIGHT))
	{
		if (sstest_push == false)
		{
			sstest_count++;
			if (sstest_count >= animax)
			{
				sstest_count = 0;
			}
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_PERIOD)) {
		if (sstest_push == false) {
			animeNum += 1;
			if (animeNum == animeNames.size()) animeNum = 0;
			ssplayer->play(animeNames[animeNum]);
			sstest_count = 0;
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_COMMA)) {
		if (sstest_push == false) {
			animeNum -= 1;
			if (animeNum < 0) animeNum = animeNames.size() - 1;
			ssplayer->play(animeNames[animeNum]);
			sstest_count = 0;
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_H)) {
		if (sstest_push == false) {
			if (hide) {
				hide = false;
			}
			else {
				hide = true;
			}
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_ADD) || CheckHitKey(KEY_INPUT_SEMICOLON)) {
		if (sstest_push == false) {
			size_factor += 0.1;
		}
		sstest_push = true;
	}
	else if (CheckHitKey(KEY_INPUT_SUBTRACT) || CheckHitKey(KEY_INPUT_MINUS)) {
		if (sstest_push == false) {
			if (size_factor > 0.2) {
				size_factor -= 0.1;
			}
		}
		sstest_push = true;
	}
	else
	{
		sstest_push = false;
	}

	if (sstest_pause == true)
	{
		ssplayer->setFrameNo(sstest_count % animax);
	}
	else
	{
		sstest_count = ssplayer->getFrameNo();
	}


	if (CheckHitKey(KEY_INPUT_A)) {
		x_pos -= size_factor;
		ssplayer->setPosition(x_pos, y_pos);
		sstest_push = true;
	}
	if (CheckHitKey(KEY_INPUT_S)) {
		y_pos -= size_factor;
		ssplayer->setPosition(x_pos, y_pos);
	}
	if (CheckHitKey(KEY_INPUT_D)) {
		x_pos += size_factor;
		ssplayer->setPosition(x_pos, y_pos);
	}
	if (CheckHitKey(KEY_INPUT_W)) {
		y_pos += size_factor;
		ssplayer->setPosition(x_pos, y_pos);
	}
	//パーツ名から座標等のステートの取得を行う場合はgetPartStateを使用します。
	ssplayer->setScale(size_factor, size_factor);


	if (!hide) {
		//アニメーションのフレームを表示
		sprintf(str, "play:%d frame:%d drawCount:%d", (int)sstest_pause, sstest_count, ssplayer->getDrawSpriteCount());
		DrawString(100, 100, str, GetColor(255, 255, 255));

		sprintf(str, "Global pos x:%.1f y:%.1f / scale:%.1f", x_pos, y_pos, size_factor);
		DrawString(100, 120, str, GetColor(255, 255, 255));

		sprintf(str, "animeNum:%d animeCount:%d", animeNum + 1, animeNames.size());
		DrawString(100, 140, str, GetColor(255, 255, 255));

		sprintf(str, "AnimeName:%s", animeNames[animeNum].c_str());
		DrawString(100, 160, str, GetColor(255, 255, 255));

		sprintf(str, ". : next anime\n, : prev anime\nZ : pause/play\narrow keys : browse frames\n-= or -+(numpad) : size\nWASD : move\nH : hide informations\nESC : exit");
		DrawString(100, 500, str, GetColor(255, 255, 255));
	}
	else {
		sprintf(str, "H : show informations");
		DrawString(100, 100, str, GetColor(255, 255, 255));
	}



	//プレイヤーの更新、引数は前回の更新処理から経過した時間
	ssplayer->update(dt);

}

//描画
void draw(void)
{
	//プレイヤーの描画
	ssplayer->draw();
}

/**
* プレイヤー終了処理
*/
void relese(void)
{

	//SS5Playerの削除
	delete (ssplayer);
	delete (resman);
}









