#include "GameScene.h"
#include "AABB.h"
#include "GuardEffect.h"
#include "HitEffect.h"
#include "Matrix4x4.h"
#include "Minion.h"
#include "StageManager.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete modelPlayer_;
	delete modelPlayerAttack_;
	delete modelEnemy_;
	delete modelSkydome_;
	delete modelHitEffect_;
	delete modelGuardEffect_;
	delete blockModel_;
	delete debugCamera_;
	delete player_;
	delete mapChipField_;
	delete deathParticles_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	for (std::vector<ObjectColor*>& floorColorLine : floorColors_) {
		for (ObjectColor* floorColor : floorColorLine) {
			delete floorColor;
		}
	}
	floorColors_.clear();

	for (BaseEnemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	hitEffects_.clear();

	for (GuardEffect* guardEffect : guardEffects_) {
		delete guardEffect;
	}
	guardEffects_.clear();
}

void GameScene::Init(StageManager* stageDataManager) {
	// 引数をメンバに記録
	stageManager_ = stageDataManager;

	// ファイル名指定でテクスチャ読み込み
	modelTextureHandle_ = TextureManager::Load("uvChecker.png");

	// ブロックモデルの生成
	blockModel_ = Model::CreateFromOBJ("block", true);
	floorModel_ = Model::CreateFromOBJ("floor", true);

	// カメラの初期化
	camera_.Initialize();

	// ライン描画が参照するカメラを指定する（アドレス渡し）
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	//// 軸方向表示の表示を有効にする
	// AxisIndicator::GetInstance()->SetVisible(true);

	//// 軸方向表示が参照するビュープロジェクションを指定する(アドレス渡し)
	// AxisIndicator::GetInstance()->SetTargetCamera(&camera_);

	// ゲームプレイフェーズから開始
	phase_ = Phase::kFadeIn;

	// フェード
	fade_ = new Fade;
	fade_->Init();
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);

	// ------------------------------------------
	// プレイヤー
	//

	// 3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelPlayerAttack_ = Model::CreateFromOBJ("hit_effect", true);

	//
	// ------------------------------------------

	// ------------------------------------------
	// エネミー
	//

	// 3Dモデルの生成
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);

	// 3Dモデルの生成
	modelShieldEnemy_ = Model::CreateFromOBJ("shieldEnemy", true);

	//
	// ------------------------------------------

	// ------------------------------------------
	// マップチップ
	//

	mapChipField_ = new MapChipField;
	aStar_.Init(mapChipField_);
	// 現在のステージデータを取得する
	const StageData& stageData = stageManager_->GetCurrentStageData();
	// ステージファイルパスの生成
	std::string stageFileName = "Resources/fields/" + stageData.name + ".csv";
	// ステージファイルの読み込み
	mapChipField_->LoadMapChipCsv(stageFileName);

	GenerateFieldObjects();

	//
	// ------------------------------------------

	// ------------------------------------------
	// ミニオン
	//
	modelMinion_ = Model::CreateSphere(); // モデル名は適宜変更

	minion_ = new Minion();
	minion_->Init(modelMinion_, &camera_, mapChipField_->GetMapChipPositionByIndex(2, 2));
	minion_->SetMapChipField(mapChipField_);

	//
	// ------------------------------------------

	// ------------------------------------------
	// スカイドーム
	//

	// 3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("sky_sphere", true);

	// 生成・初期化
	skydome_ = new Skydome();
	skydome_->Init(modelSkydome_, &camera_);

	//
	// ------------------------------------------

	// カメラコントローラーの初期化
	cameraController_ = new CameraController;
	cameraController_->Init(&camera_);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	// ------------------------------------------
	// VFX
	//

	// デスパーティクル
	modelDeathParticle_ = Model::CreateFromOBJ("deathParticle", true);

	deathParticles_ = new DeathParticles;
	deathParticles_->Init(modelDeathParticle_, &camera_, player_->GetWorldPosition());

	// ヒットエフェクト
	modelHitEffect_ = Model::CreateFromOBJ("particle", true);
	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(&camera_);

	// ガードエフェクト
	modelGuardEffect_ = Model::CreateFromOBJ("ring", true);
	GuardEffect::SetModel(modelGuardEffect_);
	GuardEffect::SetCamera(&camera_);

	//
	// ------------------------------------------
}

void GameScene::Update() {

	ChangePhase();
	// フェードの更新
	fade_->Update();

	switch (phase_) {
	case GameScene::Phase::kFadeIn: {

		// skydomeの処理
		skydome_->Update();

		// 自キャラの更新
		player_->Update();

		// エネミーの更新
		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		// デスフラグの立った敵を削除
		enemies_.remove_if([](BaseEnemy* enemy) {
			if (enemy->IsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		// カメラコントローラーの処理
		cameraController_->Update();

		// カメラの処理
		if (isDebugCameraActive_) {
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換行列の作成
				Matrix4x4 affine = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				worldTransformBlock->matWorld_ = affine;

				// 定数バッファに転送する
				worldTransformBlock->TransferMatrix();
			}
		}
	} break;
	case GameScene::Phase::kPlay: {
		// ゲームプレイフェーズ
		// --------------------------------------

		// skydomeの処理
		skydome_->Update();

		// 自キャラの更新
		player_->Update();

		// --- ミニオン更新 ---
		// プレイヤーのインデックスを取得
		MapChipField::IndexSet goalIdx = mapChipField_->GetMapChipIndexSetByPosition(player_->GetWorldPosition());

		if (goalIdx != prevGoalIdx_) {
			// インデックス適応
			prevGoalIdx_ = goalIdx;
			// ミニオンのインデックスを取得
			MapChipField::IndexSet minionIdx = mapChipField_->GetMapChipIndexSetByPosition(minion_->GetWorldPosition());
			// パスを形成
			std::vector<MapChipField::IndexSet> path = aStar_.FindPath(minionIdx, goalIdx);
			// パスをセット
			minion_->SetPas(path);
		}
		minion_->Update();

		// エネミーの更新
		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		// デスフラグの立った敵を削除
		enemies_.remove_if([](BaseEnemy* enemy) {
			if (enemy->IsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		// カメラコントローラーの処理
		cameraController_->Update();

		// カメラの処理
		if (isDebugCameraActive_) {
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換行列の作成
				Matrix4x4 affine = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				worldTransformBlock->matWorld_ = affine;

				// 定数バッファに転送する
				worldTransformBlock->TransferMatrix();
			}
		}

		// 当たり判定処理
		CheckAkkCollisions();

		// --- エフェクト ---

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		// デスフラグの立ったヒットエフェクトを削除
		hitEffects_.remove_if([](HitEffect* effect) {
			if (effect->IsDead()) {
				delete effect;
				return true;
			}
			return false;
		});

		for (GuardEffect* hitEffect : guardEffects_) {
			hitEffect->Update();
		}

		// デスフラグの立ったヒットエフェクトを削除
		guardEffects_.remove_if([](GuardEffect* effect) {
			if (effect->IsDead()) {
				delete effect;
				return true;
			}
			return false;
		});

	} break;
	case GameScene::Phase::kDeath:
		// デス演出フェーズ
		// --------------------------------------

		// skydomeの処理
		skydome_->Update();

		// エネミーの更新
		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		// デスフラグの立った敵を削除
		enemies_.remove_if([](BaseEnemy* enemy) {
			if (enemy->IsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		// パーティクル処理
		if (deathParticles_) {
			deathParticles_->Update();
		}

		// カメラの処理
		if (isDebugCameraActive_) {
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換行列の作成
				Matrix4x4 affine = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				worldTransformBlock->matWorld_ = affine;

				// 定数バッファに転送する
				worldTransformBlock->TransferMatrix();
			}
		}

		break;

	default:
		break;
	}

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_P)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	// デバッグカメラの更新
	debugCamera_->Update();

#endif // _DEBUG

#ifdef _DEBUG
	MapChipField::IndexSet playerIndex = mapChipField_->GetMapChipIndexSetByPosition(player_->GetWorldPosition());

	// ------------------------------------------
	// デバッグテキストの表示
	ImGui::Begin("Debug1");

	// 初期化
	if (ImGui::Button("Reoad")) {
		reloadRequested_ = true;
	}
	ImGui::Text("x %d, z %d", playerIndex.xIndex, playerIndex.zIndex);
	ImGui::End();

#endif // _DEBUG
}

void GameScene::Draw() {
	// ------------------------------------------
	// スプライト描画前処理
	Sprite::PreDraw();

	// スプライト描画後処理
	Sprite::PostDraw();
	// ------------------------------------------

	// ------------------------------------------
	// 3Dモデル描画前処理
	Model::PreDraw();

	// skydomeの描画
	skydome_->Draw();

	// ブロックの描画
	for (uint32_t z = 0; z < worldTransformBlocks_.size(); ++z) {
		for (uint32_t x = 0; x < worldTransformBlocks_[z].size(); ++x) {

			WorldTransform* worldTransformBlock = worldTransformBlocks_[z][x];

			if (!worldTransformBlock)
				continue;

			if (worldTransformBlock->translation_.y < 0.0f) {
				MapChipField::IndexSet playerIndex = mapChipField_->GetMapChipIndexSetByPosition(player_->GetWorldPosition());
				MapChipField::IndexSet blockIndex = mapChipField_->GetMapChipIndexSetByPosition(worldTransformBlock->translation_);

				blockModel_->Draw(*worldTransformBlock, camera_, floorColors_[z][x]);

			} else {
				floorModel_->Draw(*worldTransformBlock, camera_);
			}
		}
	}

	// エネミーの描画
	for (BaseEnemy* enemy : enemies_) {
		enemy->Draw();
	}

	// 自キャラの描画
	player_->Draw();

	minion_->Draw();

	// パーティクル描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// ヒットエフェクト描画
	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	// ガードエフェクト描画
	for (GuardEffect* guardEffect : guardEffects_) {
		guardEffect->Draw();
	}

	// 3Dモデル描画後処理
	Model::PostDraw();
	// ------------------------------------------

	fade_->Draw();
}

void GameScene::GenerateFieldObjects() {
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定(縦方向のブロック数)
	worldTransformBlocks_.resize(numBlockVirtical);
	floorColors_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		// 1列の要素数を設定(横方向のブロック数)
		worldTransformBlocks_[i].resize(numBlockHorizontal);
		floorColors_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t z = 0; z < numBlockVirtical; ++z) {
		for (uint32_t x = 0; x < numBlockHorizontal; ++x) {
			{
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[z][x] = worldTransform;
				worldTransformBlocks_[z][x]->translation_ = mapChipField_->GetMapChipPositionByIndex(x, z);
				worldTransformBlocks_[z][x]->translation_.y -= 1.0f;

				// タイルごとに色オブジェクトを生成
				KamataEngine::ObjectColor* color = new KamataEngine::ObjectColor();
				color->Initialize();
				color->SetColor({1, 1, 1, 1}); // デフォルト白
				floorColors_[z][x] = color;    // 追加
			}

			switch (mapChipField_->GetMapChipTypeByIndex(x, z)) {
			case MapChipType::kBlock: {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[z][x] = worldTransform;
				worldTransformBlocks_[z][x]->translation_ = mapChipField_->GetMapChipPositionByIndex(x, z);
				break;
			}
			case MapChipType::kPlayer: {
				assert(player_ == nullptr && "自キャラを二重に配置しようとしています。");
				// 自キャラの作成

				// 自キャラの生成・初期化
				player_ = new Player();

				// 座標を指定
				Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(x, z);

				player_->Init(modelPlayer_, modelPlayerAttack_, &camera_, playerPosition);
				player_->SetMapChipField(mapChipField_);
				break;
			}
			// case MapChipType::kEnemy: {
			//	uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(x, z);
			//	switch (subID) {
			//	case 0: {

			//		Enemy* newEnemy = new Enemy();
			//		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, z);
			//		newEnemy->Init(modelEnemy_, &camera_, enemyPosition);
			//		newEnemy->SetMapChipField(mapChipField_);
			//		newEnemy->SetGameScene(this);
			//		enemies_.push_back(newEnemy);
			//		break;
			//	}
			//	case 1: {

			//		ShieldEnemy* newEnemy = new ShieldEnemy();
			//		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, z);
			//		newEnemy->Init(modelShieldEnemy_, &camera_, enemyPosition);
			//		newEnemy->SetMapChipField(mapChipField_);
			//		newEnemy->SetGameScene(this);
			//		enemies_.push_back(newEnemy);
			//		break;
			//	}
			//	default: {
			//		break;
			//	}
			//	}
			//	break;
			//}
			default: {
				break;
			}
			}
		}
	}
}

void GameScene::CreateHitEffect(const KamataEngine::Vector3 spawnPos, const KamataEngine::Vector4 color) {
	HitEffect* newHitEffect = HitEffect::Create(spawnPos, color);
	hitEffects_.push_back(newHitEffect);
}

void GameScene::CreateGuardEffect(const KamataEngine::Vector3 spawnPos) {
	GuardEffect* newGuardEffect = GuardEffect::Create(spawnPos);
	guardEffects_.push_back(newGuardEffect);
}

void GameScene::CheckAkkCollisions() {
#pragma region 自キャラと敵キャラの当たり判定
	{
		// 判定対象1と2の座標
		AABB aabb1, aabb2;

		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾全ての当たり判定
		for (BaseEnemy* enemy : enemies_) {
			if (enemy->IsCollisionDisabled()) {
				continue;
			}

			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (IsHitCollision(aabb1, aabb2)) {
				player_->OnCollision(enemy);
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case GameScene::Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = GameScene::Phase::kPlay;
		}
		break;
	case GameScene::Phase::kPlay:
		// ゲームプレイフェーズ
		// --------------------------------------

		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			deathParticles_->Init(modelDeathParticle_, &camera_, deathParticlesPosition);
		}

		break;
	case GameScene::Phase::kDeath:
		// デス演出フェーズ
		// --------------------------------------

		// デス演出フェーズ終了
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);
		}
		break;
	case GameScene::Phase::kFadeOut:
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	default:
		break;
	}
}
void GameScene::PaintPath(MapChipField::IndexSet startIdx) {

	// プレイヤーのインデックスを取得
	MapChipField::IndexSet goalIdx = mapChipField_->GetMapChipIndexSetByPosition(player_->GetWorldPosition());

	// A*で経路探索
	std::vector<MapChipField::IndexSet> path = aStar_.FindPath(startIdx, goalIdx);

	uint32_t numZ = mapChipField_->GetNumBlockVirtical();
	uint32_t numX = mapChipField_->GetNumBlockHorizontal();

	// まず全タイルを白に戻す
	for (uint32_t z = 0; z < numZ; z++) {
		for (uint32_t x = 0; x < numX; x++) {
			floorColors_[z][x]->SetColor({1, 1, 1, 1});
		}
	}

	// 経路タイルを色付け
	for (auto& idx : path) {
		floorColors_[idx.zIndex][idx.xIndex]->SetColor({0, 1, 0, 1}); // 緑
	}

	// スタートとゴールを強調
	floorColors_[startIdx.zIndex][startIdx.xIndex]->SetColor({1, 0, 0, 1}); // 赤
	floorColors_[goalIdx.zIndex][goalIdx.xIndex]->SetColor({0, 0, 1, 1});   // 青
}