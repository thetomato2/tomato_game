#include "tom_app.hh"
#include "tom_rng_nums.hh"
#include "tom_file_io.cc"
#include "tom_d3d11.cc"
// #include "tom_camera.cc"
#include "tom_input.cc"
#include "tom_sound.cc"
#include "tom_win32.cc"
#include "tom_game.cc"

namespace tom
{

global ID3D11Texture2D *g_tex;
global ID3D11Texture2D *g_staging_tex;
global ID3D11ShaderResourceView *g_sha_rsc_view;

fn void on_device_change(AppState *app)
{
    if (app->device_changed_delay > 0.5f) {
        app->device_changed_delay = 0.0f;
#if USE_DS5
        ZeroArray(app->input.ds5_context);
        app->input.ds5_cnt = DS5_init(app->input.ds5_context);
#endif
    }
}

fn void on_resize(AppState *app)
{
    auto d3d11 = &app->d3d11;

    f32 aspect = (f32)app->win32.win_dims.w / (f32)app->win32.win_dims.h;
    // app->proj = mat_proj_persp(aspect, app->fov, 1.0f, 1000.0f);
    d3d11_on_resize(d3d11, app->win32.win_dims);

    // NOTE(Derek): for some reason, and I can't figure out why becuase the render target in the
    // graphical debugger is correct, the backbuffer's width needs to be 32 bit aligned or the pitch
    // is off and it distorts the final image in the window. And its happening after present is
    // called so I have no fucking clue.
    i32 padding = 32;
    v2i aligned_dims;
    aligned_dims.w = app->win32.win_dims.w - app->win32.win_dims.w % padding;
    aligned_dims.h = app->win32.win_dims.h - app->win32.win_dims.h % padding;

    plat_free(app->back_buffer.buf);
    app->back_buffer.width  = aligned_dims.w;
    app->back_buffer.height = aligned_dims.h;
    app->back_buffer.buf    = plat_malloc(texture_get_size(app->back_buffer));

    g_tex->Release();
    g_staging_tex->Release();
    g_sha_rsc_view->Release();

    D3D11_TEXTURE2D_DESC tex_desc = { .Width      = (u32)app->back_buffer.width,
                                      .Height     = (u32)app->back_buffer.height,
                                      .MipLevels  = 1,
                                      .ArraySize  = 1,
                                      .Format     = DXGI_FORMAT_R8G8B8A8_UNORM,
                                      .SampleDesc = { .Count = 1, .Quality = 0 },
                                      .Usage      = D3D11_USAGE_DEFAULT,
                                      .BindFlags  = D3D11_BIND_SHADER_RESOURCE };

    d3d_Check(d3d11->device->CreateTexture2D(&tex_desc, NULL, &g_tex));

    tex_desc.Usage          = D3D11_USAGE_STAGING;
    tex_desc.BindFlags      = 0;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    d3d_Check(d3d11->device->CreateTexture2D(&tex_desc, NULL, &g_staging_tex));
    d3d_Check(d3d11->device->CreateShaderResourceView(g_tex, nullptr, &g_sha_rsc_view));
}

// ===============================================================================================
// #INIT
// ===============================================================================================
fn void app_init(AppState *app)
{
    auto d3d11 = &app->d3d11;
    app->game  = (GameState *)plat_malloc(sizeof(GameState));
    app->fov   = 1.0f;

    app->input = init_input();

    app->main_shader = d3d11_create_shader_prog(d3d11, L"..\\..\\shaders\\fs_blit.hlsl");
    v2i aligned_dims;
    // aligned_dims.w =;

    app->back_buffer.width  = app->win32.win_dims.w;
    app->back_buffer.height = app->win32.win_dims.h;
    app->back_buffer.type   = Texture::Type::R8G8B8A8;
    app->back_buffer.buf    = plat_malloc(texture_get_size(app->back_buffer));

    D3D11_TEXTURE2D_DESC tex_desc = { .Width      = (u32)app->back_buffer.width,
                                      .Height     = (u32)app->back_buffer.height,
                                      .ArraySize  = 1,
                                      .Format     = DXGI_FORMAT_R8G8B8A8_UNORM,
                                      .SampleDesc = { .Count = 1, .Quality = 0 },
                                      .Usage      = D3D11_USAGE_DEFAULT,
                                      .BindFlags  = D3D11_BIND_SHADER_RESOURCE };

    d3d_Check(d3d11->device->CreateTexture2D(&tex_desc, NULL, &g_tex));

    tex_desc.Usage          = D3D11_USAGE_STAGING;
    tex_desc.BindFlags      = 0;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    d3d_Check(d3d11->device->CreateTexture2D(&tex_desc, NULL, &g_staging_tex));
    d3d_Check(d3d11->device->CreateShaderResourceView(g_tex, nullptr, &g_sha_rsc_view));

    game_init(nullptr, app);
}

// ===============================================================================================
// #UPDATE
// ===============================================================================================
fn void app_update(AppState *app)
{
    auto d3d11 = &app->d3d11;
    auto kb    = &app->input.keyboard;

#if USE_DS5
    if (key_pressed(kb->j))
        app->input.ds5_state[0].trigger_effect_R.type = DS5_TriggerEffectType::calibrate;
    if (key_pressed(kb->k))
        app->input.ds5_state[0].trigger_effect_R.type = DS5_TriggerEffectType::none;

    if (key_pressed(kb->n))
        app->input.ds5_state[0].trigger_effect_L.type = DS5_TriggerEffectType::calibrate;
    if (key_pressed(kb->m))
        app->input.ds5_state[0].trigger_effect_L.type = DS5_TriggerEffectType::none;
#endif

    if (button_pressed(app->input.ds5_state[0].ps_logo)) {
        app->win32.running = false;
        return;
    }

    if (app->win32.focus || !app->suspend_lost_focus) {
        game_update_and_render(nullptr, app);
        if (key_pressed(kb->f8)) {
            local i32 x = 0;
            auto bb     = &app->back_buffer;
            if (!dir_exists("./out")) create_dir("./out");
            char c[2]            = { itos(x++), '\0' };
            ScopedPtr<char> path = str_cat("./out/", (const char *)&c[0], "_back_buffer.png");
            stbi_write_png(path.get(), bb->width, bb->height, 4, bb->buf,
                           texture_get_texel_size(bb->type));
        }

        D3D11_MAPPED_SUBRESOURCE map;
        d3d_Check(d3d11->context->Map(g_staging_tex, 0, D3D11_MAP_READ_WRITE, 0, &map));
        memcpy(map.pData, app->back_buffer.buf, texture_get_size(app->back_buffer));
        d3d11->context->Unmap(g_staging_tex, 0);
        d3d11->context->CopyResource(g_tex, g_staging_tex);
    }

    local bool once_only = false;
    if (!once_only) {
        d3d11->context->VSSetShader(app->main_shader.vs, nullptr, 0);
        d3d11->context->PSSetShader(app->main_shader.ps, nullptr, 0);

        d3d11->context->PSSetSamplers(0, 1, &d3d11->sampler_state);
        d3d11->context->OMSetDepthStencilState(d3d11->depth_stencil_state, 1);

        d3d11->context->IASetInputLayout(nullptr);
        d3d11->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    d3d11->context->PSSetShaderResources(0, 1, &g_sha_rsc_view);
    d3d11->context->OMSetRenderTargets(1, &d3d11->render_target_view, d3d11->depth_buf_view);

    once_only = true;

    i32 glyph_ind = 0;
    m4 model      = mat_identity();

#ifdef TOM_INTERNAL
    if (key_pressed(kb->f3)) d3d11_print_info_queue(d3d11);
    if (key_pressed(kb->f4))
        d3d11->d3d_debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
#endif

    d3d11->context->OMSetRenderTargets(1, &d3d11->render_target_view, d3d11->depth_buf_view);

    v4f clear_color = {};
    d3d11->context->ClearRenderTargetView(d3d11->render_target_view, clear_color.e);
    d3d11->context->ClearDepthStencilView(d3d11->depth_buf_view,
                                          D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    d3d11->context->Draw(3, 0);

    d3d_Check(d3d11->swap_chain->Present(1, 0));

}  // namespace tom

// ===============================================================================================
// #START
// ===============================================================================================
fn i32 app_start(HINSTANCE hinst)
{
    const TCHAR *icon_path = _T("..\\..\\data\\tomato.ico");
    auto icon              = (HICON)(LoadImage(NULL, icon_path, IMAGE_ICON, 0, 0,
                                               LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));

    create_console();
    auto cons_hwnd = GetConsoleWindow();
    Assert(cons_hwnd);
    SendMessage(cons_hwnd, WM_SETICON, NULL, (LPARAM)icon);
    SetConsoleColor(FG_WHITE);

    printf("Starting...\n");

    // set sleep to 1 ms intervals
    Assert(timeBeginPeriod(1) == TIMERR_NOERROR);

#if _CPPUWIND
    PrintWarning("Exceptions are enabled!\n");
#endif

    AppState app                      = {};
    app.target_fps                    = 60;
    app.target_secs_per_frame         = 1.0f / (f32)app.target_fps;
    app.sound.frames_of_audio_latency = (1.1f / 30.f) * (f32)app.target_fps;
    app.win32.icon                    = icon;

    DWORD exe_path_len = GetModuleFileNameA(NULL, app.exe_path, sizeof(app.exe_path));
    printf("exe path %s\n", app.exe_path);

    TCHAR cwd_buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd_buf);
    // _tprintf(TEXT("cwd %s\n"), cwd_buf);

    char cwd[MAX_PATH];
    get_cwd(cwd);
    printf("cwd: %s\n", cwd);

    char *p_ = &app.exe_path[exe_path_len];
    i32 i_   = (i32)exe_path_len;
    while (i_ > -1 && app.exe_path[i_] != '\\') {
        --i_;
    }

    TCHAR set_cwd_buf[MAX_PATH];
    for (int i = 0; i < i_; ++i) {
        set_cwd_buf[i] = app.exe_path[i];
    }
    set_cwd_buf[i_] = '\0';

    bool cwd_is_exe = true;
    int it_buf      = 0;
    while (cwd_buf[it_buf]) {
        if (cwd_buf[it_buf] != set_cwd_buf[it_buf]) cwd_is_exe = false;
        ++it_buf;
    }

    if (!str_equal(cwd_buf, set_cwd_buf)) {
        printf("cwd is not exe dir!\n");
        if (!SetCurrentDirectory(set_cwd_buf)) {
            printf("Failed to set cwd!");
        } else {
            GetCurrentDirectory(MAX_PATH, cwd_buf);
            _tprintf(TEXT("set cwd to %s\n"), cwd_buf);
        }
    }

    LARGE_INTEGER performance_query_result;
    QueryPerformanceFrequency(&performance_query_result);
    app.performance_counter_frequency = performance_query_result.QuadPart;

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)Terabytes((u64)2);
#else
    LPVOID base_address = 0;
#endif

    app.memory.permanent_storage_size = Megabytes(256);
    app.memory.transient_storage_size = Gigabytes(1);
    app.total_size = app.memory.permanent_storage_size + app.memory.transient_storage_size;
    // TODO: use large pages
    app.memory_block =
        VirtualAlloc(base_address, app.total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    app.memory.permanent_storage = app.memory_block;
    app.memory.transient_storage =
        (u8 *)app.memory.permanent_storage + app.memory.permanent_storage_size;

    app.win32.win_dims.w = 1280;
    app.win32.win_dims.h = 720;
    prevent_windows_DPI_scaling();
    create_window(&app.win32);
    app.dpi = (u32)GetDpiForWindow(app.win32.hwnd);
    SetCursorPos(app.win32.win_dims.w / 2, app.win32.win_dims.h / 2);

    d3d11_init(app.win32.hwnd, &app.d3d11);

    app.d3d11.viewport          = {};
    app.d3d11.viewport.Width    = (f32)app.win32.win_dims.w;
    app.d3d11.viewport.Height   = (f32)app.win32.win_dims.h;
    app.d3d11.viewport.TopLeftX = 0.0f;
    app.d3d11.viewport.TopLeftY = 0.0f;
    app.d3d11.viewport.MinDepth = 0.0f;
    app.d3d11.viewport.MaxDepth = 1.0f;

    i64 last_counter     = get_time();
    u64 last_cycle_count = __rdtsc();

    f32 delta_time = 0.0f;
    // NOTE: dummy thread context, for now
    ThreadContext thread {};

    app.win32.running      = true;
    app.suspend_lost_focus = false;

    app.time = 0.0f;
    app_init(&app);

    while (true) {
        ++app.frame_cnt;
        // printf("%llu\n",app.frame_cnt);
        //
        if (!app.win32.running) break;

        if (app.win32.resize) {
            on_resize(&app);
            app.win32.resize = false;
        }

        app.device_changed_delay += app.dt;
        if (app.win32.device_change) {
            on_device_change(&app);
            app.win32.device_change = false;
        }

        app.target_secs_per_frame = 1.0f / (f32)app.target_fps;

        app.win32.ms_scroll = 0;
        process_pending_messages(&app.win32);
        // do_controller_input(*old_input, *new_input, hwnd);
        // NOTE: this isn't calculated and needs to be for a variable framerate
        // app.dt            = app.target_frames_per_second;
        app.dt = app.ms_frame / 1000.0f;
        app.time += app.dt;
        local u64 frame_cnt = 0;
        local f32 one_sec   = 0.0f;
        ++frame_cnt;
        one_sec += app.ms_frame;
        if (one_sec > 1000.0f) {
            one_sec -= 1000.0f;
            app.fps   = (i32)frame_cnt;
            frame_cnt = 0;
        }

        do_input(&app.input, app.win32.hwnd, app.win32.ms_scroll);

        app_update(&app);

        f32 work_secs_avg = 0.0f;
        for (u32 i = 0; i < CountOf(app.work_secs); ++i) {
            work_secs_avg += (f32)app.work_secs[i];
        }
        work_secs_avg /= (f32)CountOf(app.work_secs);

        auto work_counter = get_time();
        f32 work_seconds_elapsed =
            get_seconds_elapsed(last_counter, work_counter, app.performance_counter_frequency);
        app.work_secs[app.work_ind++] = work_seconds_elapsed;
        if (app.work_ind == CountOf(app.work_secs)) app.work_ind = 0;

        // NOTE: win32 Sleep() ony guarantees the MINIMUN amound of time the thread will
        // sleep. its not the best solution for steady FPS in a game, but as a temporary soultion to
        // get around this I will just ask for a slightly early wakeup and spin the CPU until the
        // next frame
        f32 seconds_elapsed_for_frame = work_seconds_elapsed;
        if (seconds_elapsed_for_frame < app.target_secs_per_frame) {
            // minus a ms
            f32 sleep_ms = 1000.0f * (app.target_secs_per_frame - seconds_elapsed_for_frame) - 1;
            if (sleep_ms > 0) {
                Sleep((DWORD)sleep_ms);
            }
            // spin the cpu for the remainder
            f32 test_seconds_elapsed_for_frame =
                get_seconds_elapsed(last_counter, get_time(), app.performance_counter_frequency);
            while (seconds_elapsed_for_frame < app.target_secs_per_frame) {
                seconds_elapsed_for_frame = get_seconds_elapsed(last_counter, get_time(),
                                                                app.performance_counter_frequency);
            }
        } else {
            PrintWarning("Missed frame timing!");
        }

        auto end_counter = get_time();
        app.ms_frame     = 1000.f * get_seconds_elapsed(last_counter, end_counter,
                                                        app.performance_counter_frequency);

        last_counter = end_counter;

        u64 end_cycle_count = __rdtsc();
        u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
        last_cycle_count    = end_cycle_count;
    }

    ReleaseDC(app.win32.hwnd, app.win32.hdc);
    DestroyWindow(app.win32.hwnd);

    return 0;
}

}  // namespace tom
