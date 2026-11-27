#pragma once
#include <RyoEngine.h>
#include <random>
#include "TransitionDebris.h"
#include "TransitionView.h"
#include "TransitionWall.h"

namespace App
{
	class StageCamera;

	//積み木で画面を覆って切り替え、次のシーンで積み木を落とす
	class BlockTransition
	{
	public:
		BlockTransition() = default;
		~BlockTransition() = default;

		BlockTransition(const BlockTransition&) = delete;
		BlockTransition& operator=(const BlockTransition&) = delete;

		void Initialize(const RyoEngine::Camera* camera);
		void Initialize(const StageCamera* camera);

		//次のシーンを予約して積み木で画面を覆い始める
		template <class NextScene>
		void Start()
		{
			Start(&ChangeTo<NextScene>);
		}
		void Start(void (*changeScene)());

		//演出中はゲームの操作を止めるためにtrueを返す
		bool Update();
		void Draw();
		bool IsBusy() const { return phase_ != Phase::Idle; }

	private:
		enum class Phase
		{
			Idle,
			Cover,
			Drop,
		};

		template <class NextScene>
		static void ChangeTo()
		{
			RyoEngine::SceneManager::ChangeScene<NextScene>();
		}

		void Setup(const RyoEngine::Camera* camera);
		void BeginCover();
		void UpdateFrame();

		const RyoEngine::Camera* camera_ = nullptr;
		const StageCamera* stageCamera_ = nullptr;
		RyoEngine::WorldTransform frame_;
		TransitionView view_{};
		TransitionDebris debris_;
		TransitionWall wall_;

		Phase phase_ = Phase::Idle;
		float timer_ = 0.0f;
		uint32_t coverSeed_ = 0;
		void (*changeScene_)() = nullptr;
		std::mt19937 random_{ std::random_device{}() };
	};
}
