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

		//コンボ1つをJSONにする
		json ComboToJson(const BossCombo& combo) {
			json j;
			j["name"] = combo.name;
			j["steps"] = json::array();
			for (const BossComboStep& step : combo.steps) {
				json s;
				s["attack"] = kBossAttackIds[static_cast<int32_t>(step.attack)];
				s["repeat"] = step.repeat;
				s["interval"] = step.interval;
				j["steps"].push_back(s);
			}
			return j;
		}

		//JSONからコンボ1つを読む
		BossCombo ComboFromJson(const json& j) {
			BossCombo combo{};
			combo.name = j.value("name", combo.name);
			if (j.contains("steps") && j["steps"].is_array()) {
				for (const json& s : j["steps"]) {
					BossComboStep step{};
					step.attack = AttackFromId(s.value("attack", std::string{}), step.attack);
					step.repeat = s.value("repeat", step.repeat);
					step.interval = s.value("interval", step.interval);
					combo.steps.push_back(step);
				}
			}
			return combo;
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
			j["combos"] = json::array();
			for (const BossPhaseComboEntry& entry : phase.combos) {
				json c;
				c["combo"] = entry.comboName;
				c["weight"] = entry.weight;
				j["combos"].push_back(c);
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
			if (j.contains("combos") && j["combos"].is_array()) {
				for (const json& c : j["combos"]) {
					BossPhaseComboEntry entry{};
					entry.comboName = c.value("combo", std::string{});
					entry.weight = c.value("weight", entry.weight);
					phase.combos.push_back(entry);
				}
			}
			return phase;
		}

#ifdef NRELEASE
		//std::stringをImGuiで編集する。変わったらtrue
		bool InputString(const char* label, std::string& text) {
			char buffer[64] = {};
			strncpy_s(buffer, text.c_str(), sizeof(buffer) - 1);
			if (ImGui::InputText(label, buffer, sizeof(buffer))) {
				text = buffer;
				return true;
			}
			return false;
		}
#endif
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

	//ファイルがないときの形態とコンボを作る
	//攻撃1つだけのコンボを攻撃の数だけ作り、第1形態で全部使う
	void BossPhaseTable::CreateDefault() {
		combos_.clear();
		phases_.clear();

		BossPhaseData phase{};
		phase.name = "第1形態";
		phase.maxHp = GameConfig::kBossMaxHealth;
		for (int32_t i = 0; i < kBossAttackTypeCount; ++i) {
			BossCombo combo{};
			combo.name = kBossAttackNames[i];
			combo.steps.push_back(BossComboStep{ static_cast<BossAttackType>(i), 1, 0.0f });
			combos_.push_back(combo);
			phase.combos.push_back(BossPhaseComboEntry{ combo.name, 1.0f });
		}
		phases_.push_back(phase);
		selected_ = 0;
		selectedCombo_ = 0;
	}

	//ファイルから読む。読めなければ初期の形態とコンボを作る
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
			std::vector<BossCombo> combos;
			if (j.contains("combos") && j["combos"].is_array()) {
				for (const json& combo : j["combos"]) {
					combos.push_back(ComboFromJson(combo));
				}
			}
			std::vector<BossPhaseData> phases;
			if (j.contains("phases") && j["phases"].is_array()) {
				for (const json& phase : j["phases"]) {
					phases.push_back(PhaseFromJson(phase));
				}
			}
			if (phases.empty()) {
				instance.CreateDefault();
			} else {
				instance.combos_ = std::move(combos);
				instance.phases_ = std::move(phases);
			}
		} catch (const json::exception& e) {
			Logger::LogError("[BossPhaseTable] Load: 読めませんでした: {}\n", e.what());
			instance.CreateDefault();
			instance.isDirty_ = false;
			return false;
		}
		instance.selected_ = std::clamp(instance.selected_, 0, static_cast<int32_t>(instance.phases_.size()) - 1);
		instance.selectedCombo_ = std::clamp(instance.selectedCombo_, 0, std::max(0, static_cast<int32_t>(instance.combos_.size()) - 1));
		instance.isDirty_ = false;
		return true;
	}

	//ファイルに書く
	void BossPhaseTable::Save() {
		BossPhaseTable& instance = GetInstance();
		std::error_code error;
		std::filesystem::create_directories(instance.folderPath_, error);

		json j;
		j["combos"] = json::array();
		for (const BossCombo& combo : instance.combos_) {
			j["combos"].push_back(ComboToJson(combo));
		}
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

	const std::vector<BossCombo>& BossPhaseTable::GetCombos() {
		return GetInstance().combos_;
	}

	//名前でコンボを探す
	const BossCombo* BossPhaseTable::FindCombo(const std::string& name) {
		for (const BossCombo& combo : GetInstance().combos_) {
			if (combo.name == name) {
				return &combo;
			}
		}
		return nullptr;
	}

	//ほかのコンボとかぶらない名前を作る。かぶったら後ろに番号を付ける
	std::string BossPhaseTable::MakeUniqueComboName(const std::string& base) const {
		std::string name = base;
		int32_t number = 2;
		while (std::any_of(combos_.begin(), combos_.end(), [&](const BossCombo& combo) { return combo.name == name; })) {
			name = base + " " + std::to_string(number);
			++number;
		}
		return name;
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
			//形態が減っていたらボスの番号を収める
			if (boss != nullptr) {
				boss->GetPhaseController().Sync();
			}
		}
		ImGui::Separator();

		if (ImGui::BeginTabBar("BossPhaseTabs")) {
			if (ImGui::BeginTabItem("形態")) {
				instance.DrawPhaseTab(boss);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("コンボ")) {
				instance.DrawComboTab(boss);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();

		//HPや形態の中身が変わっても収まるように、もう一度合わせる
		if (boss != nullptr) {
			boss->GetPhaseController().Sync();
		}
#else
		(void)boss;
#endif
	}

	// ---------------------------------------------------------------------
	// 形態のタブ
	// ---------------------------------------------------------------------

	//左に形態の一覧、右に選んだ形態の中身
	void BossPhaseTable::DrawPhaseTab(Boss* boss) {
#ifdef NRELEASE
		ImGui::BeginChild("PhaseList", ImVec2(220.0f, 0.0f), ImGuiChildFlags_Border);
		DrawPhaseList();
		ImGui::EndChild();
		//形態が消されたりしても、ボスの番号とHPが範囲に収まるようにする
		if (boss != nullptr) {
			boss->GetPhaseController().Sync();
		}
		ImGui::SameLine();
		ImGui::BeginChild("PhaseDetail", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border);
		DrawPhaseDetail(phases_[static_cast<size_t>(selected_)]);
		if (boss != nullptr) {
			DrawPhaseDebug(*boss);
		}
		ImGui::EndChild();
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
			changed |= InputString("名前", phase.name);
			changed |= ImGui::DragInt("HP", &phase.maxHp, 10.0f, 1, 100000);
		}

		if (ImGui::CollapsingHeader("移動・判断", ImGuiTreeNodeFlags_DefaultOpen)) {
			changed |= ImGui::DragFloat("移動の速さ", &phase.moveSpeed, 0.01f, 0.0f, 20.0f);
			changed |= ImGui::Checkbox("1マス飛び越え", &phase.canJump);
			changed |= ImGui::Checkbox("先回り避難", &phase.evacuateSmallIsland);
			changed |= ImGui::DragFloat("考える間隔の秒数", &phase.thinkInterval, 0.01f, 0.0f, 5.0f);
			changed |= ImGui::DragFloat("迷い", &phase.randomness, 0.01f, 0.0f, 10.0f);
		}

		if (ImGui::CollapsingHeader("使うコンボ", ImGuiTreeNodeFlags_DefaultOpen)) {
			DrawPhaseCombos(phase);
		}

		if (changed) {
			isDirty_ = true;
		}
#else
		(void)phase;
#endif
	}

	//形態で使うコンボの一覧。コンボボックスで名前を選ぶ
	void BossPhaseTable::DrawPhaseCombos(BossPhaseData& phase) {
#ifdef NRELEASE
		//消すのはループが終わってから
		int32_t removeIndex = -1;
		for (size_t i = 0; i < phase.combos.size(); ++i) {
			BossPhaseComboEntry& entry = phase.combos[i];
			ImGui::PushID(static_cast<int>(i) + 1000);

			//名前のコンボがもうなければ、それと分かるように出す
			const bool exists = FindCombo(entry.comboName) != nullptr;
			const std::string preview = exists ? entry.comboName : "(なし) " + entry.comboName;
			ImGui::SetNextItemWidth(180.0f);
			if (ImGui::BeginCombo("##Combo", preview.c_str())) {
				for (const BossCombo& combo : combos_) {
					if (ImGui::Selectable(combo.name.c_str(), combo.name == entry.comboName)) {
						entry.comboName = combo.name;
						isDirty_ = true;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::DragFloat("重み", &entry.weight, 0.01f, 0.0f, 10.0f)) {
				isDirty_ = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("×")) {
				removeIndex = static_cast<int32_t>(i);
			}
			ImGui::PopID();
		}
		if (removeIndex >= 0) {
			phase.combos.erase(phase.combos.begin() + removeIndex);
			isDirty_ = true;
		}
		//コンボが1つもなければ足せない
		ImGui::BeginDisabled(combos_.empty());
		if (ImGui::Button("コンボを足す")) {
			phase.combos.push_back(BossPhaseComboEntry{ combos_.front().name, 1.0f });
			isDirty_ = true;
		}
		ImGui::EndDisabled();
#else
		(void)phase;
#endif
	}

	//今のボスの形態とHP、切り替えのボタン
	void BossPhaseTable::DrawPhaseDebug(Boss& boss) {
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

	// ---------------------------------------------------------------------
	// コンボのタブ
	// ---------------------------------------------------------------------

	//左にコンボの一覧、右に選んだコンボの段
	void BossPhaseTable::DrawComboTab(Boss* boss) {
#ifdef NRELEASE
		ImGui::BeginChild("ComboList", ImVec2(220.0f, 0.0f), ImGuiChildFlags_Border);
		DrawComboList();
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("ComboDetail", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border);
		if (combos_.empty()) {
			ImGui::Text("コンボがありません。左の「追加」で作ってください");
		} else {
			BossCombo& combo = combos_[static_cast<size_t>(selectedCombo_)];
			DrawComboDetail(combo);
			//試しにボスに出させる
			if (boss != nullptr) {
				ImGui::Separator();
				if (ImGui::Button("このコンボを今すぐ出す")) {
					boss->StartCombo(combo);
				}
				if (boss->IsComboActive()) {
					ImGui::SameLine();
					ImGui::Text("コンボ中");
				}
			}
		}
		ImGui::EndChild();
#else
		(void)boss;
#endif
	}

	//コンボの一覧と、追加・複製・削除・並べ替え
	void BossPhaseTable::DrawComboList() {
#ifdef NRELEASE
		for (size_t i = 0; i < combos_.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			if (ImGui::Selectable(combos_[i].name.c_str(), selectedCombo_ == static_cast<int32_t>(i))) {
				selectedCombo_ = static_cast<int32_t>(i);
			}
			ImGui::PopID();
		}
		ImGui::Separator();

		//追加は一番後ろに、突進1回だけのコンボを足す
		if (ImGui::Button("追加")) {
			BossCombo combo{};
			combo.name = MakeUniqueComboName("コンボ");
			combo.steps.push_back(BossComboStep{});
			combos_.push_back(combo);
			selectedCombo_ = static_cast<int32_t>(combos_.size()) - 1;
			isDirty_ = true;
		}
		if (combos_.empty()) {
			return;
		}
		const size_t selected = static_cast<size_t>(selectedCombo_);
		ImGui::SameLine();
		//複製は選んでいるコンボのすぐ後ろに入れる。名前はかぶらないようにする
		if (ImGui::Button("複製")) {
			BossCombo copy = combos_[selected];
			copy.name = MakeUniqueComboName(copy.name + " のコピー");
			combos_.insert(combos_.begin() + static_cast<std::ptrdiff_t>(selected) + 1, copy);
			selectedCombo_ += 1;
			isDirty_ = true;
		}
		//消したら、それを使っている形態の行も消す
		if (ImGui::Button("削除")) {
			const std::string name = combos_[selected].name;
			combos_.erase(combos_.begin() + static_cast<std::ptrdiff_t>(selected));
			for (BossPhaseData& phase : phases_) {
				std::erase_if(phase.combos, [&](const BossPhaseComboEntry& entry) { return entry.comboName == name; });
			}
			selectedCombo_ = std::max(0, std::min(selectedCombo_, static_cast<int32_t>(combos_.size()) - 1));
			isDirty_ = true;
			return;
		}

		//上下に入れかえる
		ImGui::BeginDisabled(selected == 0);
		if (ImGui::ArrowButton("Up", ImGuiDir_Up)) {
			std::swap(combos_[selected], combos_[selected - 1]);
			selectedCombo_ -= 1;
			isDirty_ = true;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(selected + 1 >= combos_.size());
		if (ImGui::ArrowButton("Down", ImGuiDir_Down)) {
			std::swap(combos_[selected], combos_[selected + 1]);
			selectedCombo_ += 1;
			isDirty_ = true;
		}
		ImGui::EndDisabled();
#endif
	}

	//選んだコンボの名前と段
	void BossPhaseTable::DrawComboDetail(BossCombo& combo) {
#ifdef NRELEASE
		//名前を変えたら、形態の中の参照も一緒に変える
		const std::string oldName = combo.name;
		if (InputString("名前", combo.name)) {
			for (BossPhaseData& phase : phases_) {
				for (BossPhaseComboEntry& entry : phase.combos) {
					if (entry.comboName == oldName) {
						entry.comboName = combo.name;
					}
				}
			}
			isDirty_ = true;
		}
		//名前がかぶると形態から選べなくなるので知らせる
		const auto sameNameCount = std::count_if(combos_.begin(), combos_.end(), [&](const BossCombo& other) { return other.name == combo.name; });
		if (sameNameCount > 1) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "ほかのコンボと名前がかぶっています");
		}
		ImGui::Separator();

		//段の一覧。消す・動かすのはループが終わってから
		int32_t removeIndex = -1;
		int32_t moveUpIndex = -1;
		for (size_t i = 0; i < combo.steps.size(); ++i) {
			BossComboStep& step = combo.steps[i];
			ImGui::PushID(static_cast<int>(i) + 2000);
			ImGui::Text("%d段目", static_cast<int>(i) + 1);
			ImGui::SameLine();

			int32_t attack = static_cast<int32_t>(step.attack);
			ImGui::SetNextItemWidth(120.0f);
			if (ImGui::Combo("##Attack", &attack, kBossAttackNames, kBossAttackTypeCount)) {
				step.attack = static_cast<BossAttackType>(attack);
				isDirty_ = true;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::DragInt("回", &step.repeat, 0.1f, 1, 20)) {
				isDirty_ = true;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::DragFloat("秒あけて次へ", &step.interval, 0.01f, 0.0f, 5.0f)) {
				isDirty_ = true;
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(i == 0);
			if (ImGui::ArrowButton("Up", ImGuiDir_Up)) {
				moveUpIndex = static_cast<int32_t>(i);
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::SmallButton("×")) {
				removeIndex = static_cast<int32_t>(i);
			}
			ImGui::PopID();
		}
		if (moveUpIndex > 0) {
			std::swap(combo.steps[static_cast<size_t>(moveUpIndex)], combo.steps[static_cast<size_t>(moveUpIndex) - 1]);
			isDirty_ = true;
		}
		if (removeIndex >= 0) {
			combo.steps.erase(combo.steps.begin() + removeIndex);
			isDirty_ = true;
		}
		if (ImGui::Button("段を足す")) {
			combo.steps.push_back(BossComboStep{});
			isDirty_ = true;
		}
#else
		(void)combo;
#endif
	}
}
