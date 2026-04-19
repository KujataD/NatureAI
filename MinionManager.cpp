#include "MinionManager.h"
#include "Minion.h"

void MinionManager::Init(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& startPos) {
	minions_.clear();
	minions_.reserve(5); // 要素数確保（値は追加されない）

	for (int32_t i = 0; i < 5; i++) {
		Minion* minion;
		minion = new Minion;
		minion->Init(model, camera, startPos);
		minions_.push_back(minion);
	}
}

void MinionManager::Update() {
	for (Minion* minion : minions_) {
		minion->Update(this);
	}
}

void MinionManager::Draw() {
	for (Minion* minion : minions_) {
		minion->Draw();
	}
}

void MinionManager::SetPas(const std::vector<MapChipField::IndexSet>& path) {
	for (Minion* minion : minions_) {
		minion->SetPas(path);
	}
}

void MinionManager::SetMapChipField(MapChipField* mapChipField) {
	for (Minion* minion : minions_) {
		minion->SetMapChipField(mapChipField);
	}
}

void MinionManager::SetColor(const KamataEngine::Vector4& color) {
	for (Minion* minion : minions_) {
		minion->SetColor(color);
	}
}
