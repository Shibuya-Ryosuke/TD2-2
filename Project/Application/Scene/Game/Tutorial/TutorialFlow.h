#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "TutorialBlock.h"
#include "TutorialFinale.h"
#include "TutorialMap.h"
#include "PlayerLaunch.h"

namespace App
{
	struct PlayerCommand;
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;
	class PlayerAction;

	//動く、壊す、積み木を落とす、切り離す、下へ落ちるを順にやって、寝ているボスを起こすまでのチュートリアル
	class TutorialFlow
	{
	public:
		//チュートリアルが動かすもの
		struct Context
		{
			LayerFloors& floors;
			Player& player;
			Boss& boss;
			StageCamera& camera;
			PlayerAction& action;
			GameDirection& direction;
		};

		//このフレームで起きたこと
		struct Result
		{
			int32_t bossDamage = 0; //残った足場のボスへのダメージ
			bool isFinished = false; //ボスが起きてボス戦を始められる
		};

		void Initialize(); //csvを読んで積み木を作る
		//プレイヤーを最初の位置に、ボスを下の層に寝かせて始める
		void Begin(const Context& context);
		//1フレーム進める
		Result Update(const Context& context, const PlayerCommand& command, float deltaTime);

		//落ちる線を出す間か
		bool IsFalling() const;
		//床に正面の1マスの予告を出すか
		bool IsPreviewVisible() const;

		void Draw(); //積み木を描く
		//やることと残りの数とボスの寝言をデバッグ表示で出す
		void DrawText(const StageCamera& camera, const Boss& boss) const;

	private:
		//やることの段階
		enum class Step
		{
			Move, //5マス動く
			Break, //3回壊す
			BlockOneFall, //1つ目の積み木が降ってくる
			BlockOneDrop, //積み木のマスを壊して落とす
			Reshape, //最初の位置へ飛ばされて、切り離しやすく床が崩れる
			BlockTwoFall, //2つ目の積み木が島に降ってくる
			BlockTwoCut, //島を切り離す
			IslandFall, //切り離した島が揺れて落ちる
			GoDown, //穴か外へ落ちる
			Finale, //下の層でボスが起きるまで
		};

		void ChangeStep(const Context& context, Step step); //段階を変えて、その段階の始めにやることをする
		void Clear(const Context& context, Step next); //CLEAR!を出して少し待ってから次へ
		void DropBlock(const Context& context, int32_t code); //その数字のマスか、崩れるか穴ならいちばん近い崩れない床へ積み木を降らせる
		static int32_t ToBlockCode(Step step); //積み木が降ってくる段階で使うcsvの数字
		static bool CanHoldBlock(const Floor& floor, const Cell& cell); //積み木を置ける崩れる予定のない床か
		bool CanControl() const; //プレイヤーを動かせるか
		void UpdateControl(const Context& context, const PlayerCommand& command, float deltaTime); //プレイヤーを動かして落ちたら戻す
		void UpdateStep(const Context& context, bool isFloorChanged, float deltaTime); //段階ごとの進み具合を見る
		void UpdateReshape(const Context& context, float deltaTime); //飛ばして床を崩す
		void UpdateIslandFall(const Context& context); //島を揺らして落とす
		void StartRescue(const Context& context); //落ちたらトランポリンで最初の位置へ戻す
		RyoEngine::Vector3 GetStartPosition(const Floor& floor) const; //最初の位置か、穴ならいちばん近い床

		TutorialMap map_; //csvの数字
		TutorialBlock block_; //降ってくる仮の積み木
		PlayerLaunch launch_; //トランポリンで飛ばす
		TutorialFinale finale_; //下の層でボスが起きるまで

		Step step_ = Step::Move;
		bool hasPendingStep_ = false; //CLEAR!のあと次へ進むのを待っているか
		Step pendingStep_ = Step::Move;
		float pendingTimer_ = 0.0f; //次へ進むまでの残り
		float clearTimer_ = 0.0f; //CLEAR!を出す残り
		float stepTimer_ = 0.0f; //今の段階に入ってからの秒数
		int32_t jumpStart_ = 0; //5マス動くを始めたときに跳んでいた回数
		int32_t movedCount_ = 0; //5マス動くで動いたマスの数
		int32_t breakCount_ = 0; //壊した回数
		bool hasLaunched_ = false; //床を崩す前に飛ばしたか
		bool isWallScheduled_ = false; //切り離しの前の床を崩す予定にしたか
		bool isRescuing_ = false; //落ちて最初の位置へ戻している途中か
		bool isReleased_ = false; //切り離した島を穴にしたか
		bool isDropFinished_ = false; //切り離した島が落ちきったか
	};
}
