namespace tom
{

function void camera_init_angle(Camera* cam)
{
#if Z_UP
    v3f h_target = vec_normalize({ cam->forward.x, 0.0f, cam->forward.z });
    f32 angle    = to_degree(asin(abs(h_target.z)));
#else
    v3f h_target = vec_normalize({ cam->forward.x, 0.0f, -cam->forward.y });
    f32 angle    = to_degree(asin(abs(-h_target.y)));
#endif

    if (h_target.z >= 0.0f) {
        if (h_target.x >= 0.0f)
            cam->angle_h = 360.0f - angle;
        else
            cam->angle_h = 180.0f + angle;
    } else {
        if (h_target.x >= 0.0f)
            cam->angle_h = angle;
        else
            cam->angle_h = 180.0f - angle;
    }

    cam->angle_v = -to_degree(asin(cam->forward.y));
}

fn Camera camera_default_init()
{
    Camera result;

    result.speed      = 1.0f;
    result.angle_h    = 0.0f;
    result.angle_v    = 0.0f;
    result.pos        = { 0.0f, 0.0f, 0.0f };
    result.target_pos = {};
    result.up         = { 0.0f, 1.0f, 0.0f };
    result.forward    = { 0.0f, 0.0f, -1.0f };

    return result;
}

fn Camera camera_init()
{
    Camera result = camera_default_init();
    camera_init_angle(&result);
    return result;
}

fn Camera camera_init(v3f pos, v3f forward, v3f up)
{
    Camera result = camera_default_init();

    result.pos = pos;
    // NOTE: can't assume passed in vectors are normalized
    result.forward = vec_normalize(forward);
    result.up      = vec_normalize(up);
    camera_init_angle(&result);

    return result;
}

fn Camera camera_init_no_angle(v3f pos, v3f forward, v3f up)
{
    Camera result = camera_default_init();

    result.pos = pos;
    // NOTE: can't assume passed in vectors are normalized
    result.forward = vec_normalize(forward);
    result.up      = vec_normalize(up);

    return result;
}

fn void move_camera(Camera* cam, Cam_Move_Dir dir, f32 dt)
{
    switch (dir) {
        case Cam_Move_Dir::forward: {
            cam->pos += cam->forward * cam->speed * dt;
        } break;
        case Cam_Move_Dir::backward: {
            cam->pos -= cam->forward * cam->speed * dt;
        } break;
        case Cam_Move_Dir::up: {
            cam->pos.y += cam->speed * dt;
        } break;
        case Cam_Move_Dir::down: {
            cam->pos.y -= cam->speed * dt;
        } break;
        case Cam_Move_Dir::right: {
            v3f res = vec_cross(cam->up, cam->forward);
            res     = vec_normalize(res);
            res *= cam->speed * dt;
            cam->pos += res;

        } break;
        case Cam_Move_Dir::left: {
            v3f res = vec_cross(cam->forward, cam->up);
            res     = vec_normalize(res);
            res *= cam->speed * dt;
            cam->pos += res;
        } break;
        default: {
        } break;
    }
}

fn void move_camera_distance(Camera* cam, Cam_Move_Dir dir, f32 dist)
{
    switch (dir) {
        case Cam_Move_Dir::forward: {
            cam->pos += cam->forward * dist;
        } break;
        case Cam_Move_Dir::backward: {
            cam->pos -= cam->forward * dist;
        } break;
        case Cam_Move_Dir::up: {
            cam->pos.y += dist;
        } break;
        case Cam_Move_Dir::down: {
            cam->pos.y -= dist;
        } break;
        case Cam_Move_Dir::right: {
            v3f res = vec_cross(cam->up, cam->forward);
            res     = vec_normalize(res);
            res *= dist;
            cam->pos += res;

        } break;
        case Cam_Move_Dir::left: {
            v3f res = vec_cross(cam->forward, cam->up);
            res     = vec_normalize(res);
            res *= dist;
            cam->pos += res;
        } break;
        default: {
        } break;
    }
}

fn void pan_camera(Camera* cam, Cam_Move_Dir dir, f32 dt)
{
    f32 new_speed = cam->speed / 2.0f;

    switch (dir) {
        case Cam_Move_Dir::forward: {
            cam->pos += cam->forward * new_speed * dt;
            cam->target_pos += cam->forward * new_speed * dt;
        } break;
        case Cam_Move_Dir::backward: {
            cam->pos -= cam->forward * new_speed * dt;
            cam->target_pos -= cam->forward * new_speed * dt;
        } break;
        case Cam_Move_Dir::up: {
            cam->pos.y += new_speed * dt;
            cam->target_pos.y += new_speed * dt;
        } break;
        case Cam_Move_Dir::down: {
            cam->pos.y -= new_speed * dt;
            cam->target_pos.y -= new_speed * dt;
        } break;
        case Cam_Move_Dir::right: {
            v3f res = vec_cross(cam->up, cam->forward);
            res     = vec_normalize(res);
            res *= new_speed * dt;
            cam->pos += res;
            cam->target_pos += res;

        } break;
        case Cam_Move_Dir::left: {
            v3f res = vec_cross(cam->forward, cam->up);
            res     = vec_normalize(res);
            res *= cam->speed * dt;
            cam->pos += res;
            cam->target_pos += res;
        } break;
        default: {
        } break;
    }
}

fn void mouse_look_cam(Camera* cam, Mouse ms, r2i win_dims)
{
    v2f ms_delta = ms.get_delta();
    if (ms.pos.x < 0.0f || ms.pos.x > (f32)win_dims.x1 || ms.pos.u < 0.0f ||
        ms.pos.y > (f32)win_dims.y1)
        ms_delta = {};

    f32 angle_h = ms_delta.x / 20.0f;

    f32 angle_v = ms_delta.y / 20.0f;

    v3f n = vec_normalize(cam->forward);
    v3f u = vec_normalize(vec_cross(cam->up, n));
    v3f v = vec_cross(n, u);

    if (key_down(ms.l)) {
        cam->forward = quat_rotate(cam->forward, u, angle_v);
        cam->forward = quat_rotate(cam->forward, cam->up, angle_h);
    }
    // cam.target = qua::rotate(cam.target, cam.up, 1.0f);
}

fn void orbit_cam(Camera* cam, Keyboard kb, Mouse ms, r2i win_dims, f32* dist = nullptr,
                        v3f* target_pos = nullptr)
{
    if (target_pos) cam->target_pos = *target_pos;

    v2f ms_delta = ms.get_delta();
    if (ms.pos.x < 0.0f || ms.pos.x > (f32)win_dims.x1 || ms.pos.u < 0.0f ||
        ms.pos.y > (f32)win_dims.y1)
        ms_delta = {};

    f32 d1;
    if (dist) {
        d1 = *dist;
    } else {
        d1 = vec_distance(cam->pos, cam->target_pos);
    }

    f32 mouse_sens  = 0.0005f / (1 / d1);
    f32 scroll_sens = 50.0f;

    if (key_down(ms.m)) {
        if (ms_delta.x > 0.0f) {
            f32 spd          = abs(ms_delta.x) * mouse_sens;
            Cam_Move_Dir dir = Cam_Move_Dir::left;
            key_down(kb.left_shift) ? pan_camera(cam, dir, spd) : move_camera(cam, dir, spd);
        }

        if (ms_delta.x < 0.0f) {
            f32 spd          = abs(ms_delta.x) * mouse_sens;
            Cam_Move_Dir dir = Cam_Move_Dir::right;
            key_down(kb.left_shift) ? pan_camera(cam, dir, spd) : move_camera(cam, dir, spd);
        }

        if (ms_delta.y > 0.0f) {
            f32 spd          = abs(ms_delta.y) * mouse_sens;
            Cam_Move_Dir dir = Cam_Move_Dir::up;
            key_down(kb.left_shift) ? pan_camera(cam, dir, spd) : move_camera(cam, dir, spd);
        }

        if (ms_delta.y < 0.0f) {
            f32 spd          = abs(ms_delta.y) * mouse_sens;
            Cam_Move_Dir dir = Cam_Move_Dir::down;
            key_down(kb.left_shift) ? pan_camera(cam, dir, spd) : move_camera(cam, dir, spd);
        }
    }

    if (cam->pos != cam->target_pos) {
        cam->forward = vec_normalize(cam->target_pos - cam->pos);
    }

    f32 d2 = vec_distance(cam->pos, cam->target_pos);

    if (d1 > d2) {
        // moved away
        move_camera_distance(cam, Cam_Move_Dir::forward, d1 - d2);
    } else if (d1 < d2) {
        // moved toward
        move_camera_distance(cam, Cam_Move_Dir::backward, d2 - d1);
    }
    if (ms.scroll > 0) {
        f32 spd = scroll_sens * mouse_sens;
        move_camera(cam, Cam_Move_Dir::backward, spd);
    }

    if (ms.scroll < 0) {
        f32 spd = scroll_sens * mouse_sens;
        move_camera(cam, Cam_Move_Dir::forward, spd);
    }
}

fn void camera_look_at(Camera* cam, v3f target_pos)
{
    cam->target_pos = target_pos;

    f32 d1 = vec_distance(cam->pos, cam->target_pos);

    // forward
    cam->forward = vec_normalize(cam->pos - cam->target_pos);
    f32 d2       = vec_distance(cam->pos, cam->target_pos);

    v3f n = vec_normalize(cam->forward);
    v3f u = vec_normalize(vec_cross(cam->up, n));
    v3f v = vec_cross(n, u);

    cam->up = v;

    // NOTE: this keeps a constant distance
    // TODO: better way?
    if (d1 > d2) {
        // moved away
        move_camera_distance(cam, Cam_Move_Dir::forward, d1 - d2);
    } else if (d1 < d2) {
        // moved toward
        move_camera_distance(cam, Cam_Move_Dir::backward, d2 - d1);
    }
}

fn m4 camera_view(Camera cam)
{
#if 0
    v3f n = vec_normalize(cam.forward);
    v3f u = vec_normalize(vec::cross(cam.up, n));
    v3f v = vec_cross(n, u);

    m4 result = mat_row_3x3(u, v, n);
    result    = mat_translate(result, cam.pos);
    // result.m[0][3] = cam.pos.x;
    // result.m[1][3] = cam.pos.y;
    // result.m[2][3] = cam.pos.z;

    return result;
#else
    return mat_look_to(cam.pos, cam.forward, cam.up);
#endif
}

fn void camera_set_pos(Camera* cam, v3f pos)
{
#if Z_UP
    cam->pos.x = pos.x;
    cam->pos.y = pos.z;
    cam->pos.z = -pos.y;
#else
    cam->pos = pos;
#endif
}

fn v3f camera_get_pos(Camera* cam)
{
#if Z_UP
    return { cam->pos.x, -cam->pos.z, cam->pos.y };
#else
    return cam->pos;
#endif
}

}  // namespace tom