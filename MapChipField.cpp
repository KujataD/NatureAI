#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>

using namespace KamataEngine;

// 無名ネームスペースで囲むことで内部リンケージを持つようになり、グローバル変数の名前の衝突を避けることができる。
namespace {
std::map<char, MapChipType> mapChipTypeTable = {
    {'B', MapChipType::kBlock },
    {'P', MapChipType::kPlayer},
    {'E', MapChipType::kEnemy },
};
} // namespace

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockDepth);
	for (std::vector<MapChipDataUnit>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {

	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockDepth; ++i) {
		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream lineStream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {

			std::string word;
			std::getline(lineStream, word, ',');

			// 空白の場合はスキップ
			if (word.empty()) {
				continue;
			}

			// 先頭文字がいずれかのマップチップ種別に該当するか確認
			if (!mapChipTypeTable.contains(word[kChipType])) {
				continue;
			}

			// 先頭文字でマップチップのタイプを判別
			mapChipData_.data[i][j].type = mapChipTypeTable[word[kChipType]];

			// --- サブID ---

			// サブIDを含まない場合はスキップ(0番で確定)
			if (word.size() <= kChipSubID) {
				continue;
			}

			// マップチップのサブIDを設定
			mapChipData_.data[i][j].subID = static_cast<uint8_t>(word[kChipSubID] - '0');
		}
	}
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t zIndex) const {
	// 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, zIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.z - kBlockDepth / 2.0f;
	rect.top = center.z + kBlockDepth / 2.0f;

	return rect;
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) const {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.zIndex = kNumBlockDepth - 1 - static_cast<uint32_t>((position.z + kBlockDepth / 2.0f) / kBlockDepth);
	return indexSet;
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t zIndex) const {
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}

	if (zIndex < 0 || kNumBlockDepth - 1 < zIndex) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[zIndex][xIndex].type;
}

uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t zIndex) const { return mapChipData_.data[zIndex][xIndex].subID; }

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex) const { return Vector3(kBlockWidth * xIndex, 0, kBlockDepth * (kNumBlockDepth - 1 - zIndex)); }

AABB MapChipField::GetAABBByIndex(uint32_t xIndex, uint32_t zIndex) { // 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, zIndex);

	AABB aabb;
	aabb.min = {center.x - kBlockWidth / 2.0f, center.y - kBlockWidth / 2.0f, center.z - kBlockWidth / 2.0f};
	aabb.max = {center.x + kBlockWidth / 2.0f, center.y + kBlockWidth / 2.0f, center.z + kBlockWidth / 2.0f};

	return aabb;
}
