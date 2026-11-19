#include "Game.h"
#include "Clear.h"
#include "GameOver.h"
#include "../Camera/StageCamera.h"
#include "../Game/GameConfig.h"
#include "../Input/GameInput.h"
#include "../Object/Boss.h"
#include "../Object/Player.h"
#include "../Stage/GridDirection.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <string>

using namespace RyoEngine;

namespace App {
	namespace {
		// a から b へ、t (0 〜 1) の割合だけ進んだ値
		float LerpFloat(float a, float b, float t) {
			return a + (b - a) * t;
		}
	}

	// Player・Boss をヘッダーで前方宣言だけしているので、ここで作り・消す
	Game::Game() = default;
	Game::~Game() = default;

	// =====================================================================
	// 初期化
	// =====================================================================
	void Game::Initialize() {
		// カメラ (最初に作ったカメラが自動でアクティブになる)
		const uint32_t cameraIndex = CameraManager::CreateCamera<StageCamera>("Stage");
		camera_ = CameraManager::GetCamera<StageCamera>(cameraIndex);

		// エンジンの「カメラに近いモデルを網目状に消す」機能を切る。
		// この機能はカメラからの距離を遠近法の値から求めているので、平行投影だと
		// どのモデルも「すごく近い」と判断されて、網目に消えてしまう
		savedNearFadeStart_ = ModelManager::GetNearFadeStart();
		savedNearFadeEnd_ = ModelManager::GetNearFadeEnd();
		ModelManager::SetNearFade(0.0f, 0.0f);

		// プレイヤーとボスを作る
		player_ = std::make_unique<Player>();
		player_->Initialize();
		boss_ = std::make_unique<Boss>();
		boss_->Initialize();

		// 床を崩す形 (I O T S Z L J)
		shapes_ = BreakShape::CreateDefaultShapes();
		nextShapePanel_.Initialize();

		// ゲーム開始 (Unity の BeginGame)
		ResetShapes();
		layer_ = 0;
		hearts_ = GameConfig::kMaxHearts;
		bossHealth_ = GameConfig::kBossMaxHealth;
		current_ = CreateFloor(layer_);
		ResetFloor(true);
		UpdateVisuals();
	}

	// シーンを出るとき
	void Game::Finalize() {
		// ほかのシーン (遠近法のカメラ) のために元に戻す
		ModelManager::SetNearFade(savedNearFadeStart_, savedNearFadeEnd_);
	}

	// =====================================================================
	// 更新
	// =====================================================================
	void Game::Update() {
		// ポーズ
		if (GameInput::IsPauseTriggered()) {
			isPaused_ = !isPaused_;
		}
		if (!isPaused_) {
			UpdateGame(GetDeltaTime());
		}
		UpdateVisuals();

		// クリア・ゲームオーバー
		if (isGameOver_) {
			SceneManager::ChangeScene<GameOver>();
			return;
		}
		if (isCleared_) {
			SceneManager::ChangeScene<Clear>();
		}
	}

	// ゲームの1フレーム (場面ごとに進める)
	void Game::UpdateGame(float deltaTime) {
		if (isGameOver_ || isCleared_) {
			return;
		}

		// タイマーを減らす (無敵・形を使えるまでの待ち)
		invincible_ = std::max(0.0f, invincible_ - deltaTime);
		shapeCooldownTimer_ = std::max(0.0f, shapeCooldownTimer_ - deltaTime);
		UpdatePlayerVisibility();

		// 下の層へ降りる・足場が飛んでいく間は、プレイヤーは動かせない
		if (phase_ == Phase::Descent) {
			AdvanceDescent(deltaTime);
			return;
		}
		if (phase_ == Phase::LandingImpact) {
			timer_ += deltaTime;
			if (timer_ >= GameConfig::kLandingImpactDuration) {
				BeginTileBarrage();
			}
			return;
		}
		if (phase_ == Phase::TileBarrage) {
			AdvanceTileBarrage(deltaTime);
			return;
		}

		// 移動の入力
		const Vector2 move = GameInput::GetMove();
		const bool hasMovement = move.x * move.x + move.y * move.y > 0.0001f;

		// 向きの入力: 上下左右のどれかを向く
		const Vector2 look = GameInput::GetLook();
		if (look.x * look.x + look.y * look.y > 0.0001f) {
			player_->SetFacing(GridDirection::FromInput(look));
		}

		// 床を崩すボタン
		const bool isBreakPressed = GameInput::IsBreakTriggered();

		// 「方向を入力すると戦闘開始」: 移動か床を崩す入力があるまで何も進めない
		if (phase_ == Phase::Waiting) {
			if (!hasMovement && !isBreakPressed) {
				return;
			}
			phase_ = Phase::Battle;
		}

		// 床を崩す
		const bool wasJumping = player_->IsJumping();
		if (isBreakPressed) {
			BreakFloor();
		}

		// プレイヤー
		if (player_->Advance(move, hasMovement, *current_, deltaTime)) {
			FailFall();
			return;
		}

		// 床 (崩す予定のタイルの時間を進める)
		const bool isFloorChanged = current_->Tick(deltaTime);
		const Cell bossCell = current_->GetCellAt(boss_->GetPosition());

		// ボスの足元が崩れた: 跳んで逃げられなければ落ちる
		if (isFloorChanged && phase_ == Phase::Battle && !boss_->IsJumping() && current_->IsHole(bossCell)) {
			if (!boss_->TryEvadeCollapse(*current_, GetTuning())) {
				BeginBossDrop();
				return;
			}
		}

		if (phase_ == Phase::Battle) {
			// 跳んでいる途中・跳び終わった瞬間にぶつかったら「体当たり」
			bool isJumpImpact = wasJumping;
			if (player_->IsJumping()) {
				isJumpImpact = true;
			}
			UpdateBattle(isFloorChanged, bossCell, isJumpImpact, deltaTime);
			return;
		}
		if (phase_ == Phase::Escape) {
			CheckBossContact(false);
			AdvanceEscape(deltaTime);
		}
	}

	// 戦闘中: 切り離し・ボス・当たり を調べる
	void Game::UpdateBattle(bool isFloorChanged, const Cell& bossCell, bool isJumpImpact, float deltaTime) {
		// 床が変わったら、あとで「たどり着けるか」を調べ直す
		if (isFloorChanged) {
			separationNeedsCheck_ = true;
		}

		// ボスのいる島を切り離した
		if (isFloorChanged && !boss_->IsJumping() && current_->TryIsolate(bossCell)) {
			BeginEscape();
			return;
		}

		// ボス (床が減るほど速くなる)
		const BossTuning tuning = GetTuning();
		const float bossSpeed = tuning.moveSpeed * (1.0f + GameConfig::kMaxBossSpeedBonus * current_->GetCollapsedFraction());
		const bool wasVulnerable = invincible_ <= 0.0f;
		Damage(boss_->Tick(*current_, player_->GetPosition(), tuning, bossSpeed, deltaTime));

		// パンチが当たったら、はじかれる
		if (wasVulnerable && boss_->PunchHitThisTick() && !isGameOver_) {
			player_->ApplyKnockback(*current_, boss_->GetPunchPushDirection(), tuning.punchPushSpeed, tuning.punchPushDuration, tuning.punchLiftSpeed, false);
		}

		// ボスがジャンプから着地した所が、穴・切り離された島なら落ちる
		if (!isGameOver_ && boss_->LandedThisTick()) {
			const Cell landedCell = current_->GetCellAt(boss_->GetPosition());
			if (current_->IsHole(landedCell)) {
				BeginBossDrop();
			} else if (current_->TryIsolate(landedCell)) {
				BeginEscape();
			}
		}

		// ボスとの体当たり → 切り離しのチェック
		if (!isGameOver_ && phase_ == Phase::Battle) {
			CheckBossContact(isJumpImpact);
		}
		CheckForSeparatedFloor();
	}

	// 見た目を確定する (予告・床・プレイヤー・ボス)
	void Game::UpdateVisuals() {
		// 床の予告を出してから、床・キャラ・右のパネルの見た目を確定する
		UpdateShapePreview();
		if (current_ != nullptr) {
			current_->UpdateVisuals();
		}
		if (below_ != nullptr) {
			below_->UpdateVisuals();
		}
		if (upper_ != nullptr) {
			upper_->UpdateVisuals();
		}
		player_->UpdateVisuals();
		boss_->UpdateVisuals();
		UpdateNextShapePanel();
	}

	// =====================================================================
	// 描画
	// =====================================================================
	void Game::Draw() {
		// 下の層 → 今の層 → 上の層 → キャラ → 右のパネル の順に描く
		if (below_ != nullptr) {
			below_->Draw();
		}
		if (current_ != nullptr) {
			current_->Draw();
		}
		if (upper_ != nullptr) {
			upper_->Draw();
		}
		player_->Draw();
		boss_->Draw();
		nextShapePanel_.Draw();

		DrawDebugText();
	}

	// デバッグ表示 (HP・形・お知らせ・操作)
	void Game::DrawDebugText() {
		// 仮の表示 (デバッグ用。英数字だけ・Debug のときだけ出る)
		PrintText("HP {}/{}   LAYER {}   BOSS HP {}/{}", { 20.0f, 20.0f },
			hearts_, GameConfig::kMaxHearts, layer_ + 1, bossHealth_, GameConfig::kBossMaxHealth);

		// 次の形のパネルの文字 (NOW / NEXT)
		nextShapePanel_.DrawLabels();

		// 場面ごとのお知らせ
		const Vector2 noticePosition = { 20.0f, 80.0f };
		if (isPaused_) {
			PrintText("PAUSE  (ESC / START)", noticePosition);
		} else if (phase_ == Phase::Waiting) {
			PrintText("PRESS A DIRECTION TO START", noticePosition);
		} else if (phase_ == Phase::Escape) {
			PrintText("SEPARATED! GET TO SAFE GROUND  {:.1f}", noticePosition, std::max(0.0f, timer_));
		} else if (phase_ == Phase::Descent) {
			PrintText("JUMP TO THE NEXT LAYER", noticePosition);
		} else if (phase_ == Phase::LandingImpact) {
			PrintText("FALL DAMAGE {}", noticePosition, GameConfig::kBossFallDamage);
		} else if (phase_ == Phase::TileBarrage) {
			PrintText("TILES {}   BONUS DAMAGE {}", noticePosition, remainingTiles_, tileDamageApplied_);
		}

		// 操作は画面のいちばん下 (文字の位置はゲーム画面の解像度が基準。今は 1920 x 1080)
		const float bottomY = static_cast<float>(DirectXCommon::GetGameHeight()) - 40.0f;
		PrintText("MOVE WASD/LSTICK   FACE ARROWS/RSTICK/DPAD   BREAK SPACE/A   PAUSE ESC/START", { 20.0f, bottomY });
	}

	// =====================================================================
	// 床を崩す (テトリスの形)
	// =====================================================================
	// 形の順番を最初からにする
	void Game::ResetShapes() {
		shapeQueue_.Reset(GameConfig::kShuffleShapes);
		shapeCooldownTimer_ = 0.0f;
	}

	// 種類から形を探す
	const BreakShape* Game::FindShape(BreakShapeType type) const {
		for (const BreakShape& shape : shapes_) {
			if (shape.GetType() == type) {
				return &shape;
			}
		}
		return nullptr;
	}

	// 今の形
	const BreakShape* Game::GetCurrentShape() {
		return FindShape(shapeQueue_.Peek(0));
	}

	// 今の形で、プレイヤーの前の床を崩す
	void Game::BreakFloor() {
		const Cell playerCell = current_->GetCellAt(player_->GetPosition());
		const Cell facing = player_->GetFacing();
		const BreakShape* shape = GetCurrentShape();
		if (shape == nullptr || shapeCooldownTimer_ > 0.0f) {
			return;
		}

		// 形のマスを崩す。崩せる床が1つもなければ、形は使わない
		shape->CollectCells(playerCell, facing, shapeCells_);
		const int32_t count = current_->ScheduleCellsCollapse(shapeCells_, GameConfig::kCollapseDelay);
		if (count == 0) {
			return;
		}
		shapeCooldownTimer_ = GameConfig::kShapeCooldown;
		shapeQueue_.Advance();
	}

	// 床に今の形の予告を出す
	void Game::UpdateShapePreview() {
		if (current_ == nullptr) {
			return;
		}
		// 戦闘中・戦闘開始待ちのときだけ出す
		const BreakShape* shape = GetCurrentShape();
		if (shape == nullptr || (phase_ != Phase::Waiting && phase_ != Phase::Battle)) {
			current_->ClearPreview();
			return;
		}
		// プレイヤーの前に今の形のマスを集めて、床を光らせる
		shape->CollectCells(current_->GetCellAt(player_->GetPosition()), player_->GetFacing(), shapeCells_);
		current_->SetPreview(shapeCells_, shape->GetColor(), GameConfig::kShapePreviewBlend);
	}

	// 画面の右はしに 今・次・その次 の形を出す (プレイヤーの向きに合わせて回す)
	void Game::UpdateNextShapePanel() {
		// 今・次・その次 の形を集めて、パネルに渡す
		std::array<const BreakShape*, NextShapePanel::kSlotCount> shapes{};
		for (int32_t i = 0; i < NextShapePanel::kSlotCount; ++i) {
			shapes[static_cast<size_t>(i)] = FindShape(shapeQueue_.Peek(i));
		}
		nextShapePanel_.Update(*camera_, shapes, player_->GetFacing());
	}

	// =====================================================================
	// 床・配置
	// =====================================================================
	// index 層目の床を作る (CSV から読む)
	std::unique_ptr<Floor> Game::CreateFloor(int32_t index) const {
		// layer1.csv 〜 layer5.csv を順に使う (6層目からは1層目に戻る)
		const int32_t fileNumber = index % GameConfig::kLayerFileCount + 1;
		const std::string path = "Resources/ApplicationResources/Stage/layer" + std::to_string(fileNumber) + ".csv";
		const float height = -static_cast<float>(index) * GameConfig::kFloorSpacing;

		// 床を作って読み込む
		std::unique_ptr<Floor> floor = std::make_unique<Floor>();
		floor->Initialize(path, height, index == layer_);
		return floor;
	}

	// 床を元に戻して、最初の位置からやり直す (方向の入力待ちになる)
	void Game::ResetFloor(bool showBelow) {
		// 床を戻す → 下の層がなければ作る → キャラを置く → カメラを合わせる
		separationNeedsCheck_ = false;
		current_->Reset();
		if (showBelow && below_ == nullptr) {
			below_ = CreateFloor(layer_ + 1);
		}
		PlaceActors(false);
		camera_->SetFloorHeight(current_->GetHeight());
		invincible_ = 0.0f;
		UpdatePlayerVisibility();
		WaitForMovement();
	}

	// プレイヤーとボスを最初の位置に置く
	void Game::PlaceActors(bool keepBossLanding) {
		player_->Place(GetSpawnPosition(*current_, GameConfig::kPlayerSpawnX, GameConfig::kPlayerSpawnZ));
		isTouchingBoss_ = false;
		boss_->SetActive(true);

		// ボスの位置 (keepBossLanding なら、落ちてきた場所のまま)
		Vector3 bossPosition = boss_->GetPosition();
		if (!keepBossLanding) {
			bossPosition = GetSpawnPosition(*current_, GameConfig::kBossSpawnX, GameConfig::kBossSpawnZ);
		}
		boss_->Place(*current_, bossPosition);
	}

	// 1層目のマスで決めた最初の位置を、この層のマスに直す
	Vector3 Game::GetSpawnPosition(const Floor& floor, int32_t firstLayerX, int32_t firstLayerZ) const {
		// 1層目のマスで指定した場所を、この層のマスの数に合わせる
		int32_t x = static_cast<int32_t>(std::floor((static_cast<float>(firstLayerX) + 0.5f) * static_cast<float>(floor.GetWidth()) / static_cast<float>(GameConfig::kFirstLayerWidth)));
		int32_t z = static_cast<int32_t>(std::floor((static_cast<float>(firstLayerZ) + 0.5f) * static_cast<float>(floor.GetDepth()) / static_cast<float>(GameConfig::kFirstLayerDepth)));
		// 床の外に出ないようにする
		x = std::clamp(x, 0, floor.GetWidth() - 1);
		z = std::clamp(z, 0, floor.GetDepth() - 1);
		return floor.GetCellCenter({ x, z });
	}

	// =====================================================================
	// 当たり・ダメージ
	// =====================================================================
	// プレイヤーがボスに触れたか調べて、はじく
	void Game::CheckBossContact(bool isJumpImpact) {
		// ボスの体 (箱) とプレイヤーの体 (円柱) が重なったか。ボスの向きに合わせて調べる
		const Vector3 local = boss_->ToLocal(player_->GetPosition());
		const float gapX = std::max(0.0f, std::abs(local.x) - GameConfig::kBossContactSizeX * 0.5f);
		const float gapZ = std::max(0.0f, std::abs(local.z) - GameConfig::kBossContactSizeZ * 0.5f);
		const float radius = GameConfig::kPlayerContactRadius;
		bool isTouching = false;
		if (local.y <= GameConfig::kBossContactSizeY && local.y + GameConfig::kPlayerContactHeight >= 0.0f && gapX * gapX + gapZ * gapZ <= radius * radius) {
			isTouching = true;
		}

		// 触れた瞬間だけはじく (ダメージはない)
		const bool isNewTouch = isTouching && !isTouchingBoss_;
		isTouchingBoss_ = isTouching;
		if (!isNewTouch || player_->IsKnockedBack()) {
			return;
		}

		// ボスからプレイヤーへの向き (重なりすぎて向きが取れないときは、あとで別の向きを使う)
		Vector3 away = player_->GetPosition() - boss_->GetPosition();
		away.y = 0.0f;
		const bool isTooClose = away.x * away.x + away.z * away.z < 0.0001f;

		// 跳んでぶつかった: プレイヤーもボスも、おたがい反対へはじかれる
		if (isJumpImpact) {
			if (isTooClose) {
				away = player_->GetJumpOrigin() - current_->GetCellCenter(player_->GetJumpDestination());
			}
			const Cell direction = GridDirection::FromVector(away);
			ApplyContactKnockback(away);
			boss_->KnockBack(*current_, -direction, GameConfig::kContactPushDuration);
			separationNeedsCheck_ = true;
			return;
		}

		// 歩いて (立っていて) 触れた: プレイヤーだけはじかれる
		if (isTooClose) {
			away = boss_->GetForward();
		}
		ApplyContactKnockback(away);
	}

	// ボスに触れたときのはじき (2マス分。床のふちで止めない)
	void Game::ApplyContactKnockback(const Vector3& away) {
		// kContactKnockbackCells マス分を、kContactPushDuration 秒で進む速さ。床のふちで止めない
		const Vector3 offset = current_->StepOffset(GridDirection::FromVector(away));
		const float speed = Length(offset) * GameConfig::kContactKnockbackCells / GameConfig::kContactPushDuration;
		player_->ApplyKnockback(*current_, Normalize(offset), speed, GameConfig::kContactPushDuration, GameConfig::kContactLiftSpeed, true);
	}

	// おたがいにたどり着けなくなったら、床を元に戻す
	void Game::CheckForSeparatedFloor() {
		if (!separationNeedsCheck_ || isGameOver_ || phase_ != Phase::Battle || !player_->IsGrounded() || boss_->IsJumping()) {
			return;
		}
		// 2人とも床の上に立っているときだけ調べる
		const Cell playerCell = current_->GetCellAt(player_->GetPosition());
		const Cell bossCell = current_->GetCellAt(boss_->GetPosition());
		if (!current_->IsSolid(playerCell) || !current_->IsSolid(bossCell)) {
			return;
		}
		separationNeedsCheck_ = false;

		// おたがいにたどり着けなくなったら、床を元に戻してやり直し
		// (プレイヤーは穴を1つ飛び越えられるので、飛び越えあり で調べる)
		if (current_->CanReunite(playerCell, bossCell, true)) {
			return;
		}
		ResetFloor(true);
	}

	// 穴に落ちた: ハートを減らしてやり直し
	void Game::FailFall() {
		// ダメージ → 生きていれば床を戻して、少しの間無敵
		invincible_ = 0.0f;
		Damage(GameConfig::kFallDamage);
		if (isGameOver_) {
			return;
		}
		ResetFloor(true);
		invincible_ = GameConfig::kInvincibilityTime;
	}

	// プレイヤーのダメージ (無敵中は受けない)
	void Game::Damage(int32_t amount) {
		if (amount <= 0 || invincible_ > 0.0f) {
			return;
		}
		// ハートを減らす。0 ならゲームオーバー
		hearts_ = std::max(0, hearts_ - amount);
		if (hearts_ == 0) {
			boss_->Stop();
			isGameOver_ = true;
			return;
		}
		invincible_ = GameConfig::kInvincibilityTime;
	}

	// ボスのダメージ
	void Game::DamageBoss(int32_t amount) {
		bossHealth_ = std::max(0, bossHealth_ - amount);
	}

	// 無敵の間はプレイヤーを点滅させる
	void Game::UpdatePlayerVisibility() {
		// 無敵の間は点滅する
		bool isVisible = true;
		if (invincible_ > 0.0f) {
			const float wave = std::sin(invincible_ * GameConfig::kHitBlinkFrequency * std::numbers::pi_v<float> * 2.0f);
			isVisible = wave >= 0.0f;
		}
		player_->SetVisible(isVisible);
	}

	// =====================================================================
	// 場面の切り替え
	// =====================================================================
	// 方向の入力待ちにする
	void Game::WaitForMovement() {
		separationNeedsCheck_ = false;
		phase_ = Phase::Waiting;
	}

	// ボスの島が切り離された: 揺れ始める
	void Game::BeginEscape() {
		boss_->Stop();
		phase_ = Phase::Escape;
		timer_ = GameConfig::kEscapeDelay;
	}

	// 揺れる → 時間が来たらボスごと落とす
	void Game::AdvanceEscape(float deltaTime) {
		timer_ -= deltaTime;
		if (timer_ > 0.0f) {
			// 切り離された床が揺れる
			current_->Shake(GameConfig::kEscapeDelay - timer_, GameConfig::kShakeDistance, GameConfig::kShakeFrequency);
			return;
		}

		// 落ちる。プレイヤーが切り離された床・穴の上にいたら、一緒に落ちる (跳んでいる途中なら着地するマスで調べる)
		Cell playerCell = current_->GetCellAt(player_->GetPosition());
		if (player_->IsJumping()) {
			playerCell = player_->GetJumpDestination();
		}
		if (!current_->IsSolid(playerCell) || current_->IsIsolated(playerCell) || player_->GetPosition().y < current_->GetHeight()) {
			FailFall();
			return;
		}
		current_->Release();
		BeginBossDrop();
	}

	// ボスが下の層へ落ち始める
	void Game::BeginBossDrop() {
		boss_->Stop();
		// 落ち始めの位置と、プレイヤーの行き先 (下の層の最初の位置)
		dropOrigin_ = boss_->GetPosition();
		descentStart_ = player_->GetPosition();
		if (below_ == nullptr) {
			below_ = CreateFloor(layer_ + 1);
		}
		descentEnd_ = GetSpawnPosition(*below_, GameConfig::kPlayerSpawnX, GameConfig::kPlayerSpawnZ);
		isBossLanded_ = false;
		timer_ = 0.0f;
		phase_ = Phase::Descent;
	}

	// ボスが落ちる・プレイヤーとカメラも下の層へ
	void Game::AdvanceDescent(float deltaTime) {
		timer_ += deltaTime;
		// 進み具合 0 〜 1 (smooth はゆっくり始まって、ゆっくり止まる)
		const float t = std::clamp(timer_ / GameConfig::kDescentDuration, 0.0f, 1.0f);
		const float smooth = t * t * (3.0f - 2.0f * t);

		// プレイヤー: 跳ねながら下の層の最初の位置へ (マスの線にそって)
		Vector3 position = Floor::AlongGrid(descentStart_, descentEnd_, smooth);
		position.y = LerpFloat(descentStart_.y, descentEnd_.y, t * t * t) + 4.0f * GameConfig::kDescentHopHeight * t * (1.0f - t);
		player_->SetPosition(position);

		// カメラも下の層へ
		camera_->SetFloorHeight(LerpFloat(current_->GetHeight(), below_->GetHeight(), smooth));

		// ボス: 切り離された床と一緒に下の層まで落ちる
		if (!isBossLanded_) {
			const float drop = std::clamp(timer_ / GameConfig::kBossDropDuration, 0.0f, 1.0f);
			const float dropEased = drop * drop;
			current_->AnimateDrop(GameConfig::kFloorSpacing * dropEased);
			boss_->SetPosition({ dropOrigin_.x, LerpFloat(dropOrigin_.y, below_->GetHeight(), dropEased), dropOrigin_.z });
			if (drop >= 1.0f) {
				isBossLanded_ = true;
				current_->FinishDrop();
				DamageBoss(GameConfig::kBossFallDamage);
			}
		}
		// ボスもプレイヤーも着くまで待つ
		if (!isBossLanded_ || t < 1.0f) {
			return;
		}

		// 下の層に着いた: 層を1つ進める
		current_->ClearPreview();
		upper_ = std::move(current_);
		current_ = std::move(below_);
		current_->ShowAsCurrentFloor();
		++layer_;
		below_ = CreateFloor(layer_ + 1);
		hearts_ = std::min(GameConfig::kMaxHearts, hearts_ + GameConfig::kDescentHeal);
		PlaceActors(true);
		invincible_ = 0.0f;
		timer_ = 0.0f;
		phase_ = Phase::LandingImpact;
	}

	// 上の層に残った足場を、ボスへ飛ばし始める
	void Game::BeginTileBarrage() {
		// 飛ばす足場を用意する。1つもなければすぐ終わり
		remainingTiles_ = 0;
		if (upper_ != nullptr) {
			remainingTiles_ = upper_->PrepareBarrage();
		}
		tileDamageApplied_ = 0;
		timer_ = 0.0f;
		phase_ = Phase::TileBarrage;
		if (remainingTiles_ == 0) {
			FinishTileBarrage();
		}
	}

	// 足場を飛ばして、届いた分だけダメージ
	void Game::AdvanceTileBarrage(float deltaTime) {
		timer_ += deltaTime;

		// 上の層に残った足場が、ボスへ飛んでいく
		const Vector3 target = boss_->GetPosition() + Vector3{ 0.0f, 0.75f, 0.0f };
		int64_t damageUnits = 0;
		const int32_t arrived = upper_->AnimateBarrage(target, timer_, GameConfig::kTileFlightDuration, GameConfig::kTileLaunchWindow, damageUnits);

		// 届いた分だけダメージ (上限あり)
		const int64_t totalDamage = std::min(damageUnits * GameConfig::kDamagePerRemainingTile, static_cast<int64_t>(GameConfig::kMaxTileDamage));
		const int32_t damage = static_cast<int32_t>(totalDamage);
		if (damage > tileDamageApplied_) {
			DamageBoss(damage - tileDamageApplied_);
			tileDamageApplied_ = damage;
		}
		if (arrived == remainingTiles_) {
			FinishTileBarrage();
		}
	}

	// 飛ばし終わった: クリアか、次の戦闘待ちへ
	void Game::FinishTileBarrage() {
		// 上の層を消す。ボスの HP が 0 ならクリア
		upper_.reset();
		if (bossHealth_ == 0) {
			boss_->SetActive(false);
			isCleared_ = true;
			return;
		}
		WaitForMovement();
	}
}
