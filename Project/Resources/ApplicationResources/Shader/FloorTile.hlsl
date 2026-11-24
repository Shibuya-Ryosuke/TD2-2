//ハートが出てひびが入り、ぱっかーん割れてガラスみたいに砕ける床のタイル
//params[0].xはハートの大きさ
//params[0].yはハートのヒビ
//params[0].zはぱっかーん
//params[0].wはタイルのひび
//params[1].xは穴になってから砕けた割合
//params[1].yはマスごとに模様をずらす値
//params[1].zはガン！の光
//params[1].wはプレイヤーの向きのハートの向き

static const float3 kPink = float3(1.0f, 0.3f, 0.62f); //ハートとひびと光の色

//ハート
static const float kHeartSize = 0.6f; //上の面に対するハートの大きさ
static const float kOutlineWidth = 0.06f; //輪郭の太さ
static const float kOutlineBrightness = 1.8f;
static const float kHeartInnerGlow = 0.25f; //ハートの中のうっすらした光
static const float kBreakSpread = 0.6f; //ぱっかーんで欠片が外へ飛ぶ量
//ハートとタイルと欠片で共通のひび
static const float kCrackScale = 4.5f; //面にできる欠片の細かさ
static const float kCrackWidth = 0.06f; //ひびの太さ
static const float kCrackBrightness = 2.2f;
//穴になってから砕ける
static const float kShardBrightness = 1.2f;
//ガン！
static const float kFlashBrightness = 1.2f;

//0から1のでたらめな値2つ
float2 Hash22(float2 p)
{
    p = float2(dot(p, float2(127.1f, 311.7f)), dot(p, float2(269.5f, 183.3f)));
    return frac(sin(p) * 43758.5453f);
}

//内側がマイナスで先っぽが原点のハートの距離
float HeartDistance(float2 p)
{
    p.x = abs(p.x);
    if (p.y + p.x > 1.0f)
    {
        return length(p - float2(0.25f, 0.75f)) - 0.35355f;
    }
    float2 toTop = p - float2(0.0f, 1.0f);
    float2 toLine = p - 0.5f * max(p.x + p.y, 0.0f);
    return sqrt(min(dot(toTop, toTop), dot(toLine, toLine))) * sign(p.x - p.y);
}

//欠片の境目までの近さと欠片ごとのでたらめな値を出すボロノイ
float VoronoiEdge(float2 p, out float cellRandom)
{
    float2 cell = floor(p);
    float2 local = frac(p);
    float nearest = 8.0f;
    float second = 8.0f;
    cellRandom = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 offset = float2(x, y);
            float2 random = Hash22(cell + offset);
            float distanceToPoint = length(offset + random - local);
            if (distanceToPoint < nearest)
            {
                second = nearest;
                nearest = distanceToPoint;
                cellRandom = random.x;
            }
            else if (distanceToPoint < second)
            {
                second = distanceToPoint;
            }
        }
    }
    return second - nearest;
}

//どの演出も同じものを使うひびの模様
float CrackPattern(float2 uv, float seed, out float pieceRandom)
{
    return VoronoiEdge(uv * kCrackScale + seed, pieceRandom);
}

//面のUVと面ごとに模様をずらす値
float2 FaceUv(SurfaceInput input)
{
    return frac(input.uv) + input.normal.xz * 3.1f;
}

//上の面だけのピンクのハート
void DrawHeart(SurfaceInput input, inout SurfaceOutput output, float heartScale, float heartCrack, float heartBreak, float seed, float flash)
{
    //上の面のUVはタイルを上から見た位置
    float2 uv = frac(input.uv);
    float2 fromCenter = uv - 0.5f;

    //ぱっかーんで欠片が真ん中から外へ飛ぶ
    float spread = 1.0f + heartBreak * kBreakSpread;
    float2 source = fromCenter / spread;

    //ハートのふくらみをプレイヤーの向きへ向ける
    float angle = input.params[1].w;
    float2 up = float2(sin(angle), cos(angle));
    float2 right = float2(up.y, -up.x);
    float2 p = float2(dot(source, right), dot(source, up)) / (kHeartSize * max(heartScale, 0.001f)) + float2(0.0f, 0.5f);
    float heartDistance = HeartDistance(p);
    float outline = 1.0f - smoothstep(kOutlineWidth * 0.5f, kOutlineWidth, abs(heartDistance));
    float inside = 1.0f - smoothstep(-0.01f, 0.01f, heartDistance);
    float heartArea = saturate(outline + inside);

    //タイルと同じ模様のヒビをハートの真ん中から外へ走らせる
    float pieceRandom;
    float edge = CrackPattern(source + 0.5f, seed, pieceRandom);
    float crackLine = 1.0f - smoothstep(kCrackWidth * 0.5f, kCrackWidth, edge);
    crackLine *= saturate((heartCrack * 0.5f - length(source)) * 20.0f);

    //欠片のすき間が広がって外へ飛びながら消える
    float gap = 1.0f - smoothstep(kCrackWidth * (0.5f + heartBreak * 3.0f), kCrackWidth * (1.0f + heartBreak * 3.0f), edge);
    float alive = (1.0f - heartBreak) * (1.0f - gap * step(0.001f, heartBreak));

    float3 white = float3(1.0f, 1.0f, 1.0f);
    float heartLight = (outline * kOutlineBrightness + inside * kHeartInnerGlow) * (1.0f + flash);
    output.color = lerp(output.color, kPink, outline * 0.8f * alive);
    output.emissive += kPink * heartLight * alive;
    output.emissive += lerp(kPink, white, 0.6f) * (crackLine * 2.5f * heartArea + heartBreak * inside * 1.5f) * alive;
}

//ぱっかーんで一気にタイル全体へ広がるひび
void DrawTileCrack(SurfaceInput input, inout SurfaceOutput output, float tileCrack, float seed)
{
    float pieceRandom;
    float edge = CrackPattern(FaceUv(input), seed, pieceRandom);
    float crackLine = 1.0f - smoothstep(kCrackWidth * 0.5f, kCrackWidth, edge);
    //横の面は少し遅れて真ん中から外へ走る
    float reach = tileCrack * 1.1f - length(frac(input.uv) - 0.5f) * 1.4f;
    crackLine *= saturate(reach * 10.0f);
    float pulse = 0.8f + 0.2f * sin(input.time * 14.0f + seed);
    output.color = lerp(output.color, kPink, crackLine * 0.7f);
    output.emissive += kPink * crackLine * pulse * kCrackBrightness;
}

//穴になったらひびの欠片ごとにパリッと消える
void DrawShatter(SurfaceInput input, inout SurfaceOutput output, float shatter, float seed)
{
    float pieceRandom;
    float edge = CrackPattern(FaceUv(input), seed, pieceRandom);
    float threshold = shatter * 1.15f;
#ifdef MODEL_STAGE_PS
    clip(pieceRandom - threshold);
#endif
    //消える直前の欠片ほどガラスっぽく白く光る
    float soon = 1.0f - saturate((pieceRandom - threshold) * 6.0f);
    float glint = pow(saturate(sin(pieceRandom * 40.0f + input.time * 9.0f)), 12.0f);
    float3 lightPink = lerp(kPink, float3(1.0f, 1.0f, 1.0f), 0.5f);
    output.color = lerp(output.color, lightPink, 0.35f + soon * 0.4f);
    output.emissive += lightPink * (soon + glint * 0.8f) * kShardBrightness;
}

#define USE_CUSTOM_VERTEX
//ハートを向きに合わせるために上の面のUVを上から見た位置にする
void Vertex(inout VertexInput v, ModelInstance instance)
{
    if (v.normal.y > 0.5f)
    {
        v.texcoord = v.position.xz * 0.5f + 0.5f;
    }
}

void Surface(SurfaceInput input, inout SurfaceOutput output)
{
    float heartScale = input.params[0].x;
    float heartCrack = input.params[0].y;
    float heartBreak = input.params[0].z;
    float tileCrack = input.params[0].w;
    float shatter = input.params[1].x;
    float seed = input.params[1].y;
    float flash = input.params[1].z;

    //ガン！でタイル全体がピンクに光る
    output.emissive += kPink * flash * kFlashBrightness;

    if (tileCrack > 0.0f)
    {
        DrawTileCrack(input, output, tileCrack, seed);
    }
    //ハートは上の面だけで飛び散りきったら描かない
    if (heartScale > 0.0f && heartBreak < 1.0f && input.normal.y > 0.5f)
    {
        DrawHeart(input, output, heartScale, heartCrack, heartBreak, seed, flash);
    }
    if (shatter > 0.0f)
    {
        DrawShatter(input, output, shatter, seed);
    }
}
