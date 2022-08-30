// Interface for the DaulSense controller
// TODO: fix the ReadFile bluetooth buffer blocking the thread when the controller is off

#include <initguid.h>
#include <Hidclass.h>
#include <SetupAPI.h>
#include <hidsdi.h>

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "SetupAPI.lib")

#define DS5_SUCCESS(expr) ((expr) == _DS5_ReturnValue::OK)
#define DS5_FAILED(expr)  ((expr) != _DS5_ReturnValue::OK)

#define DS5_ISTATE_BTX_SQUARE   0x10
#define DS5_ISTATE_BTX_CROSS    0x20
#define DS5_ISTATE_BTX_CIRCLE   0x40
#define DS5_ISTATE_BTX_TRIANGLE 0x80
#define DS5_ISTATE_DPAD_LEFT    0x01
#define DS5_ISTATE_DPAD_DOWN    0x02
#define DS5_ISTATE_DPAD_RIGHT   0x04
#define DS5_ISTATE_DPAD_UP      0x08

#define DS5_ISTATE_BTN_A_LEFT_BUMPER   0x01
#define DS5_ISTATE_BTN_A_RIGHT_BUMPER  0x02
#define DS5_ISTATE_BTN_A_LEFT_TRIGGER  0x04
#define DS5_ISTATE_BTN_A_RIGHT_TRIGGER 0x08
#define DS5_ISTATE_BTN_A_SELECT        0x10
#define DS5_ISTATE_BTN_A_MENU          0x20
#define DS5_ISTATE_BTN_A_LEFT_STICK    0x40
#define DS5_ISTATE_BTN_A_RIGHT_STICK   0x80

#define DS5_ISTATE_BTN_B_PLAYSTATION_LOGO 0x01
#define DS5_ISTATE_BTN_B_PAD_BUTTON       0x02
#define DS5_ISTATE_BTN_B_MIC_BUTTON       0x04

#define DS5_OSTATE_PLAYER_LED_LEFT         0x01
#define DS5_OSTATE_PLAYER_LED_MIDDLE_LEFT  0x02
#define DS5_OSTATE_PLAYER_LED_MIDDLE       0x04
#define DS5_OSTATE_PLAYER_LED_MIDDLE_RIGHT 0x08
#define DS5_OSTATE_PLAYER_LED_RIGHT        0x10

#define DS5_MAX_CNT 4

namespace tom
{
constexpr u32 DS5_hashtable[256] = {
    0xd202ef8d, 0xa505df1b, 0x3c0c8ea1, 0x4b0bbe37, 0xd56f2b94, 0xa2681b02, 0x3b614ab8, 0x4c667a2e,
    0xdcd967bf, 0xabde5729, 0x32d70693, 0x45d03605, 0xdbb4a3a6, 0xacb39330, 0x35bac28a, 0x42bdf21c,
    0xcfb5ffe9, 0xb8b2cf7f, 0x21bb9ec5, 0x56bcae53, 0xc8d83bf0, 0xbfdf0b66, 0x26d65adc, 0x51d16a4a,
    0xc16e77db, 0xb669474d, 0x2f6016f7, 0x58672661, 0xc603b3c2, 0xb1048354, 0x280dd2ee, 0x5f0ae278,
    0xe96ccf45, 0x9e6bffd3, 0x762ae69,  0x70659eff, 0xee010b5c, 0x99063bca, 0xf6a70,    0x77085ae6,
    0xe7b74777, 0x90b077e1, 0x9b9265b,  0x7ebe16cd, 0xe0da836e, 0x97ddb3f8, 0xed4e242,  0x79d3d2d4,
    0xf4dbdf21, 0x83dcefb7, 0x1ad5be0d, 0x6dd28e9b, 0xf3b61b38, 0x84b12bae, 0x1db87a14, 0x6abf4a82,
    0xfa005713, 0x8d076785, 0x140e363f, 0x630906a9, 0xfd6d930a, 0x8a6aa39c, 0x1363f226, 0x6464c2b0,
    0xa4deae1d, 0xd3d99e8b, 0x4ad0cf31, 0x3dd7ffa7, 0xa3b36a04, 0xd4b45a92, 0x4dbd0b28, 0x3aba3bbe,
    0xaa05262f, 0xdd0216b9, 0x440b4703, 0x330c7795, 0xad68e236, 0xda6fd2a0, 0x4366831a, 0x3461b38c,
    0xb969be79, 0xce6e8eef, 0x5767df55, 0x2060efc3, 0xbe047a60, 0xc9034af6, 0x500a1b4c, 0x270d2bda,
    0xb7b2364b, 0xc0b506dd, 0x59bc5767, 0x2ebb67f1, 0xb0dff252, 0xc7d8c2c4, 0x5ed1937e, 0x29d6a3e8,
    0x9fb08ed5, 0xe8b7be43, 0x71beeff9, 0x6b9df6f,  0x98dd4acc, 0xefda7a5a, 0x76d32be0, 0x1d41b76,
    0x916b06e7, 0xe66c3671, 0x7f6567cb, 0x862575d,  0x9606c2fe, 0xe101f268, 0x7808a3d2, 0xf0f9344,
    0x82079eb1, 0xf500ae27, 0x6c09ff9d, 0x1b0ecf0b, 0x856a5aa8, 0xf26d6a3e, 0x6b643b84, 0x1c630b12,
    0x8cdc1683, 0xfbdb2615, 0x62d277af, 0x15d54739, 0x8bb1d29a, 0xfcb6e20c, 0x65bfb3b6, 0x12b88320,
    0x3fba6cad, 0x48bd5c3b, 0xd1b40d81, 0xa6b33d17, 0x38d7a8b4, 0x4fd09822, 0xd6d9c998, 0xa1def90e,
    0x3161e49f, 0x4666d409, 0xdf6f85b3, 0xa868b525, 0x360c2086, 0x410b1010, 0xd80241aa, 0xaf05713c,
    0x220d7cc9, 0x550a4c5f, 0xcc031de5, 0xbb042d73, 0x2560b8d0, 0x52678846, 0xcb6ed9fc, 0xbc69e96a,
    0x2cd6f4fb, 0x5bd1c46d, 0xc2d895d7, 0xb5dfa541, 0x2bbb30e2, 0x5cbc0074, 0xc5b551ce, 0xb2b26158,
    0x4d44c65,  0x73d37cf3, 0xeada2d49, 0x9ddd1ddf, 0x3b9887c,  0x74beb8ea, 0xedb7e950, 0x9ab0d9c6,
    0xa0fc457,  0x7d08f4c1, 0xe401a57b, 0x930695ed, 0xd62004e,  0x7a6530d8, 0xe36c6162, 0x946b51f4,
    0x19635c01, 0x6e646c97, 0xf76d3d2d, 0x806a0dbb, 0x1e0e9818, 0x6909a88e, 0xf000f934, 0x8707c9a2,
    0x17b8d433, 0x60bfe4a5, 0xf9b6b51f, 0x8eb18589, 0x10d5102a, 0x67d220bc, 0xfedb7106, 0x89dc4190,
    0x49662d3d, 0x3e611dab, 0xa7684c11, 0xd06f7c87, 0x4e0be924, 0x390cd9b2, 0xa0058808, 0xd702b89e,
    0x47bda50f, 0x30ba9599, 0xa9b3c423, 0xdeb4f4b5, 0x40d06116, 0x37d75180, 0xaede003a, 0xd9d930ac,
    0x54d13d59, 0x23d60dcf, 0xbadf5c75, 0xcdd86ce3, 0x53bcf940, 0x24bbc9d6, 0xbdb2986c, 0xcab5a8fa,
    0x5a0ab56b, 0x2d0d85fd, 0xb404d447, 0xc303e4d1, 0x5d677172, 0x2a6041e4, 0xb369105e, 0xc46e20c8,
    0x72080df5, 0x50f3d63,  0x9c066cd9, 0xeb015c4f, 0x7565c9ec, 0x262f97a,  0x9b6ba8c0, 0xec6c9856,
    0x7cd385c7, 0xbd4b551,  0x92dde4eb, 0xe5dad47d, 0x7bbe41de, 0xcb97148,  0x95b020f2, 0xe2b71064,
    0x6fbf1d91, 0x18b82d07, 0x81b17cbd, 0xf6b64c2b, 0x68d2d988, 0x1fd5e91e, 0x86dcb8a4, 0xf1db8832,
    0x616495a3, 0x1663a535, 0x8f6af48f, 0xf86dc419, 0x660951ba, 0x110e612c, 0x88073096, 0xff000000
};

// Hash seed
constexpr u32 DS5_crc_seed = 0xeada2d49;

enum class DS5_Connection : u8
{
    USB = 0,
    BT  = 1  // Bluetooth
};

struct DS5_Info
{
    wchar path[260];
    DS5_Connection connection;
};

struct DS5_Context
{
    bool connected;
    u32 port;
    wchar path[260];
    void* hnd;
    DS5_Connection connection;
    byt hid_buf[547];
};

enum class DS5_MicLed : byt
{
    off   = 0x00,
    on    = 0x01,
    pulse = 0x02,
};

enum class DS5_LedBrightness
{
    low    = 0x02,
    medium = 0x01,
    high   = 0x00
};

enum class DS5_TriggerEffectType : byt
{
    none       = 0x00,
    continuous = 0x01,
    Section    = 0x02,
    effect_ex  = 0x26,
    calibrate  = 0xfc

};

struct DS5_PlayerLeds
{
    bool player_led_fade;
    byt bitmask;
    DS5_LedBrightness brightness;
};

struct DS5_TriggerEffect
{
    DS5_TriggerEffectType type;

    union
    {
        byt _u1_raw[6];

        struct
        {
            byt start_pos;
            byt force;
            byt _pad[4];
        } continuous;

        struct
        {
            byt start_pos;
            byt endPosition;
            byt _pad[4];
        } section;

        struct
        {
            bool keep_effect;
            byt start_pos;
            byt begin_force;
            byt middle_force;
            byt end_force;
            byt frequency;
        } effect_ex;
    };
};

struct DS5_Battery
{
    bool charging;
    bool fully_charged;
    byt charge;
};

struct DS5_State
{
    bool headphone_connected;
    bool disable_leds;
    DS5_Battery battery;
    DS5_MicLed mic_led;
    DS5_PlayerLeds player_leds;
    v3<byt> led_color;
    // NOTE: x: ~0 - 2000, y: ~0 - 2048
    v2u touch_1, touch_2;
    byt touch_1_id, touch_2_id;
    v2<i8> stick_R, stick_L;
    byt trigger_L, trigger_R;
    Button dpad_U, dpad_R, dpad_D, dpad_L;
    Button triangle, circle, square, cross;
    Button bumper_L, bumper_R;
    // NOTE: not sure how this triggers(ha) but its part of the hid
    Button trigger_but_L, trigger_but_R;
    Button stick_but_L, stick_but_R;
    Button select, menu, ps_logo, mic, touch;
    Button touch_1_but, touch_2_but;
    byt rumble_L, rumble_R;
    DS5_TriggerEffect trigger_effect_L, trigger_effect_R;
    byt trigger_feedback_L, trigger_feedback_R;

    v3<i16> accelerometer;
    v3<i16> gyroscope;
};

fn u32 DS5_crc32_compute(byt* buffer, szt len)
{
    u32 result = DS5_crc_seed;

    for (size_t i = 0; i < len; i++) {
        result = DS5_hashtable[((byt)result) ^ ((byt)buffer[i])] ^ (result >> 8);
    }

    // Return result
    return result;
}

fn void DS5_enum_devices(void* ptr_buf, u32 arr_len, u32* ds5_cnt)
{
    // Get all hid devices from devs
    HANDLE hiddi_hnd = SetupDiGetClassDevs(&GUID_DEVINTERFACE_HID, NULL, NULL,
                                           DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (!hiddi_hnd || (hiddi_hnd == INVALID_HANDLE_VALUE)) {
        PrintWarning("Could not find HID devices");
        return;
    }

    // Enumerate over hid device
    u32 input_arr_i = 0;
    DWORD device_i  = 0;
    SP_DEVINFO_DATA hiddi_info;
    hiddi_info.cbSize = sizeof(SP_DEVINFO_DATA);
    while (SetupDiEnumDeviceInfo(hiddi_hnd, device_i, &hiddi_info)) {
        // Enumerate over all hid device interfaces
        DWORD interface_i = 0;
        SP_DEVICE_INTERFACE_DATA interface_info;
        interface_info.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        while (SetupDiEnumDeviceInterfaces(hiddi_hnd, &hiddi_info, &GUID_DEVINTERFACE_HID,
                                           interface_i, &interface_info)) {
            // Get device path
            DWORD required_sz = 0;
            SetupDiGetDeviceInterfaceDetailW(hiddi_hnd, &interface_info, NULL, 0, &required_sz,
                                             NULL);
            ScopedPtr device_path = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)plat_malloc(required_sz);
            device_path->cbSize   = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
            SetupDiGetDeviceInterfaceDetailW(hiddi_hnd, &interface_info, device_path.get(),
                                             required_sz, NULL, NULL);

            HANDLE device_hnd =
                CreateFileW(device_path->DevicePath, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

            // Check if device is reachable
            if (device_hnd && device_hnd != INVALID_HANDLE_VALUE) {
                // Get vendor and product id
                u32 vendor_id = 0, product_id = 0;
                HIDD_ATTRIBUTES deviceAttributes;
                if (HidD_GetAttributes(device_hnd, &deviceAttributes)) {
                    vendor_id  = deviceAttributes.VendorID;
                    product_id = deviceAttributes.ProductID;
                }

                // Check if ids match
                if (vendor_id == 0x054C && product_id == 0x0CE6) {
                    // Get pointer to target
                    DS5_Info* ptr_info = nullptr;
                    if (input_arr_i < arr_len) {
                        ptr_info = &(((DS5_Info*)ptr_buf)[input_arr_i]);
                    }

                    // Copy path
                    if (ptr_info) {
                        // wcscpy_s(ptr_info->path, 260, (const wchar_t*)device_path->DevicePath);
                        str_copy(device_path->DevicePath, ptr_info->path);
                    }

                    // Get preparsed data
                    PHIDP_PREPARSED_DATA ppd;
                    if (HidD_GetPreparsedData(device_hnd, &ppd)) {
                        // Get device capabilities
                        HIDP_CAPS device_caps;
                        if (HidP_GetCaps(ppd, &device_caps) == HIDP_STATUS_SUCCESS) {
                            // Check for device connection type
                            if (ptr_info) {
                                // Check if controller matches USB specifications
                                if (device_caps.InputReportByteLength == 64) {
                                    ptr_info->connection = DS5_Connection::USB;

                                    // Device found and valid -> Inrement index
                                    input_arr_i++;
                                }
                                // Check if controller matches BT specifications
                                else if (device_caps.InputReportByteLength == 78) {
                                    ptr_info->connection = DS5_Connection::BT;

                                    // Device found and valid -> Inrement index
                                    input_arr_i++;
                                }
                            }
                        }

                        // Free preparsed data
                        HidD_FreePreparsedData(ppd);
                    }
                }

                // Close device
                CloseHandle(device_hnd);
            }

            // Increment index
            interface_i++;
        }

        // Increment index
        device_i++;
    }
    // Close device enum list
    SetupDiDestroyDeviceInfoList(hiddi_hnd);

    *ds5_cnt = input_arr_i;
}

fn u32 DS5_init(DS5_Context* out)
{

    DS5_Info info[DS5_MAX_CNT];
    u32 controller_cnt = 0;
    DS5_enum_devices(info, CountOf(info), &controller_cnt);

    if (controller_cnt) {
        PrintInfo(str_fmt("Found %u DaulSense controllers.", controller_cnt));

        for (u32 i = 0; i < controller_cnt; ++i) {
            HANDLE ds5_hnd =
                CreateFileW(info[i].path, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

            if (ds5_hnd == INVALID_HANDLE_VALUE) {
                InvalidCodePath;
                continue;
            }
            out[i].port       = i;
            out[i].connected  = true;
            out[i].connection = info[i].connection;
            out[i].hnd        = ds5_hnd;
            str_copy(out[i].path, info[i].path);
        }
    }

    return controller_cnt;
}

fn void DS5_process_button(Button* but, byt is_down)
{
    if (but->ended_down && is_down) {
        ++but->half_transition_cnt;
    } else if (but->ended_down && !is_down) {
        but->ended_down          = 0;
        but->half_transition_cnt = 0;
    } else if (!but->ended_down && is_down) {
        but->ended_down          = 1;
        but->half_transition_cnt = 0;
    }
}

fn void DS5_process_buttons(DS5_State* state, byt buttons_and_dpad, byt buttons_a, byt buttons_b)
{
    DS5_process_button(&state->dpad_U, buttons_and_dpad & DS5_ISTATE_DPAD_UP);
    DS5_process_button(&state->dpad_R, buttons_and_dpad & DS5_ISTATE_DPAD_RIGHT);
    DS5_process_button(&state->dpad_D, buttons_and_dpad & DS5_ISTATE_DPAD_DOWN);
    DS5_process_button(&state->dpad_L, buttons_and_dpad & DS5_ISTATE_DPAD_LEFT);
    DS5_process_button(&state->triangle, buttons_and_dpad & DS5_ISTATE_BTX_TRIANGLE);
    DS5_process_button(&state->circle, buttons_and_dpad & DS5_ISTATE_BTX_CIRCLE);
    DS5_process_button(&state->cross, buttons_and_dpad & DS5_ISTATE_BTX_CROSS);
    DS5_process_button(&state->square, buttons_and_dpad & DS5_ISTATE_BTX_SQUARE);
    DS5_process_button(&state->bumper_L, buttons_a & DS5_ISTATE_BTN_A_LEFT_BUMPER);
    DS5_process_button(&state->bumper_R, buttons_a & DS5_ISTATE_BTN_A_RIGHT_BUMPER);
    DS5_process_button(&state->trigger_but_L, buttons_a & DS5_ISTATE_BTN_A_LEFT_TRIGGER);
    DS5_process_button(&state->trigger_but_R, buttons_a & DS5_ISTATE_BTN_A_RIGHT_TRIGGER);
    DS5_process_button(&state->select, buttons_a & DS5_ISTATE_BTN_A_SELECT);
    DS5_process_button(&state->menu, buttons_a & DS5_ISTATE_BTN_A_MENU);
    DS5_process_button(&state->stick_but_L, buttons_a & DS5_ISTATE_BTN_A_LEFT_STICK);
    DS5_process_button(&state->stick_but_R, buttons_a & DS5_ISTATE_BTN_A_RIGHT_STICK);
    DS5_process_button(&state->ps_logo, buttons_b & DS5_ISTATE_BTN_B_PLAYSTATION_LOGO);
    DS5_process_button(&state->mic, buttons_b & DS5_ISTATE_BTN_B_MIC_BUTTON);
    DS5_process_button(&state->touch, buttons_b & DS5_ISTATE_BTN_B_PAD_BUTTON);
}

fn void DS5_parse_hid_buffer(byt* hid_buf, DS5_State* state)
{
    state->stick_L.x = (byt)(((i16)(hid_buf[0x00] - 128)));
    state->stick_L.y = (byt)(((i16)(hid_buf[0x01] - 127)) * -1);
    state->stick_R.x = (byt)(((i16)(hid_buf[0x02] - 128)));
    state->stick_R.y = (byt)(((i16)(hid_buf[0x03] - 127)) * -1);

    state->trigger_L = hid_buf[0x04];
    state->trigger_R = hid_buf[0x05];

    byt buttons_and_dpad = hid_buf[0x07] & 0xF0;
    byt buttons_a        = hid_buf[0x08];
    byt buttons_b        = hid_buf[0x09];

    // Dpad
    switch (hid_buf[0x07] & 0x0F) {
            // Up
        case 0x0:
            buttons_and_dpad |= DS5_ISTATE_DPAD_UP;
            break;
            // Down
        case 0x4:
            buttons_and_dpad |= DS5_ISTATE_DPAD_DOWN;
            break;
            // Left
        case 0x6:
            buttons_and_dpad |= DS5_ISTATE_DPAD_LEFT;
            break;
            // Right
        case 0x2:
            buttons_and_dpad |= DS5_ISTATE_DPAD_RIGHT;
            break;
            // Left Down
        case 0x5:
            buttons_and_dpad |= DS5_ISTATE_DPAD_LEFT | DS5_ISTATE_DPAD_DOWN;
            break;
            // Left Up
        case 0x7:
            buttons_and_dpad |= DS5_ISTATE_DPAD_LEFT | DS5_ISTATE_DPAD_UP;
            break;
            // Right Up
        case 0x1:
            buttons_and_dpad |= DS5_ISTATE_DPAD_RIGHT | DS5_ISTATE_DPAD_UP;
            break;
            // Right Down
        case 0x3: buttons_and_dpad |= DS5_ISTATE_DPAD_RIGHT | DS5_ISTATE_DPAD_DOWN; break;
    }

    DS5_process_buttons(state, buttons_and_dpad, buttons_a, buttons_b);

    memcpy(state->accelerometer.e, &hid_buf[0x0f], sizeof(i16) * 3);
    memcpy(state->gyroscope.e, &hid_buf[0x15], sizeof(i16) * 3);

    u32 touch_1_raw  = *(u32*)(&hid_buf[0x20]);
    state->touch_1.y = (touch_1_raw & 0xFFF00000) >> 20;
    state->touch_1.x = (touch_1_raw & 0x000FFF00) >> 8;
    DS5_process_button(&state->touch_1_but, (touch_1_raw & (1 << 7)) == 0);
    state->touch_1_id = (touch_1_raw & 127);

    u32 touch_2_raw  = *(u32*)(&hid_buf[0x24]);
    state->touch_2.y = (touch_2_raw & 0xFFF00000) >> 20;
    state->touch_2.x = (touch_2_raw & 0x000FFF00) >> 8;
    DS5_process_button(&state->touch_2_but, (touch_2_raw & (1 << 7)) == 0);
    state->touch_2_id = (touch_2_raw & 127);

    state->headphone_connected = hid_buf[0x35] & 0x01;
    state->trigger_feedback_L  = hid_buf[0x2A];
    state->trigger_feedback_R  = hid_buf[0x29];

    // Battery
    state->battery.charging      = hid_buf[0x35] & 0x08;
    state->battery.fully_charged = hid_buf[0x36] & 0x20;
    state->battery.charge        = hid_buf[0x36] & 0x0F;
}

fn void DS5_process_trigger(DS5_TriggerEffect* effect, byt* buffer)
{
    // Switch on effect
    switch (effect->type) {
        // Continuous
        case DS5_TriggerEffectType::continuous:
            // Mode
            buffer[0x00] = 0x01;
            // Parameters
            buffer[0x01] = effect->continuous.start_pos;
            buffer[0x02] = effect->continuous.force;

            break;

        // Section
        case DS5_TriggerEffectType::Section:
            // Mode
            buffer[0x00] = 0x02;
            // Parameters
            buffer[0x01] = effect->continuous.start_pos;
            buffer[0x02] = effect->continuous.force;

            break;

        // EffectEx
        case DS5_TriggerEffectType::effect_ex:
            // Mode
            buffer[0x00] = 0x02 | 0x20 | 0x04;
            // Parameters
            buffer[0x01] = 0xFF - effect->effect_ex.start_pos;
            // Keep flag
            if (effect->effect_ex.keep_effect) {
                buffer[0x02] = 0x02;
            }
            // Forces
            buffer[0x04] = effect->effect_ex.begin_force;
            buffer[0x05] = effect->effect_ex.middle_force;
            buffer[0x06] = effect->effect_ex.end_force;
            // Frequency
            buffer[0x09] = max(1, effect->effect_ex.frequency / 2);

            break;

        // Calibrate
        case DS5_TriggerEffectType::calibrate:
            // Mode
            buffer[0x00] = 0xFC;
            break;

        // No resistance / default
        case DS5_TriggerEffectType::none: __fallthrough;
        default:
            // All zero
            buffer[0x00] = 0x00;
            buffer[0x01] = 0x00;
            buffer[0x02] = 0x00;

            break;
    }
}

fn void DS5_create_hid_output_buffer(byt* hid_buf, DS5_State* state)
{
    hid_buf[0x00] = 0xFF;
    hid_buf[0x01] = 0xF7;

    // Rumble motors
    hid_buf[0x02] = state->rumble_L;
    hid_buf[0x03] = state->rumble_R;

    // Mic led
    hid_buf[0x08] = (byt)state->mic_led;

    // Player led
    hid_buf[0x2B] = state->player_leds.bitmask;
    if (state->player_leds.player_led_fade) {
        hid_buf[0x2B] &= ~(0x20);
    } else {
        hid_buf[0x2B] |= 0x20;
    }

    // Player led brightness
    hid_buf[0x26] = 0x03;
    hid_buf[0x29] = state->disable_leds ? 0x01 : 0x2;
    hid_buf[0x2A] = (byt)state->player_leds.brightness;

    // Lightbar
    hid_buf[0x2C] = state->led_color.r;
    hid_buf[0x2D] = state->led_color.g;
    hid_buf[0x2E] = state->led_color.b;

    // Adaptive Triggers
    DS5_process_trigger(&state->trigger_effect_R, &hid_buf[0x0A]);
    DS5_process_trigger(&state->trigger_effect_L, &hid_buf[0x15]);
}

fn void ds5_get_input(DS5_Context* context, DS5_State* state)
{
    if (!context->connected) {
        PrintInfo(str_fmt("DS5 controller %u was disconnected.", context->port));
        return;
    }

    HidD_FlushQueue(context->hnd);
    u16 report_len;
    if (context->connection == DS5_Connection::BT) {
        report_len          = 78;
        context->hid_buf[0] = 0x031;
    } else {
        report_len          = 64;
        context->hid_buf[0] = 0x01;
    }

    if (context->connection == DS5_Connection::BT) {
        // NOTE: Readfile will block thread because turning off the controller will not disconeect
        // the device for windows when using bluetooth
#if 0
        if (SUCCEEDED(ReadFile(context->hnd, context->hid_buf, report_len, NULL, NULL)))
            DS5_parse_hid_buffer(&context->hid_buf[2], state);
        else
            PrintError(str_fmt("Failed to read Hid buffer for DS5 controller %u!", context->port));
#endif
    } else {
        if (SUCCEEDED(ReadFile(context->hnd, context->hid_buf, report_len, NULL, NULL))) {
            DS5_parse_hid_buffer(&context->hid_buf[1], state);
        } else {
            PrintError(str_fmt("Failed to read Hid buffer for DS5 controller %u!", context->port));
        }
    }
}

fn void ds5_push_output(DS5_Context* context, DS5_State* state)
{
    if (!context->connected) {
        PrintInfo(str_fmt("DS5 controller %u was disconnected.", context->port));
        return;
    }

    u16 lrmbl = 0, rrmbl = 0;

    //  lrmbl = max(lrmbl - 0x200, 0);
    //  rrmbl = max(rrmbl - 0x100, 0);

    state->rumble_L  = (lrmbl & 0xff00) >> 8UL;
    state->rumble_R  = (rrmbl & 0xff00) >> 8UL;
    state->led_color = { 0, 255, 0 };

    state->player_leds.bitmask = 0;
    state->mic_led             = DS5_MicLed::off;

    u16 report_len;
    if (context->connection == DS5_Connection::BT) {
        report_len = 547;
    } else {
        report_len = 48;
    }
    zero_size(context->hid_buf, report_len);

    if (context->connection == DS5_Connection::BT) {
        context->hid_buf[0x00] = 0x31;
        context->hid_buf[0x01] = 0x02;
        DS5_create_hid_output_buffer(&context->hid_buf[2], state);

        // check the hash, why?idk that is what the docs say
        u32 crc = DS5_crc32_compute(context->hid_buf, 74);

        context->hid_buf[0x4A] = (byt)((crc & 0x000000FF) >> 0UL);
        context->hid_buf[0x4B] = (byt)((crc & 0x0000FF00) >> 8UL);
        context->hid_buf[0x4C] = (byt)((crc & 0x00FF0000) >> 16UL);
        context->hid_buf[0x4D] = (byt)((crc & 0xFF000000) >> 24UL);
    } else {
        context->hid_buf[0x00] = 0x02;
        DS5_create_hid_output_buffer(&context->hid_buf[1], state);
    }

    if (FAILED(WriteFile(context->hnd, context->hid_buf, report_len, NULL, NULL))) {
        PrintError(str_fmt("Failed to write Hid buffer for DS5 controller # %u!", context->port));
    }
}

}  // namespace tom
