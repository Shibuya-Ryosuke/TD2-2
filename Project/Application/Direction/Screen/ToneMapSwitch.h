#pragma once

namespace App
{
	//ゲームシーンの間だけ色を仕上げるACESのオンとオフを切り替える
	class ToneMapSwitch
	{
	public:
		//今の設定を覚えてからDirectionConfigの設定にする
		void Apply();
		//覚えておいた設定に戻す
		void Restore();

	private:
		bool savedAcesEnabled_ = true; //切り替える前のACESのオンとオフ
	};
}
