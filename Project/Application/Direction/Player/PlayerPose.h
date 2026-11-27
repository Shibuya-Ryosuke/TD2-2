#pragma once
#include <RyoEngine.h>
#include "../../Object/Player/Rig/RigPose.h"

namespace App
{
	//落ちるときのじたばたと尻もちを、分けたパーツのプレイヤーの姿勢でなめらかに見せる
	class PlayerPose
	{
	public:
		//どの姿勢にするか
		enum class Kind
		{
			Normal, //いつもの姿勢でボーンつきのモデルのアニメーション
			Flail, //落ちながら手足をじたばた
			Sit, //尻もち
			Dangle, //空中で足をぶらつかせる
		};

		//なりたい姿勢を決める
		void Set(Kind kind) { kind_ = kind; }
		Kind Get() const { return kind_; }
		void SetSpin(float spin) { spin_ = spin; }
		//すぐにいつもの姿勢へ戻す
		void Reset();

		//なりたい姿勢へ近づける
		void Update(float deltaTime);
		//分けたパーツのプレイヤーで見せる間か
		bool IsRigVisible() const { return kind_ != Kind::Normal; }
		//今のじたばたと尻もちの割合からパーツの姿勢を作る
		RigPose MakeRigPose() const;

	private:
		Kind kind_ = Kind::Normal;
		float flail_ = 0.0f; //じたばたの強さ
		float sit_ = 0.0f; //尻もちの割合
		float spin_ = 0.0f; //落下演出で体を回す角度
		float time_ = 0.0f; //じたばたの手足を動かす時間
	};
}
