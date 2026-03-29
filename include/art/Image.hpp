#if !defined(ART_IMAGE_HPP)
#define ART_IMAGE_HPP

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <vector>

#include "art/export.hpp"

namespace art {

/// PBRT Film-style pixel buffer with sample accumulation.
///
/// Supports two modes of pixel writing:
///   1. Direct: set_pixel() sets the final color value.
///   2. Accumulation: add_sample() accumulates weighted color contributions;
///      resolved_pixel() divides by total weight for the final value.
///
/// Thread safety: version() and progress counters are atomic. Write methods
/// (set_pixel, add_sample, set_tile, set_scanline) are NOT synchronized —
/// callers must ensure no two threads write the same pixel concurrently.
/// The on_update callback is called from the writing thread.
class ART_API Image {
  public:
    enum class PixelFormat { RGBA8, RGBAF32 };

    Image() = default;
    Image(size_t width,
          size_t height,
          PixelFormat format = PixelFormat::RGBAF32);

    // Move-only (std::atomic members prevent implicit copy/move).
    Image(const Image &) = delete;
    auto operator=(const Image &) -> Image & = delete;
    Image(Image &&other) noexcept;
    auto operator=(Image &&other) noexcept -> Image &;

    // ── Dimensions ──

    auto width() const -> size_t;
    auto height() const -> size_t;
    auto format() const -> PixelFormat;

    // ── Direct pixel access ──

    void
        set_pixel(size_t x, size_t y, float r, float g, float b, float a = 1.f);
    auto pixel(size_t x, size_t y) const -> std::array<float, 4>;

    // ── Sample accumulation (PBRT Film pattern) ──
    //
    // An Integrator calls add_sample() for each path/light sample.
    // The Image accumulates weighted color contributions and tracks
    // the total weight per pixel for correct normalization.

    void add_sample(size_t x,
                    size_t y,
                    float r,
                    float g,
                    float b,
                    float weight = 1.f);

    /// Get the current (normalized) color at a pixel.
    /// For add_sample mode: divides accumulated color by total weight.
    /// For set_pixel mode: returns the last-set value.
    auto resolved_pixel(size_t x, size_t y) const -> std::array<float, 4>;

    // ── Bulk access ──

    /// Set a full scanline (row y). Data must be width()*4 floats (RGBA).
    void set_scanline(size_t y, std::span<const float> rgba_row);

    /// Set a rectangular tile. Data must be w*h*4 floats (RGBA), row-major.
    void set_tile(size_t x,
                  size_t y,
                  size_t w,
                  size_t h,
                  std::span<const float> rgba_data);

    /// Full resolved pixel buffer (row-major, 4 floats per pixel).
    /// For set_pixel mode: direct values.
    /// For add_sample mode: normalized (resolved) values.
    auto pixels() const -> std::span<const float>;

    /// Raw accumulated buffer (un-normalized). For internal use
    /// or advanced consumers that want to apply their own normalization.
    auto raw_pixels() const -> std::span<const float>;

    // ── Progress tracking ──
    // Thread-safe (atomic). The render loop increments this.

    void increment_progress(size_t count = 1);
    auto pixels_completed() const -> size_t;
    auto total_pixels() const -> size_t;
    auto progress_fraction() const -> float;

    // ── Dirty tracking + notification ──

    /// Version is bumped on every write operation.
    auto version() const -> uint64_t;

    /// Callback invoked after each write with the dirty region.
    /// Called from the writing thread — must be lightweight.
    using UpdateCallback =
        std::function<void(size_t x, size_t y, size_t w, size_t h)>;
    void set_on_update(UpdateCallback cb);

    // ── Conversion ──

    /// Produce an RGBA8 buffer (for saving to 8-bit formats).
    /// Applies the given exposure (EV stops) and gamma.
    auto to_rgba8(float exposure = 0.f, float gamma = 2.2f) const
        -> std::vector<uint8_t>;

    // ── State queries ──

    /// True if add_sample() has been called (weight buffer is allocated).
    auto is_accumulation_mode() const -> bool;

    /// Clear all pixel data and weights to zero. Dimensions unchanged.
    void clear();

  private:
    size_t m_width = 0;
    size_t m_height = 0;
    PixelFormat m_format = PixelFormat::RGBAF32;

    // Pixel storage: 4 floats per pixel (R, G, B, A), row-major.
    std::vector<float> m_pixels;

    // Per-pixel weight accumulator for add_sample mode.
    // Empty when using set_pixel mode only.
    std::vector<float> m_weights;

    // Resolved (normalized) cache — lazily computed from m_pixels/m_weights.
    mutable std::vector<float> m_resolved;
    mutable uint64_t m_resolved_version = 0;

    // Thread-safe progress and versioning.
    std::atomic<size_t> m_pixels_completed = 0;
    std::atomic<uint64_t> m_version = 0;

    UpdateCallback m_on_update;

    // ── Internal helpers ──

    auto pixel_index(size_t x, size_t y) const -> size_t;
    void notify(size_t x, size_t y, size_t w, size_t h);
    void ensure_resolved() const;
};

} // namespace art

#endif // ART_IMAGE_HPP
