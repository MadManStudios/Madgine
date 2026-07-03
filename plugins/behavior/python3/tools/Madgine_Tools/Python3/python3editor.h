#pragma once

#include "Madgine/behavior/behaviorhandle.h"
#include "Madgine/debug/debuglistener.h"

#include "Madgine_Tools/resources/resourceeditor.h"
#include "Madgine_Tools/resources/resourcefile.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"
#include "Python3/python3fileloader.h"
#include "python3file.h"

namespace Engine {
namespace Tools {

    struct Python3Editor : public Tool<Python3Editor, ResourceEditor>, Debug::DebugListener {

        SERIALIZABLEUNIT(Python3Editor)

        Python3Editor(ImRoot &root);
        Python3Editor(const Python3Editor &) = delete;

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void update() override;
        void render() override;

        std::string_view key() const override;

        void open(Resources::ResourceBase *res) override;
        std::string_view getCurrentName() const;

        Dialog<> closeDialog() override;

    protected:
        bool wantsPause(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type, IndexType<size_t> line) override;
        void onSuspend(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type) override;

    private:
        friend struct Python3File;
        std::map<Behavior::Python3::Python3FileLoader::Resource *, Python3File> mFiles;
    };

}
}