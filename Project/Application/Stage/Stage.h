#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace App {
	/// <summary>
	/// マスの番号。x: 左から何マス目 / z: 手前から何マス目 (どちらも 0 から)
	/// </summary>
	struct Cell {
		int32_t x = 0;
		int32_t z = 0;
	};

	/// <summary>
	/// ステージ (1層ぶんの床)。
	/// ・CSV (0: 穴 / 1: 床) を読んでタイルを並べる。床全体の真ん中が原点、タイルの上の面の高さが height_
	/// ・プレイヤーが立ったタイルは、collapseDelay_ 秒たつと消えて穴になる (StepOn)
	/// ・Reset で、CSV を読んだときの形に戻す
	/// プレイヤーは「この位置は何マス目？」「そこに床はある？」を聞いて、床の上に立つ。
	/// 数値は Unity のプロトタイプと同じ。
	/// </summary>
	class Stage {
	public:
		Stage() = default;
		~Stage() = default;

		Stage(const Stage&) = delete;
		Stage& operator=(const Stage&) = delete;

		/// <summary>
		/// CSV を読んでタイルを並べる。読めなかったら false (そのときは 10x10 の床を並べる)
		/// </summary>
		/// <param name="csvPath">例: "Resources/ApplicationResources/Stage/layer1.csv"</param>
		bool Initialize(const std::string& csvPath);

		/// <summary>踏まれたタイルの時間を進めて、時間が来たら消す。タイルの色と行列も確定する</summary>
		void Update();

		/// <summary>床があるタイルを描く</summary>
		void Draw();

		/// <summary>全部のタイルを、CSV を読んだときの形に戻す</summary>
		void Reset();

		/// <summary>その位置が何マス目か (床の外なら、範囲外の番号が返る)</summary>
		Cell GetCellAt(const RyoEngine::Vector3& position) const;

		/// <summary>マスの真ん中の位置 (高さはタイルの上の面)</summary>
		RyoEngine::Vector3 GetCellCenter(const Cell& cell) const;

		/// <summary>そのマスに今、床があるか (範囲外は false)</summary>
		bool IsSolid(const Cell& cell) const;

		/// <summary>プレイヤーが立った。まだ踏まれていない床なら、消えるまでの時間を数え始める</summary>
		void StepOn(const Cell& cell);

		/// <summary>床の外に出ないように位置を収める (radius: 体の半径)</summary>
		RyoEngine::Vector3 ClampToBoundary(const RyoEngine::Vector3& position, float radius) const;

		/// <summary>タイルの上の面の高さ</summary>
		float GetHeight() const { return height_; }

	private:
		struct Tile {
			bool isSolid = false;        // 今、床があるか
			bool isSolidAtStart = false; // CSV での形 (Reset でここに戻す)
			float remaining = -1.0f;     // 消えるまでの残り時間 (負: まだ踏まれていない)
		};

		bool LoadCsv(const std::string& csvPath); // CSV を読んで width_・depth_・tiles_ を作る
		void CreateDefaultTiles();                // CSV が読めなかったときの 10x10 の床
		void CreateModels();                      // タイルのモデルを作る
		void ApplyTileShape();                    // タイルの大きさ・位置を今の値 (すき間・厚さ) に合わせる
		bool Contains(const Cell& cell) const;    // 床の範囲の中か
		size_t ToIndex(const Cell& cell) const;   // マスの番号 → tiles_ の番号

		// --- 形 ---
		int32_t width_ = 0; // 横のマスの数
		int32_t depth_ = 0; // 奥のマスの数
		std::vector<Tile> tiles_;                                  // 手前の行から順に、左から右へ
		std::vector<std::unique_ptr<RyoEngine::Model>> models_;    // tiles_ と同じ並び (CSV で穴のマスは nullptr)

		// --- 大きさ (Unity の値) ---
		float tileSize_ = 2.0f;       // 1マスの大きさ
		float tileThickness_ = 0.45f; // タイルの厚さ
		float tileGap_ = 0.055f;      // タイルのすき間 (1マスの大きさに対する割合)
		float height_ = 0.0f;         // タイルの上の面の高さ

		// --- 消え方 (Unity の値) ---
		float collapseDelay_ = 1.5f;  // 踏んでから消えるまでの時間(秒)

		// --- 色 (Unity のマテリアルの色) ---
		RyoEngine::Vector4 normalColor_ = { 0.33f, 0.44f, 0.56f, 1.0f };  // ふつう
		RyoEngine::Vector4 warningColor_ = { 1.0f, 0.67f, 0.33f, 1.0f };  // 踏まれて消えるのを待っている間
	};
}
