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

    template <typename T>
    struct Traced;
    struct Trace;

    struct UndoStack;
}
}

namespace tinyxml2 {
class XMLDocument;
class XMLElement;
}

struct ImGuiTestEngine;
