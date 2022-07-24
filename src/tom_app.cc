#include "tom_app.hh"
#include "tom_rng_nums.hh"
#include "tom_file_io.cc"
#include "tom_graphics.cc"
// #include "tom_camera.cc"
#include "tom_input.cc"
#include "tom_sound.cc"
#include "tom_win32.cc"
#include "tom_world.cc"
#include "tom_entity.cc"
#include "tom_sim_region.cc"
#include "tom_game.cc"

namespace tom
{

global ID3D11Texture2D* g_tex;
global ID3D11Texture2D* g_staging_tex;
global ID3D11ShaderResourceView* g_sha_rsc_view;

function void on_resize(AppState* state)
{
    f32 aspect = (f32)state->win32.win_dims.x1 / (f32)state->win32.win_dims.y1;
    // state->proj = mat_proj_persp(aspect, state->fov, 1.0f, 1000.0f);
    d3d_on_resize(&state->gfx, state->win32.win_dims);
    state->proj = mat_proj_ortho(aspect);

    plat_free(state->back_buffer.buf);
    state->back_buffer.width  = state->win32.win_dims.x1;
    state->back_buffer.height = state->win32.win_dims.y1;
    state->back_buffer.pitch  = state->back_buffer.width * state->back_buffer.byt_per_pix;
    state->back_buffer.buf = plat_malloc(state->back_buffer.byt_per_pix * state->back_buffer.width *
                                         state->back_buffer.height);
}

function void app_init(AppState* state)
{
    auto gfx    = &state->gfx;
    state->game = (GameState*)plat_malloc(sizeof(GameState));

    state->fov = 1.0f;
    // state->clear_color      = { 0.086f, 0.086f, 0.086f, 1.0f };
    state->clear_color = { 0.047f, 0.047f, 0.047f, 1.0f };
    // state->text_color      = { 0.686f, 0.686f, 0.686f, 1.0f };
    state->text_color = v4_init(color_u32_to_v3f(0xafafafff), 1.0f);
    state->vars.unit  = 1.0f;
    state->view       = mat_identity();

    state->input = init_input();

    state->main_shader = d3d_create_shader_prog(gfx, L"..\\..\\shaders\\main.hlsl");

    state->back_buffer.width       = state->win32.win_dims.x1;
    state->back_buffer.height      = state->win32.win_dims.y1;
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

    d3d_Check(gfx->device->CreateTexture2D(&tex_desc, NULL, &g_tex));

    tex_desc.Usage          = D3D11_USAGE_STAGING;
    tex_desc.BindFlags      = 0;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    d3d_Check(gfx->device->CreateTexture2D(&tex_desc, NULL, &g_staging_tex));

    d3d_Check(gfx->device->CreateShaderResourceView(g_tex, nullptr, &g_sha_rsc_view));

    game_init(nullptr, state);
}

function void app_update(AppState* state)
{
    auto gfx = &state->gfx;
    auto kb  = &state->input.keyboard;

    if (key_pressed(kb->j))
        state->input.ds5_state[0].trigger_effect_R.type = DS5_TriggerEffectType::calibrate;
    if (key_pressed(kb->k))
        state->input.ds5_state[0].trigger_effect_R.type = DS5_TriggerEffectType::none;

    if (key_pressed(kb->n))
        state->input.ds5_state[0].trigger_effect_L.type = DS5_TriggerEffectType::calibrate;
    if (key_pressed(kb->m))
        state->input.ds5_state[0].trigger_effect_L.type = DS5_TriggerEffectType::none;

    local u32 stride     = sizeof(Vertex);
    local u32 offset     = 0;
    local bool once_only = false;

    game_update_and_render(nullptr, state);

    D3D11_MAPPED_SUBRESOURCE map;
    d3d_Check(gfx->context->Map(g_staging_tex, 0, D3D11_MAP_READ_WRITE, 0, &map));
    memcpy(map.pData, state->back_buffer.buf, state->back_buffer.pitch * state->back_buffer.height);
    gfx->context->Unmap(g_staging_tex, 0);
    gfx->context->CopyResource(g_tex, g_staging_tex);

    gfx->context->RSSetState(gfx->rasterizer_state);
    gfx->context->RSSetViewports(1, &gfx->viewport);

    gfx->context->VSSetShader(state->main_shader.vs, nullptr, 0);
    gfx->context->PSSetShader(state->main_shader.ps, nullptr, 0);

    gfx->context->PSSetShaderResources(0, 1, &g_sha_rsc_view);
    gfx->context->PSSetSamplers(0, 1, &gfx->sampler_state);

    gfx->context->OMSetRenderTargets(1, &gfx->render_target_view, gfx->depth_buf_view);
    gfx->context->OMSetDepthStencilState(gfx->depth_stencil_state, 1);

    gfx->context->IASetInputLayout(nullptr);
    gfx->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    once_only = true;

    i32 glyph_ind = 0;
    m4 model      = mat_identity();

    state->wvp = state->view * state->proj;

#ifdef TOM_INTERNAL
    if (key_pressed(kb->f3)) d3d_print_info_queue(gfx);
    if (key_pressed(kb->f4))
        gfx->d3d_debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
#endif

    gfx->context->OMSetRenderTargets(1, &gfx->render_target_view, gfx->depth_buf_view);
    gfx->context->ClearRenderTargetView(gfx->render_target_view, state->clear_color.e);
    gfx->context->ClearDepthStencilView(gfx->depth_buf_view,
                                        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    gfx->context->Draw(3, 0);

    d3d_Check(gfx->swap_chain->Present(1, 0));

}  // namespace tom

function i32 app_start(HINSTANCE hinst)
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

#if _CPPUWIND
    PrintWarning("Exceptions are enabled!\n");
#endif

    AppState state                      = {};
    state.game_update_hertz             = 60;
    state.target_frames_per_second      = 1.0f / (f32)state.game_update_hertz;
    state.target_fps                    = 60;
    state.sound.frames_of_audio_latency = (1.1f / 30.f) * (f32)state.game_update_hertz;
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

    state.win32.win_dims.x1 = 1600;
    state.win32.win_dims.y1 = 900;

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)Terabytes((u64)2);
#else
    LPVOID base_address = 0;
#endif

    state.memory.permanent_storage_size = Megabytes(256);
    state.memory.transient_storage_size = Megabytes(256);
    state.total_size = state.memory.permanent_storage_size + state.memory.transient_storage_size;
    // TODO: use large pages
    state.memory_block =
        VirtualAlloc(base_address, state.total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    state.memory.permanent_storage = state.memory_block;
    state.memory.transient_storage =
        (u8*)state.memory.permanent_storage + state.memory.permanent_storage_size;

    prevent_windows_DPI_scaling();
    create_window(&state.win32);
    state.dpi = (u32)GetDpiForWindow(state.win32.hwnd);
    SetCursorPos(state.win32.win_dims.x1 / 2, state.win32.win_dims.y1 / 2);

    d3d_init(state.win32.hwnd, &state.gfx);

    state.gfx.viewport          = {};
    state.gfx.viewport.Width    = (f32)state.win32.win_dims.x1;
    state.gfx.viewport.Height   = (f32)state.win32.win_dims.y1;
    state.gfx.viewport.TopLeftX = 0.0f;
    state.gfx.viewport.TopLeftY = 0.0f;
    state.gfx.viewport.MinDepth = 0.0f;
    state.gfx.viewport.MaxDepth = 1.0f;

    i64 last_counter     = get_time();
    u64 last_cycle_count = __rdtsc();

    f32 delta_time = 0.0f;
    // NOTE: dummy thread context, for now
    ThreadContext thread {};

    state.win32.running = true;

    app_init(&state);

    while (true) {
        ++state.frame_cnt;
        // printf("%llu\n",state.frame_cnt);
        if (!state.win32.running) break;
        if (state.win32.resize) {
            on_resize(&state);
            state.win32.resize = false;
        }

        state.target_frames_per_second = 1.0f / (f32)state.target_fps;

        state.win32.ms_scroll = 0;
        process_pending_messages(&state.win32);
        // do_controller_input(*old_input, *new_input, hwnd);
        // NOTE: this isn't calculated and needs to be for a variable framerate
        // state.dt            = state.target_frames_per_second;
        state.dt            = state.ms_frame / 1000.0f;
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

        // clock stuffs
        auto work_counter = get_time();
        f32 work_seconds_elapsed =
            get_seconds_elapsed(last_counter, work_counter, state.performance_counter_frequency);
        state.work_secs[state.work_ind++] = work_seconds_elapsed;
        if (state.work_ind == CountOf(state.work_secs)) state.work_ind = 0;

        bool is_sleep_granular        = false;
        f32 seconds_elapsed_for_frame = work_seconds_elapsed;
        if (seconds_elapsed_for_frame < state.target_frames_per_second) {
            if (is_sleep_granular) {
                auto sleepMs =
                    (DWORD)(1000.f * (state.target_frames_per_second - seconds_elapsed_for_frame));
                if (sleepMs > 0) {
                    ::Sleep(sleepMs);
                }
            }
            f32 test_seconds_elapsed_for_frame =
                get_seconds_elapsed(last_counter, get_time(), state.performance_counter_frequency);
            while (seconds_elapsed_for_frame < state.target_frames_per_second) {
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
    UnregisterClass(state.win32.cls_name, NULL);

    return 0;
}

}  // namespace tom