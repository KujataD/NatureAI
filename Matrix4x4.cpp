#include "Matrix4x4.h"
#include <cmath>

KamataEngine::Matrix4x4 Add(const KamataEngine::Matrix4x4 a, const KamataEngine::Matrix4x4 b) {
	return {
	    {
         {
	            a.m[0][0] + b.m[0][0],
	            a.m[0][1] + b.m[0][1],
	            a.m[0][2] + b.m[0][2],
	            a.m[0][3] + b.m[0][3],
	        }, {
	            a.m[1][0] + b.m[1][0],
	            a.m[1][1] + b.m[1][1],
	            a.m[1][2] + b.m[1][2],
	            a.m[1][3] + b.m[1][3],
	        }, {
	            a.m[2][0] + b.m[2][0],
	            a.m[2][1] + b.m[2][1],
	            a.m[2][2] + b.m[2][2],
	            a.m[2][3] + b.m[2][3],
	        }, {
	            a.m[3][0] + b.m[3][0],
	            a.m[3][1] + b.m[3][1],
	            a.m[3][2] + b.m[3][2],
	            a.m[3][3] + b.m[3][3],
	        }, }
    };
}

KamataEngine::Matrix4x4 Sub(const KamataEngine::Matrix4x4 a, const KamataEngine::Matrix4x4 b) {
	return {
	    {
         {
	            a.m[0][0] - b.m[0][0],
	            a.m[0][1] - b.m[0][1],
	            a.m[0][2] - b.m[0][2],
	            a.m[0][3] - b.m[0][3],
	        }, {
	            a.m[1][0] - b.m[1][0],
	            a.m[1][1] - b.m[1][1],
	            a.m[1][2] - b.m[1][2],
	            a.m[1][3] - b.m[1][3],
	        }, {
	            a.m[2][0] - b.m[2][0],
	            a.m[2][1] - b.m[2][1],
	            a.m[2][2] - b.m[2][2],
	            a.m[2][3] - b.m[2][3],
	        }, {
	            a.m[3][0] - b.m[3][0],
	            a.m[3][1] - b.m[3][1],
	            a.m[3][2] - b.m[3][2],
	            a.m[3][3] - b.m[3][3],
	        }, }
    };
}

KamataEngine::Matrix4x4 Mul(const KamataEngine::Matrix4x4 a, const KamataEngine::Matrix4x4 b) {
	return {
	    {
         {
	            a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0],
	            a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1],
	            a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2],
	            a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] + a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3],
	        }, {a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0],
	         a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1],
	         a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2],
	         a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] + a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3]},
         {
	            a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0],
	            a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1],
	            a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2],
	            a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] + a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3],
	        }, {
	            a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0],
	            a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1],
	            a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2],
	            a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] + a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3],
	        }, }
    };
}

KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate) {
	return {{
	    {1.0f, 0.0f, 0.0f, 0.0f},
	    {0.0f, 1.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 1.0f, 0.0f},
	    {translate.x, translate.y, translate.z, 1.0f},
	}};
}

KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale) {
	return {{
	    {scale.x, 0.0f, 0.0f, 0.0f},
	    {0.0f, scale.y, 0.0f, 0.0f},
	    {0.0f, 0.0f, scale.z, 0.0f},
	    {0.0f, 0.0f, 0.0f, 1.0f},
	}};
}

KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian) {
	return {{
	    {1.0f, 0.0f, 0.0f, 0.0f},
	    {0.0f, std::cos(radian), std::sin(radian), 0.0f},
	    {0.0f, -std::sin(radian), std::cos(radian), 0.0f},
	    {0.0f, 0.0f, 0.0f, 1.0f},
	}};
}

KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian) {
	return {{
	    {std::cos(radian), 0.0f, -std::sin(radian), 0.0f},
	    {0.0f, 1.0f, 0.0f, 0.0f},
	    {std::sin(radian), 0.0f, std::cos(radian), 0.0f},
	    {0.0f, 0.0f, 0.0f, 1.0f},
	}};
}

KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian) {
	return {{
	    {std::cos(radian), std::sin(radian), 0.0f, 0.0f},
	    {-std::sin(radian), std::cos(radian), 0.0f, 0.0f},
	    {0.0f, 0.0f, 1.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f, 1.0f},
	}};
}


KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate) {
	KamataEngine::Matrix4x4 s = MakeScaleMatrix(scale);

	KamataEngine::Matrix4x4 rx = MakeRotateXMatrix(rotate.x);
	KamataEngine::Matrix4x4 ry = MakeRotateYMatrix(rotate.y);
	KamataEngine::Matrix4x4 rz = MakeRotateZMatrix(rotate.z);

	KamataEngine::Matrix4x4 r = Mul(rx, ry);
	r = Mul(r, rz);

	KamataEngine::Matrix4x4 t = MakeTranslateMatrix(translate);

	KamataEngine::Matrix4x4 w = Mul(s, r);
	w = Mul(w, t);

	return w;
}