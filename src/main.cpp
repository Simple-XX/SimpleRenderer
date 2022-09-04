
/**
 * @file main.cpp
 * @brief 入口
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-06<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "common.h"
#include "iostream"
#include "framebuffer.h"
#include "model.h"
#include "renderer.h"
#include "gtkmm.h"

using namespace std;

/// @note TMP
uint8_t *bitmap = nullptr;

// gtk 窗体
class Screen : public Gtk::DrawingArea {
private:
protected:
    // 超时事件
    bool on_timeout(void);
    // 绘制
    void on_draw(const Cairo::RefPtr<Cairo::Context> &_context, uint32_t _width,
                 uint32_t _height);

public:
    Screen(void);
    virtual ~Screen(void);
};

// gtk 应用
class SimpleRenderer_window : public Gtk::Window {
private:
    Gtk::Box box;
    Screen   screen;
    // 键盘事件
    bool on_window_key_pressed(guint _keyval, guint, Gdk::ModifierType _mod);

protected:
public:
    SimpleRenderer_window(void);
    virtual ~SimpleRenderer_window(void);
};

bool Screen::on_timeout(void) {
    // force our program to redraw the entire clock.
    queue_draw();
    return true;
}

void Screen::on_draw(const Cairo::RefPtr<Cairo::Context> &_context,
                     uint32_t _width, uint32_t _height) {
    // 设置颜色 灰色
    _context->set_source_rgb(0.5, 0.5, 0.5);
    // 设置线宽
    _context->set_line_width(1.0);
    // 填充全屏
    _context->rectangle(0, 0, _width, _height);
    _context->fill();

    // 创建位图
    auto surface = Cairo::ImageSurface::create(
        bitmap, Cairo::Surface::Format::ARGB32, _width, _height,
        Cairo::ImageSurface::format_stride_for_width(
            Cairo::Surface::Format::ARGB32, _width));
    // 用位图填充屏幕
    _context->set_source(surface, 0, 0);
    _context->paint();

    return;
}

Screen::Screen(void) {
    set_content_width(WIDTH);
    set_content_height(HEIGHT);
    // 注册超时事件
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Screen::on_timeout),
                                   1000);
    // 注册绘制函数
    set_draw_func(sigc::mem_fun(*this, &Screen::on_draw));
    return;
}

Screen::~Screen(void) {
    return;
}

bool SimpleRenderer_window::on_window_key_pressed(guint _keyval, guint,
                                                  Gdk::ModifierType _mod) {
    if (_keyval == GDK_KEY_Escape) {
        hide();
        return true;
    }
    std::cout << Gtk::Accelerator::name(_keyval, _mod) << std::endl;
    return false;
}

SimpleRenderer_window::SimpleRenderer_window(void)
    : box(Gtk::Orientation::HORIZONTAL) {
    // 设置窗口标题
    set_title("SimpleRenderer");
    // 添加子组件
    box.append(screen);
    set_child(box);
    // 注册键盘事件
    auto controller = Gtk::EventControllerKey::create();
    controller->signal_key_pressed().connect(
        sigc::mem_fun(*this, &SimpleRenderer_window::on_window_key_pressed),
        false);
    add_controller(controller);
    return;
}

SimpleRenderer_window::~SimpleRenderer_window(void) {
    return;
}

int main(int _argc, char **_argv) {
    // obj 路径
    string obj_path;
    // 材质路径
    string mtl_path;
    // 如果没有指定那么使用默认值
    if (_argc == 1) {
        obj_path = "../../src/obj/cube.obj";
        mtl_path = "../../src/obj/";
    }
    // 否则使用指定的
    else {
        if (_argc == 2) {
            obj_path = _argv[1];
        }
        if (_argc == 3) {
            mtl_path = _argv[2];
        }
    }

    // 读取模型与材质
    model_t model(obj_path, mtl_path);
    // 启动窗口
    auto app = Gtk::Application::create();
    // Shows the window and returns when it is closed.
    return app->make_window_and_run<SimpleRenderer_window>(_argc, _argv);
    return 0;
}
