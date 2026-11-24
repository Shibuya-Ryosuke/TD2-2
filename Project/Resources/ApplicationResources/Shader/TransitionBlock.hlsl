//場面切り替えの積み木と、紙吹雪や欠片やほこりの単色
//params[0].xは積み木のドン！の縁の光
//params[0].wは1で木目を消してモデルの色だけで塗る
//params[1].xyzは縁の光の色

//面のUVに対する縁の光の太さと明るさ
static const float kEdgeWidth = 0.12f;
static const float kEdgeBrightness = 3.0f;

void Surface(SurfaceInput input, inout SurfaceOutput output)
{
    //木目を消してモデルの色だけにする
    output.color = lerp(output.color, input.instance.color.rgb, input.params[0].w);

    float glow = input.params[0].x;
    float3 glowColor = input.params[1].xyz;
    //面ごとに0から1のUVで一番近い辺までの距離で縁を出す
    float2 uv = frac(input.uv);
    float edge = min(min(uv.x, 1.0f - uv.x), min(uv.y, 1.0f - uv.y));
    float edgeMask = 1.0f - smoothstep(0.0f, kEdgeWidth, edge);
    output.emissive += glowColor * edgeMask * glow * kEdgeBrightness;
    output.color = lerp(output.color, glowColor, glow * 0.35f);
}
