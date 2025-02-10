#pragma once

#include <ecs/system/system.h>

class RenderSystem : public System {
public:
    void init() override;

    void update(float dt) override;

private:

};