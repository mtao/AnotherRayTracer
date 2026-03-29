#include "art/io/image_io.hpp"

#ifdef ART_HAS_PNG

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace art::io {

auto has_png_support() -> bool { return true; }

namespace png {

    auto load(const std::filesystem::path &path)
        -> std::expected<Image, std::string> {
        int width = 0, height = 0, channels = 0;
        unsigned char *data =
            stbi_load(path.string().c_str(), &width, &height, &channels, 4);
        if (!data) {
            return std::unexpected("Failed to load PNG: "
                                   + std::string(stbi_failure_reason()));
        }

        Image img(static_cast<size_t>(width), static_cast<size_t>(height));
        // Convert RGBA8 -> RGBAF32 (assuming sRGB input)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                size_t i = static_cast<size_t>(y * width + x) * 4;
                float r = static_cast<float>(data[i + 0]) / 255.f;
                float g = static_cast<float>(data[i + 1]) / 255.f;
                float b = static_cast<float>(data[i + 2]) / 255.f;
                float a = static_cast<float>(data[i + 3]) / 255.f;
                img.set_pixel(
                    static_cast<size_t>(x), static_cast<size_t>(y), r, g, b, a);
            }
        }
        stbi_image_free(data);

        return img;
    }

    auto save(const std::filesystem::path &path,
              const Image &image,
              const SaveOptions &opts) -> std::expected<void, std::string> {
        if (image.width() == 0 || image.height() == 0) {
            return std::unexpected("Cannot save empty image");
        }

        auto rgba8 = image.to_rgba8(opts.exposure, opts.gamma);
        int width = static_cast<int>(image.width());
        int height = static_cast<int>(image.height());

        int ret = stbi_write_png(
            path.string().c_str(), width, height, 4, rgba8.data(), width * 4);
        if (ret == 0) {
            return std::unexpected("Failed to write PNG: " + path.string());
        }

        return {};
    }

} // namespace png

} // namespace art::io

#else // !ART_HAS_PNG

namespace art::io {

auto has_png_support() -> bool { return false; }

namespace png {

    auto load(const std::filesystem::path &)
        -> std::expected<Image, std::string> {
        return std::unexpected(
            "PNG support not compiled; rebuild with -Dpng=true");
    }

    auto save(const std::filesystem::path &, const Image &, const SaveOptions &)
        -> std::expected<void, std::string> {
        return std::unexpected(
            "PNG support not compiled; rebuild with -Dpng=true");
    }

} // namespace png

} // namespace art::io

#endif // ART_HAS_PNG
