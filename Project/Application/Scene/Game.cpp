#include "Game.h"
#include "Clear.h"
#include "GameOver.h"
#include "../Camera/StageCamera.h"
#include "../Game/GameConfig.h"
#include "../Input/GameInput.h"
#include "../Object/Boss.h"
#include "../Object/Player.h"
#include "../Stage/GridDirection.h"
#include "../UI/GameHud.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	// Player・Boss をヘッダーで前方宣言だけしているので、ここで作り・消す
	Game::Game() = default;
	Game::~Game() = default;

	// =====================================================================
	// 初期化
	// =====================================================================
	void Game::Initialize()
	{
		// カメラ (最初に作ったカメラが自動でアクティブになる)
		const uint32_t cameraIndex = CameraManager::CreateCamera<StageCamera>("Stage");
		camera_ = CameraManager::GetCamera<StageCamera>(cameraIndex);

		// 平行投影だと全部のモデルが網目に消えてしまうので、エンジンの「カメラに近いモデルを消す」を切る
		nearFade_.Disable();

		// プレイヤー・ボス・形・アイテムを作る
		player_ = std::make_unique<Player>();
		player_->Initialize();
		boss_ = std::make_unique<Boss>();
		boss_->Initialize();
		shapes_.Initialize();
		items_.Initialize();

		// ゲーム開始 (Unity の BeginGame)
		health_.Reset();
		bossHealth_ = GameConfig::kBossMaxHealth;
		floors_.Start(0);
		ResetFloor(true);
		UpdateVisuals();
	}

	// シーンを出るとき
	void Game::Finalize()
	{
		// ほかのシーン (遠近法のカメラ) のために元に戻す
		nearFade_.Restore();
	}

	// =====================================================================
	// 更新
	// =====================================================================
	void Game::Update()
	{
		// ポーズ
		if (GameInput::IsPauseTriggered())
		{
			isPaused_ = !isPaused_;
		}
		if (!isPaused_)
		{
			UpdateGame(GetDeltaTime());
		}
		UpdateVisuals();

		// クリア・ゲームオーバー
		if (isGameOver_)
		{
			SceneManager::ChangeScene<GameOver>();
			return;
		}
		if (isCleared_)
		{
			SceneManager::ChangeScene<Clear>();
		}
	}

	// ゲームの1フレーム (場面ごとに進める)
	void Game::UpdateGame(float deltaTime)
	{
		if (isGameOver_ || isCleared_)
		{
			return;
		}

		// タイマーを進める (無敵・形を使えるまでの待ち)
		health_.Update(deltaTime);
		shapes_.Update(deltaTime);
		player_->SetVisible(health_.IsVisible());

		// ボスが落ちて下の層へ・足場が飛んでいく間は、プレイヤーは動かせない
		if (BossFallFlow::IsLockedPhase(phase_))
		{
			UpdateBossFall(deltaTime);
			return;
		}

		// 入力 (移動・向き・床を崩す・十字爆弾)。向きは上下左右のどれかにする
		const PlayerCommand command = GameInput::ReadCommand();
		if (command.hasLook)
		{
			player_->SetFacing(GridDirection::FromInput(command.look));
		}

		// 「方向を入力すると戦闘開始」: 移動か床を崩す入力があるまで何も進めない
		if (phase_ == GamePhase::Waiting)
		{
			if (!command.HasAction())
			{
				return;
			}
			phase_ = GamePhase::Battle;
		}

		// 床を崩す (形・十字爆弾)
		Floor& floor = floors_.GetCurrent();
		const bool wasJumping = player_->IsJumping();
		if (command.isBreak)
		{
			shapes_.TryBreak(floor, GetPlayerCell(), player_->GetFacing());
		}
		if (command.isBomb)
		{
			items_.TryUseBomb(floor, GetPlayerCell());
		}

		// プレイヤー
		if (player_->Advance(command.move, command.hasMovement, floor, deltaTime))
		{
			FailFall();
			return;
		}

		// 床 (崩す予定のタイルの時間を進める) → アイテム (戦闘中だけ降ってくる。触れたら取る)
		const bool isFloorChanged = floor.Tick(deltaTime);
		const Cell bossCell = floor.GetCellAt(boss_->GetPosition());
		items_.Update(floor, phase_ == GamePhase::Battle, health_.GetHearts(), *player_, boss_->GetPosition(), deltaTime);

		if (phase_ == GamePhase::Battle)
		{
			// ボスの足元が崩れた: 跳んで逃げられなければ落ちる
			if (battle_.CheckBossFooting(floor, *boss_, GetTuning(), isFloorChanged, bossCell) == BossBattle::Event::BossFell)
			{
				BeginBossDrop();
				return;
			}
			// 跳んでいる途中・跳び終わった瞬間にぶつかったら「体当たり」
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
			// 揺れている間もプレイヤーは動ける。時間が来たらボスごと落ちる
			battle_.CheckContact(floor, *player_, *boss_, false);
			UpdateBossFall(deltaTime);
		}
	}

	// ボスを落とす流れを進める
	void Game::UpdateBossFall(float deltaTime)
	{
		const BossFallFlow::Result result = bossFall_.Update(deltaTime, floors_, *player_, *boss_, *camera_);
		DamageBoss(result.bossDamage);

		// 揺れ終わったとき、切り離された床の上にいた: プレイヤーも落ちる
		if (result.isPlayerCaught)
		{
			FailFall();
			return;
		}
		// ボスが落ち始めたら、置いてあるアイテムは消す (持っているアイテムは持ち越す)
		if (result.isDropStarted)
		{
			items_.ClearFieldItem();
		}
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

	// 戦闘中に起きたことに合わせて場面を変える
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

	// 見た目を確定する
	void Game::UpdateVisuals()
	{
		// 床の予告 (戦闘中・戦闘開始待ちのときだけ) → 床 → キャラ → アイテム → 右のパネル
		bool isPreviewVisible = false;
		if (phase_ == GamePhase::Waiting || phase_ == GamePhase::Battle)
		{
			isPreviewVisible = true;
		}
		shapes_.UpdatePreview(floors_.GetCurrent(), GetPlayerCell(), player_->GetFacing(), isPreviewVisible);
		floors_.UpdateVisuals();
		player_->UpdateVisuals();
		boss_->UpdateVisuals();
		items_.UpdateVisuals();
		shapes_.UpdatePanel(*camera_, player_->GetFacing());
	}

	// =====================================================================
	// 描画
	// =====================================================================
	void Game::Draw()
	{
		// 床 → キャラ → アイテム → 右のパネル の順に描く
		floors_.Draw();
		player_->Draw();
		boss_->Draw();
		items_.Draw();
		shapes_.DrawPanel();

		DrawDebugText();
	}

	// デバッグ表示 (GameHud に渡す)
	void Game::DrawDebugText()
	{
		GameHudInfo info{};
		info.hearts = health_.GetHearts();
		info.layer = floors_.GetLayer();
		info.bossHealth = bossHealth_;
		info.phase = phase_;
		info.isPaused = isPaused_;
		info.escapeTimer = bossFall_.GetEscapeTimer();
		info.barrageTiles = bossFall_.GetBarrage().GetTileCount();
		info.barrageDamage = bossFall_.GetBarrage().GetDamageApplied();
		info.hasBomb = items_.HasBomb();
		info.hasJumpBoost = player_->HasJumpBoost();
		GameHud::Draw(info);

		// 次の形のパネルの文字 (NOW / NEXT)
		shapes_.DrawPanelLabels();
	}

	// =====================================================================
	// 床・配置
	// =====================================================================
	// 床を元に戻して、最初の位置からやり直す (方向の入力待ちになる)
	void Game::ResetFloor(bool showBelow)
	{
		// 床を戻す → 下の層がなければ作る → キャラを置く → カメラを合わせる
		floors_.GetCurrent().Reset();
		if (showBelow)
		{
			floors_.EnsureBelow();
		}
		PlaceActors(false);
		camera_->SetFloorHeight(floors_.GetCurrent().GetHeight());
		health_.ClearInvincible();
		player_->SetVisible(true);

		// やり直しなので、置いてあるアイテムも持っているアイテムも消える
		items_.ResetForRetry();
		player_->SetJumpBoost(false);
		WaitForMovement();
	}

	// プレイヤーとボスを最初の位置に置く
	void Game::PlaceActors(bool keepBossLanding)
	{
		const Floor& floor = floors_.GetCurrent();
		player_->Place(LayerFloors::GetSpawnPosition(floor, GameConfig::kPlayerSpawnX, GameConfig::kPlayerSpawnZ));
		battle_.Reset();
		boss_->SetActive(true);

		// ボスの位置 (keepBossLanding なら、落ちてきた場所のまま)
		Vector3 bossPosition = boss_->GetPosition();
		if (!keepBossLanding)
		{
			bossPosition = LayerFloors::GetSpawnPosition(floor, GameConfig::kBossSpawnX, GameConfig::kBossSpawnZ);
		}
		boss_->Place(floor, bossPosition);
	}

	// プレイヤーのいるマス
	Cell Game::GetPlayerCell() const
	{
		return floors_.GetCurrent().GetCellAt(player_->GetPosition());
	}

	// =====================================================================
	// 当たり・ダメージ
	// =====================================================================
	// 穴に落ちた: ハートを減らしてやり直し
	void Game::FailFall()
	{
		// ダメージ → 生きていれば床を戻して、少しの間無敵
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

	// プレイヤーの HP が 0 になった: ボスを止めてゲームオーバー
	void Game::Defeat()
	{
		boss_->Stop();
		isGameOver_ = true;
	}

	// ボスのダメージ
	void Game::DamageBoss(int32_t amount)
	{
		bossHealth_ = std::max(0, bossHealth_ - amount);
	}

	// =====================================================================
	// 場面の切り替え
	// =====================================================================
	// 方向の入力待ちにする
	void Game::WaitForMovement()
	{
		battle_.ClearSeparationCheck();
		phase_ = GamePhase::Waiting;
	}

	// ボスの島が切り離された: 揺れ始める
	void Game::BeginEscape()
	{
		boss_->Stop();
		bossFall_.BeginEscape();
		phase_ = bossFall_.GetPhase();
	}

	// ボスが下の層へ落ち始める (足元が崩れた・穴に着地した)
	void Game::BeginBossDrop()
	{
		// ボスを止めて、置いてあるアイテムは消す (持っているアイテムは持ち越す)
		boss_->Stop();
		items_.ClearFieldItem();
		bossFall_.BeginDrop(floors_, *player_, *boss_);
		phase_ = bossFall_.GetPhase();
	}

	// 下の層に着いた: ハートを少し回復して、キャラを置き直す。新しい層でアイテムがまた出るようになる
	void Game::OnArrivedBelow()
	{
		items_.ResetForNewLayer();
		health_.Heal(GameConfig::kDescentHeal);
		health_.ClearInvincible();
		PlaceActors(true);
	}

	// 足場を飛ばし終わった: クリアか、次の戦闘待ちへ
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
