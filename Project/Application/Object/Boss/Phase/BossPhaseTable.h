#pragma once
#include <string>
#include <vector>
#include "BossPhaseData.h"

namespace App {
	class Boss;

	//ボスの形態とコンボの一覧。JSONに保存して、デバッグ時はImGuiで追加・削除・編集する
	//表はゲーム中ずっと1つだけで、ボスは毎回ここを読む
	class BossPhaseTable {
	public:
		//保存するフォルダを決める
		static void SetFolderPath(const std::string& folderPath);
		//ファイルから読む。読めなければ初期の形態とコンボを作ってfalse
		static bool Load();
		//ファイルに書く
		static void Save();

		//形態の一覧。いつも1つ以上ある
		static const std::vector<BossPhaseData>& GetPhases();
		//コンボの一覧
		static const std::vector<BossCombo>& GetCombos();
		//名前でコンボを探す。なければnullptr
		static const BossCombo* FindCombo(const std::string& name);

		//エディタの画面を出す。bossを渡すとデバッグ用のボタンも出る
		static void DrawEditor(Boss* boss);

	private:
		static BossPhaseTable& GetInstance();

		//ファイルがないときの形態とコンボを作る
		void CreateDefault();
		//保存するファイルの場所
		std::string GetFullPath() const;
		//ほかのコンボとかぶらない名前を作る
		std::string MakeUniqueComboName(const std::string& base) const;

		//形態のタブ
		void DrawPhaseTab(Boss* boss);
		void DrawPhaseList();
		void DrawPhaseDetail(BossPhaseData& phase);
		void DrawPhaseCombos(BossPhaseData& phase);
		void DrawPhaseDebug(Boss& boss);

		//コンボのタブ
		void DrawComboTab(Boss* boss);
		void DrawComboList();
		void DrawComboDetail(BossCombo& combo);

		std::vector<BossPhaseData> phases_;
		std::vector<BossCombo> combos_;
		std::string folderPath_;
		int32_t selected_ = 0; //エディタで選んでいる形態
		int32_t selectedCombo_ = 0; //エディタで選んでいるコンボ
		bool isDirty_ = false; //保存していない変更があるか

		//保存するファイルの名前
		static inline const std::string kFileName = "BossPhases.json";
	};
}
