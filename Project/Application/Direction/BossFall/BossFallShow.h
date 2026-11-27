#pragma once
#include <RyoEngine.h>
#include "BossFallRig.h"
#include "BossPose.h"

namespace App
{
	//落ちてからじたばた、ドスン、痛そうに座りこみ、足場が当たってむぎゅ、起き上がるまでのパーツのボス
	class BossFallShow
	{
	public:
		BossFallShow() = default;
		~BossFallShow() = default;

		//モデルを持つパーツのボスを持つのでコピーとムーブを禁止する
		BossFallShow(const BossFallShow&) = delete;
		BossFallShow& operator=(const BossFallShow&) = delete;
		BossFallShow(BossFallShow&&) = delete;
		BossFallShow& operator=(BossFallShow&&) = delete;

		void Initialize(); //パーツと煙と光の輪を用意する
		//feetの位置でfaceYawDeg度を向いて、足元がなくなって「えっ」となる
		void Begin(const RyoEngine::Vector3& feet, float faceYawDeg);
		//じたばた落ち始める
		void StartFlail();
		//落ちている間の足元の位置
		void SetFeet(const RyoEngine::Vector3& feet) { feet_ = feet; }
		//ドスンと叩きつけられて座りこむ
		void Slam();
		//足場が当たってsquashだけむぎゅっとつぶれて白く光る
		void Hit(float squash);
		//yawDeg度の向きへ回りながら起き上がる
		void BeginGetUp(float yawDeg);
		//起き上がり終わったか
		bool IsGetUpFinished() const;
		//見えなくする
		void Hide() { state_ = State::Hidden; }
		//姿勢とつぶれと光を進めて行列を確定する
		void Update(float deltaTime);
		void Draw(); //見えているときだけパーツを描く

		//カメラで映すときの胸のあたり
		RyoEngine::Vector3 GetFocus() const;
		//足場が飛んでいく先
		RyoEngine::Vector3 GetHitTarget() const;

	private:
		//今の場面
		enum class State
		{
			Hidden, //見えない
			Notice, //足元がなくなって「えっ」となっている
			Flail, //じたばた落ちている
			Hurt, //叩きつけられて座りこんでいる
			GetUp, //起き上がっている
		};

		BossPose MakePose() const; //今の場面の姿勢

		BossFallRig rig_; //6つのパーツに分けたボス
		State state_ = State::Hidden;
		RyoEngine::Vector3 feet_{}; //足元の位置
		float yawDeg_ = 0.0f; //今向いている度数
		float faceYawDeg_ = 0.0f; //座りこむときに向く度数
		float getUpYawDeg_ = 0.0f; //起き上がったときに向く度数
		float timer_ = 0.0f; //今の場面に入ってからの秒数
		float slamTimer_ = -1.0f; //ドスンからの秒数で、マイナスならまだ
		float hitTimer_ = -1.0f; //最後に足場が当たってからの秒数で、マイナスならまだ
		float hitSquash_ = 0.0f; //最後に足場が当たったときのつぶれる量
		float flash_ = 0.0f; //白い光の強さ
		BossPose flailAtSlam_{}; //ドスンの瞬間のじたばたの姿勢
		BossPose noticeAtFlail_{}; //じたばたし始めた瞬間の姿勢
	};
}
