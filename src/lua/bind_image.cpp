/// @file src/lua/bind_image.cpp
/// Lua bindings for ART Image.

#include "bind_internal.hpp"

#include "art/Image.hpp"
#include "art/io/image_io.hpp"

namespace art::lua {

void bind_image(sol::table &tbl) {
    tbl.new_usertype<Image>(
        "Image",
        sol::no_constructor,

        "width",
        &Image::width,
        "height",
        &Image::height,
        "progress",
        &Image::progress_fraction,

        "pixel",
        [](const Image &img, size_t x, size_t y, sol::this_state ts) {
            auto px = img.pixel(x, y);
            sol::table t = sol::state_view(ts).create_table(4, 0);
            t[1] = px[0];
            t[2] = px[1];
            t[3] = px[2];
            t[4] = px[3];
            return t;
        },

        "set_pixel",
        [](Image &img, size_t x, size_t y, float r, float g, float b) {
            img.set_pixel(x, y, r, g, b, 1.f);
        });

    // Image I/O
    tbl.set_function("save_image",
                     [](const std::string &path,
                        const Image &img,
                        sol::this_state ts) -> std::pair<bool, sol::object> {
                         auto result = art::io::save(path, img);
                         if (result) { return {true, sol::lua_nil}; }
                         return {false,
                                 sol::make_object(sol::state_view(ts),
                                                  std::string(result.error()))};
                     });
}

} // namespace art::lua
