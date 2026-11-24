//積み木が吹き飛ぶときに白っぽい線が左から右へ流れる横風
//params[0].xは強さ

static const float kRowsOfLines = 45.0f; //画面の縦に線を置く行の数
static const float kLineLength = 1.2f; //小さいほど長い線の長さ
static const float kSpeedMin = 2.5f; //いちばん遅い線の速さ
static const float kSpeedRange = 2.5f; //速さのばらつき
static const float kAppearRate = 0.5f; //線が出る行の割合
static const float kBrightness = 0.7f; //線の明るさ
static const float kBlurLength = 0.015f; //強さ1のときの横のブレの長さ
static const int kBlurSamples = 5;

//0から1のでたらめな値
float Hash(float n)
{
    return frac(sin(n) * 43758.5453f);
}

//hueで決まる白っぽいパステル
float3 PalePastel(float hue)
{
    float3 rainbow = saturate(abs(frac(hue + float3(0.0f, 0.6667f, 0.3333f)) * 6.0f - 3.0f) - 1.0f);
    return lerp(float3(1.0f, 1.0f, 1.0f), rainbow, 0.35f);
}

float4 PostEffect(PostEffectInput input)
{
    float strength = input.params[0].x;
    float4 scene = SampleScene(input.uv);
    if (strength <= 0.001f)
    {
        return scene;
    }

    //左の色を引っぱって風に流されるように横にブレる
    float3 color = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = 0; i < kBlurSamples; ++i)
    {
        float offset = float(i) / float(kBlurSamples - 1) * kBlurLength * strength;
        color += SampleScene(input.uv - float2(offset, 0.0f)).rgb;
    }
    color /= float(kBlurSamples);

    //行ごとに出る出ないと速さと色を変える
    float rowOfLine = floor(input.uv.y * kRowsOfLines);
    float random = Hash(rowOfLine + 1.0f);
    float appear = step(1.0f - kAppearRate, Hash(rowOfLine * 3.7f + 11.0f));

    //頭が右でしっぽが左に流れる
    float flow = frac(-input.uv.x * kLineLength + input.time * (kSpeedMin + random * kSpeedRange) + random);
    float streak = smoothstep(0.0f, 0.04f, flow) * (1.0f - smoothstep(0.04f, 0.5f, flow));

    //行の真ん中だけ細く光らせる
    float across = abs(frac(input.uv.y * kRowsOfLines) - 0.5f);
    streak *= 1.0f - smoothstep(0.05f, 0.25f, across);

    color += PalePastel(random) * streak * appear * strength * kBrightness;
    return float4(color, scene.a);
}
