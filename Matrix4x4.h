#pragma once
#include <KamataEngine.h>

//KamataEngine::Matrix4x4 Add(const KamataEngine::Matrix4x4 a, const KamataEngine::Matrix4x4 b);
//KamataEngine::Matrix4x4 Sub(const KamataEngine::Matrix4x4 a, const KamataEngine::Matrix4x4 b);
//KamataEngine::Matrix4x4 Mul(const KamataEngine::Matrix4x4 a, const KamataEngine::Matrix4x4 b);

 KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);
 KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);
 KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian);
 KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian);
 KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);
 KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);
