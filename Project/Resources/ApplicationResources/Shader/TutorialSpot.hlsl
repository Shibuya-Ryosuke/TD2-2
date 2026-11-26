//チュートリアルで壊すマスを上から照らすスポットライトの光の柱と、マスのふちのキラキラ枠
//params[0].xは強さ
//params[0].yは0なら光の柱で1ならマスの上に置く枠

static const float3 kSpotColor = float3(1.0f, 0.93f, 0.62f); //クリーム色の光
static const float3 kFrameColor = float3(1.0f, 0.55f, 0.8f); //ピンクの枠
static const float kBeamBrightness = 0.55f; //柱の光の明るさでブルームでにじみすぎない明るさ
static const float kBeamFade = 1.8f; //大きいほど柱の上の方が早く消える
static const float kBottomClear = 0.25f; //柱の下のこの高さまでは薄くして積み木を見せる
static const float kCenterAlpha = 0.2f; //柱のまん中の濃さで、ふちへいくほど濃い
static const float kStripeCount = 5.0f; //柱を下りていく光の帯の数
static const float kStripeSpeed = 1.2f; //光の帯が下りていく速さ
static const float kStripeDepth = 0.45f; //帯で明るさが変わる割合
static const float kFrameWidth = 0.12f; //マスに対する枠の太さ
static const float kFrameBrightness = 1.4f; //枠の明るさ
static const float kPulseSpeed = 5.0f; //枠がどくんと光る速さ

#define USE_CUSTOM_VERTEX
//柱は横の面の高さと横の位置をUVに、枠は上の面の位置をUVにして、ほかの面は描かない印をつける
void Vertex(inout VertexInput v, ModelInstance instance)
{
    float isFrame = instance.params[0].y;
    float isSide = 1.0f - step(0.5f, abs(v.normal.y));
    float isTop = step(0.5f, v.normal.y);
    float2 sideUv = float2(v.position.x * abs(v.normal.z) + v.position.z * abs(v.normal.x), v.position.y) * 0.5f + 0.5f;
    float2 topUv = v.position.xz * 0.5f + 0.5f;
    float2 hidden = float2(-10.0f, -10.0f);
    float2 beamUv = lerp(hidden, sideUv, isSide);
    float2 frameUv = lerp(hidden, topUv, isTop);
    v.texcoord = lerp(beamUv, frameUv, isFrame);
}

void Surface(SurfaceInput input, inout SurfaceOutput output)
{
    //描かない面は消す
#ifdef MODEL_STAGE_PS
    clip(input.uv.x + 5.0f);
#endif
    float strength = input.params[0].x;
    float isFrame = input.params[0].y;
    output.enableLighting = false;

    //柱は中がすけて、ふちと少し上が濃く、光の帯が下りていく
    float height = saturate(input.uv.y);
    float fade = pow(1.0f - height, kBeamFade);
    float bottom = smoothstep(0.0f, kBottomClear, height);
    float stripe = 1.0f - kStripeDepth + kStripeDepth * (0.5f + 0.5f * sin((height * kStripeCount + input.time * kStripeSpeed) * 6.2832f));
    float edge = lerp(kCenterAlpha, 1.0f, pow(abs(input.uv.x * 2.0f - 1.0f), 3.0f));
    float beamAlpha = fade * bottom * stripe * edge * strength;

    //枠はマスのふちだけどくんどくんと光る
    float2 fromEdge = min(input.uv, 1.0f - input.uv);
    float edgeDistance = min(fromEdge.x, fromEdge.y);
    float frame = 1.0f - smoothstep(kFrameWidth * 0.6f, kFrameWidth, edgeDistance);
    float pulse = 0.65f + 0.35f * sin(input.time * kPulseSpeed);
    float frameAlpha = frame * pulse * strength;

    float3 color = lerp(kSpotColor * kBeamBrightness, kFrameColor * kFrameBrightness, isFrame);
    output.color = color;
    output.emissive = color;
    output.alpha = lerp(beamAlpha, frameAlpha, isFrame);
}
