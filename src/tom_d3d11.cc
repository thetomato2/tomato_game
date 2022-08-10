namespace tom
{

fn void d3d11_print_info_queue(D3D11State* d3d11)
{
    szt msg_cnt = d3d11->info_queue->GetNumStoredMessages();

    for (szt i = 0; i < msg_cnt; ++i) {
        szt msg_sz;
        d3d11->info_queue->GetMessage(i, nullptr, &msg_sz);
        auto msg = (D3D11_MESSAGE*)plat_malloc(msg_sz);  // TODO: create an arena for this
        // TODO: basic logging system and/or colors
        if (SUCCEEDED(d3d11->info_queue->GetMessage(i, msg, &msg_sz))) {
            printf("D3D:->");
            switch (msg->Severity) {
                case D3D11_MESSAGE_SEVERITY_MESSAGE:
                    PrintMessage(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case D3D11_MESSAGE_SEVERITY_INFO:
                    PrintInfo(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case D3D11_MESSAGE_SEVERITY_WARNING:
                    PrintWarning(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case D3D11_MESSAGE_SEVERITY_CORRUPTION:
                    PrintCorruption(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case D3D11_MESSAGE_SEVERITY_ERROR:
                    PrintError(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                default: printf("%s ID:%d\n", msg->pDescription, msg->ID); break;
            }
        }
        plat_free(msg);
    }
    d3d11->info_queue->ClearStoredMessages();
    msg_cnt = d3d11->dxgi_info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);

    for (szt i = 0; i < msg_cnt; ++i) {
        szt msg_sz;
        d3d11->dxgi_info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &msg_sz);
        auto msg = (DXGI_INFO_QUEUE_MESSAGE*)plat_malloc(msg_sz);  // TODO: create an arena for this
        if (SUCCEEDED(d3d11->dxgi_info_queue->GetMessage(DXGI_DEBUG_ALL, i, msg, &msg_sz))) {
            printf("DXGI->");
            switch (msg->Severity) {
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
                    PrintMessage(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
                    PrintInfo(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
                    PrintWarning(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
                    PrintCorruption(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
                    PrintError(str_fmt("%s ID:%d", msg->pDescription, msg->ID));
                    break;
                default: printf("%s ID:%d\n", msg->pDescription, msg->ID); break;
            }
        }
        plat_free(msg);
    }
    d3d11->dxgi_info_queue->ClearStoredMessages(DXGI_DEBUG_ALL);
}

fn void d3d11_on_resize(D3D11State* d3d11, v2i win_dims)
{
    ID3D11RenderTargetView* null_views[] = { nullptr };
    d3d11->context->OMSetRenderTargets(CountOf(null_views), null_views, nullptr);
    d3d11->render_target_view->Release();
    d3d11->depth_buf_view->Release();
    // d3d11->context->ClearState();
    d3d11->context->Flush();

    // d3d_Check(state->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN,
    //                                          DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));
    d3d_Check(d3d11->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN,
                                               DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

    ID3D11Texture2D* frame_buf;
    d3d_Check(d3d11->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buf));

    d3d_Check(
        d3d11->device->CreateRenderTargetView(frame_buf, nullptr, &d3d11->render_target_view));

    D3D11_TEXTURE2D_DESC depth_buf_desc;
    frame_buf->GetDesc(&depth_buf_desc);  // base on framebuffer properties
    depth_buf_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buf_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depth_buf;
    d3d_Check(d3d11->device->CreateTexture2D(&depth_buf_desc, nullptr, &depth_buf));

    d3d_Check(d3d11->device->CreateDepthStencilView(depth_buf, nullptr, &d3d11->depth_buf_view));
    d3d11->context->OMSetRenderTargets(1, &d3d11->render_target_view, d3d11->depth_buf_view);

    depth_buf->Release();
    frame_buf->Release();

    d3d11->viewport          = {};
    d3d11->viewport.Width    = (f32)win_dims.w;
    d3d11->viewport.Height   = (f32)win_dims.h;
    d3d11->viewport.TopLeftX = 0.0f;
    d3d11->viewport.TopLeftY = 0.0f;
    d3d11->viewport.MinDepth = 0.0f;
    d3d11->viewport.MaxDepth = 1.0f;
    d3d11->context->RSSetViewports(1, &d3d11->viewport);
}

fn D3D11ShaderProg d3d11_create_shader_prog(D3D11State* d3d11, LPCWSTR path)
{
    D3D11ShaderProg result;
    u32 flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef TOM_INTERNAL
    flags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* err_msg_blob;

    auto print_error = [](const char* shader_type, ID3DBlob* err_msg) {
        printf("==========================================================\n");
        PrintRed("ERROR: ");
        printf(
            "%s shader failed to "
            "compile!\n%s\n==========================================================\n",
            shader_type, (char*)err_msg->GetBufferPointer());
    };

    if (SUCCEEDED(D3DCompileFromFile(path, nullptr, nullptr, "vs_main", "vs_5_0", flags, 0,
                                     &result.vs_blob, &err_msg_blob))) {
        d3d_Check(d3d11->device->CreateVertexShader(result.vs_blob->GetBufferPointer(),
                                                    result.vs_blob->GetBufferSize(), nullptr,
                                                    &result.vs));
    } else {
        print_error("vertex", err_msg_blob);
        err_msg_blob->Release();
        InvalidCodePath;
    }

    if (SUCCEEDED(D3DCompileFromFile(path, nullptr, nullptr, "ps_main", "ps_5_0", flags, 0,
                                     &result.ps_blob, &err_msg_blob))) {
        d3d_Check(d3d11->device->CreatePixelShader(result.ps_blob->GetBufferPointer(),
                                                   result.ps_blob->GetBufferSize(), nullptr,
                                                   &result.ps));
    } else {
        print_error("pixel", err_msg_blob);
        err_msg_blob->Release();
        InvalidCodePath;
    }

    return result;
}

fn void d3d11_init(HWND hwnd, D3D11State* d3d11)
{
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                           D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

    u32 device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef TOM_INTERNAL
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* base_device;
    ID3D11DeviceContext* base_device_context;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags,
                                 feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION,
                                 &base_device, nullptr, &base_device_context))) {
        printf("ERROR!-> Failed to create base D3D11 Base Device!");
        InvalidCodePath;
    }

    ID3D11Device1* device;
    if (FAILED(base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)&device))) {
        printf("ERROR!-> Failed to create D3D11 Device!");
        InvalidCodePath;
    }
    ID3D11DeviceContext1* device_context;
    if (FAILED(base_device_context->QueryInterface(__uuidof(ID3D11DeviceContext1),
                                                   (void**)&device_context))) {
        printf("ERROR!-> Failed to create D3D11 Device Context!");
        InvalidCodePath;
    }

    IDXGIDevice1* dxgi_device;
    if (FAILED(device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device))) {
        printf("ERROR!-> Failed to create D3D11 DXGI Device!");
        InvalidCodePath;
    }
    IDXGIAdapter* dxgi_adapter;
    if (FAILED(dxgi_device->GetAdapter(&dxgi_adapter))) {
        printf("ERROR!-> Failed to create D3D11 DXGI adapter!");
        InvalidCodePath;
    }
    IDXGIFactory2* dxgi_factory;
    if (FAILED(dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory))) {
        printf("ERROR!-> Failed to create D3D11 DXGI Factory!");
        InvalidCodePath;
    }

#ifdef TOM_INTERNAL
    ID3D11Debug* d3d_debug;
    if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3d_debug))) {
        ID3D11InfoQueue* info_queue;
        if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue))) {
            info_queue->SetBreakOnSeverity((D3D11_MESSAGE_SEVERITY_CORRUPTION), true);
            info_queue->SetBreakOnSeverity((D3D11_MESSAGE_SEVERITY_ERROR), true);
            // TODO: do I need to fix the OMSETRENDERTARGET thing?
            D3D11_MESSAGE_ID hide[] { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                                      D3D11_MESSAGE_ID_OMSETRENDERTARGETS_UNBINDDELETINGOBJECT };

            D3D11_INFO_QUEUE_FILTER filter {};
            filter.DenyList.NumIDs  = (u32)CountOf(hide);
            filter.DenyList.pIDList = hide;
            info_queue->PushEmptyStorageFilter();
            info_queue->AddStorageFilterEntries(&filter);
            d3d11->info_queue = info_queue;
        } else {
            PrintError("Failed to create D3D11 Info Queue!");
            InvalidCodePath;
        }
        d3d11->d3d_debug = d3d_debug;

    } else {
        printf("ERROR!-> Failed to create D3D11 debug context!");
        InvalidCodePath;
    }

    HMODULE dxgidebug = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (dxgidebug) {
        IDXGIInfoQueue* dxgi_info_queue;
        auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
            reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));
        if (SUCCEEDED(dxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), (void**)&dxgi_info_queue))) {
            // dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL,
            //                                     DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            // dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL,
            //                                     DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION,
            //                                     true);
            d3d11->dxgi_info_queue = dxgi_info_queue;
        } else {
            PrintError("Failed to crate DXGI Info Queue!");
            InvalidCodePath;
        }
    }

#endif

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
        // .Width  = 0,  // use window width
        // .Height = 0,  // use window height
        // .Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        .Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc  = { .Count = 1, .Quality = 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .Scaling     = DXGI_SCALING_STRETCH,
        .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        // .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags     = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
    };

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC full_screen_desc = {};
    full_screen_desc.RefreshRate.Numerator           = 144;
    full_screen_desc.RefreshRate.Denominator         = 1;
    full_screen_desc.Windowed                        = true;

    IDXGISwapChain1* swap_chain;
    d3d_Check(dxgi_factory->CreateSwapChainForHwnd(device, hwnd, &swap_chain_desc,
                                                   &full_screen_desc, nullptr, &swap_chain));
    dxgi_factory->Release();

    ID3D11Texture2D* frame_buf;
    d3d_Check(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buf));

    ID3D11RenderTargetView* render_target_view;
    d3d_Check(device->CreateRenderTargetView(frame_buf, nullptr, &render_target_view));

    D3D11_TEXTURE2D_DESC depth_buf_desc;
    frame_buf->GetDesc(&depth_buf_desc);  // base on framebuffer properties
    depth_buf_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buf_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depth_buf;
    d3d_Check(device->CreateTexture2D(&depth_buf_desc, nullptr, &depth_buf));

    ID3D11DepthStencilView* depth_buf_view;
    d3d_Check(device->CreateDepthStencilView(depth_buf, nullptr, &depth_buf_view));

    depth_buf->Release();
    frame_buf->Release();

    D3D11_RENDER_TARGET_BLEND_DESC rtbd = { .BlendEnable           = true,
                                            .SrcBlend              = D3D11_BLEND_SRC_ALPHA,
                                            .DestBlend             = D3D11_BLEND_INV_SRC_ALPHA,
                                            .BlendOp               = D3D11_BLEND_OP_ADD,
                                            .SrcBlendAlpha         = D3D11_BLEND_SRC_ALPHA,
                                            .DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA,
                                            .BlendOpAlpha          = D3D11_BLEND_OP_ADD,
                                            .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL };

    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.RenderTarget[0]  = rtbd;
    d3d_Check(device->CreateBlendState(&blend_desc, &d3d11->blend_state));

    float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    UINT sample_mask      = 0xffffffff;
    device_context->OMSetBlendState(d3d11->blend_state, blend_factor, sample_mask);

    D3D11_RASTERIZER_DESC1 rasterizer_desc { .FillMode = D3D11_FILL_SOLID,
                                             .CullMode = D3D11_CULL_BACK };

    ID3D11RasterizerState1* rasterizer_state;
    d3d_Check(device->CreateRasterizerState1(&rasterizer_desc, &rasterizer_state));

    D3D11_SAMPLER_DESC sampler_desc { .Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT,
                                      .AddressU       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .AddressV       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .AddressW       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .ComparisonFunc = D3D11_COMPARISON_NEVER };

    ID3D11SamplerState* sampler_state;
    d3d_Check(device->CreateSamplerState(&sampler_desc, &sampler_state));
    d3d11->sampler_state = sampler_state;

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc { .DepthEnable    = TRUE,
                                                  .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
                                                  // NOTE: turned off depth culling
                                                  .DepthFunc     = D3D11_COMPARISON_ALWAYS,
                                                  .StencilEnable = FALSE };

    ID3D11DepthStencilState* depth_stencil_state;
    d3d_Check(device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state));

    d3d11->device              = device;
    d3d11->context             = device_context;
    d3d11->swap_chain          = swap_chain;
    d3d11->render_target_view  = render_target_view;
    d3d11->depth_buf_view      = depth_buf_view;
    d3d11->rasterizer_state    = rasterizer_state;
    d3d11->depth_stencil_state = depth_stencil_state;
}

}  // namespace tom