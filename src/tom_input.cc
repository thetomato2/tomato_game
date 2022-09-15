#include "tom_input.hh"

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

internal void load_Xinput()
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

internal void process_keyboard_message(Button& new_state, const b32 is_down)
{
    if (new_state.ended_down != (is_down != 0)) {
        new_state.ended_down = is_down;
        ++new_state.half_transition_cnt;
    }
}

internal void process_Xinput_digital_button(DWORD Xinput_button_state_, Button& old_state_,
                                            DWORD button_bit_, Button& new_state)
{
    new_state.ended_down = ((Xinput_button_state_ & button_bit_) == button_bit_);
    if (new_state.ended_down && old_state_.ended_down)
        new_state.half_transition_cnt = ++old_state_.half_transition_cnt;
}

// NOTE: mod = Shift
internal char win32key_to_char_mod(Win32Key key)
{
    switch (key) {
        case Win32Key::space: return ' ';
        case Win32Key::d1: return '!';
        case Win32Key::d2: return '@';
        case Win32Key::d3: return '#';
        case Win32Key::d4: return '$';
        case Win32Key::d5: return '%';
        case Win32Key::d6: return '^';
        case Win32Key::d7: return '&';
        case Win32Key::d8: return '*';
        case Win32Key::d9: return '(';
        case Win32Key::d0: return ')';
        case Win32Key::a: return 'A';
        case Win32Key::b: return 'B';
        case Win32Key::c: return 'C';
        case Win32Key::d: return 'D';
        case Win32Key::e: return 'E';
        case Win32Key::f: return 'F';
        case Win32Key::g: return 'G';
        case Win32Key::h: return 'H';
        case Win32Key::i: return 'I';
        case Win32Key::j: return 'J';
        case Win32Key::k: return 'K';
        case Win32Key::l: return 'L';
        case Win32Key::m: return 'M';
        case Win32Key::n: return 'N';
        case Win32Key::o: return 'O';
        case Win32Key::p: return 'P';
        case Win32Key::q: return 'Q';
        case Win32Key::r: return 'R';
        case Win32Key::s: return 'S';
        case Win32Key::t: return 'T';
        case Win32Key::u: return 'U';
        case Win32Key::v: return 'V';
        case Win32Key::w: return 'W';
        case Win32Key::x: return 'X';
        case Win32Key::y: return 'Y';
        case Win32Key::z: return 'Z';
        case Win32Key::add: return '=';
        case Win32Key::enter: return '\n';
        case Win32Key::subtract: return '_';
        case Win32Key::semicolon: return ':';
        case Win32Key::comma: return '>';
        case Win32Key::period: return '<';
        case Win32Key::quotes: return '\"';
        case Win32Key::open_brackets: return '{';
        case Win32Key::close_brackets: return '}';
        case Win32Key::tilde: return '~';
        case Win32Key::backslash: return '|';
        case Win32Key::question: return '?';
    }

    return '\0';
}

internal char win32key_to_char(Win32Key key)
{
    switch (key) {
        case Win32Key::space: return ' ';
        case Win32Key::d1: return '1';
        case Win32Key::d2: return '2';
        case Win32Key::d3: return '3';
        case Win32Key::d4: return '4';
        case Win32Key::d5: return '5';
        case Win32Key::d6: return '6';
        case Win32Key::d7: return '7';
        case Win32Key::d8: return '8';
        case Win32Key::d9: return '9';
        case Win32Key::d0: return '0';
        case Win32Key::a: return 'a';
        case Win32Key::b: return 'b';
        case Win32Key::c: return 'c';
        case Win32Key::d: return 'd';
        case Win32Key::e: return 'e';
        case Win32Key::f: return 'f';
        case Win32Key::g: return 'g';
        case Win32Key::h: return 'h';
        case Win32Key::i: return 'i';
        case Win32Key::j: return 'j';
        case Win32Key::k: return 'k';
        case Win32Key::l: return 'l';
        case Win32Key::m: return 'm';
        case Win32Key::n: return 'n';
        case Win32Key::o: return 'o';
        case Win32Key::p: return 'p';
        case Win32Key::q: return 'q';
        case Win32Key::r: return 'r';
        case Win32Key::s: return 's';
        case Win32Key::t: return 't';
        case Win32Key::u: return 'u';
        case Win32Key::v: return 'v';
        case Win32Key::w: return 'w';
        case Win32Key::x: return 'x';
        case Win32Key::y: return 'y';
        case Win32Key::z: return 'z';
        case Win32Key::add: return '+';
        case Win32Key::enter: return '\n';
        case Win32Key::subtract: return '-';
        case Win32Key::semicolon: return ';';
        case Win32Key::comma: return ',';
        case Win32Key::period: return '.';
        case Win32Key::quotes: return '\'';
        case Win32Key::open_brackets: return '[';
        case Win32Key::close_brackets: return ']';
        case Win32Key::tilde: return '`';
        case Win32Key::backslash: return '\\';
        case Win32Key::question: return '/';
    }

    return '\0';
}

#define AssignWin32Key(key) result.keyboard.key.name = Win32Key::key;

Input init_input()
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

#if USE_DS5
    // DualSense (PS5) controllers
    result.ds5_cnt = DS5_init(result.ds5_context);
#endif


    return result;
}

void do_input(Input* input, HWND hwnd, i32 ms_scroll)
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

    for (szt key = 0; key < ARR_CNT(input->mouse.buttons); ++key) {
        if (input->mouse.buttons[key].half_transition_cnt > 0 &&
            input->mouse.buttons[key].ended_down == 0)
            input->mouse.buttons[key].half_transition_cnt = 0;
    }

    // mouse buttons
    process_keyboard_message(input->mouse.buttons[0], ::GetKeyState(VK_LBUTTON) & (1 << 15));
    process_keyboard_message(input->mouse.buttons[1], ::GetKeyState(VK_RBUTTON) & (1 << 15));
    process_keyboard_message(input->mouse.buttons[2], ::GetKeyState(VK_MBUTTON) & (1 << 15));

    for (szt key = 0; key < ARR_CNT(input->keyboard.keys); ++key) {
        if (input->keyboard.keys[key].half_transition_cnt > 0 &&
            input->keyboard.keys[key].ended_down == 0)
            input->keyboard.keys[key].half_transition_cnt = 0;
    }

    for (u32 i = 0; i < Keyboard::key_cnt; ++i) {
        process_keyboard_message(input->keyboard.keys[i],
                                 ::GetKeyState((i32)input->keyboard.keys[i].name) & (1 << 15));
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

            for (szt button = 0; button < ARR_CNT(old_controller.buttons); ++button) {
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
