#pragma once

namespace Engine {
namespace Resources {
    struct ResourceManager;
    template <typename Loader>
    struct Resource;
    template <typename T, typename Data, typename Container, typename Storage>
    struct ResourceLoader;
    template <typename T, typename Data, typename Base>
    struct ResourceLoaderImpl;
    struct ResourceBase;
    struct ResourceLoaderBase;
}
}