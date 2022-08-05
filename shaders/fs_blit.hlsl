// Makes a "fullscreen triangle" and renders a defualt-uv texture

SamplerState samp : register(s0);
Texture2D<float4> Texture : register(t0);

struct Interpolators{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

Interpolators vs_main(uint vI : SV_VertexId)
{
    Interpolators output;

    // We use the 'big triangle' optimization so you only Draw 3 verticies instead of 4.
    float2 texcoord = float2((vI << 1) & 2, vI & 2);
    output.TexCoord = texcoord;
    output.Position = float4(texcoord.x * 2 - 1, -texcoord.y * 2 + 1, 0, 1);

    return output;
};

float4 ps_main(Interpolators In) : SV_Target0
{
    return Texture.Sample(samp, In.TexCoord);
}
