#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "Math.h"
#include "PlayerBehaviorAttack.h"
#include "PlayerBehaviorKnockback.h"
#include "PlayerBehaviorRoot.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

using namespace KamataEngine;
using namespace MathUtility;

Player::Player() {}

Player::~Player() {}

void Player::Init(Model* model, Model* modelAttack, Camera* camera, const Vector3& position) {
	// nullptr check
	(void)model;
	assert(modelAttack);
	assert(camera);

	// 引数の記録
	model_ = Model::CreateSphere();
	modelAttack_ = modelAttack;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f};

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	isDead_ = false;

	// ルートビヘイビアに変更
	ChangeBehavior(new PlayerBehaviorRoot);
}

void Player::Update() {
	// ふるまい変更のrequest処理
	if (behaviorRequest_) {
		ChangeBehavior(behaviorRequest_);
		behaviorRequest_ = nullptr;
	}

	if (isDead_) {
		return;
	}

	// アタックのクールタイム更新
	if (attackCoolTimer_ > 0.0f) {
		attackCoolTimer_ -= 1.0f / 60.0f;
	}

	// ビヘイビアの更新
	if (behavior_) {
		behavior_->Update(this);
	}
}

void Player::Draw() {
	if (isDead_) {
		return;
	}
	model_->Draw(worldTransform_, *camera_);

	if (isActiveAttackFX_) {
		// 表面の攻撃エフェクト
		worldTransformAttack_.translation_ = worldTransform_.translation_;
		worldTransformAttack_.rotation_ = worldTransform_.rotation_;
		worldTransformAttack_.rotation_.y += std::numbers::pi_v<float>;
		WorldTransformUpdate(worldTransformAttack_);
		modelAttack_->Draw(worldTransformAttack_, *camera_);

		// 裏面の攻撃エフェクト
		worldTransformAttack_.translation_ = worldTransform_.translation_;
		worldTransformAttack_.rotation_ = worldTransform_.rotation_;
		worldTransformAttack_.rotation_.y += std::numbers::pi_v<float> * 2.0f;
		worldTransformAttack_.rotation_.x += std::numbers::pi_v<float> * 2.0f;
		WorldTransformUpdate(worldTransformAttack_);
		modelAttack_->Draw(worldTransformAttack_, *camera_);
	}
}

void Player::OnCollision(const BaseEnemy* enemy) {
	(void)enemy;

	// 攻撃中はダメージ無効
	if (IsAttack()) {
		return;
	}

	isDead_ = true;
}

KamataEngine::Vector3 Player::GetWorldPosition() const {
	Vector3 worldPos;
	worldPos.x = worldTransform_.translation_.x;
	worldPos.y = worldTransform_.translation_.y;
	worldPos.z = worldTransform_.translation_.z;
	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::RequestBehavior(IPlayerBehavior* newBehavior, BehaviorChangeToken token) {
	behaviorRequest_ = newBehavior;
	(void)token;
}

void Player::RequestKnockbackBehavior() { RequestBehavior(new PlayerBehaviorKnockback, BehaviorChangeToken{}); }

void Player::Move() {
	Vector3 moveInput = {0.0f, 0.0f, 0.0f};
	bool isInput = false;

	if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		moveInput.x += 1.0f;
		isInput = true;
	}
	if (Input::GetInstance()->PushKey(DIK_LEFT)) {
		moveInput.x -= 1.0f;
		isInput = true;
	}
	if (Input::GetInstance()->PushKey(DIK_UP)) {
		moveInput.z += 1.0f;
		isInput = true;
	}
	if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		moveInput.z -= 1.0f;
		isInput = true;
	}

	if (isInput) {
		// 斜め移動で速くならないよう正規化
		if (Length(moveInput) > 0.0f) {
			moveInput = Normalize(moveInput);
		}

		// 加速
		velocity_.x += moveInput.x * kAcceleration;
		velocity_.z += moveInput.z * kAcceleration;

		// 旋回制御（入力があった方向へ向く）
		float targetAngle = std::atan2(moveInput.x, moveInput.z);
		// 旋回開始の記録（既存のTurnControlを活かす場合）
		if (std::abs(worldTransform_.rotation_.y - targetAngle) > 0.01f) {
			turnFirstRotationY_ = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
			// lrDirection_ の代わりに現在の目標角度を保持するようにしても良い
		}
	} else {
		// 非入力時は減衰
		velocity_.x *= (1.0f - kAttenuation);
		velocity_.z *= (1.0f - kAttenuation);
	}

	// 速度制限
	float speed = std::sqrtf(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speed > kLimitRunSpeed) {
		velocity_.x = (velocity_.x / speed) * kLimitRunSpeed;
		velocity_.z = (velocity_.z / speed) * kLimitRunSpeed;
	}
}

void Player::Knockback() {}

void Player::UpdateCollision(const Vector3& move) {
	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = move;

	// マップ衝突判定
	CheckMapCollision(collisionMapInfo);

	// 判定結果を反映
	ApplyCollision(collisionMapInfo);

	// 天井に当たったとき
	OnCeiling(collisionMapInfo);

	// 壁に当たったとき
	OnWall(collisionMapInfo);

	// 接地状態の切り替え
	UpdateGroundState(collisionMapInfo);

	// 攻撃回数の回復
	if (onGround_ && attackCounter_ < kAttackMaxCount) {
		attackCounter_ = kAttackMaxCount;
	}
}

float Player::GetBlinkVelocityX() {
	switch (direction4_) {
	case Direction4::kRight:
		return kAttackBlinkSpeed;
		break;
	case Direction4::kLeft:
		return -kAttackBlinkSpeed;
		break;
	}
	return 0.0f;
}

float Player::GetKnockbackVelocityX() {
	switch (direction4_) {
	case Direction4::kRight:
		return -kKnockbackSpeed;
		break;
	case Direction4::kLeft:
		return kKnockbackSpeed;
		break;
	}
	return 0.0f;
}

void Player::SetSizeEaseOut(const KamataEngine::Vector3& startSize, const KamataEngine::Vector3& endSize, float t) {
	worldTransform_.scale_.x = EaseLerp(startSize.x, endSize.x, t, EaseType::OutQuad);
	worldTransform_.scale_.y = EaseLerp(startSize.y, endSize.y, t, EaseType::OutQuad);
	worldTransform_.scale_.z = EaseLerp(startSize.z, endSize.z, t, EaseType::OutQuad);
}

void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionLeft(info);
	CheckMapCollisionRight(info);
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	if (info.moveAmount.z <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	bool hit = false;
	auto check = [&](const Vector3& pos) {
		MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(pos);
		if (mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.zIndex) == MapChipType::kBlock)
			hit = true;
	};
	check(positionsNew[kLeftTop]);
	check(positionsNew[kRightTop]);

	if (hit) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		// ブロックの下端 (rect.bottom) より手前で止める
		info.moveAmount.z = std::max(0.0f, rect.bottom - worldTransform_.translation_.z - kHeight / 2.0f - kBlank);
		info.isWall = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.moveAmount.z >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	bool hit = false;
	auto check = [&](const Vector3& pos) {
		MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(pos);
		if (mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.zIndex) == MapChipType::kBlock)
			hit = true;
	};
	check(positionsNew[kLeftBottom]);
	check(positionsNew[kRightBottom]);

	if (hit) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		// ブロックの上端 (rect.top) より奥で止める
		info.moveAmount.z = std::min(0.0f, rect.top - worldTransform_.translation_.z + kHeight / 2.0f + kBlank);
		info.isWall = true;
	}
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	// 右移動あり？
	if (info.moveAmount.x <= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	// 右の当たり判定を行う
	bool hit = false;
	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		info.moveAmount.x = std::min(0.0f, rect.left - worldTransform_.translation_.x - kWidth / 2.0f - kBlank);
		// 壁に当たったことを記録する
		info.isWall = true;
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	// 左移動あり？
	if (info.moveAmount.x >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 左の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.zIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.zIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		info.moveAmount.x = std::max(0.0f, rect.right - worldTransform_.translation_.x + kWidth / 2.0f + kBlank);
		// 壁に当たったことを記録する
		info.isWall = true;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, 0.0f, -kHeight / 2.0f},
        {-kWidth / 2.0f, 0.0f, -kHeight / 2.0f},
        {+kWidth / 2.0f, 0.0f, +kHeight / 2.0f},
        {-kWidth / 2.0f, 0.0f, +kHeight / 2.0f}
    };

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::ApplyCollision(const CollisionMapInfo& info) { worldTransform_.translation_ += info.moveAmount; }

void Player::OnCeiling(const CollisionMapInfo& info) {
	// 天井に当たった?
	if (info.isCeiling) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

void Player::OnWall(const CollisionMapInfo& info) {
	if (info.isWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
		velocity_.z *= (1.0f - kAttenuationWall);
	}
}

void Player::OnGround(const CollisionMapInfo& info) { info; }

void Player::UpdateGroundState(const CollisionMapInfo& info) {
	if (onGround_) {
		// 接地状態の処理

		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + Vector3(0, -kGroundCheckEpsilon, 0), static_cast<Corner>(i));
			}

			MapChipType mapChipType;

			// 真下の当たり判定を行う
			bool hit = false;

			// 左上点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.zIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右上点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.zIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 落下開始
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}
	} else {
		// 空中状態の処理

		// 接地したら
		if (info.isGround) {
			// 接地状態に移行
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
		}
	}
}

void Player::TurnControl() {
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= DeltaTime::Get();

		// Move()で計算した目標角度へ補完
		float targetAngle = std::atan2(velocity_.x, velocity_.z);

		// 角度の最短補間（Lerpの代わりにLerpShortAngleなどを使うのが理想）
		worldTransform_.rotation_.y = std::lerp(turnFirstRotationY_, targetAngle, std::clamp(1.0f - turnTimer_ / kTimeTurn, 0.0f, 1.0f));
	}
}

void Player::BehaviorRootUpdate() {
	// 移動入力
	Move();

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = velocity_;

	// マップ衝突判定
	CheckMapCollision(collisionMapInfo);

	// 判定結果を反映
	ApplyCollision(collisionMapInfo);

	// 天井に当たったとき
	OnCeiling(collisionMapInfo);

	// 壁に当たったとき
	OnWall(collisionMapInfo);

	// 接地状態の切り替え
	UpdateGroundState(collisionMapInfo);

	// 旋回制御
	TurnControl();

	WorldTransformUpdate(worldTransform_);
}

void Player::ChangeBehavior(IPlayerBehavior* newBehavior) {

	// 状態から抜けたときの処理
	if (behavior_) {
		behavior_->Exit(this);
	}

	// 状態変更
	behavior_.reset(newBehavior);

	// 状態に入ったときの処理
	if (behavior_) {
		behavior_->Enter(this);
	}
}
