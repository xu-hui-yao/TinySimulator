#pragma once

#include <assets/shader/shader.h>
#include <core/event/event.h>
#include <ecs/system/system.h>

class RenderSystem : public System {
public:
    ~RenderSystem() override;

    void init() override;

    void update(float dt) override;

private:
    std::shared_ptr<Shader> m_shader;

    void window_size_listener(const Event& event);
};