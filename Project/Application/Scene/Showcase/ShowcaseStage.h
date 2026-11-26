#pragma once
#include <RyoEngine.h>
#include <memory>
#include <vector>
#include "../../Stage/Sky/SkyDome.h"
#include "../../Direction/Screen/ToneMapSwitch.h"
#include "../../Camera/NearFadeSwitch.h"

namespace App
{
	//タイトルとクリアとゲームオーバーで共通の、遠近法のカメラとゲームと同じ空とピンクの床
	class ShowcaseStage
	{
	public:
		ShowcaseStage() = default;
		~ShowcaseStage() = default;

		//モデルを持つのでコピーを禁止する
		ShowcaseStage(const ShowcaseStage&) = delete;
		ShowcaseStage& operator=(const ShowcaseStage&) = delete;

		//eyeからtargetを見るカメラと空と床を作る
		void Initialize(const RyoEngine::Vector3& eye, const RyoEngine::Vector3& target);
		void Finalize(); //ACESとNearFadeを元に戻す
		void Update(float deltaTime); //揺れる床と落ちる床を進めて、空をカメラに合わせて雲を流す
		void Draw(); //空と床を描く

		//カメラの位置と見る所を変える
		void LookAt(const RyoEngine::Vector3& eye, const RyoEngine::Vector3& target);
		RyoEngine::Camera* GetCamera() const { return camera_; }
		//ワールドの位置を1920x1080が基準のデバッグ表示の画面の位置にする
		RyoEngine::Vector2 ToScreen(const RyoEngine::Vector3& world) const;

		//originに近い床ほど強くがたがた揺らし始める
		void RumbleFloor(const RyoEngine::Vector3& origin);
		//originに近い床から順にぴょこっと浮いて下へ落としていく
		void DropFloor(const RyoEngine::Vector3& origin);

	private:
		//床の1枚
		struct Tile
		{
			//モデルを持つのでコピーを禁止する
			Tile() = default;
			~Tile() = default;
			Tile(const Tile&) = delete;
			Tile& operator=(const Tile&) = delete;
			Tile(Tile&&) = default;
			Tile& operator=(Tile&&) = default;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			float delay = 0.0f; //落ち始めるまでの秒数
			float velocityY = 0.0f;
			float spin = 0.0f; //落ちながら傾く度
			float rumble = 0.0f; //がたがた揺れる強さの割合
			float phase = 0.0f; //がたがたのずらし
			RyoEngine::Vector3 shake{}; //がたがたで動く量
			float shakeTilt = 0.0f; //がたがたで傾く度
		};

		void UpdateRumble(); //まだ落ちていない床をだんだん強くがたがた揺らす
		void PlaceTile(Tile& tile); //行列を確定する

		RyoEngine::Camera* camera_ = nullptr; //持ち主はCameraManager
		SkyDome sky_; //ゲームと同じ空
		ToneMapSwitch toneMap_; //空の色をゲームと同じにする
		NearFadeSwitch nearFade_; //カメラのすぐ前に来る場面切り替えの積み木が薄くならないように切る
		std::vector<Tile> tiles_; //床のタイル
		bool isRumbling_ = false; //床を揺らしているか
		float rumbleTimer_ = 0.0f; //揺らし始めてからの秒数
		bool isDropping_ = false; //床を落としているか
		float dropTimer_ = 0.0f; //床を落とし始めてからの秒数
	};
}
