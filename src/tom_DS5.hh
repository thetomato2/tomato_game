// ============================================================================================
// Interface for the DaulSense controller
// TODO: fix the ReadFile bluetooth buffer blocking the thread when the controller is off
// ============================================================================================

#ifndef TOM_DS5_HH
#define TOM_DS5_HH

#include "tom_core.hh"
#include "tom_math.hh"
#include "tom_memory.hh"
#include "tom_string.hh"

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
    void *hnd;
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
    gamecube   = 0x99,
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

u32 DS5_init(DS5_Context *out);
void ds5_get_input(DS5_Context *context, DS5_State *state);
void ds5_push_output(DS5_Context *context, DS5_State *state);

}  // namespace tom

#endif
