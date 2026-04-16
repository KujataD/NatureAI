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
	worldTransform_.translation_ = startPos;
	worldTransform_.scale_ = {0.4f, 0.4f, 0.4f};
	WorldTransformUpdate(worldTransform_);

	// オブジェクトカラー
	objColor_ = new ObjectColor;
	objColor_->Initialize();
	objColor_->SetColor(Vector4{0.1f, 0.1f, 1.0f, 1.0f});
}

void Minion::Update() {
	Move();
	WorldTransformUpdate(worldTransform_);
}

void Minion::Draw() { model_->Draw(worldTransform_, *camera_, objColor_); }

void Minion::Move() {
	if (path_.empty()) {
		return;
	}

	if (currentStep_ + 1 >= (int)path_.size()) {
		return;
	}

	// 次のゴール地点
	Vector3 goalPos = mapChipField_->GetMapChipPositionByIndex(path_[currentStep_ + 1].xIndex, path_[currentStep_ + 1].zIndex);

	// 現時点のインデックスセット
	MapChipField::IndexSet currentIdxSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

	// 進行方向を取得
	Vector3 direction = goalPos - worldTransform_.translation_;
	direction = Normalize(direction);

	worldTransform_.translation_ += direction * kMoveSpeed;

	// 同じマスに到着したら次のマスを目指す。
	if (currentIdxSet == path_[currentStep_ + 1]) {
		currentStep_++;
	}
}
