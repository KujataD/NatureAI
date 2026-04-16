#pragma once
#include "AStar.h"
#include "BaseEnemy.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "ShieldEnemy.h"
#include "Skydome.h"
#include <vector>

class HitEffect;
class GuardEffect;
class StageManager;
class Minion;

class GameScene {
public:
	GameScene();
	~GameScene();

	// 初期化
	void Init(StageManager* stageDataManager);

	// 更新
	void Update();

	// 描画
	void Draw();

	// マップオブジェクト生成
	void GenerateFieldObjects();

	// エフェクト生成
	void CreateHitEffect(const KamataEngine::Vector3 spawnPos, const KamataEngine::Vector4 color = KamataEngine::Vector4{1.0f, 1.0f, 1.0f, 1.0f});

	void CreateGuardEffect(const KamataEngine::Vector3 spawnPos);

	// Accessors

	// --- get ---
	bool IsFinished() const { return finished_; }
	bool GetReloadRequested() const { return reloadRequested_; }

private:
	/// <summary>
	/// すべての当たり判定を行う
	/// </summary>
	void CheckAkkCollisions();

	/// <summary>
	/// フェーズ切替
	/// </summary>
	void ChangePhase();

	void PaintPath(MapChipField::IndexSet startIdx);

private: // enum
	enum class Phase {
		kFadeIn,  // フェードイン
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト
	};

private: // 定数
	static inline const int32_t kEnemyCount = 1;

	// フェードの時間（秒）
	static inline const float kFadeDuration = 1.0f;

private: // 変数
	// モデルのテクスチャハンドル
	uint32_t modelTextureHandle_ = 0;

	// --- ステージ ---

	// ステージマネージャ参照用のポインタ
	StageManager* stageManager_ = nullptr;

	// ブロックモデルのテクスチャ
	uint32_t blockTextureHandle_ = 0;

	// ブロックモデル
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;

	// 床の色
	std::vector<std::vector<KamataEngine::ObjectColor*>> floorColors_;

	// A*
	AStar aStar_;

	// ブロックのワールドトランスフォーム
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// --- カメラ ---

	// カメラ
	KamataEngine::Camera camera_;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// スカイドーム
	Skydome* skydome_ = nullptr;

	// --- ゲームオブジェクト ---

	// プレイヤー3Dモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	KamataEngine::Model* modelPlayerAttack_ = nullptr;

	// プレイヤー
	Player* player_ = nullptr;

	// エネミー３Dモデル
	KamataEngine::Model* modelEnemy_ = nullptr;

	// 盾エネミー３Dモデル
	KamataEngine::Model* modelShieldEnemy_ = nullptr;

	// 複数エネミー
	std::list<BaseEnemy*> enemies_;

	// スカイドームの3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// カメラコントローラー
	CameraController* cameraController_ = nullptr;

	// ミニオン
	KamataEngine::Model* modelMinion_ = nullptr;
	Minion* minion_ = nullptr;
	MapChipField::IndexSet prevGoalIdx_ = {0, 0};

	// --- VFX ---

	// パーティクルモデル
	KamataEngine::Model* modelDeathParticle_ = nullptr;

	// デスパーティクル
	DeathParticles* deathParticles_ = nullptr;

	// ヒットエフェクトモデル
	KamataEngine::Model* modelHitEffect_ = nullptr;

	// ガードエフェクトモデル
	KamataEngine::Model* modelGuardEffect_ = nullptr;

	std::list<HitEffect*> hitEffects_;

	std::list<GuardEffect*> guardEffects_;

	// --- シーン ---

	// ゲームフェーズ
	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	// フェード
	Fade* fade_ = nullptr;

	// リロードリクエスト
	bool reloadRequested_ = false;
};
