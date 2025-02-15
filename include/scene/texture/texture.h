#pragma once

#include <memory>
#include <scene/resource/resource.h>

enum TextureType {
    /** Dummy value.
     *
     *  No texture, but the value to be used as 'texture semantic'
     *  (#aiMaterialProperty::mSemantic) for all material properties
     *  *not* related to textures.
     */
    ENone = 0,

    /** LEGACY API MATERIALS
     * Legacy refers to materials which
     * Were originally implemented in the specifications around 2000.
     * These must never be removed, as most engines support them.
     */

    /** The texture is combined with the result of the diffuse
     *  lighting equation.
     *  OR
     *  PBR Specular/Glossiness
     */
    EDiffuse = 1,

    /** The texture is combined with the result of the specular
     *  lighting equation.
     *  OR
     *  PBR Specular/Glossiness
     */
    ESpecular = 2,

    /** The texture is combined with the result of the ambient
     *  lighting equation.
     */
    EAmbient = 3,

    /** The texture is added to the result of the lighting
     *  calculation. It isn't influenced by incoming light.
     */
    EEmitter = 4,

    /** The texture is a height map.
     *
     *  By convention, higher gray-scale values stand for
     *  higher elevations from the base height.
     */
    EHeight = 5,

    /** The texture is a (tangent space) normal-map.
     *
     *  Again, there are several conventions for tangent-space
     *  normal maps. Assimp does (intentionally) not
     *  distinguish here.
     */
    ENormal = 6,

    /** The texture defines the glossiness of the material.
     *
     *  The glossiness is in fact the exponent of the specular
     *  (phong) lighting equation. Usually there is a conversion
     *  function defined to map the linear color values in the
     *  texture to a suitable exponent. Have fun.
     */
    EShininess = 7,

    /** The texture defines per-pixel opacity.
     *
     *  Usually 'white' means opaque and 'black' means
     *  'transparency'. Or quite the opposite. Have fun.
     */
    EOpacity = 8,

    /** Displacement texture
     *
     *  The exact purpose and format is application-dependent.
     *  Higher color values stand for higher vertex displacements.
     */
    EDisplacement = 9,

    /** Light map texture (aka Ambient Occlusion)
     *
     *  Both 'Light maps' and dedicated 'ambient occlusion maps' are
     *  covered by this material property. The texture contains a
     *  scaling value for the final color value of a pixel. Its
     *  intensity is not affected by incoming light.
     */
    ELightMap = 10,

    /** Reflection texture
     *
     * Contains the color of a perfect mirror reflection.
     * Rarely used, almost never for real-time applications.
     */
    EReflection = 11,

    /** PBR Materials
     * PBR definitions from maya and other modelling packages now use this standard.
     * This was originally introduced around 2012.
     * Support for this is in game engines like Godot, Unreal or Unity3D.
     * Modelling packages which use this are very common now.
     */

    EBaseColor        = 12,
    ENormalCamera     = 13,
    EEmissionColor    = 14,
    EMetallic         = 15,
    EDiffuseRoughness = 16,
    EAmbientOcclusion = 17,

    /** Unknown texture
     *
     *  A texture reference that does not match any of the definitions
     *  above is considered to be 'unknown'. It is still imported,
     *  but is excluded from any further post-processing.
     */
    EUnknown = 18,

    /** PBR Material Modifiers
     * Some modern renderers have further PBR modifiers that may be overlaid
     * on top of the 'base' PBR materials for additional realism.
     * These use multiple texture maps, so only the base type is directly defined
     */

    /** Sheen
     * Generally used to simulate textiles that are covered in a layer of microfibers
     * e.g. velvet
     * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
     */
    ESheen = 19,

    /** Clear coat
     * Simulates a layer of 'polish' or 'lacquer' layered on top of a PBR substrate
     * https://autodesk.github.io/standard-surface/#closures/coating
     * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat
     */
    EClearCoat = 20,

    /** Transmission
     * Simulates transmission through the surface
     * May include further information such as wall thickness
     */
    ETransmission = 21,

    /**
     * Maya material declarations
     */
    EMayaBase              = 22,
    EMayaSpecular          = 23,
    EMayaSpecularColor     = 24,
    EMayaSpecularRoughness = 25,
};

struct TextureDescriptor : ResourceDescriptor {
    enum Format { e_float, e_uint };
    enum Color { e_linear, e_srgb };
    Format format;
    Color color;
    bool generate_mipmaps;
    int wrap_s;
    int wrap_t;
    int min_filter;
    int mag_filter;

    TextureDescriptor();
};

class Texture : public Resource {
public:
    Texture(int height, int width, int channel) noexcept;

    ~Texture() noexcept override;

    Texture(Texture &&other) noexcept;

    Texture &operator=(Texture &&other) noexcept;

    float operator()(int row, int col, int channel) const noexcept;

    float &operator()(int row, int col, int channel) noexcept;

    [[nodiscard]] int get_height() const noexcept;

    [[nodiscard]] int get_width() const noexcept;

    [[nodiscard]] int get_channel() const noexcept;

    [[nodiscard]] bool exist_data() const noexcept;

    [[nodiscard]] float *get() const noexcept;

    [[nodiscard]] TextureType get_type() const noexcept;

    void set_type(TextureType type) noexcept;

    void upload(std::shared_ptr<ResourceDescriptor> resource_descriptor) noexcept override;

    void unload() noexcept override;

    size_t &get_hash() noexcept;

    [[nodiscard]] size_t get_hash() const noexcept;

private:
    uint32_t m_id;
    size_t m_hash;
    TextureType m_type;
    std::unique_ptr<float[]> m_data;
    int m_channel, m_width, m_height;

    Texture() noexcept;
};

float srgb_to_linear(float c) noexcept;

float linear_to_srgb(float c) noexcept;
