#include "art/io/image_io.hpp"

#ifdef ART_HAS_EXR

#include <tinyexr.h>

#include <cstring>

namespace art::io {

auto has_exr_support() -> bool { return true; }

namespace exr {

    auto load(const std::filesystem::path &path)
        -> std::expected<Image, std::string> {
        float *rgba = nullptr;
        int width = 0, height = 0;
        const char *err = nullptr;

        int ret = LoadEXR(&rgba, &width, &height, path.string().c_str(), &err);
        if (ret != TINYEXR_SUCCESS) {
            std::string msg = err ? std::string(err) : "Unknown EXR error";
            if (err) FreeEXRErrorMessage(err);
            return std::unexpected("Failed to load EXR: " + msg);
        }

        Image img(static_cast<size_t>(width), static_cast<size_t>(height));
        // tinyexr LoadEXR returns RGBA float interleaved
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                size_t i = static_cast<size_t>(y * width + x) * 4;
                img.set_pixel(static_cast<size_t>(x),
                              static_cast<size_t>(y),
                              rgba[i + 0],
                              rgba[i + 1],
                              rgba[i + 2],
                              rgba[i + 3]);
            }
        }
        free(rgba);

        return img;
    }

    auto save(const std::filesystem::path &path, const Image &image)
        -> std::expected<void, std::string> {
        if (image.width() == 0 || image.height() == 0) {
            return std::unexpected("Cannot save empty image");
        }

        auto pixels = image.pixels();
        int width = static_cast<int>(image.width());
        int height = static_cast<int>(image.height());

        const char *err = nullptr;
        int ret = SaveEXR(pixels.data(),
                          width,
                          height,
                          /*num_channels=*/4,
                          /*save_as_fp16=*/1,
                          path.string().c_str(),
                          &err);
        if (ret != TINYEXR_SUCCESS) {
            std::string msg = err ? std::string(err) : "Unknown EXR error";
            if (err) FreeEXRErrorMessage(err);
            return std::unexpected("Failed to save EXR: " + msg);
        }

        return {};
    }

} // namespace exr

} // namespace art::io

#else // !ART_HAS_EXR

namespace art::io {

auto has_exr_support() -> bool { return false; }

namespace exr {

    auto load(const std::filesystem::path &)
        -> std::expected<Image, std::string> {
        return std::unexpected(
            "EXR support not compiled; rebuild with -Dexr=true");
    }

    auto save(const std::filesystem::path &, const Image &)
        -> std::expected<void, std::string> {
        return std::unexpected(
            "EXR support not compiled; rebuild with -Dexr=true");
    }

} // namespace exr

} // namespace art::io

#endif // ART_HAS_EXR
