/*
 * sqmm.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-29

Description: A simple GUI for SQMM calculation

**************************************************/

#include <elements.hpp>

using namespace cycfi::elements;

#define M_PI 3.14159265358979323846

// Main window background color
auto constexpr bkd_color = rgba(35, 35, 37, 255);
auto background = box(bkd_color);

auto box = draw([](context const &ctx) {
   auto &c = ctx.canvas;

   c.begin_path();
   c.add_round_rect(ctx.bounds, 4);
   c.fill_style(colors::gold.opacity(0.8));
   c.fill();
});

auto check_number(std::string_view text) {
   try {
      int value = std::stoi(std::string(text));
      if (value > 0) {
         return true;
      }
   } catch (const std::exception &e) {
   }
   return false;
}

auto my_label(const std::string &text) {
   return margin_right(
       10,
       label(text)
           .text_align(canvas::right)
           .font(
               font_descr{"文泉驿微米黑, \"WenQuanYi Micro Hei\""}.semi_bold())
           .font_color(colors::antique_white)
           .font_size(18));
}

auto result_dialog_content(double A20, double B20, double a0) {
   return hsize(
       300, simple_heading(margin({10, 10, 10, 10},
                                  vtile(htile(my_label("天空sqm："),
                                              my_label(std::to_string(A20)),
                                              my_label("(mag/arcsec²)")),
                                        htile(my_label("天空亮度："),
                                              my_label(std::to_string(B20)),
                                              my_label("(nit)")),
                                        htile(my_label("光污染等级："),
                                              my_label(std::to_string(a0))))),
                           "Result", 1.1));
}

auto make_result_dialog(view &_view, double A20, double B20, double a) {
   auto &&on_ok = []() {};

   auto &&on_cancel = []() {
      // Do something on cancel
   };

   auto dialog =
       dialog2(_view, result_dialog_content(A20, B20, a), on_ok, on_cancel);

   return dialog;
}

auto make_app(view &view_) {
   static float const grid[] = {0.32, 1.0};

   auto my_input = [=](auto caption, auto input) {
      return margin_bottom(10, hgrid(grid, my_label(caption), input));
   };

   // This is an example on how to add an on_text callback and showing an
   // error message if input validation fails:

   auto error_number_input =
       message_box1(view_, "Invalid Input", icons::attention, []() {});

   // Shared flag that tells us if we got the money!
   auto got_the_money = std::make_shared<bool>(false);

   auto screen_light = input_box("must > 0");
   auto screen_exposure = input_box("must > 0");
   auto sky_exposure = input_box("must > 0");
   auto image_light = input_box("must > 0");
   auto light = input_box("must > 0");

   screen_light.second->on_end_focus =
       [input = screen_light.second.get(), &view_,
        error_number_input](std::string_view text) -> bool {
      if (text == "") [[likely]] {
         return true;
      }
      if (!check_number(text)) {
         open_popup(error_number_input, view_);
      }
      return true;
   };

   screen_exposure.second->on_end_focus =
       [input = screen_exposure.second.get(), &view_,
        error_number_input](std::string_view text) -> bool {
      if (text == "") [[likely]] {
         return true;
      }
      if (!check_number(text)) {
         open_popup(error_number_input, view_);
      }
      return true;
   };

   sky_exposure.second->on_end_focus =
       [input = sky_exposure.second.get(), &view_,
        error_number_input](std::string_view text) -> bool {
      if (text == "") [[likely]] {
         return true;
      }
      if (!check_number(text)) {
         open_popup(error_number_input, view_);
      }
      return true;
   };

   image_light.second->on_end_focus =
       [input = image_light.second.get(), &view_,
        error_number_input](std::string_view text) -> bool {
      if (text == "") [[likely]] {
         return true;
      }
      if (!check_number(text)) {
         open_popup(error_number_input, view_);
      }
      return true;
   };

   light.second->on_end_focus =
       [input = light.second.get(), &view_,
        error_number_input](std::string_view text) -> bool {
      if (text == "") [[likely]] {
         return true;
      }
      if (!check_number(text)) {
         open_popup(error_number_input, view_);
      }
      return true;
   };

   auto text_input = pane(
       "Sqmm", margin({10, 5, 10, 5},
                      vtile(my_input("手机屏幕亮度", screen_light.first),
                            my_input("屏幕曝光时间", screen_exposure.first),
                            my_input("天空曝光时间", sky_exposure.first),
                            my_input("照片亮度值", image_light.first),
                            my_input("亮度值", light.first))));

   auto mbutton = share(icon_button(icons::right_circled, 1.2));

   mbutton->on_click = [input_screen_light = screen_light.second.get(),
                        input_screen_exposure = screen_exposure.second.get(),
                        input_sky_exposure = sky_exposure.second.get(),
                        input_image_light = image_light.second.get(),
                        input_light = light.second.get(), error_number_input,
                        &view_](bool) mutable {
      try {
         auto A14 = std::stod(input_screen_light->get_text());
         auto B14 = std::stod(input_screen_exposure->get_text());
         auto A17 = std::stod(input_sky_exposure->get_text());
         auto B17 = std::stod(input_image_light->get_text());
         auto A11 = std::stod(input_light->get_text());

         if (A14 != 0 && B14 != 0 && A17 != 0 && B17 != 0 && A11 != 0)
             [[likely]] {
            double A2 = 140000;
            double B2 = -26.7;
            double C2 = B2 + std::log(A2) / std::log(100);
            double F2 = 180 / M_PI;
            double D2 = std::log((std::pow(F2, 2)) * (std::pow(3600, 2)) * 4) /
                            std::log(100) +
                        C2;
            double C17 = (B14 / A14) / (B17 / A17);
            double B20 = A11 / C17;
            double A20 = std::log(1 / B20) / std::log(100) + D2;
            int a0 = 0;
            if (A20 <= 18.38) {
               a0 = 8;
            } else if (18.38 < A20 && A20 <= 18.94) {
               a0 = 7;
            } else if (18.94 < A20 && A20 <= 19.5) {
               a0 = 6;
            } else if (19.5 < A20 && A20 <= 20.49) {
               a0 = 5;
            } else if (20.49 < A20 && A20 <= 21.69) {
               a0 = 4;
            } else if (21.69 < A20 && A20 <= 21.89) {
               a0 = 3;
            } else if (21.89 < A20 && A20 <= 21.99) {
               a0 = 2;
            } else if (21.99 <= A20) {
               a0 = 1;
            }
            open_popup(make_result_dialog(view_, A20, B20, a0), view_);
         } else [[unlikely]] {
            open_popup(error_number_input, view_);
         }
      } catch (const std::exception &e) {
         open_popup(error_number_input, view_);
      }
      return true;
   };

   return margin({10, 0, 10, 10}, vtile(text_input, hold(mbutton)));
}

auto make_elements(view &view_) {
   return max_size({1280, 640},
                   margin({20, 10, 20, 10},
                          htile(margin({20, 20, 20, 20}, make_app(view_)))));
}

int main(int argc, char *argv[]) {
   app _app("Sqmm Calculator");
   window _win(_app.name());
   _win.on_close = [&_app]() { _app.stop(); };

   view view_(_win);

   view_.content(make_elements(view_), background);

   _app.run();
   return 0;
}
