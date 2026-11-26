#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>
#include "../../Object/Player/Rig/PlayerRig.h"
#include "../../Direction/Player/HeartBurst.h"

namespace App
{
	//タイトルでプレイヤーが弧の形の積み木の壁に積み木をのせたり下ろしたりして遊び、スタートで壁がカメラへ倒れてくる
	class TitleBlockPlay
	{
	public:
		TitleBlockPlay() = default;
		~TitleBlockPlay() = default;

		//モデルを持つのでコピーを禁止する
		TitleBlockPlay(const TitleBlockPlay&) = delete;
		TitleBlockPlay& operator=(const TitleBlockPlay&) = delete;

		void Initialize(); //積み木の壁と山とプレイヤーを作る
		void Update(float deltaTime); //遊ぶか崩れるのを1フレーム進める
		void Draw();

		//カメラのeyeへ向かって積み木が倒れてきてプレイヤーがあせる
		void Topple(const RyoEngine::Vector3& eye);
		bool IsToppling() const { return isToppling_; }
		//倒れ始めてからの秒数
		float GetToppleTime() const { return toppleTimer_; }
		//プレイヤーの頭の上のワールドの位置
		RyoEngine::Vector3 GetHeadPosition() const { return rig_.GetHeadPosition(); }

	private:
		//1本の積み木
		struct Brick
		{
			//モデルを持つのでコピーを禁止する
			Brick() = default;
			~Brick() = default;
			Brick(const Brick&) = delete;
			Brick& operator=(const Brick&) = delete;
			Brick(Brick&&) = default;
			Brick& operator=(Brick&&) = default;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			RyoEngine::Vector3 rotate{}; //度
			RyoEngine::Vector3 velocity{}; //倒れるときの速さ
			RyoEngine::Vector3 spin{}; //倒れるときに1秒で回る度
			float delay = 0.0f; //倒れ始めるまでの秒数
			float pop = 0.0f; //置いたときにぷるんとする残り
		};

		//のせるか下ろすかの1回の動き
		struct Move
		{
			int32_t brick = 0; //動かす積み木の番号
			RyoEngine::Vector3 from{}; //元の位置
			RyoEngine::Vector3 to{}; //置く位置
			float fromYaw = 0.0f; //元の向き
			float toYaw = 0.0f; //置く向き
			bool isFromPile = true; //下の山から取るか
		};

		void BuildWall(); //弧の形の壁と、下ろした積み木の山を作る
		void UpdatePlay(float deltaTime); //のせて喜んで下ろすのをくり返す
		void UpdateTopple(float deltaTime); //倒れてくる積み木とあせるプレイヤー
		Move MakeMove(int32_t index) const; //index回目にのせるか下ろす動き
		RyoEngine::Vector3 GetPileSpot(int32_t height) const; //山の下から数えた位置
		void PlaceBrick(Brick& brick); //行列を確定する

		std::vector<Brick> bricks_; //壁と山の全部の積み木で、うしろのkLooseBlocks本が動かす積み木
		std::array<RyoEngine::Vector3, 3> slots_{}; //壁のいちばん上であいている所
		std::array<float, 3> slotYaws_{}; //あいている所の向き
		PlayerRig rig_; //積み木で遊ぶプレイヤー
		HeartBurst hearts_; //喜んだときのハート
		std::mt19937 random_{ 7u };
		float timer_ = 0.0f; //今の動きの中の秒数
		int32_t step_ = 0; //のせる3回、喜ぶ、下ろす3回をくり返す中の番号
		bool isHolding_ = false; //積み木を持っているか
		bool isToppling_ = false;
		float toppleTimer_ = 0.0f;
	};
}
