#pragma once

#include "../scenecomponent.h"

namespace Engine {
namespace Scene {
    namespace Entity {
        template <typename T, typename C>
        struct System : SceneComponent<T> {
            using SceneComponent<T>::SerializableUnit;
            virtual ~System() = default;

            virtual void addComponent(C *c)
            {
                mComponents.push_back(c);
            }

            virtual void removeComponent(C *c)
            {
                std::erase(mComponents, c);
            }

            const std::vector<C *> &components()
            {
                return mComponents;
            }

        private:
            std::vector<C *> mComponents;
        };
    }
}
}
