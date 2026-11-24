// =====================================================================
// 場面切り替えの積み木の壁 (TransitionWall が使う)
// ・ドン！のとき縁がカラフルに光る
// ・壁全体 (5 x 5) が1つのプリンみたいに、上ほど大きく ぷるんぷるん 揺れる (下は動かない)
// ・揺れている間、白いつやの光が揺れに合わせてすべる
//   0番 (params[0].x) : ぷるぷるの強さ (0〜1)
//   1番 (params[0].y) : このブロックの段 (0 = いちばん下)
//   2番 (params[0].z) : このブロックの列 (0 = いちばん左)
//   3番 (params[0].w) : 縁の光 (0〜1)
//   4〜6番 (params[1].xyz) : 縁の光の色
// =====================================================================

// 壁の大きさ (DirectionConfig の kTransitionColumns / kTransitionRows と同じにする)
static const float kColumns = 5.0f;
static const float kRows = 5.0f;
static const float kBlockFill = 0.985f;      // ブロックの詰まり具合 (1 - kTransitionBlockGap)

// ぷるぷる (壁の幅・高さに対する割合)
static const float kSwayAmount = 0.06f;      // 左右にゆれる量 (いちばん上で)
static const float kSwaySpeed = 13.0f;       // 左右のゆれの速さ
static const float kSquashAmount = 0.05f;    // 上下につぶれる・のびる量
static const float kSquashSpeed = 17.0f;     // 上下のゆれの速さ
static const float kSquashWiden = 0.6f;      // つぶれたとき横にふくらむ量

// つや
static const float3 kGlossColor = float3(1.0f, 1.0f, 1.0f);
static const float kGlossBrightness = 0.9f;
static const float kGlossSlide = 0.25f;      // ゆれに合わせて、つやがすべる量

// 縁の光
static const float kEdgeWidth = 0.12f;
static const float kEdgeBrightness = 3.0f;

#define USE_CUSTOM_VERTEX
// 壁全体を1つのプリンとしてゆがめる (どのブロックも同じ式なので、つなぎ目がずれない)
void Vertex(inout VertexInput v, ModelInstance instance)
{
    float jiggle = instance.params[0].x;
    if (jiggle <= 0.0f)
    {
        return;
    }
    float row = instance.params[0].y;
    float column = instance.params[0].z;

    // この頂点が壁のどこか (0〜1)。cube.obj は -1〜1
    float height = (row + (v.position.y * kBlockFill + 1.0f) * 0.5f) / kRows;
    float across = (column + (v.position.x * kBlockFill + 1.0f) * 0.5f) / kColumns;

    // 左右: 上ほど大きく / 上下: 下を止めたまま、つぶれる・のびる (つぶれたら横にふくらむ)
    float sway = sin(gTime * kSwaySpeed) * pow(height, 1.5f) * kSwayAmount * jiggle;
    float squash = sin(gTime * kSquashSpeed + 1.3f) * kSquashAmount * jiggle;
    float moveX = sway + (across - 0.5f) * squash * kSquashWiden * height;
    float moveY = -height * squash;

    // 壁の割合 → ブロックの中の長さ (ブロック1個の幅・高さが 2 / kBlockFill)
    v.position.x += moveX * kColumns * 2.0f / kBlockFill;
    v.position.y += moveY * kRows * 2.0f / kBlockFill;
}

void Surface(SurfaceInput input, inout SurfaceOutput output)
{
    float jiggle = input.params[0].x;
    float glow = input.params[0].w;
    float3 glowColor = input.params[1].xyz;
    float2 uv = frac(input.uv);

    // ドン！の縁の光 (一番近い辺までの距離)
    float edge = min(min(uv.x, 1.0f - uv.x), min(uv.y, 1.0f - uv.y));
    float edgeMask = 1.0f - smoothstep(0.0f, kEdgeWidth, edge);
    output.emissive += glowColor * edgeMask * glow * kEdgeBrightness;
    output.color = lerp(output.color, glowColor, glow * 0.35f);

    // つや: ななめの白い帯が、ゆれに合わせて すべる
    if (jiggle > 0.0f)
    {
        float band = uv.y * 0.8f + uv.x * 0.3f + sin(input.time * kSwaySpeed) * kGlossSlide;
        float gloss = smoothstep(0.62f, 0.7f, band) * (1.0f - smoothstep(0.78f, 0.9f, band));
        float softGloss = smoothstep(0.2f, 0.6f, band) * 0.15f;
        output.emissive += kGlossColor * (gloss + softGloss) * jiggle * kGlossBrightness;
    }
}
