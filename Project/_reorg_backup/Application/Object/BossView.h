#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	class Boss;

	// ボスの見た目 (Boss から分けたもの)
	// 体・拳・近接攻撃の予告・パンチの予告 のモデルを持って、Boss の状態に合わせて置く
	class BossView
	{
	public:
		BossView() = default;
		~BossView() = default;

		BossView(const BossView&) = delete;
		BossView& operator=(const BossView&) = delete;

		// モデルを作る (仮: cube の色を変えたもの)
		void Initialize();
		// ボスの位置・向き・攻撃に合わせて、行列を確定する
		void Update(const Boss& boss);
		// 体・拳・攻撃の予告を描く (倒したボスは描かない)
		void Draw(const Boss& boss);

	private:
		std::unique_ptr<RyoEngine::Model> body_{};      // 体 (赤い箱)
		std::unique_ptr<RyoEngine::Model> fist_{};      // 拳
		std::unique_ptr<RyoEngine::Model> meleeTell_{}; // 近接攻撃の予告 (足元の板)
		std::unique_ptr<RyoEngine::Model> punchTell_{}; // パンチの予告 (前にのびる板)
	};
}
