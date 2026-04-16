#pragma once
#include "MapChipField.h"
#include <KamataEngine.h>
#include <vector>

class Minion {
public:
	enum class State {
		kFollow,
		kLost,
	};

public:
	Minion() = default;
	~Minion() = default;

	void Init(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& startPos);
	void Update();
	void Draw();

	// Accessor
	// ------------------------------------------

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void SetColor(const KamataEngine::Vector4& color) { objColor_->SetColor(color); }
	void SetPas(const std::vector<MapChipField::IndexSet> path) {
		path_ = path;
		currentStep_ = 0;
	}
	void SetPlayerPos(const KamataEngine::Vector3& playerPos) { playerPos_ = playerPos; }

private:
	void Move();

private:
	static inline const float kMoveSpeed = 0.05f;
	static inline const float kAcceleration = 0.01f;
	static inline const float kMaxSpeed = 0.09f;

private:
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::ObjectColor* objColor_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 playerPos_;
	KamataEngine::Vector3 velocity_;

	// 経路
	std::vector<MapChipField::IndexSet> path_;
	// 現在のステップ
	int32_t currentStep_ = 0;
};
