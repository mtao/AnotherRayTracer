#if !defined(ART_IO_IMAGE_IO_HPP)
#define ART_IO_IMAGE_IO_HPP

#include <expected>
#include <filesystem>
#include <string>

#include "art/Image.hpp"
#include "art/export.hpp"

namespace art::io {

// ── Format enum ──

enum class ImageFormat {
    EXR, // OpenEXR via tinyexr — HDR float, lossless
    PNG, // PNG via stb_image — 8-bit LDR, lossless
    PPM, // Portable PixMap — 8-bit, no dependency, always available
    Auto, // Deduce from file extension
};

// ── Save options (for LDR conversion) ──

struct SaveOptions {
    float exposure = 0.f; // EV stops
    float gamma = 2.2f; // gamma curve
};

// ── Generic load/save (dispatch on extension or explicit format) ──

/// Load an image from any supported format.
/// Format::Auto deduces from extension: .exr -> EXR, .png -> PNG, .ppm -> PPM.
ART_API auto load(const std::filesystem::path &path,
                  ImageFormat format = ImageFormat::Auto)
    -> std::expected<Image, std::string>;

/// Save an image to any supported format.
/// For LDR formats (PNG, PPM): applies tone mapping (exposure + gamma).
/// For HDR formats (EXR): writes float data directly.
ART_API auto save(const std::filesystem::path &path,
                  const Image &image,
                  ImageFormat format = ImageFormat::Auto,
                  const SaveOptions &opts = {})
    -> std::expected<void, std::string>;

// ── Query which formats are compiled in ──

ART_API auto has_exr_support() -> bool;
ART_API auto has_png_support() -> bool;
ART_API auto has_ppm_support() -> bool; // always true

// ── Format-specific functions ──

namespace exr {
    ART_API auto load(const std::filesystem::path &path)
        -> std::expected<Image, std::string>;
    ART_API auto save(const std::filesystem::path &path, const Image &image)
        -> std::expected<void, std::string>;
} // namespace exr

namespace png {
    ART_API auto load(const std::filesystem::path &path)
        -> std::expected<Image, std::string>;
    ART_API auto save(const std::filesystem::path &path,
                      const Image &image,
                      const SaveOptions &opts = {})
        -> std::expected<void, std::string>;
} // namespace png

namespace ppm {
    ART_API auto load(const std::filesystem::path &path)
        -> std::expected<Image, std::string>;
    ART_API auto save(const std::filesystem::path &path,
                      const Image &image,
                      const SaveOptions &opts = {})
        -> std::expected<void, std::string>;
} // namespace ppm

} // namespace art::io

#endif // ART_IO_IMAGE_IO_HPP
