#pragma once
#include <RyoEngine.h>
#include <cstdint>

//層の並びと、ボスが下の層へ落ちる秒数
namespace App::LayerConfig
{
	//層
	inline constexpr float kFloorSpacing = 12.0f; //層と層の間の高さ
	inline constexpr float kLowerFloorBrightness = 0.25f; //下の層の明るさ
	inline constexpr const char* kBattleCsvPath = "Resources/ApplicationResources/Stage/layer1.csv"; //本編の各層で共通に使う10マス四方の床
	inline constexpr int32_t kFirstLayerWidth = 10; //1層目のマスの数
	inline constexpr int32_t kFirstLayerDepth = 10;

	//ゲームを始めるときのボスのマスを1層目のマスで指定する
	inline constexpr int32_t kBossSpawnX = 6;
	inline constexpr int32_t kBossSpawnZ = 6;
	inline constexpr float kMinSpawnDistanceTiles = 5.0f; //層が始まるときにプレイヤーをボスから離すマスの数

	//下の層へ
	inline constexpr float kBossDropDuration = 0.8f; //ボスが下の層まで落ちる秒数
}
