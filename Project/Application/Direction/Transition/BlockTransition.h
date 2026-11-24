#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <random>
#include "TransitionDebris.h"
#include "TransitionView.h"
#include "TransitionWall.h"

namespace App
{
	class StageCamera;

	//積み木で画面を埋めて切り替え、次のシーンで横に吹き飛ばす場面切り替え
	class BlockTransition
	{
	public:
		BlockTransition() = default;
		~BlockTransition() = default;

		BlockTransition(const BlockTransition&) = delete;
		BlockTransition& operator=(const BlockTransition&) = delete;

		//ブロックを用意して、前のシーンが埋めて来たら吹き飛ぶ演出から始める
		void Initialize(const RyoEngine::Camera* camera); //遠近法のカメラで使う
		void Initialize(const StageCamera* camera); //ゲームシーンの平行投影のカメラで使う

		//次のシーンへ行く演出を始める
		template <class NextScene>
		void Start()
		{
			if (phase_ != Phase::Idle)
			{
				return;
			}
			changeScene_ = &ChangeTo<NextScene>;
			BeginDebris();
		}

		//演出を進めて、演出中で操作させない間はtrueを返す
		bool Update();
		//シーンのDrawの最後でブロックを描く
		void Draw();

		bool IsBusy() const { return phase_ != Phase::Idle; }

	private:
		enum class Phase
		{
			Idle, //何もしていない
			Debris, //がれきが落ちてくる
			Stack, //積み木が積み上がる
			Hold, //画面が埋まって少し待つ
			Blow, //次のシーンで横に吹き飛ぶ
		};

		//Startで決めたシーンを切り替える関数
		template <class NextScene>
		static void ChangeTo()
		{
			RyoEngine::SceneManager::ChangeScene<NextScene>();
		}

		void Setup(const RyoEngine::Camera* camera);
		void BeginDebris();
		void BeginBlow();
		void FinishBlow();
		//ブロックの親をカメラの位置と向きに合わせる
		void UpdateFrame();
		//横風のポストエフェクトの強さ
		void SetGust(float strength);

		const RyoEngine::Camera* camera_ = nullptr;
		const StageCamera* stageCamera_ = nullptr; //ゲームシーンのカメラ

		RyoEngine::WorldTransform frame_; //カメラに合わせるブロックの親
		TransitionView view_{};
		TransitionDebris debris_;
		TransitionWall wall_;

		Phase phase_ = Phase::Idle;
		float timer_ = 0.0f;
		void (*changeScene_)() = nullptr;
		uint32_t windEffect_ = 0; //横風のポストエフェクトの番号
		std::mt19937 random_{ std::random_device{}() };
	};
}
