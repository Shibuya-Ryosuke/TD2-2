#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "TutorialBlock.h"
#include "TutorialFinale.h"
#include "TutorialMap.h"
#include "PlayerLaunch.h"
#include "TutorialSpot.h"

namespace App
{
	struct PlayerCommand;
	class LayerFloors;
	class Player;
	class Boss;
	class StageCamera;
	class GameDirection;
	class PlayerAction;

	//動く、壊す、積み木を落とす、切り離す、下へ落ちるを順にやって、寝ているボスに足場を当てるまでのチュートリアルの流れ
	class TutorialFlow
	{
	public:
		//チュートリアルが動かすもの
		struct Context
		{
			//全部の参照を受け取って作る
			Context(LayerFloors& floorsRef, Player& playerRef, Boss& bossRef, StageCamera& cameraRef, PlayerAction& actionRef, GameDirection& directionRef)
				: floors(floorsRef), player(playerRef), boss(bossRef), camera(cameraRef), action(actionRef), direction(directionRef)
			{
			}
			//参照を持つので作るときにコピーはできて、あとから代入はできない
			Context(const Context&) = default;
			Context(Context&&) = default;
			Context& operator=(const Context&) = delete;
			Context& operator=(Context&&) = delete;
			~Context() = default;

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
			bool isFinished = false; //足場を当て終わってゲームシーンへ渡せる
		};

		TutorialFlow() = default;
		~TutorialFlow() = default;

		//モデルを持つのでコピーを禁止する
		TutorialFlow(const TutorialFlow&) = delete;
		TutorialFlow& operator=(const TutorialFlow&) = delete;

		void Initialize(); //csvを読んで積み木と光と足場が当たったときのハートを作る
		//プレイヤーを最初の位置に、ボスを下の層に寝かせて始める
		void Begin(const Context& context);
		//1フレーム進める
		Result Update(const Context& context, const PlayerCommand& command, float deltaTime);

		//落ちる線を出す間か
		bool IsFalling() const;
		//床に正面の1マスの予告を出すか
		bool IsPreviewVisible() const;

		void Draw(); //積み木とスポットライトを描く
		//やることと残りの数と押すボタンをデバッグ表示で出す
		void DrawText() const;

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
		void ApplyBreakRule(const Context& context); //段階に合わせて壊せるマスを決めてスポットライトで照らす
		void UpdateStep(const Context& context, bool isFloorChanged); //段階ごとの進み具合を見る
		void UpdateReshape(const Context& context); //少し待って切り離しやすく床を崩す
		static bool NeedsRepair(Step next); //次へ行く前に飛ばして床を直すか
		void StartRepair(const Context& context); //最初の位置へ飛ばし始める
		void UpdateRepair(const Context& context, float deltaTime); //飛んでいる間に床を直して、立ち上がったら次へ
		void UpdateIslandFall(const Context& context); //島を揺らして落とす
		void StartRescue(const Context& context); //落ちたら穴の下までしっかり落ちてから最初の位置へ吹っ飛ばす
		RyoEngine::Vector3 GetStartPosition(const Floor& floor) const; //最初の位置か、穴ならいちばん近い床

		TutorialMap map_; //csvの数字
		TutorialBlock block_; //降ってくる仮の積み木
		PlayerLaunch launch_; //足場のばねで吹っ飛ばす
		TutorialSpot spot_; //壊すマスを照らすスポットライト
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
		bool isRepairing_ = false; //やることを終えて最初の位置へ飛ばしながら床を直している途中か
		bool hasRepaired_ = false; //飛んでいる間に床を直したか
		float repairTimer_ = 0.0f; //飛ばし始めてからの秒数
		bool isWallScheduled_ = false; //切り離しの前の床を崩す予定にしたか
		bool isRescuing_ = false; //落ちて最初の位置へ戻している途中か
		bool isReleased_ = false; //切り離した島を穴にしたか
		bool isDropFinished_ = false; //切り離した島が落ちきったか
		float rejectTimer_ = 0.0f; //壊せないマスを叩いたときの文字を出す残り
	};
}
