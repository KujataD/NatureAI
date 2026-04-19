#pragma once
#include "MapChipField.h"
#include <KamataEngine.h>
#include <vector>

class Minion;
class MinionManager {
public:
	void Init(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& startPos);

	void Update();

	void Draw();

	void SetPas(const std::vector<MapChipField::IndexSet>& path);


	void SetPushMinion(Minion* pushMinion) { minions_.push_back(pushMinion); }

	std::vector<MapChipField::IndexSet> GetPath() const { return path_; }
	const std::vector<Minion*>& GetMinions() const { return minions_; }
	void SetMapChipField(MapChipField* mapChipField);
	void SetColor(const KamataEngine::Vector4& color);

public:
	std::vector<Minion*> minions_;
	std::vector<MapChipField::IndexSet> path_;
};