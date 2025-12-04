#pragma once

#include "entity.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        template <typename F, typename Rec>
        struct EntityState {

            EntityState(EntityHandle &handle, SceneContainer &container, std::string name, F init, Rec &&rec)
                : mEntity(handle, container, name)
                , mState(Execution::connect(mEntity.lifetimeSender(), std::forward<Rec>(rec)))
            {
                std::forward<F>(init)(mEntity);
            }

            void start()
            {
                mState.start();
            }

            void stop()
            {
                mEntity.endLifetime();
            }

            using State = Execution::connect_result_t<std::invoke_result_t<decltype(&Entity::lifetimeSender), Entity &>, Rec>;

            Entity mEntity;
            State mState;
        };

        template <typename F>
        struct EntitySender : Execution::base_sender {

            using result_type = void;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, EntitySender &&sender, Rec &&rec)
            {
                return EntityState<F, Rec> { sender.mHandle, sender.mContainer, std::move(sender.mName), std::forward<F>(sender.mInit), std::forward<Rec>(rec) };
            }

            EntityHandle &mHandle;
            SceneContainer &mContainer;
            std::string mName;
            F mInit;
        };

        template <typename F>
        EntitySender(Execution::base_sender, EntityHandle &, SceneContainer &, const std::string &, F &&) -> EntitySender<F>;

    }
}
}