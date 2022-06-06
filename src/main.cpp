
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
#include "geometry.h"
#include "image.h"
#include "iostream"
#include "model.h"
#include "renderer.h"
#include "test.h"
#include "vector.hpp"
#include "gtkmm.h"

using namespace std;

Test                          test;
static constexpr const size_t WIDTH  = 800;
static constexpr const size_t HEIGHT = 600;

class Screen : public Gtk::DrawingArea {
private:
protected:
    bool on_timeout(void) {
        // force our program to redraw the entire clock.
        queue_draw();
        return true;
    }
    void on_draw(const Cairo::RefPtr<Cairo::Context> &_context, uint32_t _width,
                 uint32_t _height) {
        _context->set_source_rgb(0.5, 0.5, 0.5);
        _context->set_line_width(1.0);
        _context->rectangle(0, 0, _width, _height);
        _context->fill();

        // while_tmp();

        // auto surface = Cairo::ImageSurface::create(
        //     (uint8_t *)device.framebuffer, Cairo::Surface::Format::ARGB32,
        //     _width, _height,
        //     Cairo::ImageSurface::format_stride_for_width(
        //         Cairo::Surface::Format::ARGB32, _width));
        _context->set_source_rgb(0.5, 0, 0);
        // _context->set_source(surface, 0, 0);
        _context->paint();

        return;
    }

public:
    Screen(void) {
        set_content_width(WIDTH);
        set_content_height(HEIGHT);
        Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &Screen::on_timeout), 1000);
        set_draw_func(sigc::mem_fun(*this, &Screen::on_draw));
        return;
    }
    virtual ~Screen(void) {
        return;
    }
};

class ExampleWindow : public Gtk::Window {
private:
    Gtk::Box m_HBox;
    Screen   m_Area_Lines;
    bool on_window_key_pressed(guint _keyval, guint, Gdk::ModifierType _mod) {
        cout << Gtk::Accelerator::name(_keyval, _mod) << endl;
        if (_keyval == GDK_KEY_Escape) {
            hide();
            return true;
        }
        return false;
    }

protected:
public:
    ExampleWindow(void) : m_HBox(Gtk::Orientation::HORIZONTAL) {
        set_title("SimpleRenderer");
        m_HBox.append(m_Area_Lines);
        set_child(m_HBox);
        // Events.
        auto controller = Gtk::EventControllerKey::create();
        controller->signal_key_pressed().connect(
            sigc::mem_fun(*this, &ExampleWindow::on_window_key_pressed), false);
        add_controller(controller);
        return;
    }

    virtual ~ExampleWindow(void) {
        return;
    }
};

int main(int _argc, char **_argv) {
    string filename;
    if (2 == _argc) {
        filename = _argv[1];
    }
    else {
        filename = "../../src/obj/test.obj";
    }
    Model    model(filename);
    TGAImage image(width, height, TGAImage::RGBA);
    Geometry painter(image);
    Renderer render = Renderer(painter, model);
    render.render();
    render.save();
    // test.test_matrix();
    auto app = Gtk::Application::create();
    // Shows the window and returns when it is closed.
    return app->make_window_and_run<ExampleWindow>(_argc, _argv);
    return 0;
}
