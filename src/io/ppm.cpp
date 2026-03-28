#include "art/io/image_io.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

namespace art::io::ppm {

auto load(const std::filesystem::path &path)
    -> std::expected<Image, std::string> {
    std::ifstream file(path, std::ios::binary);
    if (!file) { return std::unexpected("Cannot open file: " + path.string()); }

    // Read magic number
    std::string magic;
    file >> magic;
    if (magic != "P6") {
        return std::unexpected("Not a binary PPM file (expected P6, got "
                               + magic + ")");
    }

    // Skip whitespace and comments
    auto skip_comments = [&file]() {
        file >> std::ws;
        while (file.peek() == '#') {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            file >> std::ws;
        }
    };

    skip_comments();

    size_t width, height;
    file >> width;
    skip_comments();
    file >> height;
    skip_comments();

    int max_val;
    file >> max_val;

    // Consume the single whitespace character after maxval
    file.get();

    if (!file || width == 0 || height == 0) {
        return std::unexpected("Invalid PPM header in " + path.string());
    }

    if (max_val != 255) {
        return std::unexpected("Only 8-bit PPM supported (maxval="
                               + std::to_string(max_val) + ")");
    }

    // Read RGB pixels
    std::vector<uint8_t> rgb(width * height * 3);
    file.read(reinterpret_cast<char *>(rgb.data()),
              static_cast<std::streamsize>(rgb.size()));
    if (!file) {
        return std::unexpected("Truncated pixel data in " + path.string());
    }

    // Convert RGB8 -> RGBAF32
    Image img(width, height);
    for (size_t i = 0; i < width * height; ++i) {
        float r = static_cast<float>(rgb[i * 3 + 0]) / 255.f;
        float g = static_cast<float>(rgb[i * 3 + 1]) / 255.f;
        float b = static_cast<float>(rgb[i * 3 + 2]) / 255.f;
        size_t x = i % width;
        size_t y = i / width;
        img.set_pixel(x, y, r, g, b, 1.f);
    }

    return img;
}

auto save(const std::filesystem::path &path,
          const Image &image,
          const SaveOptions &opts) -> std::expected<void, std::string> {
    if (image.width() == 0 || image.height() == 0) {
        return std::unexpected("Cannot save empty image");
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected("Cannot open file for writing: "
                               + path.string());
    }

    auto rgba8 = image.to_rgba8(opts.exposure, opts.gamma);

    // Write PPM P6 header
    file << "P6\n"
         << image.width() << " " << image.height() << "\n"
         << "255\n";

    // Write RGB pixels (strip alpha)
    for (size_t i = 0; i < image.width() * image.height(); ++i) {
        file.put(static_cast<char>(rgba8[i * 4 + 0]));
        file.put(static_cast<char>(rgba8[i * 4 + 1]));
        file.put(static_cast<char>(rgba8[i * 4 + 2]));
    }

    if (!file) { return std::unexpected("Write error to " + path.string()); }

    return {};
}

} // namespace art::io::ppm
