#pragma once
#include <KamataEngine.h>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank,  // 空白
	kBlock,  // ブロック
	kPlayer, // プレイヤー
	kEnemy,  // 敵
};

struct MapChipDataUnit {
	MapChipType type; // マップチップの種別
	uint8_t subID;    // 種類ごとのサブID
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

class MapChipField {
public:
	// 矩形
	struct Rect {
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};
	struct IndexSet {
		uint32_t xIndex;
		uint32_t zIndex;
		bool operator==(const IndexSet& idx) const { return xIndex == idx.xIndex && zIndex == idx.zIndex; }
	};

	// マップチップCSVの文字番号
	enum MapChipCharIndex {
		kChipType = 0, // マップチップタイプ
		kChipSubID = 1 // タイプごとのサブID
	};

public:
	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	// Accessor
	// ------------------------------------------

	// --- get ---

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const;

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) const;

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t zIndex) const;

	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t zIndex) const;

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex) const;

	float GetBlockWidth() const { return kBlockWidth; }
	float GetBlockHeight() const { return kBlockDepth; }

	uint32_t GetNumBlockVirtical() const { return kNumBlockDepth; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockDepth = 1.0f;

	// ブロック個数
	static inline const uint32_t kNumBlockDepth = 20;
	static inline const uint32_t kNumBlockHorizontal = 20;

	MapChipData mapChipData_;
};
