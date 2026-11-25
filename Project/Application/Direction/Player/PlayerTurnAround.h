#pragma once

namespace App
{
	//向きが変わったときに近い方へクルッと回る見た目だけの向き
	class PlayerTurnAround
	{
	public:
		//回さずにすぐその向きにする
		void Reset(float yawDeg) { yawDeg_ = yawDeg; }
		//目標の度数へ近い回り方で少しずつ回す
		void Update(float targetYawDeg);
		//今の見た目の度数
		float GetYawDeg() const { return yawDeg_; }

	private:
		float yawDeg_ = 0.0f; //見た目の度数
	};
}
