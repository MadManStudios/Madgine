#pragma once

namespace Engine {
namespace Tools {
    struct ToolBase;
    struct ImRoot;
    struct Inspector;

    struct SceneEditor;

    template <typename... T>
    struct Dialog;
        
    struct DialogPromise;
}
}

namespace tinyxml2 {
class XMLDocument;
class XMLElement;
}