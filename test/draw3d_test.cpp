
/**
 * @file draw3d_test.cpp
 * @brief draw3d_t 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-06<td>Zone.N<td>创建文件
 * </table>
 */

#include "gtest/gtest.h"

#include "camera.h"
#include "display.h"
#include "draw3d.h"
#include "vector.hpp"

static constexpr const uint32_t                  WIDTH  = 1920;
static constexpr const uint32_t                  HEIGHT = 1080;
[[maybe_unused]] static constexpr const uint32_t RED    = 0xFFFF0000;
[[maybe_unused]] static constexpr const uint32_t GREEN  = 0xFF00FF00;
[[maybe_unused]] static constexpr const uint32_t BLUE   = 0xFF0000FF;
[[maybe_unused]] static constexpr const uint32_t WHITE  = 0xFFFFFFFF;
[[maybe_unused]] static constexpr const uint32_t BLACK  = 0xFF000000;

TEST(draw3d_t, line) {
    auto     framebuffer = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);
    draw3d_t draw3d(framebuffer);

    // 右对角线
    draw3d.line(0, HEIGHT - 1, WIDTH - 1, 0, WHITE);
    // 左对角线
    draw3d.line(WIDTH - 1, HEIGHT - 1, 0, 0, WHITE);
    // 水平平分线
    draw3d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, WHITE);
    // 垂直平分线
    draw3d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, WHITE);

    // 红色三角形
    vector4f_t v0(80, 80);
    vector4f_t v1(100, 100);
    vector4f_t v2(50, 110);
    draw3d.triangle2d(v0, v1, v2, RED);

    // 绿色三角形
    vector4f_t v3(80, 200);
    vector4f_t v4(100, 300);
    vector4f_t v5(50, 400);
    draw3d.triangle2d(v3, v4, v5, GREEN);

    // 红白绿线
    vector4f_t line1(100, 200, 300);
    auto       mmm = matrix4f_t().translate(1000, 500, 0);
    line1          = mmm * line1;
    draw3d.line(0, 0, line1.x, line1.y, RED);
    mmm = matrix4f_t().translate(1000, 500, 0).rotate(vector4f_t(0, 0, 1), -15);
    line1 = vector4f_t(100, 200, 300);
    line1 = mmm * line1;
    draw3d.line(0, 0, line1.x, line1.y, WHITE);
    mmm = matrix4f_t().translate(1000, 500, 0).rotate(vector4f_t(0, 0, 1), 15);
    line1 = vector4f_t(100, 200, 300);
    line1 = mmm * line1;
    draw3d.line(0, 0, line1.x, line1.y, GREEN);

    // display_t display(framebuffer);
    // display.loop();

    return;
}

// 模型变换
const matrix4f_t
get_model_matrix(const vector4f_t& _scale, const vector4f_t& _rotate,
                 const float& _angle, const vector4f_t& _translate) {
    // 缩放
    auto scale    = matrix4f_t().scale(_scale.x, _scale.y, _scale.z);

    // 旋转
    auto rotation = matrix4f_t().rotate(_rotate, _angle);

    // 平移
    auto translate
      = matrix4f_t().translate(_translate.x, _translate.y, _translate.z);

    // 应用到向量上时先线性变换(缩放，旋转)再平移
    /// @bug 临时写成 tsr 顺序
    // return translate * rotation * scale;
    return translate * scale * rotation;
}

// 视图变换
// 将相机移动到原点，方向指向 -z，即屏幕里，up 为 -y，即屏幕向上
const matrix4f_t get_view_matrix(const camera_t& _camera) {
    // 相机位置
    const auto& pos      = camera.pos;
    // 相机方向
    const auto& target   = camera.target;
    // 相机上
    const auto& up       = camera.up;
    // 相机 z 轴方向
    const auto  camera_c = target ^ up;

    // auto rotation = matrix4f_t().rotate_from_to(up, vector4f_t(0, 0, 1));
    auto        rotation = matrix4f_t().rotate(up.normalize(), 180);

    std::cout << "++++++++++" << std::endl;
    // std::cout << g << std::endl;
    // std::cout << t << std::endl;
    // std::cout << e << std::endl;

    // throw(66);

    std::cout << rotation << std::endl;
    // std::cout << translate * rotation << std::endl;
    //
    // std::cout << rotation * (translate * target) << std::endl;
    // std::cout << rotation * (translate * up) << std::endl;
    // std::cout << rotation * (translate * pos) << std::endl;
    std::cout << "--------" << std::endl;

    // 先平移再旋转
    // 移动到原点
    auto translate = matrix4f_t().translate(-pos.x, -pos.y, -pos.z);

    return translate * rotation;
}

const matrix4f_t get_projection_matrix(float _eye_fov, float _aspect_ratio,
                                       float _zNear, float _zFar) {
    /// @see
    /// https://www.yuque.com/sugelameiyoudi-jadcc/okgm7e/1ee187f999897025b219d35c36826359
    matrix4f_t projection  = matrix4f_t();

    float      arr_m[4][4] = {
        {_zNear,      0,              0,               0},
        {     0, _zNear,              0,               0},
        {     0,      0, _zNear + _zFar, -_zNear * _zFar},
        {     0,      0,              1,               0}
    };
    matrix4f_t m(arr_m);

    float      halve       = _eye_fov / 2 * M_PI / 180;
    float      top         = std::tan(halve) * _zNear;
    float      bottom      = -top;
    float      right       = top * _aspect_ratio;
    float      left        = -right;

    float      arr_n[4][4] = {
        {2 / (right - left),                  0,                    0, 0},
        {                 0, 2 / (top - bottom),                    0, 0},
        {                 0,                  0, 2 / (_zNear - _zFar), 0},
        {                 0,                  0,                    0, 1}
    };
    matrix4f_t n(arr_n);

    float      arr_p[4][4] = {
        {1, 0, 0,   -(right + left) / 2},
        {0, 1, 0,   -(top + bottom) / 2},
        {0, 0, 1, -(_zFar + _zNear) / 2},
        {0, 0, 0,                     1}
    };
    matrix4f_t p(arr_p);

    projection = n * p * m;

    return matrix4f_t(projection);
}

TEST(draw3d_t, obj) {
    auto     framebuffer = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);
    draw3d_t draw3d(framebuffer);

    // 茶壶
    auto     obj_path = "../../obj/utah-teapot/utah-teapot.obj";
    model_t  model(obj_path);
    auto     model_mat
      = get_model_matrix(vector4f_t(10, 10, 1), vector4f_t(1, 1, 1).normalize(),
                         190, vector4f_t(500, 500, 0));
    camera.pos    = vector4f_t(900, 0, 0);
    camera.up     = vector4f_t(0, 0, -1);
    camera.target = vector4f_t(1, 1, 0);
    auto view_mat = get_view_matrix(camera);

    draw3d.model(model, model_mat, view_mat, matrix4f_t());

    // 颜色正方体
    obj_path  = "../../obj/cube3.obj";
    model     = model_t(obj_path);
    model_mat = get_model_matrix(vector4f_t(1000, 1000, 1),
                                 vector4f_t(1, 1, 1).normalize(), 190,
                                 vector4f_t(1000, 500, 0));
    // draw3d.model(model, model_mat, view_mat, matrix4f_t());

    display_t display(framebuffer);
    display.loop();

    return;
}
