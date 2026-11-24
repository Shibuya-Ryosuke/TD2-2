//ダメージで画面の四隅にピンクから赤がにじんで色が少しずれる
//params[0].xは強さ

static const float3 kInnerColor = float3(1.0f, 0.5f, 0.72f); //にじみの内側のピンク
static const float3 kOuterColor = float3(1.0f, 0.22f, 0.32f); //四隅の赤
static const float kVignetteStart = 0.45f; //ここから外がにじむ
static const float kVignetteAmount = 0.5f; //強さ1のときのにじみの濃さ
static const float kColorShift = 0.006f; //強さ1の四隅での色のずれ

float4 PostEffect(PostEffectInput input)
{
    float strength = input.params[0].x;
    float4 scene = SampleScene(input.uv);
    if (strength <= 0.001f)
    {
        return scene;
    }

    //四隅が1になる丸い真ん中からの距離
    float aspect = input.screenSize.x / input.screenSize.y;
    float2 fromCenter = input.uv - 0.5f;
    float radius = length(fromCenter * float2(aspect, 1.0f)) / length(float2(aspect, 1.0f) * 0.5f);

    //ふちほど赤は外へ青は内へずらす
    float2 shift = fromCenter * (kColorShift * 2.0f) * strength * radius;
    float3 color = float3(SampleScene(input.uv + shift).r, scene.g, SampleScene(input.uv - shift).b);

    //ピンクから赤へふんわりにじむ
    float vignette = smoothstep(kVignetteStart, 1.0f, radius) * strength;
    float3 edgeColor = lerp(kInnerColor, kOuterColor, smoothstep(0.7f, 1.0f, radius));
    color = lerp(color, edgeColor, vignette * kVignetteAmount);

    return float4(color, scene.a);
}
