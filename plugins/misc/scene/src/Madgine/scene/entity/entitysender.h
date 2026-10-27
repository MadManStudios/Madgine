#pragma once

#include "entity.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        template <typename Rec>
        struct EntityState {

            EntityState(SceneContainer &container, std::string name, EntityPtr parent, EntityHandle *handle, EntityDescriptor init, EntityPtr &outWrite, Rec &&rec)
                : mEntity(container, name, std::move(parent), handle)
                , mState(Execution::connect(mEntity.lifetimeSender(outWrite), std::forward<Rec>(rec)))
            {
                // The caller is expected to hold the Scene Lock while attaching this state
                init.apply(mEntity);                 
            }

            void start()
            {
                // Start is guaranteed and required to happen as well while holding the lock and attaching the state
                mState.start();
            }

            void stop()
            {
                mEntity.endLifetime();
            }

            using State = Execution::connect_result_t<std::invoke_result_t<decltype(&Entity::lifetimeSender), Entity &, EntityPtr &>, Rec>;

            Entity mEntity;
            State mState;
        };

        struct EntitySender : Execution::base_sender {

            using result_type = void;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, EntitySender &&sender, Rec &&rec)
            {
                return EntityState<Rec> { sender.mContainer, std::move(sender.mName), std::move(sender.mParent), sender.mHandle, std::move(sender.mInit), sender.mOutWrite, std::forward<Rec>(rec) };
            }

            SceneContainer &mContainer;
            std::string mName;
            EntityPtr mParent;
            EntityDescriptor mInit;
            EntityPtr &mOutWrite;
            EntityHandle *mHandle = nullptr;
        };

    }
}
}