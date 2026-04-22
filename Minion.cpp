#include "Minion.h"
#include "Transform.h"
#include <cmath>

using namespace KamataEngine;
using namespace MathUtility;

void Minion::Init(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& startPos) {

	model_ = model;
	camera_ = camera;

	// トランスフォーム
	worldTransform_.Initialize();
	float offsetX = (float)(rand() % 100 - 50) * 0.01f;
	float offsetZ = (float)(rand() % 100 - 50) * 0.01f;

	worldTransform_.translation_ = {startPos.x + offsetX, startPos.y, startPos.z + offsetZ};
	goalPos_ = worldTransform_.translation_;
	worldTransform_.scale_ = {0.4f, 0.4f, 0.4f};
	WorldTransformUpdate(worldTransform_);

	// オブジェクトカラー
	objColor_ = new ObjectColor;
	objColor_->Initialize();
}

void Minion::Update(MinionManager* manager) {
	Move(manager);
	WorldTransformUpdate(worldTransform_);
}

void Minion::Draw() { model_->Draw(worldTransform_, *camera_, objColor_); }

KamataEngine::Vector3 Minion::Limit(Vector3 v, float max) {
	float length = Length(v);
	if (length > max) {
		Vector3 result = Normalize(v);
		result.x *= max;
		result.y *= max;
		result.z *= max;
		return result;
	}
	return v;
}

void Minion::Move(MinionManager* manager) {
	// 次のゴール地点
	bool canPathFinding = false;
	// パスファインディング
	if (!path_.empty()) {
		// 範囲外阻止
		if (currentStep_ + 1 < (int)path_.size()) {
			canPathFinding = true;
		}
	}

	if (canPathFinding) {
		goalPos_ = mapChipField_->GetMapChipPositionByIndex(path_[currentStep_ + 1].xIndex, path_[currentStep_ + 1].zIndex);
	}

	// 追跡行動
	Vector3 acceleration = Seek(goalPos_) + Separation(manager) * kSeparationWeight;
	velocity_ += acceleration;
	velocity_ = Limit(velocity_, kMaxSpeed);
	worldTransform_.translation_ += velocity_;

	// 同じマスに到着したら次のマスを目指す。
	if (canPathFinding) {
		// 現時点のインデックスセット
		MapChipField::IndexSet currentIdxSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

		if (currentIdxSet == path_[currentStep_ + 1]) {
			currentStep_++;
		}
	}
}

KamataEngine::Vector3 Minion::Separation(MinionManager* manager) {
	Vector3 separation = {0.0f, 0.0f, 0.0f};
	for (Minion* other : manager->GetMinions()) {
		if (other == this) {
			continue;
		}
		// ほかのミニオン→ミニオン
		Vector3 difference = worldTransform_.translation_ - other->GetWorldPosition();
		float distance = Length(difference);
		if (distance < 2.0f) {
			separation += Normalize(difference) / distance;
		}
	}
	separation = Limit(separation, kMaxSteer);
	return separation;
}

KamataEngine::Vector3 Minion::Alignment(MinionManager* manager) {
	manager;
	return KamataEngine::Vector3();
}

KamataEngine::Vector3 Minion::Cohesion(MinionManager* manager) {
	manager;
	return KamataEngine::Vector3();
}

KamataEngine::Vector3 Minion::Seek(const KamataEngine::Vector3& targetPos) {
	// 目標速度を求める
	Vector3 desired = targetPos - worldTransform_.translation_;
	desired = Normalize(desired) * kMaxSpeed;

	// どれくらい速度を変化させるか
	Vector3 steer = desired - velocity_;

	// 急カーブを防ぐ
	steer = Limit(steer, kMaxSteer);

	return steer;
}
