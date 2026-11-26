//カメラの方を向いた箱の前と後ろの面に、中がすけた線だけのハートを描く
//params[0].xは光の強さ
//params[0].yは面の中で回すラジアン
//params[0].zはキラッと光るタイミングをずらす値

static const float kHeartScale = 0.82f; //面に対するハートの大きさ
static const float kLineWidth = 0.16f; //ハートの線の太さ
static const float kLineSoftness = 0.03f; //線のふちのぼかし
static const float kCoreWidth = 0.35f; //線の太さに対する線のまん中の白っぽい所の割合
static const float kTwinkleSpeed = 11.0f; //キラッと光る速さ
static const float kGlowBrightness = 0.12f; //線の光でまぶしくない明るさ

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

#define USE_CUSTOM_VERTEX
//前と後ろの面だけ面の中の位置をUVにして、ほかの面は描かない印をつける
void Vertex(inout VertexInput v, ModelInstance instance)
{
    if (abs(v.normal.z) > 0.5f)
    {
        float angle = instance.params[0].y;
        float2 p = v.position.xy;
        float2 rotated = float2(p.x * cos(angle) - p.y * sin(angle), p.x * sin(angle) + p.y * cos(angle));
        v.texcoord = rotated * 0.5f + 0.5f;
    }
    else
    {
        v.texcoord = float2(-10.0f, -10.0f);
    }
}

void Surface(SurfaceInput input, inout SurfaceOutput output)
{
    //ハートの外と、線より内側は消して中をすけさせる
    float2 p = (input.uv - 0.5f) * 2.0f * 0.62f / kHeartScale + float2(0.0f, 0.55f);
    float heartDistance = HeartDistance(p);
#ifdef MODEL_STAGE_PS
    clip(-heartDistance);
    clip(heartDistance + kLineWidth);
#endif

    //線のまん中を少し白っぽくして、たまにキラッ
    float center = abs(heartDistance + kLineWidth * 0.5f) / (kLineWidth * 0.5f);
    float core = 1.0f - smoothstep(kCoreWidth - kLineSoftness, kCoreWidth + kLineSoftness, center);
    float twinkle = pow(saturate(sin(input.time * kTwinkleSpeed + input.params[0].z * 6.28f)), 8.0f);
    float glow = input.params[0].x;

    float3 baseColor = input.instance.color.rgb;
    float3 white = float3(1.0f, 1.0f, 1.0f);
    output.color = lerp(baseColor, white, core * 0.45f);
    output.enableLighting = false;
    output.emissive += baseColor * glow * kGlowBrightness * (1.0f + twinkle);
}
