#pragma once
#include <RyoEngine.h>
#include <memory>
#include <vector>

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

		//mtlの色のままのボスのモデルと拳と予告を作る
		void Initialize();
		//調整する値をParamEditorに登録する
		void RegisterParams();
		//ボスの位置と向きと攻撃に合わせて行列を確定する
		void Update(const Boss& boss);
		//倒したボス以外の体と、パンチ中の拳と予告を描く
		void Draw(const Boss& boss);
		//体と拳をalphaの濃さに透かして、0なら描かない
		void SetFade(float alpha);

	private:
		void UpdateBody(const Boss& boss); //立っているか仰向けに寝ているかで体の行列を確定する
		std::unique_ptr<RyoEngine::Model> body_{}; //ボスのモデル
		std::unique_ptr<RyoEngine::Model> fist_{}; //パンチ中だけ出す箱の拳
		std::unique_ptr<RyoEngine::Model> meleeTell_{}; //足元の板の近接攻撃の予告
		std::unique_ptr<RyoEngine::Model> punchTell_{}; //前にのびる板のパンチの予告
		float breath_ = 0.0f; //寝ているときの息の時間
		std::vector<RyoEngine::Vector4> bodyColors_{}; //透かす前の体のメッシュごとの色
		std::vector<RyoEngine::BlendMode> bodyBlends_{}; //透かす前の体のメッシュごとの混ぜ方
		float fade_ = 1.0f; //体と拳の濃さで、1で透けていない

		std::vector<std::unique_ptr<RyoEngine::Model>> rushTells_{}; //突進のルートのマスに置く板
		int32_t rushTellCount_ = 0; //今描く板の数

		//突進の警告の板
		float rushTellY_ = 0.05f;       //床からの高さ
		float rushTellHeight_ = 0.035f; //板の厚さ
		float rushTellScale_ = 0.9f;    //1マスに対する板の大きさ
	};
}
