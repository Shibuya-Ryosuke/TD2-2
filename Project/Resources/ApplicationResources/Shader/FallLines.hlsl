//層を降りるときにパステルの線が下から上へ流れる
//params[0].xは強さ

static const float kColumns = 80.0f; //画面の横に線を置く列の数
static const float kLineLength = 1.6f; //小さいほど長い線の長さ
static const float kSpeedMin = 2.0f; //いちばん遅い線の速さ
static const float kSpeedRange = 2.5f; //速さのばらつき
static const float kAppearRate = 0.45f; //線が出る列の割合
static const float kBrightness = 0.55f; //線の明るさ
static const float kCenterClear = 0.2f; //大きいほど真ん中の線が少ない
static const float kBlurLength = 0.012f; //強さ1のときの縦のブレの長さ
static const int kBlurSamples = 5;

//0から1のでたらめな値
float Hash(float n)
{
    return frac(sin(n) * 43758.5453f);
}

//hueで決まるパステルの虹色
float3 PastelRainbow(float hue)
{
    float3 rainbow = saturate(abs(frac(hue + float3(0.0f, 0.6667f, 0.3333f)) * 6.0f - 3.0f) - 1.0f);
    return lerp(float3(1.0f, 1.0f, 1.0f), rainbow, 0.65f);
}

float4 PostEffect(PostEffectInput input)
{
    float strength = input.params[0].x;
    float4 scene = SampleScene(input.uv);
    if (strength <= 0.001f)
    {
        return scene;
    }

    //落ちている速さで縦に少しブレる
    float3 color = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = 0; i < kBlurSamples; ++i)
    {
        float offset = (float(i) / float(kBlurSamples - 1) - 0.5f) * kBlurLength * strength;
        color += SampleScene(input.uv + float2(0.0f, offset)).rgb;
    }
    color /= float(kBlurSamples);

    //列ごとに出る出ないと速さと色を変える
    float column = floor(input.uv.x * kColumns);
    float random = Hash(column + 1.0f);
    float appear = step(1.0f - kAppearRate, Hash(column * 3.7f + 11.0f));

    //時間がたつほど模様が上へ動く
    float flow = frac(input.uv.y * kLineLength + input.time * (kSpeedMin + random * kSpeedRange) + random);
    float streak = smoothstep(0.0f, 0.04f, flow) * (1.0f - smoothstep(0.04f, 0.45f, flow));

    //列の真ん中だけ細く光らせる
    float across = abs(frac(input.uv.x * kColumns) - 0.5f);
    streak *= 1.0f - smoothstep(0.05f, 0.2f, across);

    //外ほど線を多くする
    float side = abs(input.uv.x - 0.5f) * 2.0f;
    streak *= lerp(1.0f - kCenterClear, 1.0f, smoothstep(0.0f, 0.6f, side));

    color += PastelRainbow(random) * streak * appear * strength * kBrightness;
    return float4(color, scene.a);
}
