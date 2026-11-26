#pragma once
#include <RyoEngine.h>
#include "../Config/FloorConfig.h"

namespace App
{
	//ステージを斜め上から映し、寄せる・ずらす・傾ける・揺らすができる平行投影のカメラ
	class StageCamera : public RyoEngine::Camera
	{
	public:
		StageCamera() = default;
		~StageCamera() override = default;

		StageCamera(const StageCamera&) = delete;
		StageCamera& operator=(const StageCamera&) = delete;

		void Initialize() override; //位置と向きを決める
		void Update() override; //平行投影の行列を毎フレーム作る

		//今映している床の高さに合わせて上下させる
		void SetFloorHeight(float floorHeight) { floorHeight_ = floorHeight; }
		float GetFloorHeight() const { return floorHeight_; }

		//画面の縦の半分に映す長さで、寄せたいときだけ変えて元の広さはImGuiの値のまま残す
		void SetOrthoSize(float orthoSize) { zoomOrthoSize_ = orthoSize; }
		//寄せていなければImGuiで決めた広さ
		float GetOrthoSize() const;
		//ImGuiで決めたいつもの広さ
		float GetBaseOrthoSize() const { return baseOrthoSize_; }

		//クリアのときなどに映す所をずらす
		void SetFocusOffset(const RyoEngine::Vector3& offset) { focusOffset_ = offset; }
		//見上げたり見下ろしたりするのに、いつもの向きから縦に傾けるラジアン
		void SetPitchOffset(float pitch) { pitchOffset_ = pitch; }
		//画面を揺らすのにずらす分
		void SetShakeOffset(const RyoEngine::Vector3& offset) { shakeOffset_ = offset; }

		//寄せる・ずらす・傾ける・揺らすを全部やめていつもの映し方に戻す
		void ResetShot();

		//今の床の高さとずらした分から求めたこのフレームのカメラの位置
		RyoEngine::Vector3 GetViewPosition() const;
		//傾けた分も入れたこのフレームの向き
		RyoEngine::Vector3 GetViewRotate() const;
		//傾けた分も入れたこのフレームの正面の向き
		RyoEngine::Vector3 GetViewForward() const;
		//targetが画面の真ん中に来るようにするSetFocusOffsetの値
		RyoEngine::Vector3 GetCenteringOffset(const RyoEngine::Vector3& target) const;
		//ワールドの位置を1920x1080が基準のデバッグ表示の画面の位置にする
		RyoEngine::Vector2 WorldToScreen(const RyoEngine::Vector3& world) const;

	private:
		//ImGuiで調整してシーンをまたいで残る値で、ゲーム中には書きかえない
		RyoEngine::Vector3 origin_ = { 15.0f, 24.0f, -23.0f }; //1層目を映すときの位置
		RyoEngine::Vector3 baseRotate_ = { 0.7183f, -0.5779f, 0.0f }; //原点を見下ろすいつもの向き
		float baseOrthoSize_ = 12.6f * FloorConfig::kWorldScale; //床の大きさに合わせた画面の縦の半分に映す長さ

		//ゲーム中だけ変えてシーンが変わると消える値
		float floorHeight_ = 0.0f; //今映している床の高さ
		RyoEngine::Vector3 focusOffset_{}; //映す所をずらした分
		float zoomOrthoSize_ = 0.0f; //寄せたときの広さで0なら寄せていない
		float pitchOffset_ = 0.0f; //縦に傾けたラジアン
		RyoEngine::Vector3 shakeOffset_{}; //揺らしてずらした分
	};
}
