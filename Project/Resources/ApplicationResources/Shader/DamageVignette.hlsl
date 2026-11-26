//ダメージで画面のふちが赤くぎらっと光り、ハートの形にどくんと脈打って色が少しずれる
//params[0].xは強さ
//params[0].yは光り始めてからの秒数

static const float3 kRimColor = float3(1.0f, 0.08f, 0.16f); //ふちのいちばん濃い赤
static const float3 kGlowColor = float3(1.0f, 0.35f, 0.45f); //内側へにじむ光のピンクがかった赤
static const float3 kFlashColor = float3(1.0f, 0.85f, 0.9f); //当たった瞬間の白っぽい光
static const float kRimStart = 0.55f; //ここから外が濃い赤になる
static const float kGlowStart = 0.2f; //ここから外へ光がにじむ
static const float kGlowAmount = 0.9f; //にじむ光の明るさ
static const float kRimAmount = 0.85f; //ふちの赤の濃さ
static const float kFlashTime = 0.08f; //白っぽく光る秒数
static const float kBeatSpeed = 18.0f; //どくんと脈打つ速さ
static const float kBeatDepth = 0.25f; //脈打つ大きさ
static const float kColorShift = 0.012f; //強さ1の四隅での色のずれ

float4 PostEffect(PostEffectInput input)
{
    float strength = input.params[0].x;
    float elapsed = input.params[0].y;
    float4 scene = SampleScene(input.uv);
    if (strength <= 0.001f)
    {
        return scene;
    }

    //四隅が1になる丸い真ん中からの距離を、上下がふくらんだハートっぽい形にゆがめる
    float aspect = input.screenSize.x / input.screenSize.y;
    float2 fromCenter = input.uv - 0.5f;
    float2 shape = fromCenter * float2(aspect, 1.0f);
    shape.y -= abs(shape.x) * 0.25f;
    float radius = length(shape) / length(float2(aspect, 1.0f) * 0.5f);

    //どくんと脈打つ
    float beat = 1.0f + kBeatDepth * sin(input.time * kBeatSpeed) * strength;

    //ふちほど赤は外へ青は内へずらす
    float2 shift = fromCenter * kColorShift * strength * radius * beat;
    float3 color = float3(SampleScene(input.uv + shift).r, scene.g, SampleScene(input.uv - shift).b);

    //濃い赤のふちと、内側へ広がる光を足してぎらっと光らせる
    float rim = smoothstep(kRimStart, 1.05f, radius * beat) * strength;
    float glow = pow(smoothstep(kGlowStart, 1.1f, radius * beat), 2.0f) * strength;
    color = lerp(color, kRimColor, rim * kRimAmount);
    color += kGlowColor * glow * kGlowAmount;

    //当たった瞬間だけふち全体が白っぽく光る
    float flash = (1.0f - saturate(elapsed / kFlashTime)) * smoothstep(0.3f, 1.0f, radius);
    color += kFlashColor * flash * strength;

    return float4(saturate(color), scene.a);
}
