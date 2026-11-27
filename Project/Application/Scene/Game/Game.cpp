#include "Game.h"
#include "../Clear/Clear.h"
#include "../GameOver/GameOver.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/BossConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Config/PlayerConfig.h"
#include "../../Input/GameInput.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Boss/Phase/BossPhaseTable.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Light/SunLight.h"
#include "BossContact.h"
#include "GameHud.h"
#include "SpawnPlanner.h"
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
		//空の画像の色がそのまま出るようにゲーム中だけACESを切って、太陽の光をそろえる
		toneMap_.Apply();
		SunLight::Apply();

		//プレイヤーとボスを作る
		player_ = std::make_unique<Player>();
		player_->Initialize();
		boss_ = std::make_unique<Boss>();
		boss_->Initialize();
		bossFall_.Initialize(); //ボスが落ちる間だけ見せるパーツのボスを用意する
		direction_.Initialize(); //UpdateVisualsより先に画面の演出を用意する
		sky_.Initialize(); //UpdateVisualsより先に背景の天球を用意する
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// items_.Initialize();

		//チュートリアルから来たらその続きから、リトライならボス戦の最初から
		health_.Reset();
		boss_->ResetHealth();
		if (startMode_ == StartMode::AfterTutorial)
		{
			StartAfterTutorial();
		}
		else
		{
			floors_.Start(0);
			ResetFloor(true);
		}
		//次にこのシーンを開き直したらボス戦の最初から
		startMode_ = StartMode::Battle;
		UpdateVisuals();
		//前のシーンから画面を覆っていた積み木を落とす
		transition_.Initialize(camera_);
	}

	//シーンを出るとき
	void Game::Finalize()
	{
		//ほかのシーンのために元に戻す
		defeatSequence_.RestoreTime();
		nearFade_.Restore();
		toneMap_.Restore();
	}

	void Game::Update()
	{
		if (isGameOver_)
		{
			UpdateDefeat();
			return;
		}
		//ボスの形態エディタ。デバッグのときだけ出る
		BossPhaseTable::DrawEditor(boss_.get());

		//場面切り替えの間は操作もゲームも止める
		if (transition_.Update())
		{
			UpdateVisuals();
			return;
		}
		//ポーズ中はメニューだけ動かして、閉じたフレームも決定のAで床を崩さないようにゲームを止める
		if (pauseMenu_.IsOpen())
		{
			UpdatePause();
			UpdateVisuals();
			return;
		}
		if (GameInput::IsPauseTriggered())
		{
			pauseMenu_.Open();
			UpdateVisuals();
			return;
		}
		UpdateGame(GetDeltaTime());
		UpdateVisuals();
		if (isGameOver_)
		{
			defeatSequence_.Begin(floors_.GetCurrent(), floors_.GetBelow(), *camera_, *player_, *boss_);
			sky_.Update(*camera_, 0.0f);
			return;
		}
		//クリアはプレイヤーのポーズを見せてから
		if (isCleared_ && clearPose_.Update(GetDeltaTime(), *camera_, *player_))
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

		//ボスが起きる所と穴に落ちた流れはそれぞれで動かす
		if (phase_ == GamePhase::BossWake)
		{
			UpdateBossWake(deltaTime);
			return;
		}
		if (phase_ == GamePhase::PlayerFall)
		{
			UpdatePlayerFall(deltaTime);
			return;
		}

		//ボスが落ちて足場が飛んでいく間はプレイヤーは動かせない
		if (BossFallFlow::IsLockedPhase(phase_))
		{
			UpdateBossFall(deltaTime);
			return;
		}

		//入力を読んで、向きの入力か進む方を向く
		const PlayerCommand command = GameInput::ReadCommand();
		action_.ApplyLook(command, *player_);

		//ボスと同時に正面の床を崩して移動する
		Floor& floor = floors_.GetCurrent();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// if (command.isBomb)
		// {
			// items_.TryUseBomb(floor, GetPlayerCell());
		// }
		if (action_.Apply(command, *player_, floor, deltaTime))
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
			if (battle_.CheckBossFooting(floor, *boss_, GetTuning(), isFloorChanged, bossCell) == BossBattle::Event::BossFell)
			{
				BeginBossDrop();
				return;
			}
			HandleBattleEvent(battle_.Update(floor, *player_, *boss_, health_, GetTuning(), isFloorChanged, bossCell, deltaTime));
			return;
		}
		if (phase_ == GamePhase::Escape)
		{
			//揺れている間もプレイヤーは動けてボスに触れるとハートが減り、時間が来たらボスごと落ちる
			BossContact::Check(*player_, *boss_, health_);
			if (health_.IsDead())
			{
				Defeat();
				return;
			}
			UpdateBossFall(deltaTime);
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
		//層を降りる間は落ちる線を出し、揺れはカメラに渡す
		const bool isFalling = playerFall_.IsFalling() || bossFall_.IsPlayerFalling();
		direction_.Update(health_, isFalling, *player_, *boss_, floors_.GetCurrent());
		camera_->SetShakeOffset(direction_.GetShakeOffset());

		//戦闘中だけ床に正面の1マスの予告を出す
		action_.UpdatePreview(floors_.GetCurrent(), *player_, phase_ == GamePhase::Battle);
		floors_.UpdateVisuals();
		player_->UpdateVisuals();
		boss_->UpdateVisuals();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// items_.UpdateVisuals();
		sky_.Update(*camera_);
	}

	void Game::Draw()
	{
		//背景の天球、床とキャラと落ちている間のパーツのボスと積み木の順に描く
		sky_.Draw();
		floors_.Draw();
		player_->Draw();
		boss_->Draw();
		bossFall_.Draw();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// items_.Draw();
		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
		if (!transition_.IsBusy() && !isGameOver_)
		{
			GameHud::Draw(GameHud::MakeInfo(health_, floors_, *boss_, phase_, pauseMenu_.IsOpen(), bossFall_));
			pauseMenu_.Draw();
		}
	}

	//床を元に戻して最初の位置からすぐに戦闘を始める
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
		StartBattle();
	}

	//ボスを落ちてきた所か最初のマスの近くに置き、プレイヤーをその反対側の離れた所に置く
	void Game::PlaceActors(bool keepBossLanding)
	{
		const Floor& floor = floors_.GetCurrent();
		Vector3 fallingFrom = boss_->GetPosition();
		if (!keepBossLanding)
		{
			fallingFrom = LayerFloors::GetSpawnPosition(floor, LayerConfig::kBossSpawnX, LayerConfig::kBossSpawnZ);
		}
		const Vector3 bossPosition = SpawnPlanner::FindBossPosition(floor, fallingFrom);
		player_->Place(SpawnPlanner::FindPlayerPosition(floor, bossPosition));
		battle_.Reset();
		boss_->SetActive(true);
		boss_->Place(floor, bossPosition);
	}

	//プレイヤーのいるマス
	Cell Game::GetPlayerCell() const
	{
		return floors_.GetCurrent().GetCellAt(player_->GetPosition());
	}

	//穴に落ちたらハートを減らして、じたばた下の層へ落ちる
	void Game::FailFall()
	{
		health_.ClearInvincible();
		health_.Damage(PlayerConfig::kFallDamage);
		if (health_.IsDead())
		{
			Defeat();
			return;
		}
		playerFall_.Begin(floors_, *player_, *boss_);
		phase_ = GamePhase::PlayerFall;
	}

	//HPがなくなったら進行をやられ演出へ切り替える
	void Game::Defeat()
	{
		isGameOver_ = true;
	}

	//ボスや床を止めたまま倒れる姿を映し、最後に積み木で画面を覆う
	void Game::UpdateDefeat()
	{
		if (!transition_.Update() && defeatSequence_.Update(GetDeltaTime(), *camera_, *player_, *boss_))
		{
			transition_.Start<GameOver>();
		}
		sky_.Update(*camera_, 0.0f);
	}

	//今の層のボスの強さに、今の形態の速さなどを入れたもの
	BossTuning Game::GetTuning() const {
		BossTuning tuning = GetBossTuning(floors_.GetLayer());
		boss_->ApplyPhaseTuning(tuning);
		return tuning;
	}

	//ボスのダメージ
	void Game::DamageBoss(int32_t amount)
	{
		boss_->Damage(amount);
	}

	//すぐに戦闘を始める
	void Game::StartBattle()
	{
		battle_.ClearSeparationCheck();
		phase_ = GamePhase::Battle;
	}
}
