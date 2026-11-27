//プレイヤーの左腕を肩を中心に大きくして、着地の瞬間にふちがふんわり白く光るのは体と同じ
//params[0].xは着地ですぐ消えるピカッの強さ
//params[0].yは腕の大きさの倍率で、0なら大きくしない
//params[0].zwとparams[1].xは左の肩、params[1].yzwは右の肩の、モデルの原点から見た位置

static const float3 kGlowColor = float3(1.0f, 0.92f, 0.97f); //ほんのりピンクの白
static const float kRimPower = 2.0f; //大きいほどふちだけ光る細さ
static const float kRimBrightness = 1.5f; //ふちの光の強さ
static const float kFillBrightness = 0.35f; //体全体のうっすらした光

#define USE_CUSTOM_VERTEX
//動いたあとの腕の頂点を、肩からの向きはそのままで倍率だけ遠くへ動かす
void Vertex(inout VertexInput v, ModelInstance instance)
{
    float armScale = instance.params[0].y;
    if (armScale <= 0.0f)
    {
        return;
    }
    float3 shoulder = float3(instance.params[0].z, instance.params[0].w, instance.params[1].x);
    v.position.xyz = shoulder + (v.position.xyz - shoulder) * armScale;
}

void Surface(SurfaceInput input, inout SurfaceOutput output)
{
    float glow = input.params[0].x;
    if (glow <= 0.001f)
    {
        return;
    }
    //カメラから見て横を向いている面ほど光る
    float3 toCamera = normalize(gCameraPosition - input.worldPosition);
    float rim = pow(1.0f - saturate(dot(normalize(input.normal), toCamera)), kRimPower);
    output.emissive += kGlowColor * (rim * kRimBrightness + kFillBrightness) * glow;
}
