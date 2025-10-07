#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Madgine/nodegraph/nodegraphloader.h"
#include "Madgine/nodegraph/pins.h"

#include "Madgine_Tools/resourceeditor.h"
#include "nodegraphfile.h"

namespace Engine {
namespace Tools {

    namespace ed = ax::NodeEditor;

    struct NodeGraphEditor : public Tool<NodeGraphEditor, ResourceEditor> {

        SERIALIZABLEUNIT(NodeGraphEditor)

        NodeGraphEditor(ImRoot &root);
        NodeGraphEditor(const NodeGraphEditor &) = delete;

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void render() override;

        std::string_view key() const override;

        void save(const Filesystem::Path &path);
        void open(Resources::ResourceBase *res) override;
        std::string_view getCurrentName() const;



    protected:
        bool saveImpl(std::string_view view);
        size_t loadImpl(char *data);

        void createEditor();

        void queryLink();

        void verify();

        void renderHierarchy();
        void renderSelection();


        void setDragPin(NodeGraph::PinDesc pin);

    private:
        std::unique_ptr<ed::EditorContext, void (*)(ed::EditorContext *)> mEditor = { nullptr, nullptr };
        bool mHierarchyVisible = true;
        bool mNodeDetailsVisible = true;

        NodeGraph::NodeGraphLoader::Handle mGraphHandle;
        NodeGraph::NodeGraph mGraph;
        Filesystem::Path mFilePath;

        struct NodeMessages {
            std::vector<std::string> mErrorMessages;
            std::vector<std::string> mWarningMessages;
        };

        IndexType<uint32_t> mSelectedNodeIndex;
        bool mSelectedInputs = false;

        Vector2 mPopupPosition;

        std::optional<NodeGraph::PinDesc> mDragPin;
        std::optional<ExtendedValueTypeDesc> mDragType;
        uint32_t mDragMask;

        bool mSaveQueued = false;
        bool mIsDirty = false;
        bool mInitialLoad = false;
    };

}
}