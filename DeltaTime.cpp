#include "DeltaTime.h"

float DeltaTime::dt_ = 0.0f;
LARGE_INTEGER DeltaTime::frequency_ = {0};
LARGE_INTEGER DeltaTime::prevTime_ = {0};


void DeltaTime::Init() {
	QueryPerformanceFrequency(&frequency_);
	QueryPerformanceCounter(&prevTime_);
}

void DeltaTime::Update() { 
	// 現在の時刻を取得
	LARGE_INTEGER currentTime; 
	QueryPerformanceCounter(&currentTime);

	dt_ = static_cast <float> (currentTime.QuadPart - prevTime_.QuadPart) / static_cast<float>(frequency_.QuadPart);

	prevTime_ = currentTime;
}
