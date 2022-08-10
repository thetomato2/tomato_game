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

#define AssignWin32Key(key) result.keyboard.key.name = Win32Key::key;

fn Input init_input()
{
    Input result = {};

    AssignWin32Key(space);
    AssignWin32Key(d1);
    AssignWin32Key(d2);
    AssignWin32Key(d3);
    AssignWin32Key(d4);
    AssignWin32Key(d5);
    AssignWin32Key(d6);
    AssignWin32Key(d7);
    AssignWin32Key(d8);
    AssignWin32Key(d9);
    AssignWin32Key(d0);
    AssignWin32Key(a);
    AssignWin32Key(b);
    AssignWin32Key(c);
    AssignWin32Key(d);
    AssignWin32Key(e);
    AssignWin32Key(f);
    AssignWin32Key(g);
    AssignWin32Key(h);
    AssignWin32Key(i);
    AssignWin32Key(j);
    AssignWin32Key(k);
    AssignWin32Key(l);
    AssignWin32Key(m);
    AssignWin32Key(n);
    AssignWin32Key(o);
    AssignWin32Key(p);
    AssignWin32Key(q);
    AssignWin32Key(r);
    AssignWin32Key(s);
    AssignWin32Key(t);
    AssignWin32Key(u);
    AssignWin32Key(v);
    AssignWin32Key(w);
    AssignWin32Key(x);
    AssignWin32Key(y);
    AssignWin32Key(z);
    AssignWin32Key(enter);
    AssignWin32Key(escape);
    AssignWin32Key(left_alt);
    AssignWin32Key(left_shift);
    AssignWin32Key(left_control);
    AssignWin32Key(tab);
    AssignWin32Key(back);
    AssignWin32Key(add);
    AssignWin32Key(subtract);
    AssignWin32Key(semicolon);
    AssignWin32Key(comma);
    AssignWin32Key(period);
    AssignWin32Key(quotes);
    AssignWin32Key(open_brackets);
    AssignWin32Key(close_brackets);
    AssignWin32Key(tilde);
    AssignWin32Key(backslash);
    AssignWin32Key(question);
    AssignWin32Key(pipe);
    AssignWin32Key(left);
    AssignWin32Key(up);
    AssignWin32Key(right);
    AssignWin32Key(down);
    AssignWin32Key(f1);
    AssignWin32Key(f2);
    AssignWin32Key(f3);
    AssignWin32Key(f4);
    AssignWin32Key(f5);
    AssignWin32Key(f6);
    AssignWin32Key(f7);
    AssignWin32Key(f8);
    AssignWin32Key(f9);
    AssignWin32Key(f10);
    AssignWin32Key(f11);
    AssignWin32Key(f12);

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