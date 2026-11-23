#pragma once

namespace App
{
	//平行投影だと全部消えてしまうNearFadeをゲーム中だけ切る
	class NearFadeSwitch
	{
	public:
		//今の設定を覚えてから切る
		void Disable();
		//覚えておいた設定に戻す
		void Restore();

	private:
		float savedStart_ = 0.0f; //切る前の消し始める距離
		float savedEnd_ = 0.0f; //切る前の消し終わる距離
	};
}
