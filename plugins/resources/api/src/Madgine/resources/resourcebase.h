#pragma once

#include "Generic/execution/sender.h"

#include "Platform/filesystem/async.h"

#include "Meta/serialize/streams/streamresult.h"

namespace Engine {
namespace Resources {

    struct MADGINE_RESOURCES_EXPORT ResourceBase {
        ResourceBase(const std::string &name, Platform::Filesystem::Path path = {});

        ~ResourceBase() noexcept = default;

        void setPath(const Platform::Filesystem::Path &path);
        const Platform::Filesystem::Path &path() const;
        std::string_view extension();
        std::string_view name();

        std::string plugin() const;

        Stream readAsStream(bool isBinary = false) const;
        std::string readAsText() const;
        std::vector<unsigned char> readAsBlob() const;

        Execution::Sender<GenericResult, Memory::ByteBuffer> readAsync() const;

        template <std::derived_from<Resources::ResourceBase> T, typename Context>
        friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, T*& p, Serialize::FormattedSerializeStream& in, bool success, Context&& context) {
            return { };
        }

    private:
        std::string mName;
        Platform::Filesystem::Path mPath;
    };

}

namespace Serialize {

    template <std::derived_from<Resources::ResourceBase> T>
    struct Operations<T *> {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, T *&p, const char *name, Context &&context)
        {
            std::string resourceName;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, resourceName, name));
            p = T::Loader::getSingleton().get(resourceName);
            return {};
        }

        template <typename Context>
        static void write(FormattedSerializeStream &out, T *const &p, const char *name, Context &&context)
        {
            Serialize::write(out, p->name(), name);
        }
    };

}

}