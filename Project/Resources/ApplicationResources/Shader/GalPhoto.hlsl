//ギャルピースの瞬間に淡いフラッシュとハート入りの写真の枠を重ねる

//画面の縦横比に合わせて角の丸い枠までの距離を求める
float RoundedFrameDistance(float2 position, float2 halfSize, float radius)
{
    float2 edge = abs(position) - (halfSize - radius);
    return length(max(edge, 0.0f)) + min(max(edge.x, edge.y), 0.0f) - radius;
}

//丸い上半分と先が細い下半分をつないでハートを描く
float HeartDistance(float2 position)
{
    position.x = abs(position.x);
    if (position.y + position.x > 1.0f)
    {
        return length(position - float2(0.25f, 0.75f)) - 0.353553f;
    }

    float2 tip = position - float2(0.0f, 1.0f);
    float2 edge = position - 0.5f * max(position.x + position.y, 0.0f);
    return sqrt(min(dot(tip, tip), dot(edge, edge))) * sign(position.x - position.y);
}

//指定した位置に細いハートのシールを描く
float HeartSticker(float2 position, float2 center, float size, float angle, float pixelSize)
{
    float2 offset = (position - center) / size;
    float s;
    float c;
    sincos(angle, s, c);
    float2 turned = float2(c * offset.x - s * offset.y, s * offset.x + c * offset.y);
    float distanceToEdge = abs(HeartDistance(float2(turned.x, 0.55f - turned.y))) * size;
    return 1.0f - smoothstep(size * 0.044f, size * 0.044f + pixelSize, distanceToEdge);
}

float4 PostEffect(PostEffectInput input)
{
    float elapsed = input.params[0].x;
    float4 scene = SampleScene(input.uv);
    float flash = 1.0f - saturate(elapsed / 0.11f);
    float frameFade = 1.0f - smoothstep(0.47f, 0.86f, elapsed);
    float aspect = input.screenSize.x / max(input.screenSize.y, 1.0f);
    float pixelSize = 1.5f / max(input.screenSize.y, 1.0f);
    float2 position = (input.uv - 0.5f) * float2(aspect, 1.0f);

    //淡いピンクの縁と白い内側の線で写真の枠を作る
    float2 halfSize = float2(aspect * 0.5f, 0.5f) - 0.027f;
    float distanceToFrame = RoundedFrameDistance(position, halfSize, 0.075f);
    float edgeTint = smoothstep(-0.19f, 0.023f, distanceToFrame);
    float border = 1.0f - smoothstep(0.003f, 0.003f + pixelSize, abs(distanceToFrame));
    float innerLine = 1.0f - smoothstep(pixelSize, pixelSize * 2.0f, abs(distanceToFrame + 0.016f));

    //四隅だけにハートを添えて中央の表情を見せる
    float2 corner = halfSize - float2(0.14f, 0.10f);
    float stickers = HeartSticker(position, float2(-corner.x, -corner.y), 0.105f, -0.28f, pixelSize);
    stickers = max(stickers, HeartSticker(position, float2(-corner.x + 0.10f, -corner.y + 0.055f), 0.058f, 0.24f, pixelSize));
    stickers = max(stickers, HeartSticker(position, float2(corner.x, -corner.y), 0.078f, 0.22f, pixelSize));
    stickers = max(stickers, HeartSticker(position, float2(corner.x, corner.y), 0.105f, 0.3f, pixelSize));
    stickers = max(stickers, HeartSticker(position, float2(-corner.x, corner.y), 0.065f, -0.23f, pixelSize));

    float3 pink = float3(1.0f, 0.66f, 0.86f);
    float3 cream = float3(1.0f, 0.91f, 0.97f);
    float3 color = lerp(scene.rgb, pink, edgeTint * frameFade * 0.39f);
    color = lerp(color, cream, saturate(border * 0.92f + innerLine * 0.57f) * frameFade);
    color = lerp(color, cream, stickers * frameFade * 0.96f);
    color = lerp(color, cream, flash * flash * 0.48f);
    return float4(color, scene.a);
}
