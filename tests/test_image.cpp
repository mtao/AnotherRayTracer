#include <catch2/catch_all.hpp>

#include "art/Image.hpp"

using namespace art;

TEST_CASE("image_construction", "[image]") {
    SECTION("default construction") {
        Image img;
        CHECK(img.width() == 0);
        CHECK(img.height() == 0);
        CHECK(img.total_pixels() == 0);
        CHECK(img.progress_fraction() == 0.f);
    }

    SECTION("sized construction") {
        Image img(100, 50);
        CHECK(img.width() == 100);
        CHECK(img.height() == 50);
        CHECK(img.format() == Image::PixelFormat::RGBAF32);
        CHECK(img.total_pixels() == 5000);
        CHECK(img.pixels_completed() == 0);
    }

    SECTION("pixels initialized to zero") {
        Image img(4, 4);
        auto px = img.pixel(0, 0);
        CHECK(px[0] == 0.f);
        CHECK(px[1] == 0.f);
        CHECK(px[2] == 0.f);
        CHECK(px[3] == 0.f);

        auto px2 = img.pixel(3, 3);
        CHECK(px2[0] == 0.f);
    }
}

TEST_CASE("image_set_pixel", "[image]") {
    Image img(10, 10);

    img.set_pixel(5, 3, 1.f, 0.5f, 0.25f, 0.75f);

    auto px = img.pixel(5, 3);
    CHECK(px[0] == 1.f);
    CHECK(px[1] == 0.5f);
    CHECK(px[2] == 0.25f);
    CHECK(px[3] == 0.75f);

    // Other pixels unaffected
    auto other = img.pixel(0, 0);
    CHECK(other[0] == 0.f);
}

TEST_CASE("image_set_pixel_default_alpha", "[image]") {
    Image img(4, 4);
    img.set_pixel(0, 0, 0.5f, 0.5f, 0.5f);

    auto px = img.pixel(0, 0);
    CHECK(px[3] == 1.f);
}

TEST_CASE("image_resolved_pixel_direct_mode", "[image]") {
    Image img(4, 4);
    img.set_pixel(2, 1, 0.8f, 0.6f, 0.4f, 1.f);

    auto px = img.resolved_pixel(2, 1);
    CHECK(px[0] == 0.8f);
    CHECK(px[1] == 0.6f);
    CHECK(px[2] == 0.4f);
    CHECK(px[3] == 1.f);
}

TEST_CASE("image_sample_accumulation", "[image]") {
    Image img(4, 4);

    CHECK_FALSE(img.is_accumulation_mode());

    // Add two samples with equal weight
    img.add_sample(1, 1, 1.f, 0.f, 0.f, 1.f); // red, weight 1
    img.add_sample(1, 1, 0.f, 0.f, 1.f, 1.f); // blue, weight 1

    CHECK(img.is_accumulation_mode());

    auto px = img.resolved_pixel(1, 1);
    CHECK(px[0] == Catch::Approx(0.5f)); // (1*1 + 0*1) / 2
    CHECK(px[1] == Catch::Approx(0.f));
    CHECK(px[2] == Catch::Approx(0.5f)); // (0*1 + 1*1) / 2
    CHECK(px[3] == Catch::Approx(1.f));
}

TEST_CASE("image_sample_accumulation_weighted", "[image]") {
    Image img(4, 4);

    // Add samples with different weights
    img.add_sample(0, 0, 1.f, 0.f, 0.f, 3.f); // red, weight 3
    img.add_sample(0, 0, 0.f, 1.f, 0.f, 1.f); // green, weight 1

    auto px = img.resolved_pixel(0, 0);
    // total weight = 4
    CHECK(px[0] == Catch::Approx(0.75f)); // (1*3) / 4
    CHECK(px[1] == Catch::Approx(0.25f)); // (1*1) / 4
    CHECK(px[2] == Catch::Approx(0.f));
}

TEST_CASE("image_sample_accumulation_zero_weight", "[image]") {
    Image img(4, 4);
    img.add_sample(0, 0, 1.f, 1.f, 1.f, 0.f);

    auto px = img.resolved_pixel(0, 0);
    CHECK(px[0] == 0.f);
    CHECK(px[1] == 0.f);
    CHECK(px[2] == 0.f);
    CHECK(px[3] == 0.f);
}

TEST_CASE("image_set_scanline", "[image]") {
    Image img(3, 2);

    // Row 0: red, green, blue
    std::vector<float> row = {
        1.f,
        0.f,
        0.f,
        1.f, // red
        0.f,
        1.f,
        0.f,
        1.f, // green
        0.f,
        0.f,
        1.f,
        1.f, // blue
    };
    img.set_scanline(0, row);

    auto r = img.pixel(0, 0);
    CHECK(r[0] == 1.f);
    CHECK(r[1] == 0.f);

    auto g = img.pixel(1, 0);
    CHECK(g[1] == 1.f);

    auto b = img.pixel(2, 0);
    CHECK(b[2] == 1.f);

    // Row 1 should still be zero
    auto zero = img.pixel(0, 1);
    CHECK(zero[0] == 0.f);
}

TEST_CASE("image_set_tile", "[image]") {
    Image img(8, 8);

    // Set a 2x2 tile at position (3, 4)
    std::vector<float> tile = {
        0.1f,
        0.2f,
        0.3f,
        1.f, // (3,4)
        0.4f,
        0.5f,
        0.6f,
        1.f, // (4,4)
        0.7f,
        0.8f,
        0.9f,
        1.f, // (3,5)
        1.0f,
        0.9f,
        0.8f,
        1.f, // (4,5)
    };
    img.set_tile(3, 4, 2, 2, tile);

    auto p00 = img.pixel(3, 4);
    CHECK(p00[0] == Catch::Approx(0.1f));
    CHECK(p00[1] == Catch::Approx(0.2f));

    auto p10 = img.pixel(4, 4);
    CHECK(p10[0] == Catch::Approx(0.4f));

    auto p01 = img.pixel(3, 5);
    CHECK(p01[0] == Catch::Approx(0.7f));

    auto p11 = img.pixel(4, 5);
    CHECK(p11[0] == Catch::Approx(1.0f));

    // Adjacent pixel unchanged
    auto adj = img.pixel(2, 4);
    CHECK(adj[0] == 0.f);
}

TEST_CASE("image_pixels_span_direct_mode", "[image]") {
    Image img(2, 2);
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f);
    img.set_pixel(1, 0, 0.f, 1.f, 0.f, 1.f);

    auto span = img.pixels();
    CHECK(span.size() == 16); // 2*2*4
    CHECK(span[0] == 1.f); // (0,0).r
    CHECK(span[5] == 1.f); // (1,0).g
}

TEST_CASE("image_pixels_span_accumulation_mode", "[image]") {
    Image img(2, 1);
    img.add_sample(0, 0, 2.f, 0.f, 0.f, 2.f);
    img.add_sample(1, 0, 0.f, 4.f, 0.f, 4.f);

    auto span = img.pixels();
    CHECK(span.size() == 8); // 2*1*4
    // pixel (0,0): accumulated r = 2*2 = 4, weight = 2, resolved = 4/2 = 2
    CHECK(span[0] == Catch::Approx(2.f));
    // pixel (1,0): accumulated g = 4*4 = 16, weight = 4, resolved = 16/4 = 4
    CHECK(span[5] == Catch::Approx(4.f));
}

TEST_CASE("image_raw_pixels", "[image]") {
    Image img(2, 1);
    img.add_sample(0, 0, 2.f, 0.f, 0.f, 2.f);

    auto raw = img.raw_pixels();
    CHECK(raw[0] == Catch::Approx(4.f)); // 2 * 2 (accumulated)
}

TEST_CASE("image_progress_tracking", "[image]") {
    Image img(10, 10);

    CHECK(img.pixels_completed() == 0);
    CHECK(img.progress_fraction() == Catch::Approx(0.f));

    img.increment_progress(50);
    CHECK(img.pixels_completed() == 50);
    CHECK(img.progress_fraction() == Catch::Approx(0.5f));

    img.increment_progress(50);
    CHECK(img.pixels_completed() == 100);
    CHECK(img.progress_fraction() == Catch::Approx(1.f));
}

TEST_CASE("image_version_tracking", "[image]") {
    Image img(4, 4);

    uint64_t v0 = img.version();
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f);
    uint64_t v1 = img.version();
    CHECK(v1 > v0);

    img.set_pixel(1, 1, 0.f, 1.f, 0.f, 1.f);
    uint64_t v2 = img.version();
    CHECK(v2 > v1);
}

TEST_CASE("image_update_callback", "[image]") {
    Image img(8, 8);

    size_t cb_count = 0;
    size_t last_x = 0, last_y = 0, last_w = 0, last_h = 0;

    img.set_on_update([&](size_t x, size_t y, size_t w, size_t h) {
        cb_count++;
        last_x = x;
        last_y = y;
        last_w = w;
        last_h = h;
    });

    img.set_pixel(3, 5, 1.f, 0.f, 0.f);
    CHECK(cb_count == 1);
    CHECK(last_x == 3);
    CHECK(last_y == 5);
    CHECK(last_w == 1);
    CHECK(last_h == 1);

    std::vector<float> row(8 * 4, 0.5f);
    img.set_scanline(2, row);
    CHECK(cb_count == 2);
    CHECK(last_x == 0);
    CHECK(last_y == 2);
    CHECK(last_w == 8);
    CHECK(last_h == 1);

    std::vector<float> tile(2 * 3 * 4, 0.5f);
    img.set_tile(1, 4, 2, 3, tile);
    CHECK(cb_count == 3);
    CHECK(last_x == 1);
    CHECK(last_y == 4);
    CHECK(last_w == 2);
    CHECK(last_h == 3);
}

TEST_CASE("image_to_rgba8", "[image]") {
    Image img(2, 1);

    // Set known pixel values
    img.set_pixel(0, 0, 1.f, 0.f, 0.f, 1.f); // pure red
    img.set_pixel(1, 0, 0.f, 0.f, 0.f, 0.f); // black, transparent

    // Default exposure=0, gamma=2.2
    auto bytes = img.to_rgba8();
    CHECK(bytes.size() == 8); // 2*1*4

    // Red pixel: pow(1.0 * 1.0, 1/2.2) = 1.0 -> 255
    CHECK(bytes[0] == 255);
    CHECK(bytes[1] == 0);
    CHECK(bytes[2] == 0);
    CHECK(bytes[3] == 255);

    // Black pixel
    CHECK(bytes[4] == 0);
    CHECK(bytes[5] == 0);
    CHECK(bytes[6] == 0);
    CHECK(bytes[7] == 0);
}

TEST_CASE("image_to_rgba8_exposure", "[image]") {
    Image img(1, 1);
    img.set_pixel(0, 0, 0.5f, 0.5f, 0.5f, 1.f);

    // +1 EV doubles the value: 0.5 * 2 = 1.0
    auto bytes = img.to_rgba8(1.f, 1.f); // gamma=1 for simple math
    CHECK(bytes[0] == 255);
    CHECK(bytes[1] == 255);
    CHECK(bytes[2] == 255);
}

TEST_CASE("image_clear", "[image]") {
    Image img(4, 4);

    img.set_pixel(2, 2, 1.f, 1.f, 1.f, 1.f);
    img.increment_progress(5);

    uint64_t v_before = img.version();
    img.clear();

    auto px = img.pixel(2, 2);
    CHECK(px[0] == 0.f);
    CHECK(px[1] == 0.f);
    CHECK(px[2] == 0.f);
    CHECK(px[3] == 0.f);
    CHECK(img.pixels_completed() == 0);
    CHECK(img.version() > v_before);
}

TEST_CASE("image_clear_accumulation_mode", "[image]") {
    Image img(4, 4);

    img.add_sample(0, 0, 1.f, 0.f, 0.f, 1.f);
    CHECK(img.is_accumulation_mode());

    img.clear();
    // Weight buffer still allocated, so still in accumulation mode,
    // but values should be zero
    auto px = img.resolved_pixel(0, 0);
    CHECK(px[0] == 0.f);
    CHECK(px[1] == 0.f);
    CHECK(px[2] == 0.f);
    CHECK(px[3] == 0.f);
}
