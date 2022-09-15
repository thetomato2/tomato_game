#ifndef TOM_D3D11_HH
#define TOM_D3D11_HH

#include "tom_core.hh"
#include "tom_dx_error.hh"

namespace tom
{

#ifdef TOM_INTERNAL
    #define d3d_Check(x)                                              \
        {                                                             \
            if (FAILED(x)) {                                          \
                ScopedPtr<char> hr_err = d3d_error_code(x);           \
                PRINT_RED("ERROR: ");                                  \
                printf("DirectX Check Failed! - %s\n", hr_err.get()); \
                d3d11_print_info_queue(d3d11);                        \
                INVALID_CODE_PATH;                                      \
            }                                                         \
        }
#else
    #define d3d_Check(x)
#endif

struct D3D11ShaderProg
{
    ID3DBlob *vs_blob;
    ID3DBlob *ps_blob;
    ID3D11VertexShader *vs;
    ID3D11PixelShader *ps;
};

struct D3D11State
{
    ID3D11Device1 *device;
    ID3D11DeviceContext1 *context;
    IDXGISwapChain1 *swap_chain;
    ID3D11InfoQueue *info_queue;
    ID3D11RenderTargetView *render_target_view;
    ID3D11DepthStencilView *depth_buf_view;
    ID3D11DepthStencilState *depth_stencil_state;
    ID3D11RasterizerState1 *rasterizer_state;
    ID3D11SamplerState *sampler_state;
    ID3D11BlendState *blend_state;
    D3D11_VIEWPORT viewport;

#ifdef TOM_INTERNAL
    ID3D11Debug *d3d_debug;
    IDXGIInfoQueue *dxgi_info_queue;
#endif
};

////////////////////////////////////////////////////////////////////////////////////////////////
// #DECLARES
void d3d11_init(HWND hwnd, D3D11State *d3d11);
void d3d11_print_info_queue(D3D11State* d3d11);
void d3d11_on_resize(D3D11State* d3d11, v2i win_dims);
D3D11ShaderProg d3d11_create_shader_prog(D3D11State *d3d11, LPCWSTR path);

}  // namespace tom

#endif
