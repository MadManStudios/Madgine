#pragma once

namespace Engine {
namespace Serialize {

    struct META_EXPORT ReadMessage {
        ReadMessage(MessageId id = 0, Formatter *formatter = nullptr)
            : mId(id)
            , mFormatter(formatter)
        {
        }
        ReadMessage(ReadMessage &&other) = default;
        ~ReadMessage();

        ReadMessage &operator=(ReadMessage &&other);

        StreamResult end();

        StreamResult beginHeaderRead();
        StreamResult endHeaderRead();

        SerializeStream &stream();

        explicit operator bool() const;

        MessageId mId;
        Formatter *mFormatter;
    };

}
}