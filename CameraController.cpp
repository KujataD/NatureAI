#include "CameraController.h"
#include "Math.h"
#include "Player.h"
#include <algorithm>

using namespace KamataEngine;
using namespace MathUtility;

void CameraController::Init(Camera* camera) {
	// カメラの設定
	camera_ = camera;

	// カメラの初期化
	camera_->Initialize();

	camera_->rotation_ = rotation_;
}

void CameraController::Update() {
	
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	Vector3 targetVelocity = {
		target_->GetVelocity().x * kVelocityBiasHorizontal,
		target_->GetVelocity().y * kVelocityBiasVartical,
		target_->GetVelocity().z
	};

	goalPosition_ = targetWorldTransform.translation_ + targetOffset_ + targetVelocity;

	// 座標補間によりゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, goalPosition_, kInterpolationRate);

	// 追従対象が画面外に出ないように補正
	camera_->translation_.x = std::clamp(camera_->translation_.x, targetWorldTransform.translation_.x + kMarginArea.left, targetWorldTransform.translation_.x + kMarginArea.right);
	camera_->translation_.z = std::clamp(camera_->translation_.z, targetWorldTransform.translation_.z + kMarginArea.bottom, targetWorldTransform.translation_.z + kMarginArea.top);

	// 移動範囲制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.z = std::clamp(camera_->translation_.z, movableArea_.bottom, movableArea_.top);

	// 行列を更新する
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}
