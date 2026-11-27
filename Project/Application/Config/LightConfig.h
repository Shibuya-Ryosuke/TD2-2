#pragma once
#include <RyoEngine.h>

//全部のシーンで使う太陽みたいな光と、影の側を明るくする光の数値
namespace App::LightConfig
{
	//太陽の光で、影を落とすのはこの光だけ
	inline constexpr const char* kSunName = "MainLight"; //エンジンが最初に作る0番の光の名前
	inline constexpr RyoEngine::Vector3 kSunDirection = { 0.3f, -1.0f, 0.85f }; //光が進む向きで、カメラの手前の左上から差しこんで顔とおなかを照らす
	inline constexpr RyoEngine::Vector4 kSunColor = { 1.0f, 0.95f, 0.86f, 1.0f }; //少しあたたかい白
	inline constexpr float kSunIntensity = 0.8f; //太陽の明るさで、底上げと合わせて上の面が白飛びしない明るさ

	//影の側をうっすら照らす光で、影は落とさない
	inline constexpr const char* kFillName = "FillLight";
	inline constexpr RyoEngine::Vector3 kFillDirection = { -0.8f, -0.35f, 0.3f }; //太陽の当たらない右の面を、右から照らす
	inline constexpr RyoEngine::Vector4 kFillColor = { 0.85f, 0.82f, 1.0f, 1.0f }; //空の色に合わせたうすい紫
	inline constexpr float kFillIntensity = 0.25f;

	//どこにでも当たる底上げの光で、影の中や下の層も暗くなりすぎない
	inline constexpr RyoEngine::Vector4 kAmbientColor = { 1.0f, 0.94f, 0.98f, 1.0f }; //うすいピンクの白
	inline constexpr float kAmbientIntensity = 0.35f; //前は0.1でまっ暗だった

	//上の層の床の影で下の層がまっ暗にならないように、床のタイルは影を落とさない
	inline constexpr bool kFloorCastsShadow = false;
}
