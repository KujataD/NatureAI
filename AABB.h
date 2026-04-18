#pragma once

#include <KamataEngine.h>

struct AABB {
	KamataEngine::Vector3 min;
	KamataEngine::Vector3 max;
};

struct Segment {
	KamataEngine::Vector3 origin; // 始点
	KamataEngine::Vector3 diff;   // 終点の差分ベクトル
};

static bool IsHitCollision(const AABB& aabb1, const AABB& aabb2) {
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}

static bool IsHitCollision(const AABB& aabb, const Segment& segment) {
	float txMin = (aabb.min.x - segment.origin.x) / segment.diff.x;
	float txMax = (aabb.max.x - segment.origin.x) / segment.diff.x;
	float tyMin = (aabb.min.y - segment.origin.y) / segment.diff.y;
	float tyMax = (aabb.max.y - segment.origin.y) / segment.diff.y;
	float tzMin = (aabb.min.z - segment.origin.z) / segment.diff.z;
	float tzMax = (aabb.max.z - segment.origin.z) / segment.diff.z;

	float tNearX = (std::min)(txMin, txMax);
	float tNearY = (std::min)(tyMin, tyMax);
	float tNearZ = (std::min)(tzMin, tzMax);
	float tFarX = (std::max)(txMin, txMax);
	float tFarY = (std::max)(tyMin, tyMax);
	float tFarZ = (std::max)(tzMin, tzMax);

	// AABBとの衝突点(貫通点)のtが小さい方
	float tmin = (std::max)((std::max)(tNearX, tNearY), tNearZ);
	// AABBとの衝突点(貫通点)のtが大きい方
	float tmax = (std::min)((std::min)(tFarX, tFarY), tFarZ);
	if (tmin <= tmax) {
		if (tmax >= 0.0f && tmin <= 1.0f) {
			return true;
		}
	}

	return false;
}
