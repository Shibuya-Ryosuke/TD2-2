// [アイテム] アイテムは一旦使わない (チームで相談中)。コメントアウトしている
// (使うときは、このファイルの行頭の "// " を外す)
// #include "ItemManager.h"
// #include "GameConfig.h"
// #include "../Object/Player.h"
// #include "../Stage/Floor.h"
// #include <algorithm>
// #include <vector>

// using namespace RyoEngine;

// namespace App
// {
	// namespace
	// {
		// // 十字の4つの向き (奥・右・手前・左)
		// constexpr Cell kCrossDirections[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	// }

	// // 乱数を毎回ちがう値で始める
	// ItemManager::ItemManager() : random_(std::random_device{}())
	// {
	// }

	// // モデルを作る
	// void ItemManager::Initialize()
	// {
		// item_.Initialize();
		// ResetForRetry();
	// }

	// // やり直し
	// void ItemManager::ResetForRetry()
	// {
		// item_.Remove();
		// isSpawnedThisLayer_ = false;
		// hasBomb_ = false;
	// }

	// // 下の層へ
	// void ItemManager::ResetForNewLayer()
	// {
		// item_.Remove();
		// isSpawnedThisLayer_ = false;
	// }

	// // 置いてあるアイテムだけ消す
	// void ItemManager::ClearFieldItem()
	// {
		// item_.Remove();
	// }

	// // 出す・動かす・取る
	// void ItemManager::Update(const Floor& floor, bool canSpawn, int32_t hearts, Player& player, const Vector3& bossPosition, float deltaTime)
	// {
		// // 戦闘中・HP が少ない・この層でまだ出ていない なら降らせる
		// if (canSpawn && hearts <= GameConfig::kItemSpawnHearts && !isSpawnedThisLayer_)
		// {
			// if (TrySpawn(floor, floor.GetCellAt(player.GetPosition()), floor.GetCellAt(bossPosition)))
			// {
				// isSpawnedThisLayer_ = true;
			// }
		// }

		// // 降ってくる・回る・足場と一緒に落ちる → 触れたら取る
		// item_.Update(floor, deltaTime);
		// PickUp(player);
	// }

	// // 出せるマスからランダムに1つ選んで降らせる
	// bool ItemManager::TrySpawn(const Floor& floor, const Cell& playerCell, const Cell& bossCell)
	// {
		// // 出せるマス: 床があって、崩れる予定・切り離しでなく、プレイヤー・ボスのいるマスでもない所
		// std::vector<Cell> candidates;
		// for (int32_t z = 0; z < floor.GetDepth(); ++z)
		// {
			// for (int32_t x = 0; x < floor.GetWidth(); ++x)
			// {
				// const Cell cell = { x, z };
				// if (!floor.IsSolid(cell) || floor.IsIsolated(cell) || floor.IsCollapseScheduled(cell))
				// {
					// continue;
				// }
				// if (cell == playerCell || cell == bossCell)
				// {
					// continue;
				// }
				// candidates.push_back(cell);
			// }
		// }
		// if (candidates.empty())
		// {
			// return false; // 出せる所がない (次のフレームでまた探す)
		// }

		// // 場所と種類 (半々) をランダムに決める
		// std::uniform_int_distribution<size_t> cellDistribution(0, candidates.size() - 1);
		// std::uniform_int_distribution<int32_t> typeDistribution(0, static_cast<int32_t>(ItemType::Count) - 1);
		// const Cell cell = candidates[cellDistribution(random_)];
		// const ItemType type = static_cast<ItemType>(typeDistribution(random_));
		// item_.Spawn(type, floor, cell);
		// return true;
	// }

	// // プレイヤーが触れたら取る
	// void ItemManager::PickUp(Player& player)
	// {
		// if (!item_.CanPickUp() || !item_.IsTouching(player.GetPosition(), GameConfig::kPlayerSize))
		// {
			// return;
		// }

		// // 十字爆弾: ストックできないので、持っているときは取れない
		// if (item_.GetType() == ItemType::CrossBomb)
		// {
			// if (hasBomb_)
			// {
				// return;
			// }
			// hasBomb_ = true;
		// }

		// // 2マス乗り越え: 取った瞬間から効果が始まる
		// if (item_.GetType() == ItemType::JumpBoost)
		// {
			// player.SetJumpBoost(true);
		// }

		// // 取ったのでステージから消す
		// item_.Remove();
	// }

	// // 十字爆弾を使う
	// bool ItemManager::TryUseBomb(Floor& floor, const Cell& center)
	// {
		// if (!hasBomb_)
		// {
			// return false;
		// }
		// hasBomb_ = false;

		// // 真ん中 (プレイヤーの足元) から崩す
		// floor.ScheduleCollapse(center, GameConfig::kCollapseDelay);

		// // 1マスずつ外へ広げる。外ほど遅れて崩れる (パパパッと)
		// const int32_t maxDistance = std::max(floor.GetWidth(), floor.GetDepth());
		// for (int32_t distance = 1; distance <= maxDistance; ++distance)
		// {
			// const float delay = GameConfig::kCollapseDelay + GameConfig::kBombStepDelay * static_cast<float>(distance);
			// for (const Cell& direction : kCrossDirections)
			// {
				// floor.ScheduleCollapse(center + direction * distance, delay);
			// }
		// }
		// return true;
	// }

	// void ItemManager::UpdateVisuals()
	// {
		// item_.UpdateVisuals();
	// }

	// void ItemManager::Draw()
	// {
		// item_.Draw();
	// }
// }
