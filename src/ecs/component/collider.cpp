#include <ecs/component/collider.h>
#include <glm/gtx/quaternion.hpp>
#include <scene/model/primitive_generator.h>

void Collider::generate_visualize_model() {
    std::unordered_map<std::string, std::any> params;
    std::unordered_map<std::string, std::any> color = { { "color_r", 0.0f }, { "color_g", 1.0f }, { "color_b", 0.0f } };
    switch (shape) {
        case SPHERE:
            params          = { { "radius", radius }, { "material", color } };
            visualize_model = PrimitiveGenerator::generate("sphere", params);
            visualize_model->upload(nullptr);
            break;
        case BOX:
            params          = { { "width", half_extents.x },
                                { "height", half_extents.y },
                                { "depth", half_extents.z },
                                { "material", color } };
            visualize_model = PrimitiveGenerator::generate("cube", params);
            visualize_model->upload(nullptr);
            break;
        case CAPSULE:
            params          = { { "height", capsule_height }, { "radius", capsule_radius }, { "material", color } };
            visualize_model = PrimitiveGenerator::generate("capsule", params);
            visualize_model->upload(nullptr);
            break;
        default:
            get_logger()->error("Unsupported collider shape");
            break;
    }
}
