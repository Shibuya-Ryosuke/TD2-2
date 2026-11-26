#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "../Game/TileBarrage.h"

namespace App
{
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;

	//チュートリアルの床から寄りながらじたばた落ちて尻もちをつき、残った足場が寝ているボスへ次々当たるまで
	class TutorialFinale
	{
	public:
		//このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0; //足場が届いたボスへのダメージ
			bool isFinished = false; //カメラがいつもの映し方に戻ってゲームシーンへ渡せる
		};

		TutorialFinale() = default;
		~TutorialFinale() = default;

		//足場を飛ばす流れを持つのでコピーを禁止する
		TutorialFinale(const TutorialFinale&) = delete;
		TutorialFinale& operator=(const TutorialFinale&) = delete;

		void Initialize(); //始まっていない状態にする
		//ボスの反対側の床へじたばた落ち始める
		void Begin(LayerFloors& floors, Player& player, const Boss& boss);
		//1フレーム進める
		Result Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction);

		//落ちる線を出す間か
		bool IsFalling() const { return stage_ == Stage::Fall; }
		//動いている間か
		bool IsActive() const { return stage_ != Stage::Idle; }


	private:
		//流れの段階
		enum class Stage
		{
			Idle, //始まっていない
			Fall, //寄りながらじたばた落ちる
			Sit, //尻もちをついている
			Barrage, //残った足場が寝ているボスへ次々当たる
			ZoomBack, //いつもの映し方へ戻る
			Done, //終わった
		};

		void UpdateFall(LayerFloors& floors, Player& player, StageCamera& camera, GameDirection& direction);
		int32_t UpdateBarrage(float deltaTime, LayerFloors& floors, Boss& boss, StageCamera& camera, GameDirection& direction);
		void UpdateZoomBack(StageCamera& camera);
		void HitBoss(Boss& boss, LayerFloors& floors, GameDirection& direction); //当たった足場の分だけつぶれて揺れる

		Stage stage_ = Stage::Idle;
		float timer_ = 0.0f; //今の段階に入ってからの秒数
		RyoEngine::Vector3 fallStart_{}; //落ち始めた位置
		RyoEngine::Vector3 landing_{}; //尻もちをつく位置
		RyoEngine::Vector3 bossRest_{}; //寝ているボスの足元
		float startFloorHeight_ = 0.0f;
		float landingFloorHeight_ = 0.0f;
		float zoomFrom_ = 0.0f; //ZoomBackを始めたときの映す広さ
		RyoEngine::Vector3 focusFrom_{}; //ZoomBackを始めたときの映す所のずれ
		TileBarrage barrage_; //ボスへ飛ぶ足場
	};
}
