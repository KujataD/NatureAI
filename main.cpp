#include "KamataEngine.h"
#include <Windows.h>
#include <cassert>
#include <fstream>

#include "DeltaTime.h"
#include "Random.h"

#include "GameScene.h"
#include "TitleScene.h"

#include "StageManager.h"

using namespace KamataEngine;

enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
};

StageManager* stageManager = nullptr;

Scene scene = Scene::kUnknown;

// ゲームシーン
GameScene* gameScene = nullptr;

// タイトルシーン
TitleScene* titleScene;

void InitScene();

void ChangeScene();

void UpdateScene();

void DrawScene();

void LoadDebugSettings();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LC1B_07_オオツカ_ダイチ_AL2");

	// DirectXCommonのインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// タイトルシーンの初期化
	scene = Scene::kTitle;

	// クラスインスタンスの生成
	stageManager = new StageManager;
	// ステージデータファイルを読み込む
	stageManager->LoadStageDataFile();

#ifdef _DEBUG
	// デバッグ設定ファイル読み込み
	LoadDebugSettings();

	scene = Scene::kGame;
#endif // _DEBUG
	
	// シーン初期化
	InitScene();

	// DeltaTimeの初期化
	DeltaTime::Init();

	// Randomのシード初期化
	Random::Init();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// DeltaTime更新
		DeltaTime::Update();

		// --------------------------------------
		// ImGui受付開始
		imguiManager->Begin();

		ChangeScene();

		UpdateScene();

		// ImGui受付終了
		imguiManager->End();
		// --------------------------------------

		// --------------------------------------
		// 描画開始
		dxCommon->PreDraw();

		DrawScene();

		// 軸表示の描画
		AxisIndicator::GetInstance()->Draw();

		// ImGui描画
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
		// --------------------------------------
	}

	// 解放処理
	// ------------------------------------------

	// シーンの解放
	delete titleScene;
	titleScene = nullptr;
	delete gameScene;
	gameScene = nullptr;
	delete stageManager;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}

void InitScene() {
	switch (scene) {
	case Scene::kUnknown:
		break;
	case Scene::kTitle:
		if (!titleScene) {
			titleScene = new TitleScene;
			titleScene->Init();
		}
		break;
	case Scene::kGame:
		if (!gameScene) {
			gameScene = new GameScene;
			gameScene->Init(stageManager);
		}
		break;
	default:
		break;
	}
}

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			gameScene = new GameScene;
			gameScene->Init(stageManager);
		}
		break;
	case Scene::kGame:
		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete gameScene;
			gameScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene;
			titleScene->Init();
		} else if (gameScene->GetReloadRequested()) {
			// シーンリロード
			delete gameScene;
			gameScene = nullptr;
			gameScene = new GameScene;
			gameScene->Init(stageManager);
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	default:
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	default:
		break;
	}
}

void LoadDebugSettings() {
	// ステージデータファイルのパス
	const std::string filePath = "DebugSettings.ini";

	// ifstreamでステージデータファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open() && "DebugSettingsが存在しません");

	std::string key;
	std::string value;
	file >> key >> value;

	if (key == "InitialStage") {
		stageManager->SetCurrentStageIndexByName(value);
	}
}
