#include "AStar.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include <unordered_set>

// aはbより後回しにすべきかどうか
// 例) a>bの時、小さいbを優先すべきなので、後回しにすべき
struct CompareFCost {
	bool operator()(Node* a, Node* b) const { return a->f > b->f; }
};

void AStar::Init(MapChipField* mapChipField) {
	assert(mapChipField);
	mapChipField_ = mapChipField;
}

std::vector<MapChipField::IndexSet> AStar::FindPath(MapChipField::IndexSet start, MapChipField::IndexSet goal) {
	// 返り値
	std::vector<MapChipField::IndexSet> path;

	// 優先度付きキューを用いて、次に調べる候補ノードをここにいれる
	std::priority_queue<Node*, std::vector<Node*>, CompareFCost> openList;

	// 記録してきたすべてのノード
	std::vector<Node*> allNodes;

	// すでに調べた場所
	std::vector<std::vector<bool>> closed(mapChipField_->GetNumBlockVirtical(), std::vector<bool>(mapChipField_->GetNumBlockHorizontal(), false));

	// オープンリストに入っている場所のリスト
	std::vector<std::vector<bool>> inOpen(mapChipField_->GetNumBlockVirtical(), std::vector<bool>(mapChipField_->GetNumBlockHorizontal(), false));

	// --- スタートノード ---
	Node* startNode = new Node(start.xIndex, start.zIndex);
	startNode->g = 0.0f;
	startNode->h = Heuristic(start, goal);
	startNode->f = startNode->h;
	startNode->parent = nullptr;

	openList.push(startNode); // push() で追加
	allNodes.push_back(startNode);

	// 調べていないノードがなくなるまでループ
	while (!openList.empty()) {

		Node* currentNode = openList.top(); // 先頭取得
		openList.pop();                     // 先頭削除

		// --- ゴール判定 ---
		if (currentNode->x == static_cast<int32_t>(goal.xIndex) && currentNode->y == static_cast<int32_t>(goal.zIndex)) {

			Node* node = currentNode;
			while (node) {
				path.push_back({(uint32_t)node->x, (uint32_t)node->y});
				node = node->parent;
			}

			std::reverse(path.begin(), path.end());

			// メモリ解放
			for (Node* n : allNodes) {
				delete n;
			}

			return path;
		}

		closed[currentNode->y][currentNode->x] = true;

		// --- 隣接しているノードを調べる ---

		// 上下左右の組み合わせ
		const int dx[8] = {1, -1, 0, 0, 1, -1, 1, -1};
		const int dz[8] = {0, 0, 1, -1, 1, 1, -1, -1};

		// 8方向アルゴリズムの場合は4
		int indexNum = 8;

		// 4方向アルゴリズムの場合は4
		if (heuristicAlgorithm_ == HeuristicAlgorithm::kManhattan) {
			indexNum = 4;
		}

		for (int i = 0; i < indexNum; i++) {

			// 近接マス
			uint32_t neighborX = currentNode->x + dx[i];
			uint32_t neighborZ = currentNode->y + dz[i];

			// 範囲チェック
			if (neighborX < 0 || neighborX >= mapChipField_->GetNumBlockHorizontal()) {
				continue;
			}
			if (neighborZ < 0 || neighborZ >= mapChipField_->GetNumBlockVirtical()) {
				continue;
			}

			// 通行チェック
			if (!IsWalkable(neighborX, neighborZ)) {
				continue;
			}

			// すでに調べていたらスキップ
			if (closed[neighborZ][neighborX]) {
				continue;
			}

			// 隣接ノードを追加する前にチェック
			if (inOpen[neighborZ][neighborX]) {
				continue; // 重複追加を防ぐ
			}

			bool isDiagonal = (i >= 4); // 4番目以降が斜め
			// 斜め移動する際、壁にめり込まないようにする。
			if (isDiagonal) {
				// 右上に進む場合、右が壁でないかどうか
				bool xWalkable = IsWalkable(currentNode->x + dx[i], currentNode->y); 
				// 右上に進む場合、上が壁でないかどうか
				bool zWalkable = IsWalkable(currentNode->x, currentNode->y + dz[i]);
				if (!xWalkable || !zWalkable) {
					continue;
				}
			}

			// 近接ノード
			Node* neighbor = new Node(neighborX, neighborZ);
			float stepCost = isDiagonal ? std::numbers::sqrt2_v<float> : 1.0f; // 斜め移動の時はコストを上げる
			neighbor->g = currentNode->g + stepCost;
			neighbor->h = Heuristic({(uint32_t)neighborX, (uint32_t)neighborZ}, goal);
			neighbor->f = neighbor->g + neighbor->h;
			neighbor->parent = currentNode;

			openList.push(neighbor); // push() で追加
			allNodes.push_back(neighbor);
		}
	}

	// 見つからなかった場合
	for (Node* n : allNodes) {
		delete n;
	}

	return {};
}

float AStar::Heuristic(MapChipField::IndexSet a, MapChipField::IndexSet b) const {
	float baX = std::abs(static_cast<float>(a.xIndex) - static_cast<float>(b.xIndex));
	float baZ = std::abs(static_cast<float>(a.zIndex) - static_cast<float>(b.zIndex));

	switch (heuristicAlgorithm_) {
	case HeuristicAlgorithm::kManhattan:
		return baX + baZ;

	case HeuristicAlgorithm::kEuclidean:
		return std::sqrt(baX * baX + baZ * baZ);
	}
	return baX + baZ;
}
