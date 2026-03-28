#include <catch2/catch_all.hpp>

#include <filesystem>
#include <fstream>

#include "art/io/image_io.hpp"

using namespace art;
using namespace art::io;

TEST_CASE("ppm_save_load_roundtrip", "[io][ppm]") {
    Image img(4, 3);

    // Set some known pixels
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f); // red
    img.set_pixel(1, 0, 0.f, 1.f, 0.f, 1.f); // green
    img.set_pixel(2, 0, 0.f, 0.f, 1.f, 1.f); // blue
    img.set_pixel(3, 0, 1.f, 1.f, 1.f, 1.f); // white
    img.set_pixel(0, 1, 0.5f, 0.5f, 0.5f, 1.f); // gray
    img.set_pixel(0, 2, 0.f, 0.f, 0.f, 1.f); // black

    auto tmp = std::filesystem::temp_directory_path() / "test_art.ppm";

    // Save with gamma=1 and exposure=0 for predictable roundtrip
    SaveOptions opts{.exposure = 0.f, .gamma = 1.f};
    auto save_result = ppm::save(tmp, img, opts);
    REQUIRE(save_result.has_value());

    // Load back
    auto load_result = ppm::load(tmp);
    REQUIRE(load_result.has_value());

    auto &loaded = *load_result;
    CHECK(loaded.width() == 4);
    CHECK(loaded.height() == 3);

    // Red pixel: 1.0 -> 255 -> 1.0 (within 8-bit quantization)
    auto red = loaded.pixel(0, 0);
    CHECK(red[0] == Catch::Approx(1.f).margin(1.f / 255.f));
    CHECK(red[1] == Catch::Approx(0.f).margin(1.f / 255.f));
    CHECK(red[2] == Catch::Approx(0.f).margin(1.f / 255.f));

    // Green pixel
    auto green = loaded.pixel(1, 0);
    CHECK(green[0] == Catch::Approx(0.f).margin(1.f / 255.f));
    CHECK(green[1] == Catch::Approx(1.f).margin(1.f / 255.f));

    // Gray pixel
    auto gray = loaded.pixel(0, 1);
    CHECK(gray[0] == Catch::Approx(0.5f).margin(1.f / 255.f));
    CHECK(gray[1] == Catch::Approx(0.5f).margin(1.f / 255.f));
    CHECK(gray[2] == Catch::Approx(0.5f).margin(1.f / 255.f));

    // Black pixel
    auto black = loaded.pixel(0, 2);
    CHECK(black[0] == Catch::Approx(0.f).margin(1.f / 255.f));

    std::filesystem::remove(tmp);
}

TEST_CASE("ppm_load_nonexistent", "[io][ppm]") {
    auto result = ppm::load("/nonexistent/path.ppm");
    CHECK_FALSE(result.has_value());
}

TEST_CASE("ppm_save_empty_image", "[io][ppm]") {
    Image img;
    auto tmp = std::filesystem::temp_directory_path() / "test_art_empty.ppm";
    auto result = ppm::save(tmp, img);
    CHECK_FALSE(result.has_value());
    std::filesystem::remove(tmp);
}

TEST_CASE("generic_load_save_auto_format", "[io]") {
    Image img(2, 2);
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f);
    img.set_pixel(1, 1, 0.f, 0.f, 1.f, 1.f);

    auto tmp = std::filesystem::temp_directory_path() / "test_art_auto.ppm";

    SaveOptions opts{.exposure = 0.f, .gamma = 1.f};
    auto save_result = save(tmp, img, ImageFormat::Auto, opts);
    REQUIRE(save_result.has_value());

    auto load_result = load(tmp, ImageFormat::Auto);
    REQUIRE(load_result.has_value());

    CHECK(load_result->width() == 2);
    CHECK(load_result->height() == 2);

    std::filesystem::remove(tmp);
}

TEST_CASE("generic_load_unknown_extension", "[io]") {
    auto result = load("/some/file.xyz");
    CHECK_FALSE(result.has_value());
    CHECK(result.error().find("Unknown") != std::string::npos);
}

TEST_CASE("format_support_queries", "[io]") {
    CHECK(has_ppm_support());
    // EXR and PNG depend on build options; just check they return a bool
    [[maybe_unused]] bool exr = has_exr_support();
    [[maybe_unused]] bool png_val = has_png_support();
}

TEST_CASE("exr_stub_returns_error_when_unavailable", "[io][exr]") {
    if (!has_exr_support()) {
        auto result = exr::load("/some/file.exr");
        CHECK_FALSE(result.has_value());
        CHECK(result.error().find("not compiled") != std::string::npos);

        Image img(1, 1);
        auto save_result = exr::save("/some/file.exr", img);
        CHECK_FALSE(save_result.has_value());
    }
}

TEST_CASE("exr_save_load_roundtrip", "[io][exr]") {
    if (!has_exr_support()) { SKIP("EXR support not compiled"); }

    Image img(4, 3);
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f);
    img.set_pixel(1, 0, 0.f, 1.f, 0.f, 1.f);
    img.set_pixel(2, 0, 0.f, 0.f, 1.f, 1.f);
    img.set_pixel(3, 0, 1.f, 1.f, 1.f, 1.f);
    img.set_pixel(0, 1, 0.5f, 0.5f, 0.5f, 1.f);
    img.set_pixel(0, 2, 0.f, 0.f, 0.f, 1.f);

    auto tmp = std::filesystem::temp_directory_path() / "test_art_exr.exr";

    auto save_result = exr::save(tmp, img);
    REQUIRE(save_result.has_value());

    auto load_result = exr::load(tmp);
    REQUIRE(load_result.has_value());

    auto &loaded = *load_result;
    CHECK(loaded.width() == 4);
    CHECK(loaded.height() == 3);

    // EXR uses fp16, so tolerance is larger than 8-bit
    auto red = loaded.pixel(0, 0);
    CHECK(red[0] == Catch::Approx(1.f).margin(0.01f));
    CHECK(red[1] == Catch::Approx(0.f).margin(0.01f));
    CHECK(red[2] == Catch::Approx(0.f).margin(0.01f));

    auto green = loaded.pixel(1, 0);
    CHECK(green[0] == Catch::Approx(0.f).margin(0.01f));
    CHECK(green[1] == Catch::Approx(1.f).margin(0.01f));

    auto gray = loaded.pixel(0, 1);
    CHECK(gray[0] == Catch::Approx(0.5f).margin(0.01f));
    CHECK(gray[1] == Catch::Approx(0.5f).margin(0.01f));
    CHECK(gray[2] == Catch::Approx(0.5f).margin(0.01f));

    std::filesystem::remove(tmp);
}

TEST_CASE("png_stub_returns_error_when_unavailable", "[io][png]") {
    if (!has_png_support()) {
        auto result = png::load("/some/file.png");
        CHECK_FALSE(result.has_value());
        CHECK(result.error().find("not compiled") != std::string::npos);

        Image img(1, 1);
        auto save_result = png::save("/some/file.png", img);
        CHECK_FALSE(save_result.has_value());
    }
}

TEST_CASE("png_save_load_roundtrip", "[io][png]") {
    if (!has_png_support()) { SKIP("PNG support not compiled"); }

    Image img(4, 3);
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f);
    img.set_pixel(1, 0, 0.f, 1.f, 0.f, 1.f);
    img.set_pixel(2, 0, 0.f, 0.f, 1.f, 1.f);
    img.set_pixel(3, 0, 1.f, 1.f, 1.f, 1.f);
    img.set_pixel(0, 1, 0.5f, 0.5f, 0.5f, 1.f);
    img.set_pixel(0, 2, 0.f, 0.f, 0.f, 1.f);

    auto tmp = std::filesystem::temp_directory_path() / "test_art_png.png";

    // Save with gamma=1 for predictable roundtrip
    SaveOptions opts{.exposure = 0.f, .gamma = 1.f};
    auto save_result = png::save(tmp, img, opts);
    REQUIRE(save_result.has_value());

    auto load_result = png::load(tmp);
    REQUIRE(load_result.has_value());

    auto &loaded = *load_result;
    CHECK(loaded.width() == 4);
    CHECK(loaded.height() == 3);

    // PNG is 8-bit, so tolerance matches PPM
    auto red = loaded.pixel(0, 0);
    CHECK(red[0] == Catch::Approx(1.f).margin(1.f / 255.f));
    CHECK(red[1] == Catch::Approx(0.f).margin(1.f / 255.f));
    CHECK(red[2] == Catch::Approx(0.f).margin(1.f / 255.f));

    auto green = loaded.pixel(1, 0);
    CHECK(green[0] == Catch::Approx(0.f).margin(1.f / 255.f));
    CHECK(green[1] == Catch::Approx(1.f).margin(1.f / 255.f));

    auto gray = loaded.pixel(0, 1);
    CHECK(gray[0] == Catch::Approx(0.5f).margin(1.f / 255.f));
    CHECK(gray[1] == Catch::Approx(0.5f).margin(1.f / 255.f));
    CHECK(gray[2] == Catch::Approx(0.5f).margin(1.f / 255.f));

    std::filesystem::remove(tmp);
}
