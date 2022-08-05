namespace tom
{

//! this is a roundabout way of extracting a method out of a header...
#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef XINPUT_GET_STATE(xinput_get_state);
XINPUT_GET_STATE(_xinput_get_state)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
xinput_get_state* XInputGetState_ { _xinput_get_state };

#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef XINPUT_SET_STATE(xinput_set_state);
XINPUT_SET_STATE(_xinput_set_state)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
xinput_set_state* XInputSetState_ { _xinput_set_state };

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

fn void load_Xinput()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary) {
        HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if (XInputLibrary) {
        XInputGetState = (xinput_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinput_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        printf("ERROR->failed to load XInput!\n");
    }
}

fn void process_keyboard_message(Button& new_state, const b32 is_down)
{
    if (new_state.ended_down != (is_down != 0)) {
        new_state.ended_down = is_down;
        ++new_state.half_transition_cnt;
    }
}

fn void process_Xinput_digital_button(DWORD Xinput_button_state_, Button& old_state_,
                                            DWORD button_bit_, Button& new_state)
{
    new_state.ended_down = ((Xinput_button_state_ & button_bit_) == button_bit_);
    if (new_state.ended_down && old_state_.ended_down)
        new_state.half_transition_cnt = ++old_state_.half_transition_cnt;
}

// NOTE: mod = Shift
fn char win32key_to_char_mod(Win32Key key)
{
    switch (key) {
        case space: return ' ';
        case d1: return '!';
        case d2: return '@';
        case d3: return '#';
        case d4: return '$';
        case d5: return '%';
        case d6: return '^';
        case d7: return '&';
        case d8: return '*';
        case d9: return '(';
        case d0: return ')';
        case a: return 'A';
        case b: return 'B';
        case c: return 'C';
        case d: return 'D';
        case e: return 'E';
        case f: return 'F';
        case g: return 'G';
        case h: return 'H';
        case i: return 'I';
        case j: return 'J';
        case k: return 'K';
        case l: return 'L';
        case m: return 'M';
        case n: return 'N';
        case o: return 'O';
        case p: return 'P';
        case q: return 'Q';
        case r: return 'R';
        case s: return 'S';
        case t: return 'T';
        case u: return 'U';
        case v: return 'V';
        case w: return 'W';
        case x: return 'X';
        case y: return 'Y';
        case z: return 'Z';
        case add: return '=';
        case enter: return '\n';
        case subtract: return '_';
        case semicolon: return ':';
        case comma: return '>';
        case period: return '<';
        case quotes: return '\"';
        case open_brackets: return '{';
        case close_brackets: return '}';
        case tilde: return '~';
        case backslash: return '|';
        case question: return '?';
    }

    return '\0';
}

fn char win32key_to_char(Win32Key key)
{
    switch (key) {
        case space: return ' ';
        case d1: return '1';
        case d2: return '2';
        case d3: return '3';
        case d4: return '4';
        case d5: return '5';
        case d6: return '6';
        case d7: return '7';
        case d8: return '8';
        case d9: return '9';
        case d0: return '0';
        case a: return 'a';
        case b: return 'b';
        case c: return 'c';
        case d: return 'd';
        case e: return 'e';
        case f: return 'f';
        case g: return 'g';
        case h: return 'h';
        case i: return 'i';
        case j: return 'j';
        case k: return 'k';
        case l: return 'l';
        case m: return 'm';
        case n: return 'n';
        case o: return 'o';
        case p: return 'p';
        case q: return 'q';
        case r: return 'r';
        case s: return 's';
        case t: return 't';
        case u: return 'u';
        case v: return 'v';
        case w: return 'w';
        case x: return 'x';
        case y: return 'y';
        case z: return 'z';
        case add: return '+';
        case enter: return '\n';
        case subtract: return '-';
        case semicolon: return ';';
        case comma: return ',';
        case period: return '.';
        case quotes: return '\'';
        case open_brackets: return '[';
        case close_brackets: return ']';
        case tilde: return '`';
        case backslash: return '\\';
        case question: return '/';
    }

    return '\0';
}

#define ASSIGN_WIN32_KEY(key) result.keyboard.##key.name = Win32Key::##key;

fn Input init_input()
{
    Input result = {};

    ASSIGN_WIN32_KEY(space);
    ASSIGN_WIN32_KEY(d1);
    ASSIGN_WIN32_KEY(d2);
    ASSIGN_WIN32_KEY(d3);
    ASSIGN_WIN32_KEY(d4);
    ASSIGN_WIN32_KEY(d5);
    ASSIGN_WIN32_KEY(d6);
    ASSIGN_WIN32_KEY(d7);
    ASSIGN_WIN32_KEY(d8);
    ASSIGN_WIN32_KEY(d9);
    ASSIGN_WIN32_KEY(d0);
    ASSIGN_WIN32_KEY(a);
    ASSIGN_WIN32_KEY(b);
    ASSIGN_WIN32_KEY(c);
    ASSIGN_WIN32_KEY(d);
    ASSIGN_WIN32_KEY(e);
    ASSIGN_WIN32_KEY(f);
    ASSIGN_WIN32_KEY(g);
    ASSIGN_WIN32_KEY(h);
    ASSIGN_WIN32_KEY(i);
    ASSIGN_WIN32_KEY(j);
    ASSIGN_WIN32_KEY(k);
    ASSIGN_WIN32_KEY(l);
    ASSIGN_WIN32_KEY(m);
    ASSIGN_WIN32_KEY(n);
    ASSIGN_WIN32_KEY(o);
    ASSIGN_WIN32_KEY(p);
    ASSIGN_WIN32_KEY(q);
    ASSIGN_WIN32_KEY(r);
    ASSIGN_WIN32_KEY(s);
    ASSIGN_WIN32_KEY(t);
    ASSIGN_WIN32_KEY(u);
    ASSIGN_WIN32_KEY(v);
    ASSIGN_WIN32_KEY(w);
    ASSIGN_WIN32_KEY(x);
    ASSIGN_WIN32_KEY(y);
    ASSIGN_WIN32_KEY(z);
    ASSIGN_WIN32_KEY(enter);
    ASSIGN_WIN32_KEY(escape);
    ASSIGN_WIN32_KEY(left_alt);
    ASSIGN_WIN32_KEY(left_shift);
    ASSIGN_WIN32_KEY(left_control);
    ASSIGN_WIN32_KEY(tab);
    ASSIGN_WIN32_KEY(back);
    ASSIGN_WIN32_KEY(add);
    ASSIGN_WIN32_KEY(subtract);
    ASSIGN_WIN32_KEY(semicolon);
    ASSIGN_WIN32_KEY(comma);
    ASSIGN_WIN32_KEY(period);
    ASSIGN_WIN32_KEY(quotes);
    ASSIGN_WIN32_KEY(open_brackets);
    ASSIGN_WIN32_KEY(close_brackets);
    ASSIGN_WIN32_KEY(tilde);
    ASSIGN_WIN32_KEY(backslash);
    ASSIGN_WIN32_KEY(question);
    ASSIGN_WIN32_KEY(pipe);
    ASSIGN_WIN32_KEY(left);
    ASSIGN_WIN32_KEY(up);
    ASSIGN_WIN32_KEY(right);
    ASSIGN_WIN32_KEY(down);
    ASSIGN_WIN32_KEY(f1);
    ASSIGN_WIN32_KEY(f2);
    ASSIGN_WIN32_KEY(f3);
    ASSIGN_WIN32_KEY(f4);
    ASSIGN_WIN32_KEY(f5);
    ASSIGN_WIN32_KEY(f6);
    ASSIGN_WIN32_KEY(f7);
    ASSIGN_WIN32_KEY(f8);
    ASSIGN_WIN32_KEY(f9);
    ASSIGN_WIN32_KEY(f10);
    ASSIGN_WIN32_KEY(f11);
    ASSIGN_WIN32_KEY(f12);

    // Daulsense (PS5) controllers

    result.ds5_cnt = DS5_init(result.ds5_context);

    for (u32 i = 0; i < result.ds5_cnt; ++i) {
        result.ds5_state[i].trigger_effect_L.type                 = DS5_TriggerEffectType::none;
        result.ds5_state[i].trigger_effect_L.continuous.force     = 0xff;
        result.ds5_state[i].trigger_effect_L.continuous.start_pos = 0x00;

        result.ds5_state[i].trigger_effect_R.type                 = DS5_TriggerEffectType::none;
        result.ds5_state[i].trigger_effect_R.continuous.force     = 0xff;
        result.ds5_state[i].trigger_effect_R.continuous.start_pos = 0x00;
    }

    return result;
}

fn void do_input(Input* input, HWND hwnd, i32 ms_scroll)
{
    // mouse cursor
    POINT mouse_point;
    GetCursorPos(&mouse_point);
    ScreenToClient(hwnd, &mouse_point);
    input->mouse.pos_last.x = input->mouse.pos.x;
    input->mouse.pos_last.y = input->mouse.pos.y;
    input->mouse.pos.x      = (f32)mouse_point.x;
    input->mouse.pos.y      = (f32)mouse_point.y;

    input->mouse.scroll = ms_scroll;

    for (szt key = 0; key < CountOf(input->mouse.buttons); ++key) {
        if (input->mouse.buttons[key].half_transition_cnt > 0 &&
            input->mouse.buttons[key].ended_down == 0)
            input->mouse.buttons[key].half_transition_cnt = 0;
    }

    // mouse buttons
    process_keyboard_message(input->mouse.buttons[0], ::GetKeyState(VK_LBUTTON) & (1 << 15));
    process_keyboard_message(input->mouse.buttons[1], ::GetKeyState(VK_RBUTTON) & (1 << 15));
    process_keyboard_message(input->mouse.buttons[2], ::GetKeyState(VK_MBUTTON) & (1 << 15));

    for (szt key = 0; key < CountOf(input->keyboard.keys); ++key) {
        if (input->keyboard.keys[key].half_transition_cnt > 0 &&
            input->keyboard.keys[key].ended_down == 0)
            input->keyboard.keys[key].half_transition_cnt = 0;
    }

    for (u32 i = 0; i < Keyboard::key_cnt; ++i) {
        process_keyboard_message(input->keyboard.keys[i],
                                 ::GetKeyState(input->keyboard.keys[i].name) & (1 << 15));
    }

#if USE_DS5
    for (u32 i = 0; i < input->ds5_cnt; ++i) {
        ds5_get_input(&input->ds5_context[i], &input->ds5_state[i]);
        ds5_push_output(&input->ds5_context[i], &input->ds5_state[i]);
    }
#endif

#if 0
    // controller
    // poll the input device
    i32 max_controller_count = XUSER_MAX_COUNT;
    if (max_controller_count > 4) {
        max_controller_count = 4;
    }

    for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
        controller &old_controller = old_input->controllers[controller_index];
        controller &new_controller = new_input->controllers[controller_index];

        XINPUT_STATE controller_state;
        if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
            //! the controller is plugged in
            XINPUT_GAMEPAD &pad = controller_state.Gamepad;

            new_controller.is_connected = true;

            // NOTE: this is hardcoded for convenience
            // newController.isAnalog = oldController->isAnalog;
            new_controller.is_analog = true;

            //  no rmal stick input
            auto normalize = [](SHORT val) -> f32 {
                if (val < 0)
                    return scast(f32, val) / 32768.0f;
                else
                    return scast(f32, val) / 32767.0f;
            };

            f32 stick_left_x  = normalize(pad.sThumbLX);
            f32 stick_left_y  = normalize(pad.sThumbLY);
            f32 stick_right_x = normalize(pad.sThumbRX);
            f32 stick_right_y = normalize(pad.sThumbRY);

            new_controller.min_x = new_controller.max_x = new_controller.end_left_stick_x =
                stick_left_x;
            new_controller.min_y = new_controller.max_y = new_controller.end_left_stick_y =
                stick_left_y;

            for (szt button = 0; button < CountOf(old_controller.buttons); ++button) {
                if (!old_controller.buttons[button].ended_down)
                    old_controller.buttons[button].half_transition_cnt = 0;
            }

            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_up,
                                          XINPUT_GAMEPAD_DPAD_UP, new_controller.dpad_up);
            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_right,
                                          XINPUT_GAMEPAD_DPAD_RIGHT, new_controller.dpad_right);
            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_down,
                                          XINPUT_GAMEPAD_DPAD_DOWN, new_controller.dpad_down);
            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_left,
                                          XINPUT_GAMEPAD_DPAD_LEFT, new_controller.dpad_left);
            process_Xinput_digital_button(pad.wButtons, old_controller.a, XINPUT_GAMEPAD_A,
                                          new_controller.a);
            process_Xinput_digital_button(pad.wButtons, old_controller.b, XINPUT_GAMEPAD_B,
                                          new_controller.b);
            process_Xinput_digital_button(pad.wButtons, old_controller.x, XINPUT_GAMEPAD_X,
                                          new_controller.x);
            process_Xinput_digital_button(pad.wButtons, old_controller.y, XINPUT_GAMEPAD_Y,
                                          new_controller.y);
            process_Xinput_digital_button(pad.wButtons, old_controller.rb,
                                          XINPUT_GAMEPAD_RIGHT_SHOULDER, new_controller.rb);
            process_Xinput_digital_button(pad.wButtons, old_controller.lb,
                                          XINPUT_GAMEPAD_LEFT_SHOULDER, new_controller.lb);
            process_Xinput_digital_button(pad.wButtons, old_controller.back, XINPUT_GAMEPAD_BACK,
                                          new_controller.back);
            process_Xinput_digital_button(pad.wButtons, old_controller.start, XINPUT_GAMEPAD_START,
                                          new_controller.start);

            // NOTE: Not currently used
            // float dpadStickRX = pad.sThumbRX;
            // float dpadStickRY = pad.sThumbRY;
            // bool dPadStart = (pad->wButtons & XINPUT_GAMEPAD_START);
            // bool dPadBack = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            // bool dPadRB = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            // bool dPadLB = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            // bool dPadR3 = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            // bool dPadL3 = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            // unsigned TCHAR dPadRT = pad->bRightTrigger;
            // unsigned TCHAR dPadLT = pad->bLeftTrigger;
        }
    }
#endif
}

}  // namespace tom