//着地の瞬間にふちがふんわり白く光るプレイヤーの体
//params[0].xは着地ですぐ消えるピカッの強さ

static const float3 kGlowColor = float3(1.0f, 0.92f, 0.97f); //ほんのりピンクの白
static const float kRimPower = 2.0f; //大きいほどふちだけ光る細さ
static const float kRimBrightness = 1.5f; //ふちの光の強さ
static const float kFillBrightness = 0.35f; //体全体のうっすらした光

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
