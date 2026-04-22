#pragma once
#include "MinionManager.h"
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
	void Update(MinionManager* manager);
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

private:

	KamataEngine::Vector3 Limit(KamataEngine::Vector3 v, float max);
	void Move(MinionManager* manager);

	/// <summary>
	/// 分離
	/// </summary>
	/// <param name="manager"></param>
	/// <returns></returns>
	KamataEngine::Vector3 Separation(MinionManager* manager);
	
	/// <summary>
	/// 整列
	/// </summary>
	/// <param name="manager"></param>
	/// <returns></returns>
	KamataEngine::Vector3 Alignment(MinionManager* manager);
	
	/// <summary>
	/// 結束
	/// </summary>
	/// <param name="manager"></param>
	/// <returns></returns>
	KamataEngine::Vector3 Cohesion(MinionManager* manager);
	KamataEngine::Vector3 Seek(const KamataEngine::Vector3& targetPos);


private:
	static inline const float kMoveSpeed = 0.05f;
	static inline const float kAcceleration = 0.01f;
	static inline const float kMaxSpeed = 0.075f; // 最高スピード
	static inline const float kMaxSteer = 0.005f; // ハンドルの切れやすさ

	static inline const float kSeparationRange = 1.0f;	// 分離の対象半径
	static inline const float kSeparationWeight = 0.7f; // 分離の重み付け

private:
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::ObjectColor* objColor_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// 初期座標
	KamataEngine::Vector3 initPosition_;
	
	// 速度
	KamataEngine::Vector3 velocity_;
	KamataEngine::Vector3 goalPos_;
	

	// 経路
	std::vector<MapChipField::IndexSet> path_;
	
	// 現在のステップ
	int32_t currentStep_ = 0;
};
