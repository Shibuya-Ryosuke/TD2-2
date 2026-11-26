#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "FloorConfig.h"

//プレイヤーの動きとハートとモデル
namespace App::PlayerConfig
{
	//動き
	inline constexpr float kJumpHeight = 1.5f * FloorConfig::kWorldScale; //となりのマスへ跳ぶときの高さ
	inline constexpr float kJumpDuration = 0.48f; //となりのマスへ跳ぶ秒数
	inline constexpr float kGravity = 20.0f; //重力の強さ
	inline constexpr float kFallThreshold = 1.1f; //床よりこれだけ下まで落ちたら落ちたことにする
	inline constexpr float kPlayerSize = 0.78f; //仮の箱だったときの1辺でアイテムの当たりに使う
	inline constexpr float kPlayerContactRadius = 0.39f; //床のふちとボスとの当たりに使う体の半径
	inline constexpr float kPlayerContactHeight = 1.3f; //ボスとの当たりに使う体の高さ

	//ハート
	inline constexpr int32_t kMaxHearts = 5;
	inline constexpr int32_t kFallDamage = 1; //穴から落ちたときのダメージ
	inline constexpr int32_t kDescentHeal = 1; //下の層へ降りたときに回復する数
	inline constexpr float kInvincibilityTime = 1.0f; //当たったあと無敵の秒数
	inline constexpr float kHitBlinkFrequency = 9.0f; //無敵の間に1秒に点滅する回数

	//モデルで色はmtlのまま使う
	inline constexpr const char* kPlayerModel = "animation/Player/playerIdol.glb"; //待機のアニメーションが入ったボーンつきのプレイヤーのモデル
	inline constexpr float kPlayerModelScale = 0.75f; //プレイヤーのモデルの大きさの倍率
	inline constexpr float kPlayerModelYaw = 90.0f; //横を向いているプレイヤーのモデルを正面に向ける度数
	inline constexpr float kPlayerModelLift = 0.04f; //足の裏が床に来るようにプレイヤーのモデルを上げる高さ

	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// //HPが少ないとき1層に1個だけ降ってくるアイテム
	// inline constexpr int32_t kItemSpawnHearts = 3;          //HPがこれ以下になったら降ってくる
	// inline constexpr float kItemSize = 0.5f;                //箱の1辺
	// inline constexpr float kItemHover = 0.35f;              //床から箱の下の面まで浮かせる高さ
	// inline constexpr float kItemDropHeight = 10.0f;         //床から降ってくる高さ
	// inline constexpr float kItemDropDuration = 0.7f;        //降ってくる秒数
	// inline constexpr float kItemSpinSpeed = 120.0f;         //1秒に回る度数
	// inline constexpr float kItemTilt = 30.0f;               //度の傾き
	// inline constexpr float kItemFallTime = 0.5f;            //足場と一緒に落ちて消えるまでの秒数
	// inline constexpr RyoEngine::Vector4 kCrossBombColor = { 1.0f, 0.25f, 0.75f, 1.0f }; //ピンクの十字爆弾
	// inline constexpr RyoEngine::Vector4 kJumpBoostColor = { 0.45f, 1.0f, 0.35f, 1.0f }; //黄緑の2マス乗り越え

	// //十字爆弾
	// inline constexpr float kBombStepDelay = 0.1f;           //1マス外へ行くごとに崩れるのが遅れる秒数

	//2マス乗り越え
	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// inline constexpr int32_t kBoostedJumpGap = 2;           //アイテムを取ったあと飛び越えられる穴の数
	// //trueなら次のジャンプで使い切り、falseなら穴2つを飛び越えたときだけ使い切る
	// inline constexpr bool kJumpBoostUseOnNextJump = false;
}
