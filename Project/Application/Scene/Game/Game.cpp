#include "Game.h"
#include "../Clear/Clear.h"
#include "../GameOver/GameOver.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/GameConfig.h"
#include "../../Input/GameInput.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Grid/GridDirection.h"
#include "GameHud.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//ヘッダーで前方宣言だけしているのでここで作って消す
	Game::Game() = default;
	Game::~Game() = default;

	void Game::Initialize()
	{
		//最初に作ったカメラが自動でアクティブになる
		const uint32_t cameraIndex = CameraManager::CreateCamera<StageCamera>("Stage");
		camera_ = CameraManager::GetCamera<StageCamera>(cameraIndex);

		//平行投影だと全部網目に消えてしまうのでNearFadeを切る
		nearFade_.Disable();

		//プレイヤーとボスと形を作る
		player_ = std::make_unique<Player>();
		player_->Initialize();
		boss_ = std::make_unique<Boss>();
		boss_->Initialize();
		shapes_.Initialize();
		direction_.Initialize(); //UpdateVisualsより先に画面の演出と残像を用意する
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// items_.Initialize();

		//ゲーム開始
		health_.Reset();
		bossHealth_ = GameConfig::kBossMaxHealth;
		floors_.Start(0);
		ResetFloor(true);
		UpdateVisuals();
		//タイトルから来たら積み木が吹き飛ぶ演出から始まる
		transition_.Initialize(camera_);
	}

	//シーンを出るとき
	void Game::Finalize()
	{
		//ほかのシーンのために元に戻す
		nearFade_.Restore();
	}

	void Game::Update()
	{
		//場面切り替えの間は操作もゲームも止める
		if (transition_.Update())
		{
			UpdateVisuals();
			return;
		}
		//ポーズ
		if (GameInput::IsPauseTriggered())
		{
			isPaused_ = !isPaused_;
		}
		if (!isPaused_)
		{
			UpdateGame(GetDeltaTime());
		}
		UpdateVisuals();
		//積み木で画面を埋めてからクリアかゲームオーバーへ
		if (isGameOver_)
		{
			transition_.Start<GameOver>();
			return;
		}
		if (isCleared_)
		{
			transition_.Start<Clear>();
		}
	}

	//場面ごとにゲームを1フレーム進める
	void Game::UpdateGame(float deltaTime)
	{
		if (isGameOver_ || isCleared_)
		{
			return;
		}

		//無敵のタイマーを進める
		health_.Update(deltaTime);
		player_->SetVisible(health_.IsVisible());

		//ボスが落ちて足場が飛んでいく間はプレイヤーは動かせない
		if (BossFallFlow::IsLockedPhase(phase_))
		{
			UpdateBossFall(deltaTime);
			return;
		}

		//入力を読んで、ボスの番でなければ向きを変える
		const PlayerCommand command = GameInput::ReadCommand();
		PlayerTurn& turn = battle_.GetTurn();
		turn.ApplyLook(command, phase_, *player_, *boss_);

		//LBで前、RBで次の形を選ぶ
		shapes_.Select(command.isPreviousShape, command.isNextShape);

		//移動か床を崩す入力があるまで戦闘を始めない
		if (phase_ == GamePhase::Waiting)
		{
			if (!command.HasAction())
			{
				return;
			}
			phase_ = GamePhase::Battle;
		}

		//プレイヤーの番に床を崩すか移動して、動いたらボスの番になる
		Floor& floor = floors_.GetCurrent();
		const bool wasJumping = player_->IsJumping();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// if (command.isBomb)
		// {
			// items_.TryUseBomb(floor, GetPlayerCell());
		// }
		if (turn.ApplyAction(command, phase_, *player_, shapes_, floor, *boss_, deltaTime))
		{
			FailFall();
			return;
		}

		//崩す予定のタイルの時間を進める
		const bool isFloorChanged = floor.Tick(deltaTime);
		const Cell bossCell = floor.GetCellAt(boss_->GetPosition());
		//[アイテム] 戦闘中だけ降ってきて触れたら取るのでコメントアウト
		// items_.Update(floor, phase_ == GamePhase::Battle, health_.GetHearts(), *player_, boss_->GetPosition(), deltaTime);

		if (phase_ == GamePhase::Battle)
		{
			//ボスの足元が崩れて逃げられなければ落ちる
			if (battle_.CheckBossFooting(floor, *player_, *boss_, GetTuning(), isFloorChanged, bossCell) == BossBattle::Event::BossFell)
			{
				BeginBossDrop();
				return;
			}
			//跳んでいる途中か跳び終わった瞬間にぶつかったら体当たり
			bool isJumpImpact = wasJumping;
			if (player_->IsJumping())
			{
				isJumpImpact = true;
			}
			HandleBattleEvent(battle_.Update(floor, *player_, *boss_, health_, GetTuning(), isFloorChanged, bossCell, isJumpImpact, deltaTime));
			return;
		}
		if (phase_ == GamePhase::Escape)
		{
			//揺れている間もプレイヤーは動けて、時間が来たらボスごと落ちる
			battle_.CheckContact(floor, *player_, *boss_, false);
			UpdateBossFall(deltaTime);
		}
	}

	//ボスを落とす流れを進める
	void Game::UpdateBossFall(float deltaTime)
	{
		const BossFallFlow::Result result = bossFall_.Update(deltaTime, floors_, *player_, *boss_, *camera_);
		DamageBoss(result.bossDamage);

		//揺れ終わったとき切り離された床にいたらプレイヤーも落ちる
		if (result.isPlayerCaught)
		{
			FailFall();
			return;
		}
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //ボスが落ち始めたら置いてあるアイテムを消して持っているものは持ち越す
		// if (result.isDropStarted)
		// {
			// items_.ClearFieldItem();
		// }
		phase_ = bossFall_.GetPhase();
		if (result.isArrived)
		{
			OnArrivedBelow();
		}
		if (result.isFinished)
		{
			FinishBossFall();
		}
	}

	//戦闘中に起きたことに合わせて場面を変える
	void Game::HandleBattleEvent(BossBattle::Event event)
	{
		if (event == BossBattle::Event::BossIsolated)
		{
			BeginEscape();
		}
		else if (event == BossBattle::Event::BossFell)
		{
			BeginBossDrop();
		}
		else if (event == BossBattle::Event::Separated)
		{
			ResetFloor(true);
		}
		else if (event == BossBattle::Event::PlayerDefeated)
		{
			Defeat();
		}
	}

	//見た目を確定する
	void Game::UpdateVisuals()
	{
		//戦闘中と戦闘開始待ちのときだけ床に予告を出す
		direction_.Update(health_, phase_, *player_, *boss_, floors_.GetCurrent(), GetTuning());
		bool isPreviewVisible = false;
		if (phase_ == GamePhase::Waiting || phase_ == GamePhase::Battle)
		{
			isPreviewVisible = true;
		}
		shapes_.UpdatePreview(floors_.GetCurrent(), GetPlayerCell(), player_->GetFacing(), isPreviewVisible);
		floors_.UpdateVisuals();
		player_->UpdateVisuals();
		boss_->UpdateVisuals();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// items_.UpdateVisuals();
		shapes_.UpdatePanel(*camera_, player_->GetFacing());
	}

	void Game::Draw()
	{
		//床とキャラと右上の輪と積み木の順に描く
		floors_.Draw();
		player_->Draw();
		boss_->Draw();
		direction_.Draw(); //半透明のボスの残像はキャラのあと
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// items_.Draw();
		shapes_.DrawPanel();
		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
		//デバッグ表示
		GameHud::Draw(GameHud::MakeInfo(health_, floors_, bossHealth_, phase_, isPaused_, bossFall_));
	}

	//床を元に戻して最初の位置から方向の入力待ちにする
	void Game::ResetFloor(bool showBelow)
	{
		//床を戻して下の層を作り、キャラを置いてカメラを合わせる
		floors_.GetCurrent().Reset();
		if (showBelow)
		{
			floors_.EnsureBelow();
		}
		PlaceActors(false);
		camera_->SetFloorHeight(floors_.GetCurrent().GetHeight());
		health_.ClearInvincible();
		player_->SetVisible(true);

		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //やり直しなので置いてあるアイテムも持っているアイテムも消える
		// items_.ResetForRetry();
		// player_->SetJumpBoost(false);
		WaitForMovement();
	}

	//プレイヤーとボスを最初の位置に置く
	void Game::PlaceActors(bool keepBossLanding)
	{
		const Floor& floor = floors_.GetCurrent();
		player_->Place(LayerFloors::GetSpawnPosition(floor, GameConfig::kPlayerSpawnX, GameConfig::kPlayerSpawnZ));
		battle_.Reset();
		boss_->SetActive(true);

		//keepBossLandingなら落ちてきた場所のままのボスの位置
		Vector3 bossPosition = boss_->GetPosition();
		if (!keepBossLanding)
		{
			bossPosition = LayerFloors::GetSpawnPosition(floor, GameConfig::kBossSpawnX, GameConfig::kBossSpawnZ);
		}
		boss_->Place(floor, bossPosition);
	}

	//プレイヤーのいるマス
	Cell Game::GetPlayerCell() const
	{
		return floors_.GetCurrent().GetCellAt(player_->GetPosition());
	}

	//穴に落ちたらハートを減らしてやり直す
	void Game::FailFall()
	{
		//生きていれば床を戻して少しの間無敵
		health_.ClearInvincible();
		health_.Damage(GameConfig::kFallDamage);
		if (health_.IsDead())
		{
			Defeat();
			return;
		}
		ResetFloor(true);
		health_.StartInvincible();
	}

	//HPが0になったらボスを止めてゲームオーバー
	void Game::Defeat()
	{
		boss_->Stop();
		isGameOver_ = true;
	}

	//ボスのダメージ
	void Game::DamageBoss(int32_t amount)
	{
		bossHealth_ = std::max(0, bossHealth_ - amount);
	}

	//方向の入力待ちにする
	void Game::WaitForMovement()
	{
		battle_.ClearSeparationCheck();
		phase_ = GamePhase::Waiting;
	}

	//ボスの島が切り離されたら揺れ始める
	void Game::BeginEscape()
	{
		boss_->Stop();
		bossFall_.BeginEscape();
		phase_ = bossFall_.GetPhase();
	}

	//ボスが下の層へ落ち始める
	void Game::BeginBossDrop()
	{
		//ボスを止める
		boss_->Stop();
		//[アイテム] 置いてあるアイテムを消して持っているものは持ち越すのでコメントアウト
		// items_.ClearFieldItem();
		bossFall_.BeginDrop(floors_, *player_, *boss_);
		phase_ = bossFall_.GetPhase();
	}

	//下の層に着いたらハートを少し回復してキャラを置き直す
	void Game::OnArrivedBelow()
	{
		//[アイテム] 新しい層でアイテムがまた出るようにするのでコメントアウト
		// items_.ResetForNewLayer();
		health_.Heal(GameConfig::kDescentHeal);
		health_.ClearInvincible();
		PlaceActors(true);
	}

	//足場を飛ばし終わったらクリアか次の戦闘待ちへ
	void Game::FinishBossFall()
	{
		if (bossHealth_ == 0)
		{
			boss_->SetActive(false);
			isCleared_ = true;
			return;
		}
		WaitForMovement();
	}
}
