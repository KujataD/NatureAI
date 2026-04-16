#pragma once
#include <windows.h>

// 参考資料:
// https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/game-timing-and-multicore-processors


class DeltaTime {
public:
	
	static void Init();

	static void Update();

	static float Get() { return dt_; }

private:
	static float dt_;

	// 周波数取得
	static LARGE_INTEGER frequency_;

	// 開始時刻を取得
	static LARGE_INTEGER prevTime_;

};
