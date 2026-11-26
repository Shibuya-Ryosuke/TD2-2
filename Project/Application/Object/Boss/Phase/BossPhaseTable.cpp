#include "BossPhaseTable.h"
#include "../Boss.h"
#include "../../../Config/GameConfig.h"
#include <RyoEngine.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>

#ifdef NRELEASE
#include <imgui.h>
#endif

#pragma warning(disable: 4866)
#include <json.hpp>

using namespace RyoEngine;

namespace App {
	namespace {
		using json = nlohmann::json;

		//攻撃の名前から種類にする。知らない名前ならfallback
		BossAttackType AttackFromId(const std::string& id, BossAttackType fallback) {
			for (int32_t i = 0; i < kBossAttackTypeCount; ++i) {
				if (id == kBossAttackIds[i]) {
					return static_cast<BossAttackType>(i);
				}
			}
			return fallback;
		}

		//形態1つをJSONにする
		json PhaseToJson(const BossPhaseData& phase) {
			json j;
			j["name"] = phase.name;
			j["maxHp"] = phase.maxHp;
			j["moveSpeed"] = phase.moveSpeed;
			j["canJump"] = phase.canJump;
			j["evacuateSmallIsland"] = phase.evacuateSmallIsland;
			j["thinkInterval"] = phase.thinkInterval;
			j["randomness"] = phase.randomness;
			j["attacks"] = json::array();
			for (int32_t i = 0; i < kBossAttackTypeCount; ++i) {
				json attack;
				attack["type"] = kBossAttackIds[i];
				attack["enabled"] = phase.enabledAttacks[static_cast<size_t>(i)];
				attack["weight"] = phase.attackWeights[static_cast<size_t>(i)];
				j["attacks"].push_back(attack);
			}
			j["triggers"] = json::array();
			for (const BossHpTrigger& trigger : phase.triggers) {
				json t;
				t["hpRate"] = trigger.hpRate;
				t["action"] = kBossAttackIds[static_cast<int32_t>(trigger.action)];
				j["triggers"].push_back(t);
			}
			return j;
		}

		//JSONから形態1つを読む。書いていない値は初期値のまま
		BossPhaseData PhaseFromJson(const json& j) {
			BossPhaseData phase{};
			phase.name = j.value("name", phase.name);
			phase.maxHp = j.value("maxHp", phase.maxHp);
			phase.moveSpeed = j.value("moveSpeed", phase.moveSpeed);
			phase.canJump = j.value("canJump", phase.canJump);
			phase.evacuateSmallIsland = j.value("evacuateSmallIsland", phase.evacuateSmallIsland);
			phase.thinkInterval = j.value("thinkInterval", phase.thinkInterval);
			phase.randomness = j.value("randomness", phase.randomness);
			if (j.contains("attacks") && j["attacks"].is_array()) {
				for (const json& attack : j["attacks"]) {
					const int32_t index = static_cast<int32_t>(AttackFromId(attack.value("type", std::string{}), BossAttackType::Count));
					if (index >= kBossAttackTypeCount) {
						continue;
					}
					phase.enabledAttacks[static_cast<size_t>(index)] = attack.value("enabled", true);
					phase.attackWeights[static_cast<size_t>(index)] = attack.value("weight", 1.0f);
				}
			}
			if (j.contains("triggers") && j["triggers"].is_array()) {
				for (const json& t : j["triggers"]) {
					BossHpTrigger trigger{};
					trigger.hpRate = t.value("hpRate", trigger.hpRate);
					trigger.action = AttackFromId(t.value("action", std::string{}), trigger.action);
					phase.triggers.push_back(trigger);
				}
			}
			return phase;
		}
	}

	BossPhaseTable& BossPhaseTable::GetInstance() {
		static BossPhaseTable instance;
		//形態がないとボスが動けないので、いつも1つは用意する
		if (instance.phases_.empty()) {
			instance.CreateDefault();
		}
		return instance;
	}

	//保存するフォルダを決める
	void BossPhaseTable::SetFolderPath(const std::string& folderPath) {
		GetInstance().folderPath_ = folderPath;
	}

	std::string BossPhaseTable::GetFullPath() const {
		return folderPath_ + kFileName;
	}

	//ファイルがないときの形態を1つ作る
	void BossPhaseTable::CreateDefault() {
		phases_.clear();
		BossPhaseData phase{};
		phase.name = "第1形態";
		phase.maxHp = GameConfig::kBossMaxHealth;
		phases_.push_back(phase);
		selected_ = 0;
	}

	//ファイルから読む。読めなければ形態を1つだけ作る
	bool BossPhaseTable::Load() {
		BossPhaseTable& instance = GetInstance();
		std::ifstream file(instance.GetFullPath());
		if (!file.is_open()) {
			instance.CreateDefault();
			instance.isDirty_ = false;
			return false;
		}
		try {
			json j;
			file >> j;
			std::vector<BossPhaseData> phases;
			if (j.contains("phases") && j["phases"].is_array()) {
				for (const json& phase : j["phases"]) {
					phases.push_back(PhaseFromJson(phase));
				}
			}
			if (phases.empty()) {
				instance.CreateDefault();
			} else {
				instance.phases_ = std::move(phases);
			}
		} catch (const json::exception& e) {
			Logger::LogError("[BossPhaseTable] Load: 読めませんでした: {}\n", e.what());
			instance.CreateDefault();
			instance.isDirty_ = false;
			return false;
		}
		instance.selected_ = std::clamp(instance.selected_, 0, static_cast<int32_t>(instance.phases_.size()) - 1);
		instance.isDirty_ = false;
		return true;
	}

	//ファイルに書く
	void BossPhaseTable::Save() {
		BossPhaseTable& instance = GetInstance();
		std::error_code error;
		std::filesystem::create_directories(instance.folderPath_, error);

		json j;
		j["phases"] = json::array();
		for (const BossPhaseData& phase : instance.phases_) {
			j["phases"].push_back(PhaseToJson(phase));
		}
		std::ofstream file(instance.GetFullPath());
		if (!file.is_open()) {
			Logger::LogError("[BossPhaseTable] Save: ファイルを開けませんでした: {}\n", instance.GetFullPath());
			return;
		}
		file << j.dump(4);
		instance.isDirty_ = false;
	}

	const std::vector<BossPhaseData>& BossPhaseTable::GetPhases() {
		return GetInstance().phases_;
	}

	// =====================================================================
	// エディタ
	// =====================================================================

	//エディタの画面を出す
	void BossPhaseTable::DrawEditor(Boss* boss) {
#ifdef NRELEASE
		BossPhaseTable& instance = GetInstance();
		//保存していない変更があればタイトルに*を付ける。###の後ろで窓を見分ける
		const char* title = instance.isDirty_ ? "ボス形態エディタ *###BossPhaseEditor" : "ボス形態エディタ###BossPhaseEditor";
		if (!ImGui::Begin(title)) {
			ImGui::End();
			return;
		}

		if (ImGui::Button("保存")) {
			Save();
		}
		ImGui::SameLine();
		if (ImGui::Button("ファイルから読み直す")) {
			Load();
		}
		ImGui::Separator();

		//左に一覧、右に選んだ形態の中身
		ImGui::BeginChild("PhaseList", ImVec2(220.0f, 0.0f), ImGuiChildFlags_Border);
		instance.DrawPhaseList();
		ImGui::EndChild();
		//形態が消されたりしても、ボスの番号とHPが範囲に収まるようにする
		if (boss != nullptr) {
			boss->GetPhaseController().Sync();
		}
		ImGui::SameLine();
		ImGui::BeginChild("PhaseDetail", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border);
		instance.DrawPhaseDetail(instance.phases_[static_cast<size_t>(instance.selected_)]);
		if (boss != nullptr) {
			instance.DrawDebug(*boss);
		}
		ImGui::EndChild();

		ImGui::End();

		//HPや形態の中身が変わっても収まるように、もう一度合わせる
		if (boss != nullptr) {
			boss->GetPhaseController().Sync();
		}
#else
		(void)boss;
#endif
	}

	//形態の一覧と、追加・複製・削除・並べ替え
	void BossPhaseTable::DrawPhaseList() {
#ifdef NRELEASE
		for (size_t i = 0; i < phases_.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			const std::string label = std::to_string(i + 1) + ": " + phases_[i].name + "  HP " + std::to_string(phases_[i].maxHp);
			if (ImGui::Selectable(label.c_str(), selected_ == static_cast<int32_t>(i))) {
				selected_ = static_cast<int32_t>(i);
			}
			ImGui::PopID();
		}
		ImGui::Separator();

		const size_t selected = static_cast<size_t>(selected_);
		//追加は一番後ろに初期値の形態を足す
		if (ImGui::Button("追加")) {
			BossPhaseData phase{};
			phase.name = "第" + std::to_string(phases_.size() + 1) + "形態";
			phases_.push_back(phase);
			selected_ = static_cast<int32_t>(phases_.size()) - 1;
			isDirty_ = true;
		}
		ImGui::SameLine();
		//複製は選んでいる形態のすぐ後ろに入れる
		if (ImGui::Button("複製")) {
			BossPhaseData copy = phases_[selected];
			copy.name += " のコピー";
			phases_.insert(phases_.begin() + static_cast<std::ptrdiff_t>(selected) + 1, copy);
			selected_ += 1;
			isDirty_ = true;
		}
		//最後の1つは消せない
		ImGui::BeginDisabled(phases_.size() <= 1);
		if (ImGui::Button("削除")) {
			phases_.erase(phases_.begin() + static_cast<std::ptrdiff_t>(selected));
			selected_ = std::min(selected_, static_cast<int32_t>(phases_.size()) - 1);
			isDirty_ = true;
		}
		ImGui::EndDisabled();

		//上下に入れかえる
		ImGui::BeginDisabled(selected == 0);
		if (ImGui::ArrowButton("Up", ImGuiDir_Up)) {
			std::swap(phases_[selected], phases_[selected - 1]);
			selected_ -= 1;
			isDirty_ = true;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(selected + 1 >= phases_.size());
		if (ImGui::ArrowButton("Down", ImGuiDir_Down)) {
			std::swap(phases_[selected], phases_[selected + 1]);
			selected_ += 1;
			isDirty_ = true;
		}
		ImGui::EndDisabled();
#endif
	}

	//選んだ形態の中身
	void BossPhaseTable::DrawPhaseDetail(BossPhaseData& phase) {
#ifdef NRELEASE
		bool changed = false;

		if (ImGui::CollapsingHeader("基本", ImGuiTreeNodeFlags_DefaultOpen)) {
			//名前はstd::stringなので、いったん文字の配列に写して編集する
			char name[64] = {};
			strncpy_s(name, phase.name.c_str(), sizeof(name) - 1);
			if (ImGui::InputText("名前", name, sizeof(name))) {
				phase.name = name;
				changed = true;
			}
			changed |= ImGui::DragInt("HP", &phase.maxHp, 10.0f, 1, 100000);
		}

		if (ImGui::CollapsingHeader("移動・判断", ImGuiTreeNodeFlags_DefaultOpen)) {
			changed |= ImGui::DragFloat("移動の速さ", &phase.moveSpeed, 0.01f, 0.0f, 20.0f);
			changed |= ImGui::Checkbox("1マス飛び越え", &phase.canJump);
			changed |= ImGui::Checkbox("先回り避難", &phase.evacuateSmallIsland);
			changed |= ImGui::DragFloat("考える間隔の秒数", &phase.thinkInterval, 0.01f, 0.0f, 5.0f);
			changed |= ImGui::DragFloat("迷い", &phase.randomness, 0.01f, 0.0f, 10.0f);
		}

		if (ImGui::CollapsingHeader("攻撃", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (int32_t i = 0; i < kBossAttackTypeCount; ++i) {
				ImGui::PushID(i);
				bool enabled = phase.enabledAttacks[static_cast<size_t>(i)];
				if (ImGui::Checkbox(kBossAttackNames[i], &enabled)) {
					phase.enabledAttacks[static_cast<size_t>(i)] = enabled;
					changed = true;
				}
				ImGui::SameLine(160.0f);
				ImGui::SetNextItemWidth(100.0f);
				changed |= ImGui::DragFloat("重み", &phase.attackWeights[static_cast<size_t>(i)], 0.01f, 0.0f, 10.0f);
				ImGui::PopID();
			}
		}

		if (ImGui::CollapsingHeader("HPトリガー", ImGuiTreeNodeFlags_DefaultOpen)) {
			DrawTriggers(phase);
		}

		if (changed) {
			isDirty_ = true;
		}
#else
		(void)phase;
#endif
	}

	//HPトリガーの一覧と追加・削除
	void BossPhaseTable::DrawTriggers(BossPhaseData& phase) {
#ifdef NRELEASE
		//消すのはループが終わってから
		int32_t removeIndex = -1;
		for (size_t i = 0; i < phase.triggers.size(); ++i) {
			BossHpTrigger& trigger = phase.triggers[i];
			ImGui::PushID(static_cast<int>(i) + 1000);

			//割合は%で見せる
			float percent = trigger.hpRate * 100.0f;
			ImGui::SetNextItemWidth(120.0f);
			if (ImGui::SliderFloat("%以下で", &percent, 0.0f, 100.0f, "%.0f")) {
				trigger.hpRate = percent / 100.0f;
				isDirty_ = true;
			}
			ImGui::SameLine();
			int32_t action = static_cast<int32_t>(trigger.action);
			ImGui::SetNextItemWidth(120.0f);
			if (ImGui::Combo("##Action", &action, kBossAttackNames, kBossAttackTypeCount)) {
				trigger.action = static_cast<BossAttackType>(action);
				isDirty_ = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("×")) {
				removeIndex = static_cast<int32_t>(i);
			}
			ImGui::PopID();
		}
		if (removeIndex >= 0) {
			phase.triggers.erase(phase.triggers.begin() + removeIndex);
			isDirty_ = true;
		}
		if (ImGui::Button("トリガーを追加")) {
			phase.triggers.push_back(BossHpTrigger{});
			isDirty_ = true;
		}
#else
		(void)phase;
#endif
	}

	//今のボスの形態とHP、切り替えのボタン
	void BossPhaseTable::DrawDebug(Boss& boss) {
#ifdef NRELEASE
		if (!ImGui::CollapsingHeader("デバッグ", ImGuiTreeNodeFlags_DefaultOpen)) {
			return;
		}
		const BossPhaseController& controller = boss.GetPhaseController();
		ImGui::Text("今の形態: %d / %d  %s", controller.GetIndex() + 1, static_cast<int>(phases_.size()), controller.GetPhase().name.c_str());
		ImGui::Text("残りHP: %d / %d", controller.GetHp(), controller.GetMaxHp());
		if (controller.IsDefeated()) {
			ImGui::Text("撃破済み");
		}
		if (ImGui::Button("選んでいる形態にする")) {
			boss.DebugForcePhase(selected_);
		}
		ImGui::SameLine();
		if (ImGui::Button("今の形態のHPを0にする")) {
			boss.Damage(controller.GetHp());
		}
#else
		(void)boss;
#endif
	}
}
