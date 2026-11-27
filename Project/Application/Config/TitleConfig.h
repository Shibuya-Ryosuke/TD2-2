#pragma once
#include <RyoEngine.h>

//タイトルでプレイヤーが積み木を持ってゆらゆらし、スタートで落としてあせって床ごと落ちる演出の数値
namespace App::TitleConfig
{
	//カメラ
	inline constexpr RyoEngine::Vector3 kEye = { -0.6f, 2.6f, -8.5f }; //ゆらゆらしているときのカメラの位置
	inline constexpr RyoEngine::Vector3 kTarget = { -0.4f, 1.8f, 0.0f }; //ゆらゆらしているときにカメラが見る所
	inline constexpr RyoEngine::Vector3 kCloseEye = { 0.3f, 2.2f, -6.0f }; //積み木を落としてあせる顔に寄るカメラの位置
	inline constexpr float kCloseTime = 0.5f; //顔に寄るまでの秒数
	inline constexpr float kFaceHeight = 1.45f; //寄ったときに足元から見る高さ
	inline constexpr RyoEngine::Vector3 kWideEye = { -0.4f, 4.4f, -8.4f }; //床ががたがたしたら床ごと見せる引きのカメラの位置
	inline constexpr float kWideHeight = 1.2f; //引きのときに足元から見る高さ
	inline constexpr float kWideTime = 0.7f; //引きになるまでの秒数
	inline constexpr RyoEngine::Vector3 kFallEye = { 0.6f, 2.6f, -5.4f }; //落ちるプレイヤーを追うときのカメラの、床があった高さでの位置
	inline constexpr float kFollowHeight = 0.6f; //落ちるプレイヤーを追うときに足元から見る高さ
	inline constexpr float kFollowRatio = 0.95f; //落ちるプレイヤーをカメラの位置が追う割合で、見る所はぴったり追うので少し見下ろしていく

	//プレイヤーと持っている積み木
	inline constexpr RyoEngine::Vector3 kPlayer = { 1.5f, 0.0f, 0.4f }; //画面のまん中寄りの右の足元
	inline constexpr float kBlockSize = 0.75f; //持っている積み木の1辺
	inline constexpr RyoEngine::Vector4 kBlockColor = { 1.0f, 0.82f, 0.9f, 1.0f }; //持っている積み木のうすいピンク
	inline constexpr float kSwaySpeed = 1.3f; //ゆらゆらの1秒のラジアン
	inline constexpr float kSwayRoll = 6.0f; //ゆらゆらで体が傾くいちばん大きい度

	//スタートからの流れで、積み木を落とす、あせる、床ががたがた、床が落ちる、プレイヤーが落ちる、仰向けでじたばた
	inline constexpr float kSlipTime = 0.7f; //積み木がつるっと落ちてびっくりしている秒数
	inline constexpr float kBlockGravity = 12.0f; //落とした積み木の重力で、目で追えるくらいゆっくり
	inline constexpr float kBlockBounce = 0.35f; //落とした積み木が床ではねる割合
	inline constexpr float kBlockSlide = 0.7f; //落とした積み木が前へすべり出る速さ
	inline constexpr float kRumbleTime = 1.6f; //スタートから床ががたがたし始めるまでの秒数
	inline constexpr float kFloorDropTime = 2.6f; //スタートから床が落ち始めるまでの秒数
	inline constexpr float kPlayerFallDelay = 0.35f; //床が落ち始めてから、宙で固まったプレイヤーが落ち始めるまでの秒数
	inline constexpr float kPlayerGravity = 9.0f; //落ちるプレイヤーの重力
	inline constexpr float kPlayerMaxFallSpeed = 5.0f; //落ちるプレイヤーのいちばん速い速さで、じたばたが見えるくらい
	inline constexpr float kFlipDelay = 0.3f; //落ち始めてから仰向けに倒れ始めるまでの秒数
	inline constexpr float kFlipTime = 0.6f; //仰向けに倒れきるまでの秒数
	inline constexpr float kSupineTurn = 75.0f; //仰向けのとき体の長さが画面で見えるように横へ向ける度
	inline constexpr float kSupineSpinDelay = 0.28f; //仰向けになり始めてから回転を始めるまでの秒数
	inline constexpr float kSupineSpinTime = 0.92f; //仰向けで一回転する秒数
	inline constexpr float kSupineSpinDegrees = 360.0f; //足をばたつかせながら水平に回る角度
	inline constexpr float kTransitionTime = 4.4f; //スタートから場面切り替えを始めるまでの秒数で、仰向けのじたばたを少し見せてから
}
