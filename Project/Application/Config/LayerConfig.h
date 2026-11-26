#pragma once
#include <RyoEngine.h>
#include <cstdint>

//層の並びと、ボスが下の層へ落ちて足場が飛んでいく流れ
namespace App::LayerConfig
{
	//層
	inline constexpr float kFloorSpacing = 12.0f; //層と層の間の高さ
	inline constexpr float kLowerFloorBrightness = 0.25f; //下の層の明るさ
	inline constexpr int32_t kLayerFileCount = 5; //層のCSVの数で、6層目からは1層目に戻る
	inline constexpr int32_t kFirstLayerWidth = 10; //1層目のマスの数
	inline constexpr int32_t kFirstLayerDepth = 10;

	//ゲームを始めるときのボスのマスを1層目のマスで指定する
	inline constexpr int32_t kBossSpawnX = 6;
	inline constexpr int32_t kBossSpawnZ = 6;
	inline constexpr float kMinSpawnDistanceTiles = 5.0f; //層が始まるときにプレイヤーをボスから離すマスの数

	//下の層へ
	inline constexpr float kBossDropDuration = 0.8f; //ボスが下の層まで落ちる秒数
	inline constexpr float kDescentDuration = 0.8f; //ボスが落ちるときにカメラが上の層と下の層の間まで下がる秒数
	inline constexpr float kLandingImpactDuration = 0.5f; //ボスが落ちてから足場が飛んでいくまでの秒数
	inline constexpr float kTileFlightDuration = 0.8f; //足場が1枚ボスまで飛ぶ秒数
	inline constexpr float kTileLaunchWindow = 1.0f; //1つのまとまりの足場が飛び立ち終わるまでの秒数
	inline constexpr float kTileBarrageGroupPause = 0.25f; //穴のとなりの床を飛ばし終わってからふつうの床を飛ばすまでの秒数
}
