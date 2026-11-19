#pragma once
#include <RyoEngine.h>

namespace App
{
	// 床の1マスぶんの情報 (Floor が持つ。FloorView・TileBarrage は読むだけ / 決められた関数で動かす)
	struct FloorTile
	{
		bool isSolid = false;         // 今、床があるか
		bool isSolidAtStart = false;  // CSV での形 (Reset でここに戻す)
		bool isIsolated = false;      // 切り離された床か
		bool isVisible = false;       // 描くか
		bool isPreview = false;       // 崩す形の予告を出しているか
		float remaining = -1.0f;      // 崩れるまでの残り時間 (負: 崩す予定はない)
		float fallTime = 0.0f;        // 穴になってから落ちている時間
		RyoEngine::Vector3 position{};  // 今の位置 (揺れ・落下・飛んでいく途中も)
		RyoEngine::Vector3 rotate{};    // 今の回転 (度。飛んでいく途中だけ回る)
		float scaleRate = 1.0f;         // 今の大きさの割合 (飛んでいく途中だけ小さくなる)
	};
}
