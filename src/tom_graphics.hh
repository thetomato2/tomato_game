
#define MAX_TEXTURE_COUNT        16384
#define MAX_GLYPH_COUNT          16384
#define MAX_RENDER_COMMAND_COUNT 1028

#define SIZE_OF_GLYPH_INSTANCE_IN_BYTES (sizeof(float) * 14)

#define GLYPH_INSTANCE_DATA_TOTAL_SIZE_IN_BYTES MAX_GLYPH_COUNT* SIZE_OF_GLYPH_INSTANCE_IN_BYTES

#define SIZE_OF_TEXTURE_INSTANCE_IN_BYTES (sizeof(float) * 14)
#define TEXTURE_INSTANCE_DATA_TOTAL_SIZE_IN_BYTES \
    MAX_TEXTURE_COUNT* SIZE_OF_TEXTURE_INSTANCE_IN_BYTES

struct TOM_GUID
{
    u32 Data1;
    u16 Data2;
    u16 Data3;
    u8 Data4[8];
};

// #define TOM_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) const GUID FAR name

// TOM_DEFINE_GUID(DXGI_DEBUG_ALL, 0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf,
//                 0xda, 0x8);

#ifdef TOM_INTERNAL
    #define d3d_Check(x)                                              \
        {                                                             \
            if (FAILED(x)) {                                          \
                ScopedPtr<char> hr_err = d3d_error_code(x);           \
                PrintRed("ERROR: ");                                  \
                printf("DirectX Check Failed! - %s\n", hr_err.get()); \
                d3d_print_info_queue(gfx);                            \
                InvalidCodePath;                                      \
            }                                                         \
        }
#else
    #define D3D_CHECK(x)
#endif

namespace tom
{

struct BackBuffer
{
    void* buf;
    i32 width, height, pitch, byt_per_pix;
};

struct d3d_Constants
{
    m4 transform;
    m4 projection;
    v3f light_v3;
};

struct ShaderProg
{
    ID3DBlob* vs_blob;
    ID3DBlob* ps_blob;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
};

struct GfxState
{
    ID3D11Device1* device;
    ID3D11DeviceContext1* context;
    IDXGISwapChain1* swap_chain;
    ID3D11InfoQueue* info_queue;
    ID3D11RenderTargetView* render_target_view;
    ID3D11DepthStencilView* depth_buf_view;
    ID3D11DepthStencilState* depth_stencil_state;
    ID3D11RasterizerState1* rasterizer_state;
    ID3D11SamplerState* sampler_state;
    ID3D11BlendState* blend_state;
    D3D11_VIEWPORT viewport;

#ifdef TOM_INTERNAL
    ID3D11Debug* d3d_debug;
    IDXGIInfoQueue* dxgi_info_queue;
#endif
};

// pos, col
union Vertex
{
    struct
    {
        v4f pos;
        v2f uv;
    };
    f32 e[6];
};

struct Quad
{
    Vertex e[4];
};

}  // namespace tom