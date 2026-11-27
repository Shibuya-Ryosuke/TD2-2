#pragma once
#include <RyoEngine.h>

namespace App
{
	//待機と移動と足場を壊すとクリアのポーズのアニメーションを切り替える
	class PlayerAnimation
	{
	public:
		//別のファイルのアニメーションを足して待機をループで流す
		void Initialize(RyoEngine::Model& body);

		void PlayIdle(); //待機をループで流す
		void PlayJump(); //1回跳ぶけどハンマーを振っている間は振るのを続ける
		void PlaySwing(); //ハンマーを振り下ろして首をかしげる
		void PlayPose(); //クリアのポーズを1回する
		void Freeze(); //じたばたや尻もちのために、待機の最初の姿勢で止める

		//1回だけのアニメーションが終わったら待機へ戻し、ハンマーが当たったら首かしげの速さにする
		void Update();

		bool IsSwinging() const { return state_ == State::Swing; } //足場を壊すアニメーションの途中か
		bool IsPoseFinished() const; //クリアのポーズが終わったか

		//このフレームでハンマーが床に当たったか
		bool WasImpact() const { return isImpactNow_; }

	private:
		//今流しているアニメーション
		enum class State
		{
			Idle, //待機
			Jump, //移動
			Swing, //足場を壊す
			Pose, //クリアのポーズ
			Frozen, //待機の最初の姿勢で止めている
		};

		//アニメーションを最初から速さを決めて流す
		void Play(const char* clip, bool isLoop, float speed, State state);
		//混ぜる秒数も決めて流す
		void Play(const char* clip, bool isLoop, float speed, State state, float blend);

		RyoEngine::Model* body_ = nullptr; //持ち主はPlayer
		State state_ = State::Idle; //今流しているアニメーション
		bool isAfterImpact_ = false; //振るアニメーションでハンマーが当たったあとか
		bool isImpactNow_ = false; //このフレームでハンマーが床に当たったか
		bool isClipMissing_ = false; //流そうとしたアニメーションが見つからなかったか
	};
}
