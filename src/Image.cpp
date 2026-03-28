#include "art/Image.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace art {

Image::Image(size_t width, size_t height, PixelFormat format)
  : _width(width), _height(height), _format(format),
    _pixels(width * height * 4, 0.f) {}

Image::Image(Image &&other) noexcept
  : _width(other._width), _height(other._height), _format(other._format),
    _pixels(std::move(other._pixels)), _weights(std::move(other._weights)),
    _resolved(std::move(other._resolved)),
    _resolved_version(other._resolved_version),
    _pixels_completed(other._pixels_completed.load(std::memory_order_relaxed)),
    _version(other._version.load(std::memory_order_relaxed)),
    _on_update(std::move(other._on_update)) {
    other._width = 0;
    other._height = 0;
}

Image &Image::operator=(Image &&other) noexcept {
    if (this != &other) {
        _width = other._width;
        _height = other._height;
        _format = other._format;
        _pixels = std::move(other._pixels);
        _weights = std::move(other._weights);
        _resolved = std::move(other._resolved);
        _resolved_version = other._resolved_version;
        _pixels_completed.store(
            other._pixels_completed.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        _version.store(other._version.load(std::memory_order_relaxed),
                       std::memory_order_relaxed);
        _on_update = std::move(other._on_update);
        other._width = 0;
        other._height = 0;
    }
    return *this;
}

// ── Dimensions ──

auto Image::width() const -> size_t { return _width; }
auto Image::height() const -> size_t { return _height; }
auto Image::format() const -> PixelFormat { return _format; }

// ── Direct pixel access ──

void Image::set_pixel(size_t x, size_t y, float r, float g, float b, float a) {
    size_t idx = _pixel_index(x, y);
    _pixels[idx + 0] = r;
    _pixels[idx + 1] = g;
    _pixels[idx + 2] = b;
    _pixels[idx + 3] = a;
    _version.fetch_add(1, std::memory_order_release);
    _notify(x, y, 1, 1);
}

auto Image::pixel(size_t x, size_t y) const -> std::array<float, 4> {
    size_t idx = _pixel_index(x, y);
    return {
        _pixels[idx + 0], _pixels[idx + 1], _pixels[idx + 2], _pixels[idx + 3]};
}

// ── Sample accumulation ──

void Image::add_sample(size_t x,
                       size_t y,
                       float r,
                       float g,
                       float b,
                       float weight) {
    // Lazily allocate weight buffer on first use.
    if (_weights.empty()) { _weights.resize(_width * _height, 0.f); }
    size_t idx = _pixel_index(x, y);
    size_t pidx = (y * _width + x);
    _pixels[idx + 0] += r * weight;
    _pixels[idx + 1] += g * weight;
    _pixels[idx + 2] += b * weight;
    _pixels[idx + 3] += weight; // alpha accumulates weight for now
    _weights[pidx] += weight;
    _version.fetch_add(1, std::memory_order_release);
    _notify(x, y, 1, 1);
}

auto Image::resolved_pixel(size_t x, size_t y) const -> std::array<float, 4> {
    size_t idx = _pixel_index(x, y);

    if (_weights.empty()) {
        // Direct mode — return as-is.
        return {_pixels[idx + 0],
                _pixels[idx + 1],
                _pixels[idx + 2],
                _pixels[idx + 3]};
    }

    // Accumulation mode — normalize by weight.
    size_t pidx = (y * _width + x);
    float w = _weights[pidx];
    if (w == 0.f) { return {0.f, 0.f, 0.f, 0.f}; }
    float inv_w = 1.f / w;
    return {_pixels[idx + 0] * inv_w,
            _pixels[idx + 1] * inv_w,
            _pixels[idx + 2] * inv_w,
            1.f};
}

// ── Bulk access ──

void Image::set_scanline(size_t y, std::span<const float> rgba_row) {
    assert(rgba_row.size() >= _width * 4);
    size_t idx = _pixel_index(0, y);
    std::copy_n(rgba_row.data(), _width * 4, _pixels.data() + idx);
    _version.fetch_add(1, std::memory_order_release);
    _notify(0, y, _width, 1);
}

void Image::set_tile(size_t x,
                     size_t y,
                     size_t w,
                     size_t h,
                     std::span<const float> rgba_data) {
    assert(rgba_data.size() >= w * h * 4);
    for (size_t row = 0; row < h; ++row) {
        size_t dst_idx = _pixel_index(x, y + row);
        size_t src_idx = row * w * 4;
        std::copy_n(
            rgba_data.data() + src_idx, w * 4, _pixels.data() + dst_idx);
    }
    _version.fetch_add(1, std::memory_order_release);
    _notify(x, y, w, h);
}

auto Image::pixels() const -> std::span<const float> {
    if (_weights.empty()) { return _pixels; }
    _ensure_resolved();
    return _resolved;
}

auto Image::raw_pixels() const -> std::span<const float> { return _pixels; }

// ── Progress tracking ──

void Image::increment_progress(size_t count) {
    _pixels_completed.fetch_add(count, std::memory_order_relaxed);
}

auto Image::pixels_completed() const -> size_t {
    return _pixels_completed.load(std::memory_order_relaxed);
}

auto Image::total_pixels() const -> size_t { return _width * _height; }

auto Image::progress_fraction() const -> float {
    size_t total = total_pixels();
    if (total == 0) return 0.f;
    return static_cast<float>(pixels_completed()) / static_cast<float>(total);
}

// ── Dirty tracking + notification ──

auto Image::version() const -> uint64_t {
    return _version.load(std::memory_order_acquire);
}

void Image::set_on_update(UpdateCallback cb) { _on_update = std::move(cb); }

// ── Conversion ──

auto Image::to_rgba8(float exposure, float gamma) const
    -> std::vector<uint8_t> {
    auto src = pixels(); // resolved if in accumulation mode
    size_t num_pixels = _width * _height;
    std::vector<uint8_t> out(num_pixels * 4);

    float exposure_scale = std::exp2(exposure);
    float inv_gamma = 1.f / gamma;

    for (size_t i = 0; i < num_pixels; ++i) {
        size_t si = i * 4;
        for (size_t c = 0; c < 3; ++c) {
            float v = src[si + c] * exposure_scale;
            v = std::pow(std::max(v, 0.f), inv_gamma);
            v = std::clamp(v, 0.f, 1.f);
            out[si + c] = static_cast<uint8_t>(v * 255.f + 0.5f);
        }
        // Alpha: linear, no tone mapping
        float a = std::clamp(src[si + 3], 0.f, 1.f);
        out[si + 3] = static_cast<uint8_t>(a * 255.f + 0.5f);
    }
    return out;
}

// ── State queries ──

auto Image::is_accumulation_mode() const -> bool { return !_weights.empty(); }

void Image::clear() {
    std::fill(_pixels.begin(), _pixels.end(), 0.f);
    std::fill(_weights.begin(), _weights.end(), 0.f);
    _resolved.clear();
    _resolved_version = 0;
    _pixels_completed.store(0, std::memory_order_relaxed);
    _version.fetch_add(1, std::memory_order_release);
    _notify(0, 0, _width, _height);
}

// ── Internal helpers ──

auto Image::_pixel_index(size_t x, size_t y) const -> size_t {
    assert(x < _width && y < _height);
    return (y * _width + x) * 4;
}

void Image::_notify(size_t x, size_t y, size_t w, size_t h) {
    if (_on_update) { _on_update(x, y, w, h); }
}

void Image::_ensure_resolved() const {
    uint64_t ver = _version.load(std::memory_order_acquire);
    if (_resolved_version == ver && !_resolved.empty()) { return; }

    size_t num_pixels = _width * _height;
    _resolved.resize(num_pixels * 4);

    for (size_t i = 0; i < num_pixels; ++i) {
        size_t idx = i * 4;
        float w = _weights[i];
        if (w == 0.f) {
            _resolved[idx + 0] = 0.f;
            _resolved[idx + 1] = 0.f;
            _resolved[idx + 2] = 0.f;
            _resolved[idx + 3] = 0.f;
        } else {
            float inv_w = 1.f / w;
            _resolved[idx + 0] = _pixels[idx + 0] * inv_w;
            _resolved[idx + 1] = _pixels[idx + 1] * inv_w;
            _resolved[idx + 2] = _pixels[idx + 2] * inv_w;
            _resolved[idx + 3] = 1.f;
        }
    }
    _resolved_version = ver;
}

} // namespace art
