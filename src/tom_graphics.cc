namespace tom
{

function void d3d_print_info_queue(GfxState* gfx)
{
    szt msg_cnt = gfx->info_queue->GetNumStoredMessages();

    for (szt i = 0; i < msg_cnt; ++i) {
        szt msg_sz;
        gfx->info_queue->GetMessage(i, nullptr, &msg_sz);
        auto msg = (D3D11_MESSAGE*)plat_malloc(msg_sz);  // TODO: create an arena for this
        // TODO: basic logging system and/or colors
        if (SUCCEEDED(gfx->info_queue->GetMessage(i, msg, &msg_sz))) {
            printf("D3D:%d->", msg->ID);
            switch (msg->Severity) {
                case D3D11_MESSAGE_SEVERITY_MESSAGE: PrintMessage(msg->pDescription); break;
                case D3D11_MESSAGE_SEVERITY_INFO: PrintInfo(msg->pDescription); break;
                case D3D11_MESSAGE_SEVERITY_WARNING: PrintWarning(msg->pDescription); break;
                case D3D11_MESSAGE_SEVERITY_CORRUPTION: PrintCorruption(msg->pDescription); break;
                case D3D11_MESSAGE_SEVERITY_ERROR: PrintError(msg->pDescription); break;
                default: printf("%s\n", msg->pDescription); break;
            }
        }
        plat_free(msg);
    }
    gfx->info_queue->ClearStoredMessages();
    msg_cnt = gfx->dxgi_info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);

    for (szt i = 0; i < msg_cnt; ++i) {
        szt msg_sz;
        gfx->dxgi_info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &msg_sz);
        auto msg = (DXGI_INFO_QUEUE_MESSAGE*)plat_malloc(msg_sz);  // TODO: create an arena for this
        if (SUCCEEDED(gfx->dxgi_info_queue->GetMessage(DXGI_DEBUG_ALL, i, msg, &msg_sz))) {
            printf("DXGI->");
            switch (msg->Severity) {
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
                    PrintMessage(msg->pDescription);
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO: PrintInfo(msg->pDescription); break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
                    PrintWarning(msg->pDescription);
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
                    PrintCorruption(msg->pDescription);
                    break;
                case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR: PrintError(msg->pDescription); break;
                default: printf("%s\n", msg->pDescription); break;
            }
        }
        plat_free(msg);
    }
    gfx->dxgi_info_queue->ClearStoredMessages(DXGI_DEBUG_ALL);
}

function void d3d_on_resize(GfxState* gfx, r2i win_dims)
{
    ID3D11RenderTargetView* null_views[] = { nullptr };
    gfx->context->OMSetRenderTargets(CountOf(null_views), null_views, nullptr);
    gfx->render_target_view->Release();
    gfx->depth_buf_view->Release();
    // gfx->context->ClearState();
    gfx->context->Flush();

    // d3d_Check(gfx->swap_chain->ResizeBuffers(0, win_dims.x1, win_dims.y1, DXGI_FORMAT_UNKNOWN,
    //                                          DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

    d3d_Check(gfx->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN,
                                             DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

    ID3D11Texture2D* frame_buf;
    d3d_Check(gfx->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buf));

    d3d_Check(gfx->device->CreateRenderTargetView(frame_buf, nullptr, &gfx->render_target_view));

    D3D11_TEXTURE2D_DESC depth_buf_desc;
    frame_buf->GetDesc(&depth_buf_desc);  // base on framebuffer properties
    depth_buf_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buf_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depth_buf;
    d3d_Check(gfx->device->CreateTexture2D(&depth_buf_desc, nullptr, &depth_buf));

    d3d_Check(gfx->device->CreateDepthStencilView(depth_buf, nullptr, &gfx->depth_buf_view));
    gfx->context->OMSetRenderTargets(1, &gfx->render_target_view, gfx->depth_buf_view);

    depth_buf->Release();
    frame_buf->Release();

    gfx->viewport          = {};
    gfx->viewport.Width    = (f32)win_dims.x1;
    gfx->viewport.Height   = (f32)win_dims.y1;
    gfx->viewport.TopLeftX = 0.0f;
    gfx->viewport.TopLeftY = 0.0f;
    gfx->viewport.MinDepth = 0.0f;
    gfx->viewport.MaxDepth = 1.0f;
    gfx->context->RSSetViewports(1, &gfx->viewport);
}

function ShaderProg d3d_create_shader_prog(GfxState* gfx, LPCWSTR path)
{
    ShaderProg result;
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
        d3d_Check(gfx->device->CreateVertexShader(result.vs_blob->GetBufferPointer(),
                                                  result.vs_blob->GetBufferSize(), nullptr,
                                                  &result.vs));
    } else {
        print_error("vertex", err_msg_blob);
        err_msg_blob->Release();
        InvalidCodePath;
    }

    if (SUCCEEDED(D3DCompileFromFile(path, nullptr, nullptr, "ps_main", "ps_5_0", flags, 0,
                                     &result.ps_blob, &err_msg_blob))) {
        d3d_Check(gfx->device->CreatePixelShader(result.ps_blob->GetBufferPointer(),
                                                 result.ps_blob->GetBufferSize(), nullptr,
                                                 &result.ps));
    } else {
        print_error("pixel", err_msg_blob);
        err_msg_blob->Release();
        InvalidCodePath;
    }

    return result;
}

function void d3d_init(HWND hwnd, GfxState* gfx)
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
            gfx->info_queue = info_queue;
        } else {
            PrintError("Failed to create D3D11 Info Queue!");
            InvalidCodePath;
        }
        gfx->d3d_debug = d3d_debug;

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
            gfx->dxgi_info_queue = dxgi_info_queue;
        } else {
            PrintError("Failed to crate DXGI Info Queue!");
            InvalidCodePath;
        }
    }

#endif

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
        // .Width  = 0,  // use window width
        // .Height = 0,  // use window height
        //.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
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
    d3d_Check(device->CreateBlendState(&blend_desc, &gfx->blend_state));

    float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    UINT sample_mask      = 0xffffffff;
    device_context->OMSetBlendState(gfx->blend_state, blend_factor, sample_mask);

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
    gfx->sampler_state = sampler_state;

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc { .DepthEnable    = TRUE,
                                                  .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
                                                  // NOTE: turned off depth culling
                                                  .DepthFunc     = D3D11_COMPARISON_ALWAYS,
                                                  .StencilEnable = FALSE };

    ID3D11DepthStencilState* depth_stencil_state;
    d3d_Check(device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state));

    gfx->device              = device;
    gfx->context             = device_context;
    gfx->swap_chain          = swap_chain;
    gfx->render_target_view  = render_target_view;
    gfx->depth_buf_view      = depth_buf_view;
    gfx->rasterizer_state    = rasterizer_state;
    gfx->depth_stencil_state = depth_stencil_state;
}

function void clear_buffer(BackBuffer* buffer, Color_argb color)
{
    byt* row = (byt*)buffer->buf;
    for (i32 y = 0; y < buffer->height; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = 0; x < buffer->width; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer->pitch;
    }
}

function void draw_rect(BackBuffer* buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, Color color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    byt* row = (byt*)buffer->buf + min_x * buffer->byt_per_pix + min_y * buffer->pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = min_x; x < max_x; ++x) {
            Color_argb dest_col { (u32)*pixel };
            Color_argb blended_col;
            blended_col.a = 0xff;

            f32 alpha = (f32)color.a / 255.0f;

            blended_col.r = (u8)((1.0f - alpha) * (f32)dest_col.r + alpha * (f32)color.r);
            blended_col.g = (u8)((1.0f - alpha) * (f32)dest_col.g + alpha * (f32)color.g);
            blended_col.b = (u8)((1.0f - alpha) * (f32)dest_col.b + alpha * (f32)color.b);

            *pixel++ = blended_col.argb;
        }
        row += buffer->pitch;
    }
}

function void draw_rect(BackBuffer* buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, v3f color)
{
    draw_rect(buffer, min_x, min_y, max_x, max_y, v3_to_color(color));
}

function void draw_rect(BackBuffer* buffer, r2i rect, Color color)
{
    draw_rect(buffer, rect.x0, rect.y0, rect.x1, rect.y1, color);
}
function void draw_rect(BackBuffer* buffer, r2f rect, Color color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, color);
}

function void draw_rect(BackBuffer* buffer, r2i rect, v3f color)
{
    draw_rect(buffer, rect.x0, rect.y0, rect.x1, rect.y1, v3_to_color(color));
}
function void draw_rect(BackBuffer* buffer, r2f rect, v3f color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, v3_to_color(color));
}

function void draw_rect_outline(BackBuffer* buffer, const f32 min_x_f32, const f32 min_y_f32,
                                const f32 max_x_f32, f32 max_y_f32, i32 thickness, Color color)
{
    i32 min_x = round_f32_to_i32(min_x_f32);
    i32 min_y = round_f32_to_i32(min_y_f32);
    i32 max_x = round_f32_to_i32(max_x_f32);
    i32 max_y = round_f32_to_i32(max_y_f32);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    byt* row = (byt*)buffer->buf + min_x * buffer->byt_per_pix + min_y * buffer->pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = min_x; x < max_x; ++x) {
            if (x <= min_x + thickness || x >= max_x - thickness - 1 || y <= min_y + thickness ||
                y >= max_y - thickness - 1) {
                *pixel = color.rgba;
            }
            ++pixel;
        }
        row += buffer->pitch;
    }
}

function void push_piece(EntityVisiblePieceGroup* group, argb_img* img, v2f mid_p, f32 z_offset,
                         f32 alpha = 1.f)
{
    Assert(group->piece_cnt < CountOf(group->pieces));
    EntityVisiblePiece* piece = group->pieces + group->piece_cnt++;
    piece->img                = img;
    piece->mid_p              = mid_p;
    piece->z                  = z_offset;
    piece->alpha              = alpha;
}

function void push_piece(EntityVisiblePieceGroup* group, f32 width, f32 height, Color color,
                         v2f mid_p, f32 z_offset, f32 alpha = 1.0f)
{
    Assert(group->piece_cnt < CountOf(group->pieces));
    EntityVisiblePiece* piece = group->pieces + group->piece_cnt++;
    piece->img                = nullptr;
    piece->mid_p              = mid_p;
    piece->z                  = z_offset;
    piece->alpha              = alpha;
    piece->rect.x0            = mid_p.x - width / 2;
    piece->rect.y0            = mid_p.y - height / 2;
    piece->rect.x1            = mid_p.x + width / 2;
    piece->rect.y1            = mid_p.y + height / 2;
    piece->color              = color;
}

function argb_img load_argb_or_default(ThreadContext* thread, argb_img* default_img,
                                       const char* file_name, const char* name = nullptr)
{
    const char* argb_dir     = "../../data/";
    ScopedPtr<char> img_path = str_cat(argb_dir, file_name, ".argb");

    auto file = read_file(img_path.get());
    if (file.size == 0) {
        PrintMessage(str_fmt("%s - not found!", img_path.get()));
        Assert(default_img);
        return *default_img;
    }

    argb_img result;

    if (file.size != 0) {
        if (name)
            result.name = name;
        else
            result.name = file_name;

        // TODO: why didn't I just cast this?
        u32* file_ptr    = (u32*)file.buf;
        result.width     = *file_ptr++;
        result.height    = *file_ptr++;
        result.size      = *file_ptr++;
        result.pixel_ptr = file_ptr;
    }

    return result;
}

function void draw_argb(BackBuffer* buffer, argb_img& img, v2f pos)
{
    i32 min_y = round_f32_to_i32(pos.y - ((f32)img.height) / 2.0f);
    i32 min_x = round_f32_to_i32(pos.x - ((f32)img.width) / 2.0f);
    i32 max_y = round_f32_to_i32(pos.y + ((f32)img.height) / 2.0f);
    i32 max_x = round_f32_to_i32(pos.x + ((f32)img.width) / 2.0f);

    i32 x_offset_left = 0, x_offset_right = 0, y_offset = 0;

    if (min_y < 0) {
        y_offset = min_y * -1;
        min_y    = 0;
    }
    if (min_x < 0) {
        x_offset_left = min_x * -1;
        min_x         = 0;
    }
    if (max_x > buffer->width) {
        x_offset_right = max_x - buffer->width;
        max_x          = buffer->width;
    }
    if (max_y > buffer->height) max_y = buffer->height;

    u32* source = img.pixel_ptr + (y_offset * img.width);
    byt* row    = (byt*)buffer->buf + min_x * buffer->byt_per_pix + min_y * buffer->pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32* dest = (u32*)row;
        source += x_offset_left;
        for (i32 x = min_x; x < max_x; ++x) {
            Color_argb dest_col { (u32)*dest };
            Color_argb source_col { (u32)*source };
            Color_argb blended_col;
            blended_col.a = 0xff;

            f32 alpha = (f32)source_col.a / 255.0f;

            blended_col.r = (u8)((1.0f - alpha) * (f32)dest_col.r + alpha * (f32)source_col.b);
            blended_col.g = (u8)((1.0f - alpha) * (f32)dest_col.g + alpha * (f32)source_col.g);
            blended_col.b = (u8)((1.0f - alpha) * (f32)dest_col.b + alpha * (f32)source_col.r);

            *dest = blended_col.argb;

            ++dest, ++source;
        }
        source += x_offset_right;
        row += buffer->pitch;
    }
}

}  // namespace tom