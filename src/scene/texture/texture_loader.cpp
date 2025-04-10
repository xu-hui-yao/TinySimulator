#include <scene/texture/texture_loader.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include <stb_image_write.h>
#ifndef TINYEXR_IMPLEMENTATION
#define TINYEXR_IMPLEMENTATION
#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#undef min
#undef max
#endif
#include <core/fwd.h>
#include <tinyexr.h>

std::shared_ptr<Resource> TextureLoader::load(const std::filesystem::path &path,
                                              const std::unordered_map<std::string, std::any> &param) {
    /**
     * Default store in linear, float format
     */
    auto [resolve_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("TextureLoader::load(): file " + path.string() + " does not exist");
        return nullptr;
    }

    std::string ext = resolve_path.extension().string();
    std::ranges::transform(ext, ext.begin(),
                           [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });

    if (ext == ".exr") {
        return load_exr(resolve_path);
    }

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
        return load_srgb(resolve_path);
    }

    if (ext == ".hdr") {
        return load_hdr(resolve_path);
    }

    get_logger()->error("Unsupported image format: " + ext);
    return nullptr;
}

bool TextureLoader::save(std::shared_ptr<Resource> resource, const std::filesystem::path &path) {
    if (!path.is_absolute()) {
        get_logger()->warn("TextureLoader::save(): path is not absolute");
    }

    std::string ext = path.extension().string();
    std::ranges::transform(ext, ext.begin(),
                           [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });

    if (ext == ".exr") {
        return save_exr(std::dynamic_pointer_cast<Texture>(resource), path);
    }
    if (ext == ".hdr") {
        return save_hdr(std::dynamic_pointer_cast<Texture>(resource), path);
    }
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
        return save_srgb(std::dynamic_pointer_cast<Texture>(resource), path);
    }
    get_logger()->error("Unsupported image format for saving: " + ext);
    return false;
}

std::shared_ptr<Texture> TextureLoader::load_exr(const std::filesystem::path &path) noexcept {
    std::string filename = path.string();

    EXRVersion exr_version;
    EXRHeader exr_header;
    const char *err = nullptr;
    InitEXRHeader(&exr_header);
    if (ParseEXRHeaderFromFile(&exr_header, &exr_version, filename.c_str(), &err) != TINYEXR_SUCCESS) {
        std::string error_message = err ? std::string(err) : "Unknown error";
        if (err) {
            FreeEXRErrorMessage(err);
        }
        get_logger()->error("Failed to parse EXR header: " + error_message);
        return nullptr;
    }

    float *exr_data;
    int height, width;
    int channel = exr_header.num_channels;

    FreeEXRHeader(&exr_header);

    if (LoadEXR(&exr_data, &width, &height, filename.c_str(), &err) != TINYEXR_SUCCESS) {
        std::string error_message = err ? std::string(err) : "Unknown error";
        if (err)
            FreeEXRErrorMessage(err);
        get_logger()->error("Failed to parse EXR file: " + error_message);
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(height, width, channel);
    // Default linear
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channel; ++c) {
                texture->operator()(y, x, c) = exr_data[(y * width + x) * channel + c];
            }
        }
    }
    free(exr_data);

    return texture;
}

std::shared_ptr<Texture> TextureLoader::load_srgb(const std::filesystem::path &path) noexcept {
    std::string filename = path.string();
    int height, width, channel;

    unsigned char *img_data = stbi_load(filename.c_str(), &width, &height, &channel, 0);
    if (!img_data) {
        get_logger()->error("Failed to load image: " + std::string(stbi_failure_reason()));
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(height, width, channel);
    // Default rgb to linear
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channel; ++c) {
                auto value                   = static_cast<float>(img_data[(y * width + x) * channel + c]) / 255.0f;
                value                        = srgb_to_linear(value);
                texture->operator()(y, x, c) = value;
            }
        }
    }
    stbi_image_free(img_data);

    return texture;
}

std::shared_ptr<Texture> TextureLoader::load_hdr(const std::filesystem::path &path) noexcept {
    std::string filename = path.string();
    int height, width, channel;

    float *hdr_data = stbi_loadf(filename.c_str(), &width, &height, &channel, 0);
    if (!hdr_data) {
        get_logger()->error("Failed to load hdr image: " + std::string(stbi_failure_reason()));
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(height, width, channel);
    // Default linear
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channel; ++c) {
                texture->operator()(y, x, c) = hdr_data[(y * width + x) * channel + c];
            }
        }
    }
    stbi_image_free(hdr_data);

    return texture;
}

bool TextureLoader::save_exr(const std::shared_ptr<Texture> &texture, const std::filesystem::path &path) noexcept {
    if (!texture->exist_data()) {
        get_logger()->error("No data to save for EXR.");
        return false;
    }

    std::string filename = path.string();

    EXRHeader exr_header;
    InitEXRHeader(&exr_header);
    EXRImage exr_image;
    InitEXRImage(&exr_image);

    exr_image.num_channels = texture->get_channel();

    std::vector<float> images[4];
    for (int c = 0; c < texture->get_channel(); c++) {
        images[c].resize(static_cast<size_t>(texture->get_width()) * texture->get_height());
    }

    // Default linear
    for (int y = 0; y < texture->get_height(); ++y) {
        for (int x = 0; x < texture->get_width(); ++x) {
            for (int c = 0; c < texture->get_channel(); ++c) {
                images[c][y * texture->get_width() + x] = texture->operator()(y, x, c);
            }
        }
    }

    std::vector<float *> image_ptr(texture->get_channel());
    for (int c = 0; c < texture->get_channel(); c++) {
        image_ptr[texture->get_channel() - 1 - c] = images[c].data();
    }

    exr_image.images = reinterpret_cast<unsigned char **>(image_ptr.data());
    exr_image.width  = texture->get_width();
    exr_image.height = texture->get_height();

    exr_header.num_channels = texture->get_channel();
    exr_header.channels     = static_cast<EXRChannelInfo *>(malloc(sizeof(EXRChannelInfo) * exr_header.num_channels));
    for (int c = 0; c < exr_header.num_channels; c++) {
        snprintf(exr_header.channels[c].name, 255, c == 0 ? "A" : c == 1 ? "B" : c == 2 ? "G" : "R");
    }

    exr_header.pixel_types           = static_cast<int *>(malloc(sizeof(int) * exr_header.num_channels));
    exr_header.requested_pixel_types = static_cast<int *>(malloc(sizeof(int) * exr_header.num_channels));
    for (int c = 0; c < exr_header.num_channels; c++) {
        exr_header.pixel_types[c]           = TINYEXR_PIXELTYPE_FLOAT;
        exr_header.requested_pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;
    }

    const char *err = nullptr;
    int ret         = SaveEXRImageToFile(&exr_image, &exr_header, filename.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
        std::string error_message = err ? std::string(err) : "Unknown error";
        if (err) {
            FreeEXRErrorMessage(err);
        }
        get_logger()->error("Failed to save EXR: " + error_message);
        return false;
    }

    free(exr_header.channels);
    free(exr_header.pixel_types);
    free(exr_header.requested_pixel_types);

    return true;
}

bool TextureLoader::save_srgb(const std::shared_ptr<Texture> &texture, const std::filesystem::path &path) noexcept {
    if (!texture->exist_data()) {
        get_logger()->error("No data to save for PNG.");
        return false;
    }

    std::string filename = path.string();
    std::vector<unsigned char> out_data(texture->get_width() * texture->get_height() * texture->get_channel());

    // Default srgb
    for (int y = 0; y < texture->get_height(); ++y) {
        for (int x = 0; x < texture->get_width(); ++x) {
            for (int c = 0; c < texture->get_channel(); ++c) {
                float value = texture->operator()(y, x, c);
                value       = linear_to_srgb(value);
                auto uc     = static_cast<unsigned char>(std::clamp(value * 255.0f, 0.0f, 255.0f));
                out_data[(y * texture->get_width() + x) * texture->get_channel() + c] = uc;
            }
        }
    }

    int stride_in_bytes = texture->get_width() * texture->get_channel();
    if (!stbi_write_png(filename.c_str(), texture->get_width(), texture->get_height(), texture->get_channel(),
                        out_data.data(), stride_in_bytes)) {
        get_logger()->error("Failed to save PNG file: " + filename);
        return false;
    }

    return true;
}

bool TextureLoader::save_hdr(const std::shared_ptr<Texture> &texture, const std::filesystem::path &path) noexcept {
    if (!texture->exist_data()) {
        get_logger()->error("No data to save for HDR.");
        return false;
    }

    std::string filename = path.string();
    if (!stbi_write_hdr(filename.c_str(), texture->get_width(), texture->get_height(), texture->get_channel(),
                        texture->get())) {
        get_logger()->error("Failed to save HDR file: " + filename);
        return false;
    }

    return true;
}

std::shared_ptr<TextureLoader> get_texture_loader() {
    static auto texture_loader = std::make_shared<TextureLoader>();
    return texture_loader;
}
