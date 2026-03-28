#include "art/io/image_io.hpp"

#include <algorithm>

namespace art::io {

namespace {

    auto deduce_format(const std::filesystem::path &path)
        -> std::expected<ImageFormat, std::string> {
        auto ext = path.extension().string();
        std::transform(ext.begin(),
                       ext.end(),
                       ext.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (ext == ".exr") return ImageFormat::EXR;
        if (ext == ".png") return ImageFormat::PNG;
        if (ext == ".ppm") return ImageFormat::PPM;
        return std::unexpected("Unknown image format for extension '" + ext
                               + "'");
    }

} // namespace

auto load(const std::filesystem::path &path, ImageFormat format)
    -> std::expected<Image, std::string> {
    if (format == ImageFormat::Auto) {
        auto fmt = deduce_format(path);
        if (!fmt) return std::unexpected(fmt.error());
        format = *fmt;
    }

    switch (format) {
    case ImageFormat::EXR:
        return exr::load(path);
    case ImageFormat::PNG:
        return png::load(path);
    case ImageFormat::PPM:
        return ppm::load(path);
    case ImageFormat::Auto:
        break; // unreachable
    }
    return std::unexpected("Unknown image format");
}

auto save(const std::filesystem::path &path,
          const Image &image,
          ImageFormat format,
          const SaveOptions &opts) -> std::expected<void, std::string> {
    if (format == ImageFormat::Auto) {
        auto fmt = deduce_format(path);
        if (!fmt) return std::unexpected(fmt.error());
        format = *fmt;
    }

    switch (format) {
    case ImageFormat::EXR:
        return exr::save(path, image);
    case ImageFormat::PNG:
        return png::save(path, image, opts);
    case ImageFormat::PPM:
        return ppm::save(path, image, opts);
    case ImageFormat::Auto:
        break; // unreachable
    }
    return std::unexpected("Unknown image format");
}

auto has_ppm_support() -> bool { return true; }

} // namespace art::io
