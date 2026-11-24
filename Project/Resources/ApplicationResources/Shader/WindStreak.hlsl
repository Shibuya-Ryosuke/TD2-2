// =====================================================================
// 風の筋 (場面切り替えで積み木が倒れるときのポストエフェクト)
// 画面の奥から手前へ風が吹き抜ける: 真ん中から外へ向かう放射状のブラー + 外へ流れる白い筋
//   0番 (params[0].x) : 強さ (0 で何もしない)
// =====================================================================

static const int kBlurSamples = 8;       // ブラーで読む回数
static const float kBlurLength = 0.08f;  // ブラーの長さ (強さ 1 のとき)
static const float kStreakCount = 90.0f; // 筋の本数 (ぐるっと1周で)
static const float kStreakBrightness = 0.6f;

// 0〜1 のでたらめな値
float Hash(float n)
{
    return frac(sin(n) * 43758.5453f);
}

float4 PostEffect(PostEffectInput input)
{
    float strength = input.params[0].x;
    float4 scene = SampleScene(input.uv);
    if (strength <= 0.001f)
    {
        return scene;
    }

    // 真ん中から外へのブラー (風が手前に吹き抜ける感じ)
    float2 center = float2(0.5f, 0.5f);
    float2 toPixel = input.uv - center;
    float3 color = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = 0; i < kBlurSamples; ++i)
    {
        float scale = 1.0f - strength * kBlurLength * (float(i) / float(kBlurSamples - 1));
        color += SampleScene(center + toPixel * scale).rgb;
    }
    color /= float(kBlurSamples);

    // 外へ流れる筋 (角度で細い帯に分けて、帯ごとに速さ・出る出ないを変える)
    float aspect = input.screenSize.x / input.screenSize.y;
    float2 offset = float2(toPixel.x * aspect, toPixel.y);
    float radius = length(offset);
    float around = atan2(offset.y, offset.x) / 6.2831853f + 0.5f;
    float band = floor(around * kStreakCount);
    float random = Hash(band + 1.0f);
    float flow = frac(radius * 1.2f - input.time * (1.5f + random * 2.0f) + random);
    float streak = smoothstep(0.0f, 0.05f, flow) * (1.0f - smoothstep(0.05f, 0.3f, flow));
    // 帯の真ん中だけ細く光らせる / 半分くらいの帯だけ / 真ん中付近は出さない
    float across = abs(frac(around * kStreakCount) - 0.5f);
    streak *= 1.0f - smoothstep(0.05f, 0.2f, across);
    streak *= step(0.5f, random);
    streak *= smoothstep(0.15f, 0.6f, radius);
    color += streak * strength * kStreakBrightness;

    return float4(color, scene.a);
}
