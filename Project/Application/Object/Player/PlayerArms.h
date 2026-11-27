#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <string>
#include <vector>

namespace App
{
	//ボーンつきのプレイヤーの腕のメッシュだけを、肩を中心に大きくして見せる
	class PlayerArms
	{
	public:
		PlayerArms() = default;
		~PlayerArms() = default;

		PlayerArms(const PlayerArms&) = delete;
		PlayerArms& operator=(const PlayerArms&) = delete;

		//glbの中から腕のメッシュを探して、左右の腕に肩から大きくするシェーダーをつける
		void Initialize(RyoEngine::Model& body, const std::string& modelName);
		//体の行列を確定したあとに、今の肩の位置と倍率をシェーダーに渡す
		void Update();

		//右手のボーンの位置を、大きくした腕の手の位置へ動かす
		RyoEngine::Transform ScaleRightHand(const RyoEngine::Transform& hand) const;

	private:
		//glbの中のノードをエンジンと同じ順にたどった、メッシュごとのノードの名前とマテリアルの名前
		struct MeshEntry
		{
			std::string node;
			std::string material;
		};
		static std::vector<MeshEntry> ListMeshes(const std::string& modelName);
		//ノードの名前が腕のメッシュなら左1か右2で、それ以外は0
		static int32_t ToArmSide(const MeshEntry& mesh);

		RyoEngine::Model* body_ = nullptr;
		RyoEngine::WorldTransform* shoulderLeft_ = nullptr; //モデルの原点から見た左の肩のボーン
		RyoEngine::WorldTransform* shoulderRight_ = nullptr; //モデルの原点から見た右の肩のボーン
		bool isReady_ = false; //腕のメッシュが見つかってシェーダーをつけたか
	};
}
