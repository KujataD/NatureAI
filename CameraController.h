#pragma once
#include "Rect.h"
#include <KamataEngine.h>
#include <numbers>

class Player;

/// <summary>
/// カメラコントローラ
/// </summary>
class CameraController {
public:
	void Init(KamataEngine::Camera* camera);

	void Update();

	void Reset();

	void SetTarget(Player* target) { target_ = target; }

	void SetMovableArea(Rect& area) { movableArea_ = area; }

private:
	// 定数
	// ------------------------------------------

	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	// 速度掛け算
	static inline const float kVelocityBiasHorizontal = 15.0f;
	static inline const float kVelocityBiasVartical = 2.0f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect kMarginArea = {-10.0f, 10.0f, -5.0f, 5.0f};

	// 変数
	// ------------------------------------------

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// ターゲット
	Player* target_ = nullptr;

	// ターゲットのオフセット
	KamataEngine::Vector3 targetOffset_ = {0.0f, 25.0f, 0.0f};
	KamataEngine::Vector3 rotation_ = {std::numbers::pi_v<float> * 0.49f, 0.0f, 0.0f};

	// カメラ移動範囲
	Rect movableArea_ = {10.5f, 88.5f, 5.5f, 100.0f};

	// カメラの目標座標
	KamataEngine::Vector3 goalPosition_;
};
