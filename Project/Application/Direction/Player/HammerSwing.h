#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	//足場を壊すアニメーションの間だけ右手に持たせて、床にめり込まないように振るハンマー
	class HammerSwing
	{
	public:
		HammerSwing() = default;
		~HammerSwing() = default;

		HammerSwing(const HammerSwing&) = delete;
		HammerSwing& operator=(const HammerSwing&) = delete;

		//ハンマーを作ってプレイヤーの右手のボーンにつける
		void Initialize(RyoEngine::Model& body);

		void Start(); //振り始めにポンっと出す
		void Hide(); //置き直したときなどにすぐ消す

		//振っている間は手について動き、終わったらしゅっと消える
		void Update(bool isSwinging, float floorY);
		//出ている間だけ描く
		void Draw();

	private:
		//床にめり込むのを直すために柄の持つ所を中心にangle度起こしたときの右手から見たハンマーの行列
		RyoEngine::Matrix4x4 MakeLocalMatrix(float angle, float scale) const;
		//その行列のときのハンマーの頭のいちばん低い高さ
		float FindLowestHeadY(const RyoEngine::Matrix4x4& local) const;
		//頭が床より下に行かないように起こす度数
		float FindLiftAngle(float floorY) const;

		std::unique_ptr<RyoEngine::Model> model_{}; //ハンマー
		RyoEngine::WorldTransform* hand_ = nullptr; //持ち主はプレイヤーのモデルの右手のボーン
		bool isShown_ = false; //出ているか
		float scale_ = 0.0f; //ポンっと出てしゅっと消える大きさの倍率
	};
}
