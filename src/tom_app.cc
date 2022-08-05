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

global ID3D11Texture2D* g_tex;
global ID3D11Texture2D* g_staging_tex;
global ID3D11ShaderResourceView* g_sha_rsc_view;

fn void on_device_change(AppState* state)
{
    if (state->device_changed_delay > 0.5f) {
        state->device_changed_delay = 0.0f;
#if USE_DS5
        ZeroArray(state->input.ds5_context);
        state->input.ds5_cnt = DS5_init(state->input.ds5_context);
#endif
    }
}

fn void on_resize(AppState* state)
{
    auto d3d11 = &state->d3d11;

    f32 aspect = (f32)state->win32.win_dims.w / (f32)state->win32.win_dims.h;
    // state->proj = mat_proj_persp(aspect, state->fov, 1.0f, 1000.0f);
    d3d11_on_resize(d3d11, state->win32.win_dims);

    // NOTE(Derek): for some reason, and I can't figure out why becuase the render target in the
    // graphical debugger is correct, the backbuffer's width needs to be 32 bit aligned or the pitch
    // is off and it distorts the final image in the window. And its happening after present is
    // called so I have no fucking clue.
    i32 padding = 32;
    v2i aligned_dims;
    aligned_dims.w = state->win32.win_dims.w - state->win32.win_dims.w % padding;
    aligned_dims.h = state->win32.win_dims.h - state->win32.win_dims.h % padding;

    plat_free(state->back_buffer.buf);
    state->back_buffer.width  = aligned_dims.w;
    state->back_buffer.height = aligned_dims.h;
    state->back_buffer.pitch  = state->back_buffer.width * state->back_buffer.byt_per_pix;
    state->back_buffer.buf = plat_malloc(state->back_buffer.byt_per_pix * state->back_buffer.width *
                                         state->back_buffer.height);

    g_tex->Release();
    g_staging_tex->Release();
    g_sha_rsc_view->Release();

    D3D11_TEXTURE2D_DESC tex_desc = { .Width      = (u32)state->back_buffer.width,
                                      .Height     = (u32)state->back_buffer.height,
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
fn void app_init(AppState* state)
{
    auto d3d11  = &state->d3d11;
    state->game = (GameState*)plat_malloc(sizeof(GameState));
    state->fov  = 1.0f;

    state->input = init_input();

    state->main_shader = d3d11_create_shader_prog(d3d11, L"..\\..\\shaders\\fs_blit.hlsl");
    v2i aligned_dims;
    // aligned_dims.w =;

    state->back_buffer.width       = state->win32.win_dims.w;
    state->back_buffer.height      = state->win32.win_dims.h;
    state->back_buffer.byt_per_pix = 4;
    state->back_buffer.pitch       = state->back_buffer.width * state->back_buffer.byt_per_pix;
    state->back_buffer.buf = plat_malloc(state->back_buffer.byt_per_pix * state->back_buffer.width *
                                         state->back_buffer.height);

    D3D11_TEXTURE2D_DESC tex_desc = { .Width      = (u32)state->back_buffer.width,
                                      .Height     = (u32)state->back_buffer.height,
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

    game_init(nullptr, state);
}

// ===============================================================================================
// #UPDATE
// ===============================================================================================
fn void app_update(AppState* state)
{
    auto d3d11 = &state->d3d11;
    auto kb    = &state->input.keyboard;

#if USE_DS5
    if (key_pressed(kb->j))
        state->input.ds5_state[0].trigger_effect_R.type = DS5_TriggerEffectType::calibrate;
    if (key_pressed(kb->k))
        state->input.ds5_state[0].trigger_effect_R.type = DS5_TriggerEffectType::none;

    if (key_pressed(kb->n))
        state->input.ds5_state[0].trigger_effect_L.type = DS5_TriggerEffectType::calibrate;
    if (key_pressed(kb->m))
        state->input.ds5_state[0].trigger_effect_L.type = DS5_TriggerEffectType::none;
#endif

    if (button_pressed(state->input.ds5_state[0].ps_logo)) {
        state->win32.running = false;
        return;
    }

    if (key_pressed(kb->f7)) {
        v2i win_rect = get_window_dimensions(state->win32.hwnd);
        printf("b: %d, %d\n", state->back_buffer.width, state->back_buffer.height);
        printf("w: %d, %d\n", win_rect.w, win_rect.h);
    }

    if (state->win32.focus) {
        game_update_and_render(nullptr, state);
        if (key_pressed(kb->f8)) {
            local i32 x = 0;
            auto bb     = &state->back_buffer;
            if (!dir_exists("./out")) create_dir("./out");
            char c[2]            = { itos(x++), '\0' };
            ScopedPtr<char> path = str_cat("./out/", (const char*)&c[0], "_back_buffer.png");
            stbi_write_png(path.get(), bb->width, bb->height, 4, bb->buf, bb->pitch);
        }

        D3D11_MAPPED_SUBRESOURCE map;
        d3d_Check(d3d11->context->Map(g_staging_tex, 0, D3D11_MAP_READ_WRITE, 0, &map));
        memcpy(map.pData, state->back_buffer.buf,
               state->back_buffer.pitch * state->back_buffer.height);
        d3d11->context->Unmap(g_staging_tex, 0);
        d3d11->context->CopyResource(g_tex, g_staging_tex);
    }

    local bool once_only = false;
    if (!once_only) {
        d3d11->context->VSSetShader(state->main_shader.vs, nullptr, 0);
        d3d11->context->PSSetShader(state->main_shader.ps, nullptr, 0);

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
    const TCHAR* icon_path = _T("..\\..\\data\\tomato.ico");
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

    AppState state                      = {};
    state.target_fps                    = 30;
    state.target_secs_per_frame         = 1.0f / (f32)state.target_fps;
    state.sound.frames_of_audio_latency = (1.1f / 30.f) * (f32)state.target_fps;
    state.win32.icon                    = icon;

    DWORD exe_path_len = GetModuleFileNameA(NULL, state.exe_path, sizeof(state.exe_path));
    printf("exe path %s\n", state.exe_path);

    TCHAR cwd_buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd_buf);
    // _tprintf(TEXT("cwd %s\n"), cwd_buf);

    char cwd[MAX_PATH];
    get_cwd(cwd);
    printf("cwd: %s\n", cwd);

    char* p_ = &state.exe_path[exe_path_len];
    i32 i_   = (i32)exe_path_len;
    while (i_ > -1 && state.exe_path[i_] != '\\') {
        --i_;
    }

    TCHAR set_cwd_buf[MAX_PATH];
    for (int i = 0; i < i_; ++i) {
        set_cwd_buf[i] = state.exe_path[i];
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
    state.performance_counter_frequency = performance_query_result.QuadPart;

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)Terabytes((u64)2);
#else
    LPVOID base_address = 0;
#endif

    state.memory.permanent_storage_size = Megabytes(256);
    state.memory.transient_storage_size = Gigabytes(1);
    state.total_size = state.memory.permanent_storage_size + state.memory.transient_storage_size;
    // TODO: use large pages
    state.memory_block =
        VirtualAlloc(base_address, state.total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    state.memory.permanent_storage = state.memory_block;
    state.memory.transient_storage =
        (u8*)state.memory.permanent_storage + state.memory.permanent_storage_size;

    state.win32.win_dims.w = 1291;
    state.win32.win_dims.h = 720;
    prevent_windows_DPI_scaling();
    create_window(&state.win32);
    state.dpi = (u32)GetDpiForWindow(state.win32.hwnd);
    SetCursorPos(state.win32.win_dims.w / 2, state.win32.win_dims.h / 2);

    d3d11_init(state.win32.hwnd, &state.d3d11);

    state.d3d11.viewport          = {};
    state.d3d11.viewport.Width    = (f32)state.win32.win_dims.w;
    state.d3d11.viewport.Height   = (f32)state.win32.win_dims.h;
    state.d3d11.viewport.TopLeftX = 0.0f;
    state.d3d11.viewport.TopLeftY = 0.0f;
    state.d3d11.viewport.MinDepth = 0.0f;
    state.d3d11.viewport.MaxDepth = 1.0f;

    i64 last_counter     = get_time();
    u64 last_cycle_count = __rdtsc();

    f32 delta_time = 0.0f;
    // NOTE: dummy thread context, for now
    ThreadContext thread {};

    state.win32.running = true;

    state.time = 0.0f;
    app_init(&state);

    while (true) {
        ++state.frame_cnt;
        // printf("%llu\n",state.frame_cnt);
        //
        if (!state.win32.running) break;

        if (state.win32.resize) {
            on_resize(&state);
            state.win32.resize = false;
        }

        state.device_changed_delay += state.dt;
        if (state.win32.device_change) {
            on_device_change(&state);
            state.win32.device_change = false;
        }

        state.target_secs_per_frame = 1.0f / (f32)state.target_fps;

        state.win32.ms_scroll = 0;
        process_pending_messages(&state.win32);
        // do_controller_input(*old_input, *new_input, hwnd);
        // NOTE: this isn't calculated and needs to be for a variable framerate
        // state.dt            = state.target_frames_per_second;
        state.dt = state.ms_frame / 1000.0f;
        state.time += state.dt;
        local u64 frame_cnt = 0;
        local f32 one_sec   = 0.0f;
        ++frame_cnt;
        one_sec += state.ms_frame;
        if (one_sec > 1000.0f) {
            one_sec -= 1000.0f;
            state.fps = (i32)frame_cnt;
            frame_cnt = 0;
        }

        do_input(&state.input, state.win32.hwnd, state.win32.ms_scroll);

        app_update(&state);

        f32 work_secs_avg = 0.0f;
        for (u32 i = 0; i < CountOf(state.work_secs); ++i) {
            work_secs_avg += (f32)state.work_secs[i];
        }
        work_secs_avg /= (f32)CountOf(state.work_secs);

        auto work_counter = get_time();
        f32 work_seconds_elapsed =
            get_seconds_elapsed(last_counter, work_counter, state.performance_counter_frequency);
        state.work_secs[state.work_ind++] = work_seconds_elapsed;
        if (state.work_ind == CountOf(state.work_secs)) state.work_ind = 0;

        // NOTE: win32 Sleep() ony guarantees the MINIMUN amound of time the thread will
        // sleep. its not the best solution for steady FPS in a game, but as a temporary soultion to
        // get around this I will just ask for a slightly early wakeup and spin the CPU until the
        // next frame
        f32 seconds_elapsed_for_frame = work_seconds_elapsed;
        if (seconds_elapsed_for_frame < state.target_secs_per_frame) {
            // minus a ms
            f32 sleep_ms = 1000.0f * (state.target_secs_per_frame - seconds_elapsed_for_frame) - 1;
            if (sleep_ms > 0) {
                Sleep((DWORD)sleep_ms);
            }
            // spin the cpu for the remainder
            f32 test_seconds_elapsed_for_frame =
                get_seconds_elapsed(last_counter, get_time(), state.performance_counter_frequency);
            while (seconds_elapsed_for_frame < state.target_secs_per_frame) {
                seconds_elapsed_for_frame = get_seconds_elapsed(
                    last_counter, get_time(), state.performance_counter_frequency);
            }
        } else {
            PrintWarning("Missed frame timing!");
        }

        auto end_counter = get_time();
        state.ms_frame   = 1000.f * get_seconds_elapsed(last_counter, end_counter,
                                                        state.performance_counter_frequency);

        last_counter = end_counter;

        u64 end_cycle_count = __rdtsc();
        u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
        last_cycle_count    = end_cycle_count;
    }

    ReleaseDC(state.win32.hwnd, state.win32.hdc);
    DestroyWindow(state.win32.hwnd);

    return 0;
}

}  // namespace tom