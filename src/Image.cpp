#include "art/Image.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace art {

Image::Image(size_t width, size_t height, PixelFormat format)
  : m_width(width), m_height(height), m_format(format),
    m_pixels(width * height * 4, 0.f) {}

Image::Image(Image &&other) noexcept
  : m_width(other.m_width), m_height(other.m_height), m_format(other.m_format),
    m_pixels(std::move(other.m_pixels)), m_weights(std::move(other.m_weights)),
    m_resolved(std::move(other.m_resolved)),
    m_resolved_version(other.m_resolved_version),
    m_pixels_completed(
        other.m_pixels_completed.load(std::memory_order_relaxed)),
    m_version(other.m_version.load(std::memory_order_relaxed)),
    m_on_update(std::move(other.m_on_update)) {
    other.m_width = 0;
    other.m_height = 0;
}

auto Image::operator=(Image &&other) noexcept -> Image & {
    if (this != &other) {
        m_width = other.m_width;
        m_height = other.m_height;
        m_format = other.m_format;
        m_pixels = std::move(other.m_pixels);
        m_weights = std::move(other.m_weights);
        m_resolved = std::move(other.m_resolved);
        m_resolved_version = other.m_resolved_version;
        m_pixels_completed.store(
            other.m_pixels_completed.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        m_version.store(other.m_version.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
        m_on_update = std::move(other.m_on_update);
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}

// ── Dimensions ──

auto Image::width() const -> size_t { return m_width; }
auto Image::height() const -> size_t { return m_height; }
auto Image::format() const -> PixelFormat { return m_format; }

// ── Direct pixel access ──

void Image::set_pixel(size_t x, size_t y, float r, float g, float b, float a) {
    size_t idx = pixel_index(x, y);
    m_pixels[idx + 0] = r;
    m_pixels[idx + 1] = g;
    m_pixels[idx + 2] = b;
    m_pixels[idx + 3] = a;
    m_version.fetch_add(1, std::memory_order_release);
    notify(x, y, 1, 1);
}

auto Image::pixel(size_t x, size_t y) const -> std::array<float, 4> {
    size_t idx = pixel_index(x, y);
    return {m_pixels[idx + 0],
            m_pixels[idx + 1],
            m_pixels[idx + 2],
            m_pixels[idx + 3]};
}

// ── Sample accumulation ──

void Image::add_sample(size_t x,
                       size_t y,
                       float r,
                       float g,
                       float b,
                       float weight) {
    // Lazily allocate weight buffer on first use.
    if (m_weights.empty()) { m_weights.resize(m_width * m_height, 0.f); }
    size_t idx = pixel_index(x, y);
    size_t pidx = (y * m_width + x);
    m_pixels[idx + 0] += r * weight;
    m_pixels[idx + 1] += g * weight;
    m_pixels[idx + 2] += b * weight;
    m_pixels[idx + 3] += weight; // alpha accumulates weight for now
    m_weights[pidx] += weight;
    m_version.fetch_add(1, std::memory_order_release);
    notify(x, y, 1, 1);
}

auto Image::resolved_pixel(size_t x, size_t y) const -> std::array<float, 4> {
    size_t idx = pixel_index(x, y);

    if (m_weights.empty()) {
        // Direct mode — return as-is.
        return {m_pixels[idx + 0],
                m_pixels[idx + 1],
                m_pixels[idx + 2],
                m_pixels[idx + 3]};
    }

    // Accumulation mode — normalize by weight.
    size_t pidx = (y * m_width + x);
    float w = m_weights[pidx];
    if (w == 0.f) { return {0.f, 0.f, 0.f, 0.f}; }
    float inv_w = 1.f / w;
    return {m_pixels[idx + 0] * inv_w,
            m_pixels[idx + 1] * inv_w,
            m_pixels[idx + 2] * inv_w,
            1.f};
}

// ── Bulk access ──

void Image::set_scanline(size_t y, std::span<const float> rgba_row) {
    if (rgba_row.size() < m_width * 4) { return; }
    size_t idx = pixel_index(0, y);
    std::copy_n(rgba_row.data(), m_width * 4, m_pixels.data() + idx);
    m_version.fetch_add(1, std::memory_order_release);
    notify(0, y, m_width, 1);
}

void Image::set_tile(size_t x,
                     size_t y,
                     size_t w,
                     size_t h,
                     std::span<const float> rgba_data) {
    if (rgba_data.size() < w * h * 4) { return; }
    for (size_t row = 0; row < h; ++row) {
        size_t dst_idx = pixel_index(x, y + row);
        size_t src_idx = row * w * 4;
        std::copy_n(
            rgba_data.data() + src_idx, w * 4, m_pixels.data() + dst_idx);
    }
    m_version.fetch_add(1, std::memory_order_release);
    notify(x, y, w, h);
}

auto Image::pixels() const -> std::span<const float> {
    if (m_weights.empty()) { return m_pixels; }
    ensure_resolved();
    return m_resolved;
}

auto Image::raw_pixels() const -> std::span<const float> { return m_pixels; }

// ── Progress tracking ──

void Image::increment_progress(size_t count) {
    m_pixels_completed.fetch_add(count, std::memory_order_relaxed);
}

auto Image::pixels_completed() const -> size_t {
    return m_pixels_completed.load(std::memory_order_relaxed);
}

auto Image::total_pixels() const -> size_t { return m_width * m_height; }

auto Image::progress_fraction() const -> float {
    size_t total = total_pixels();
    if (total == 0) return 0.f;
    return static_cast<float>(pixels_completed()) / static_cast<float>(total);
}

// ── Dirty tracking + notification ──

auto Image::version() const -> uint64_t {
    return m_version.load(std::memory_order_acquire);
}

void Image::set_on_update(UpdateCallback cb) { m_on_update = std::move(cb); }

// ── Conversion ──

auto Image::to_rgba8(float exposure, float gamma) const
    -> std::vector<uint8_t> {
    auto src = pixels(); // resolved if in accumulation mode
    size_t num_pixels = m_width * m_height;
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

auto Image::is_accumulation_mode() const -> bool {
    return !m_weights.empty();
}

void Image::clear() {
    std::fill(m_pixels.begin(), m_pixels.end(), 0.f);
    std::fill(m_weights.begin(), m_weights.end(), 0.f);
    m_resolved.clear();
    m_resolved_version = 0;
    m_pixels_completed.store(0, std::memory_order_relaxed);
    m_version.fetch_add(1, std::memory_order_release);
    notify(0, 0, m_width, m_height);
}

// ── Internal helpers ──

auto Image::pixel_index(size_t x, size_t y) const -> size_t {
    if (x >= m_width || y >= m_height) { std::unreachable(); }
    return (y * m_width + x) * 4;
}

void Image::notify(size_t x, size_t y, size_t w, size_t h) {
    if (m_on_update) { m_on_update(x, y, w, h); }
}

void Image::ensure_resolved() const {
    uint64_t ver = m_version.load(std::memory_order_acquire);
    if (m_resolved_version == ver && !m_resolved.empty()) { return; }

    size_t num_pixels = m_width * m_height;
    m_resolved.resize(num_pixels * 4);

    for (size_t i = 0; i < num_pixels; ++i) {
        size_t idx = i * 4;
        float w = m_weights[i];
        if (w == 0.f) {
            m_resolved[idx + 0] = 0.f;
            m_resolved[idx + 1] = 0.f;
            m_resolved[idx + 2] = 0.f;
            m_resolved[idx + 3] = 0.f;
        } else {
            float inv_w = 1.f / w;
            m_resolved[idx + 0] = m_pixels[idx + 0] * inv_w;
            m_resolved[idx + 1] = m_pixels[idx + 1] * inv_w;
            m_resolved[idx + 2] = m_pixels[idx + 2] * inv_w;
            m_resolved[idx + 3] = 1.f;
        }
    }
    m_resolved_version = ver;
}

} // namespace art
