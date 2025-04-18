#include <glad.h>

#include <core/fwd.h>
#include <scene/texture/texture.h>

std::string texture_type_to_string(TextureType type) {
    // Convert the enum value to its string representation
    switch (type) {
        case ENone:
            return "None";
        case EDiffuse:
            return "Diffuse";
        case ESpecular:
            return "Specular";
        case EAmbient:
            return "Ambient";
        case EEmitter:
            return "Emitter";
        case EHeight:
            return "Height";
        case ENormal:
            return "Normal";
        case EShininess:
            return "Shininess";
        case EOpacity:
            return "Opacity";
        case EDisplacement:
            return "Displacement";
        case ELightMap:
            return "LightMap";
        case EReflection:
            return "Reflection";
        case EBaseColor:
            return "BaseColor";
        case ENormalCamera:
            return "NormalCamera";
        case EEmissionColor:
            return "EmissionColor";
        case EMetallic:
            return "Metallic";
        case EDiffuseRoughness:
            return "DiffuseRoughness";
        case EAmbientOcclusion:
            return "AmbientOcclusion";
        case EUnknown:
            return "Unknown";
        case ESheen:
            return "Sheen";
        case EClearCoat:
            return "ClearCoat";
        case ETransmission:
            return "Transmission";
        case EMayaBase:
            return "MayaBase";
        case EMayaSpecular:
            return "MayaSpecular";
        case EMayaSpecularColor:
            return "MayaSpecularColor";
        case EMayaSpecularRoughness:
            return "MayaSpecularRoughness";
        default:
            return "Unknown";
    }
}

TextureDescriptor::TextureDescriptor()
    : generate_mipmaps(true), wrap_s(GL_REPEAT), wrap_t(GL_REPEAT), min_filter(GL_LINEAR_MIPMAP_LINEAR),
      mag_filter(GL_LINEAR) {}

Texture::Texture(int height, int width, int channel) noexcept
    : Resource(std::filesystem::path()), m_id(0), m_type(ENone), m_channel(channel), m_width(width), m_height(height) {
    m_data = std::make_unique<float[]>(width * height * channel);
}

Texture::Texture() noexcept
    : Resource(std::filesystem::path()), m_id(0), m_type(ENone), m_channel(0), m_width(0), m_height(0) {}

Texture::Texture(Texture &&other) noexcept
    : Resource(other.m_path), m_id(std::exchange(other.m_id, 0)), m_type(other.m_type), m_data(std::move(other.m_data)),
      m_channel(other.m_channel), m_width(other.m_width), m_height(other.m_height) {}

Texture &Texture::operator=(Texture &&other) noexcept {
    if (this != &other) {
        unload(); // Release existing resources
        m_id      = std::exchange(other.m_id, 0);
        m_type    = other.m_type;
        m_data    = std::move(other.m_data);
        m_channel = other.m_channel;
        m_width   = other.m_width;
        m_height  = other.m_height;
    }
    return *this;
}

Texture::~Texture() noexcept {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
    m_id = 0;
}

float Texture::operator()(int row, int col, int channel) const noexcept {
#ifdef M_DEBUG
    assert(row >= 0 && row < m_height);
    assert(col >= 0 && col < m_width);
    assert(channel >= 0 && channel < m_channel);
#endif
    return m_data[(row * m_width + col) * m_channel + channel];
}

float &Texture::operator()(int row, int col, int channel) noexcept {
#ifdef M_DEBUG
    assert(row >= 0 && row < m_height);
    assert(col >= 0 && col < m_width);
    assert(channel >= 0 && channel < m_channel);
#endif
    return m_data[(row * m_width + col) * m_channel + channel];
}

int Texture::get_height() const noexcept { return m_height; }

int Texture::get_width() const noexcept { return m_width; }

int Texture::get_channel() const noexcept { return m_channel; }

bool Texture::exist_data() const noexcept { return m_data != nullptr; }

float *Texture::get() const noexcept { return m_data.get(); }

TextureType Texture::get_type() const noexcept { return m_type; }

uint32_t Texture::get_id() const noexcept { return m_id; }

void Texture::set_type(TextureType type) noexcept { m_type = type; }

void Texture::upload(std::shared_ptr<ResourceDescriptor> resource_descriptor) noexcept {
    if (m_id != 0) {
        return;
    }

    auto texture_descriptor = std::dynamic_pointer_cast<TextureDescriptor>(resource_descriptor);

    bool generate_mipmaps = texture_descriptor->generate_mipmaps;

    GLint format;
    GLint internal_format;
    switch (m_channel) {
        case 1:
            format          = GL_RED;
            internal_format = GL_R32F;
            break;
        case 3:
            format          = GL_RGB;
            internal_format = GL_RGB32F;
            break;
        case 4:
            format          = GL_RGBA;
            internal_format = GL_RGBA32F;
            break;
        default:
            get_logger()->error("Unsupported m_channel count: " + std::to_string(m_channel));
            return;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, format, GL_FLOAT, m_data.get());

    if (generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_descriptor->wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_descriptor->wrap_t);
    glTexParameteri(GL_TEXTURE_2D, texture_descriptor->min_filter, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, texture_descriptor->mag_filter, GL_LINEAR);
}

void Texture::unload() noexcept {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
    m_id = 0;
}

float srgb_to_linear(float c) noexcept {
    if (c <= 0.04045f)
        return c / 12.92f;

    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float linear_to_srgb(float c) noexcept {
    if (c < 0.0031308f)
        return 12.92f * c;

    return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

std::shared_ptr<Texture> Texture::create_solid_color(float r, float g, float b, float alpha, TextureType type) {
    auto tex    = std::make_shared<Texture>(1, 1, 4);
    tex->m_type = type;
    tex->m_data[0] = r;
    tex->m_data[1] = g;
    tex->m_data[2] = b;
    tex->m_data[3] = alpha;
    return tex;
}
