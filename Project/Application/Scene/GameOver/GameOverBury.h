#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>
#include "../../Object/Player/Rig/PlayerRig.h"

namespace App
{
	//ゲームオーバーでプレイヤーがきょろきょろしたあと、シャワーのように降ってくる積み木に埋もれる
	class GameOverBury
	{
	public:
		GameOverBury() = default;
		~GameOverBury() = default;

		//モデルを持つのでコピーを禁止する
		GameOverBury(const GameOverBury&) = delete;
		GameOverBury& operator=(const GameOverBury&) = delete;

		void Initialize(); //プレイヤーと降らせる積み木を作る
		void Update(float deltaTime); //きょろきょろと積み木のシャワーを1フレーム進める
		void Draw();

		//積み木が全部降り積もって埋もれきったか
		bool IsBuried() const { return isBuried_; }

	private:
		//降ってくる1個の積み木
		struct Block
		{
			//モデルを持つのでコピーを禁止する
			Block() = default;
			~Block() = default;
			Block(const Block&) = delete;
			Block& operator=(const Block&) = delete;
			Block(Block&&) = default;
			Block& operator=(Block&&) = default;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			RyoEngine::Vector3 rotate{}; //度
			RyoEngine::Vector3 spin{}; //1秒で回る度
			float velocityY = 0.0f;
			float size = 0.5f; //1辺
			float spawnTime = 0.0f; //降り始める秒数
			bool isLanded = false;
		};

		void UpdatePlayer(); //きょろきょろして、降ってきたら頭をかばってだんだん縮こまる
		void UpdateBlock(Block& block, float deltaTime); //落として、積もった高さに乗せる
		void RollDown(Block& block); //まわりより高すぎるときは低い方へ転がす
		float& PileAt(const RyoEngine::Vector3& position); //その位置の升目の積もった高さ

		PlayerRig rig_; //埋もれるプレイヤー
		std::vector<Block> blocks_; //降らせる全部の積み木
		std::vector<float> pile_; //升目ごとの積もった高さ
		float outside_ = 0.0f; //升目の外に落ちたときの高さ
		std::mt19937 random_{ 11u };
		float timer_ = 0.0f;
		float lookTime_ = 0.0f; //きょろきょろする秒数
		int32_t landedCount_ = 0;
		bool isBuried_ = false;
	};
}
