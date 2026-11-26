#pragma once
#include <string>
#include <vector>
#include "BossPhaseData.h"

namespace App {
	class Boss;

	//ボスの形態の一覧。JSONに保存して、デバッグ時はImGuiで追加・削除・編集する
	//表はゲーム中ずっと1つだけで、ボスは毎回ここを読む
	class BossPhaseTable {
	public:
		//保存するフォルダを決める
		static void SetFolderPath(const std::string& folderPath);
		//ファイルから読む。読めなければ形態を1つだけ作ってfalse
		static bool Load();
		//ファイルに書く
		static void Save();

		//形態の一覧。いつも1つ以上ある
		static const std::vector<BossPhaseData>& GetPhases();

		//エディタの画面を出す。bossを渡すとデバッグ用のボタンも出る
		static void DrawEditor(Boss* boss);

	private:
		static BossPhaseTable& GetInstance();

		//ファイルがないときの形態を1つ作る
		void CreateDefault();
		//保存するファイルの場所
		std::string GetFullPath() const;

		//エディタの部品
		void DrawPhaseList();
		void DrawPhaseDetail(BossPhaseData& phase);
		void DrawTriggers(BossPhaseData& phase);
		void DrawDebug(Boss& boss);

		std::vector<BossPhaseData> phases_;
		std::string folderPath_;
		int32_t selected_ = 0; //エディタで選んでいる形態
		bool isDirty_ = false; //保存していない変更があるか

		//保存するファイルの名前
		static inline const std::string kFileName = "BossPhases.json";
	};
}
