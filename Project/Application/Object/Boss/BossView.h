#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	class Boss;

	//ボスの体と拳と攻撃の予告のモデルをボスの状態に合わせて置く
	class BossView
	{
	public:
		BossView() = default;
		~BossView() = default;

		BossView(const BossView&) = delete;
		BossView& operator=(const BossView&) = delete;

		//cubeの色を変えたモデルを作る
		void Initialize();
		//ボスの位置と向きと攻撃に合わせて行列を確定する
		void Update(const Boss& boss);
		//倒したボス以外の体と拳と予告を描く
		void Draw(const Boss& boss);

	private:
		std::unique_ptr<RyoEngine::Model> body_{}; //赤い箱の体
		std::unique_ptr<RyoEngine::Model> fist_{}; //拳
		std::unique_ptr<RyoEngine::Model> meleeTell_{}; //足元の板の近接攻撃の予告
		std::unique_ptr<RyoEngine::Model> punchTell_{}; //前にのびる板のパンチの予告
	};
}
