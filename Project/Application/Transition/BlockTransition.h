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

	// 場面切り替えの演出 (積み木)
	// 出ていくシーン: がれきが落ちる → 積み木が1個ずつ積み上がって画面を埋める → 次のシーンへ
	// 入ってきたシーン: 埋まった画面から始まって、風で積み木が手前に倒れる
	// 演出の間 (IsBusy) は、シーンは操作を受け付けない
	//
	// 使い方 (シーン側):
	//   Initialize でカメラを作ってから transition_.Initialize(カメラ)
	//   Update の最初で if (transition_.Update()) { return; }  ← 演出中は何もしない
	//   次のシーンへ行くとき transition_.Start<次のシーン>()
	//   Draw の最後で transition_.Draw()
	class BlockTransition
	{
	public:
		BlockTransition() = default;
		~BlockTransition() = default;

		BlockTransition(const BlockTransition&) = delete;
		BlockTransition& operator=(const BlockTransition&) = delete;

		// ブロック・シェーダーを用意する。前のシーンが画面を埋めて来たなら、倒れる演出から始める
		void Initialize(const RyoEngine::Camera* camera); // 遠近法のカメラ
		void Initialize(const StageCamera* camera);       // ゲームシーンの平行投影のカメラ

		// 次のシーンへ行く演出を始める (もう始まっていたら何もしない)
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

		// 演出を進める。演出中 (= 操作させない間) は true
		bool Update();
		// ブロックを描く (シーンの Draw の最後に呼ぶ)
		void Draw();

		bool IsBusy() const { return phase_ != Phase::Idle; }

	private:
		enum class Phase
		{
			Idle,    // 何もしていない (操作できる)
			Debris,  // がれきが落ちてくる
			Stack,   // 積み木が積み上がる
			Hold,    // 画面が埋まって少し待つ → 次のシーンへ
			Topple,  // (次のシーンで) 積み木が倒れる
		};

		// シーンを切り替える関数 (Start<次のシーン> で決まる)
		template <class NextScene>
		static void ChangeTo()
		{
			RyoEngine::SceneManager::ChangeScene<NextScene>();
		}

		void Setup(const RyoEngine::Camera* camera);
		void BeginDebris();
		void BeginTopple();
		void FinishTopple();
		// ブロックの親 (カメラの位置・向き) をカメラに合わせる
		void UpdateFrame();
		// 風の筋 (ポストエフェクト) の強さ
		void SetWindStreak(float strength);

		const RyoEngine::Camera* camera_ = nullptr;
		const StageCamera* stageCamera_ = nullptr; // ゲームシーンのときだけ

		RyoEngine::WorldTransform frame_; // ブロックの親 (ブロックより先に作って、後に消す)
		TransitionView view_{};
		TransitionDebris debris_;
		TransitionWall wall_;

		Phase phase_ = Phase::Idle;
		float timer_ = 0.0f;
		void (*changeScene_)() = nullptr;
		uint32_t windEffect_ = 0; // 風の筋 (0 = 使っていない)
		std::mt19937 random_{ std::random_device{}() };
	};
}
