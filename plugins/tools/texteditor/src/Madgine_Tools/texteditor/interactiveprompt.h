#pragma once

#include "Interfaces/filesystem/path.h"

#include "Interfaces/log/log.h"

#include <zep.h>
#include <zep/mode_repl.h>

namespace Engine {
namespace Tools {

    struct Interpreter {
        virtual std::string_view name() = 0;

        virtual bool interpret(std::string_view command) = 0;
    };

    struct Highlighter;

    struct MADGINE_TEXTEDITOR_EXPORT InteractivePrompt : Log::Log {

        InteractivePrompt(TextEditor *editor, Interpreter *interpreter);

        void render();

        void resume();

        std::string getName() override;
        void log(std::string_view msg, Engine::Log::MessageType lvl, const char *file = nullptr, size_t line = 0) override;

    protected:
        bool handleKeyPress(uint32_t key, uint32_t modifier);

        void prompt();
        void moveToEnd();

    private:
        Zep::ZepEditor_ImGui mEditor;

        Zep::ZepBuffer *mBuffer;
        Zep::ZepWindow *mWindow;
        Zep::GlyphIterator mStartLocation;

        Interpreter *mInterpreter;

        std::shared_ptr<Highlighter> mHighlighter;
    };

}
}