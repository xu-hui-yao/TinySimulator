#include <assets/texture/texture_loader.h>
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
#endif
#include <tinyexr.h>

std::shared_ptr<Resource> TextureLoader::load(const filesystem::path &path) {
    std::string ext = path.extension();
    std::ranges::transform(ext, ext.begin(),
                           [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });

    if (ext == "exr") {
        return load_exr(path);
    }

    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga") {
        return load_srgb(path);
    }

    if (ext == "hdr") {
        return load_hdr(path);
    }

    global::get_logger()->error("Unsupported image format: " + ext);
    return nullptr;
}

bool TextureLoader::save(std::shared_ptr<Resource> resource, const filesystem::path &path) {
    std::string ext = path.extension();
    std::ranges::transform(ext, ext.begin(),
                           [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });

    if (ext == "exr") {
        return save_exr(std::dynamic_pointer_cast<Texture>(resource), path);
    }
    if (ext == "hdr") {
        return save_hdr(std::dynamic_pointer_cast<Texture>(resource), path);
    }
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga") {
        return save_srgb(std::dynamic_pointer_cast<Texture>(resource), path);
    }
    global::get_logger()->error("Unsupported image format for saving: " + ext);
    return false;
}

std::shared_ptr<Texture> TextureLoader::load_exr(const filesystem::path &path) noexcept {
    std::string filename = path.make_absolute().str();

    EXRVersion exr_version;
    EXRHeader exr_header;
    const char *err = nullptr;
    InitEXRHeader(&exr_header);
    if (ParseEXRHeaderFromFile(&exr_header, &exr_version, filename.c_str(), &err) != TINYEXR_SUCCESS) {
        std::string error_message = err ? std::string(err) : "Unknown error";
        if (err) {
            FreeEXRErrorMessage(err);
        }
        global::get_logger()->error("Failed to parse EXR header: " + error_message);
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
        global::get_logger()->error("Failed to parse EXR file: " + error_message);
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(height, width, channel);
    // Default linear
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channel; ++c) {
                texture->m_data[(y * width + x) * channel + c] = exr_data[(y * width + x) * channel + c];
            }
        }
    }
    free(exr_data);

    return texture;
}

std::shared_ptr<Texture> TextureLoader::load_srgb(const filesystem::path &path) noexcept {
    std::string filename = path.make_absolute().str();
    int height, width, channel;

    unsigned char *img_data = stbi_load(filename.c_str(), &width, &height, &channel, 0);
    if (!img_data) {
        global::get_logger()->error("Failed to load image: " + std::string(stbi_failure_reason()));
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(height, width, channel);
    // Default rgb to linear
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channel; ++c) {
                auto value = static_cast<float>(img_data[(y * width + x) * channel + c]) / 255.0f;
                value      = srgb_to_linear(value);
                texture->m_data[(y * width + x) * channel + c] = value;
            }
        }
    }
    stbi_image_free(img_data);

    return texture;
}

std::shared_ptr<Texture> TextureLoader::load_hdr(const filesystem::path &path) noexcept {
    std::string filename = path.make_absolute().str();
    int height, width, channel;

    float *hdr_data = stbi_loadf(filename.c_str(), &width, &height, &channel, 0);
    if (!hdr_data) {
        global::get_logger()->error("Failed to load hdr image: " + std::string(stbi_failure_reason()));
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(height, width, channel);
    // Default linear
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channel; ++c) {
                texture->m_data[(y * width + x) * channel + c] = hdr_data[(y * width + x) * channel + c];
            }
        }
    }
    stbi_image_free(hdr_data);

    return texture;
}

bool TextureLoader::save_exr(const std::shared_ptr<Texture>& texture, const filesystem::path &path) noexcept {
    if (!texture->m_data) {
        global::get_logger()->error("No data to save for EXR.");
        return false;
    }

    std::string filename  = path.make_absolute().str();
    const float *data_ptr = texture->m_data.get();

    EXRHeader exr_header;
    InitEXRHeader(&exr_header);
    EXRImage exr_image;
    InitEXRImage(&exr_image);

    exr_image.num_channels = texture->m_channel;

    std::vector<float> images[4];
    for (int c = 0; c < texture->m_channel; c++) {
        images[c].resize(static_cast<size_t>(texture->m_width) * texture->m_height);
    }

    // Default linear
    for (int i = 0; i < texture->m_width * texture->m_height; i++) {
        for (int c = 0; c < texture->m_channel; c++) {
            images[c][i] = data_ptr[i * texture->m_channel + c];
        }
    }

    std::vector<float *> image_ptr(texture->m_channel);
    for (int c = 0; c < texture->m_channel; c++) {
        image_ptr[texture->m_channel - 1 - c] = images[c].data();
    }

    exr_image.images = reinterpret_cast<unsigned char **>(image_ptr.data());
    exr_image.width  = texture->m_width;
    exr_image.height = texture->m_height;

    exr_header.num_channels = texture->m_channel;
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
        global::get_logger()->error("Failed to save EXR: " + error_message);
        return false;
    }

    free(exr_header.channels);
    free(exr_header.pixel_types);
    free(exr_header.requested_pixel_types);

    return true;
}

bool TextureLoader::save_srgb(const std::shared_ptr<Texture>& texture, const filesystem::path &path) noexcept {
    if (!texture->m_data) {
        global::get_logger()->error("No data to save for PNG.");
        return false;
    }

    std::string filename = path.make_absolute().str();
    std::vector<unsigned char> out_data(texture->m_width * texture->m_height * texture->m_channel);

    // Default srgb
    for (int i = 0; i < texture->m_width * texture->m_height; i++) {
        for (int c = 0; c < texture->m_channel; c++) {
            float value                          = texture->m_data[i * texture->m_channel + c];
            value                                = linear_to_srgb(value);
            auto uc                              = static_cast<unsigned char>(value * 255.0f);
            out_data[i * texture->m_channel + c] = uc;
        }
    }

    int stride_in_bytes = texture->m_width * texture->m_channel;
    if (!stbi_write_png(filename.c_str(), texture->m_width, texture->m_height, texture->m_channel, out_data.data(),
                        stride_in_bytes)) {
        global::get_logger()->error("Failed to save PNG file: " + filename);
        return false;
    }

    return true;
}

bool TextureLoader::save_hdr(const std::shared_ptr<Texture>& texture, const filesystem::path &path) noexcept {
    if (!texture->m_data) {
        global::get_logger()->error("No data to save for HDR.");
        return false;
    }

    std::string filename = path.make_absolute().str();
    if (!stbi_write_hdr(filename.c_str(), texture->m_width, texture->m_height, texture->m_channel,
                        texture->m_data.get())) {
        global::get_logger()->error("Failed to save HDR file: " + filename);
        return false;
    }

    return true;
}

float TextureLoader::srgb_to_linear(float c) noexcept {
    if (c <= 0.04045f)
        return c / 12.92f;

    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float TextureLoader::linear_to_srgb(float c) noexcept {
    if (c < 0.0031308f)
        return 12.92f * c;

    return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}
