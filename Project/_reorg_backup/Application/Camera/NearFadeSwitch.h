#pragma once

namespace App
{
	// エンジンの「カメラに近いモデルを網目状に消す」機能 (NearFade) を切る・元に戻す
	// この機能はカメラからの距離を遠近法の値から求めているので、平行投影 (StageCamera) だと
	// どのモデルも「すごく近い」と判断されて、網目に消えてしまう。なのでゲームシーンの間だけ切る
	class NearFadeSwitch
	{
	public:
		// 今の設定を覚えてから切る (シーンに入るとき)
		void Disable();
		// 覚えておいた設定に戻す (シーンを出るとき。ほかのシーンは遠近法のカメラなので)
		void Restore();

	private:
		float savedStart_ = 0.0f; // 切る前の「消し始める距離」
		float savedEnd_ = 0.0f;   // 切る前の「消し終わる距離」
	};
}
