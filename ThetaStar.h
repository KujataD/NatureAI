#pragma once
#include "MapChipField.h"
#include <numbers>
#include <vector>

struct Node {
	int32_t x, y;
	float g; // スタート地点から調査マスまでの距離（実コスト）
	float h; // 調査マスからゴールまでの距離（推定コスト）
	float f; // g + h（優先度）
	Node* parent;

	Node(int32_t x, int32_t y) : x(x), y(y), g(0.0f), h(0.0f), f(0.0f), parent(nullptr) {}
	bool operator>(const Node& other) const { return f > other.f; }
};

class ThetaStar {
public:
	enum class HeuristicAlgorithm {
		kManhattan,
		kEuclidean,
	};

public:
	ThetaStar() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="mapChipField"></param>
	void Init(MapChipField* mapChipField);

	/// <summary>
	/// 探索関数
	/// </summary>
	/// <param name="start"></param>
	/// <param name="goal"></param>
	/// <returns></returns>
	std::vector<MapChipField::IndexSet> FindPath(MapChipField::IndexSet start, MapChipField::IndexSet goal);

	/// <summary>
	/// ヒューリスティックの距離アルゴリズムを変更する
	/// </summary>
	/// <param name="heuristicAlgorithm"></param>
	void SetHeuristicAlgorithm(HeuristicAlgorithm heuristicAlgorithm) { heuristicAlgorithm_ = heuristicAlgorithm; }

private:
	const MapChipField* mapChipField_;
	HeuristicAlgorithm heuristicAlgorithm_ = HeuristicAlgorithm::kEuclidean;

private:
	/// <summary>
	/// 歩ける場所の設定
	/// </summary>
	/// <param name="x"></param>
	/// <param name="z"></param>
	/// <returns></returns>
	bool IsWalkable(uint32_t x, uint32_t z) const { return mapChipField_->GetMapChipTypeByIndex(x, z) != MapChipType::kBlock; }

	/// <summary>
	/// ヒューリスティック関数
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	float Heuristic(MapChipField::IndexSet a, MapChipField::IndexSet b) const;

	bool HasLineOfSight(const Node* a, const Node* b);

	float Distance(const Node* a, const Node* b);

};