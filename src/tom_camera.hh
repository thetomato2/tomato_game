namespace tom
{

enum class Cam_Move_Dir
{
    forward,
    backward,
    up,
    down,
    right,
    left,
};

struct Camera
{
    f32 speed;
    // NOTE: in open_gl coords
    v3f pos;
    f32 angle_h;
    f32 angle_v;
    v2f mouse_pos;
    v3f up;
    v3f forward;
    v3f target_pos;
};

}  // namespace tom
