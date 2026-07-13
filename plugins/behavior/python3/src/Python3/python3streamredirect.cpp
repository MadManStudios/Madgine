#include "python3lib.h"

#include "python3streamredirect.h"

#include "Platform/log/log.h"

#include "Meta/reflect/metatable_impl.h"

#include "util/pyexecution.h"
#include "util/pyobjectutil.h"

METATABLE_BEGIN(Engine::Behavior::Python3::Python3StreamRedirect)
    FUNCTION(write, text)
METATABLE_END(Engine::Behavior::Python3::Python3StreamRedirect)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3StreamRedirect::Python3StreamRedirect()
        {
        }

        Python3StreamRedirect::~Python3StreamRedirect()
        {
            while (!mOldStreams.empty()) {
                reset(mOldStreams.begin()->first);
            }
        }

        void Python3StreamRedirect::redirect(std::string_view name)
        {
            if (!mOldStreams[name]) {
                mOldStreams[name] = PySys_GetObject(name.data()); // borrowed
            }

            PySys_SetObject(name.data(), toPyObject(Reflect::ScopePtr { this }));
        }

        void Python3StreamRedirect::reset(std::string_view name)
        {
            auto it = mOldStreams.find(name);
            if (it != mOldStreams.end()) {
                Py_DECREF(PySys_GetObject(name.data()));
                PySys_SetObject(name.data(), it->second);
                mOldStreams.erase(it);
            }
        }

        int Python3StreamRedirect::write(std::string_view text)
        {
            Python3InnerLock lock;

            if (text == "\n")
                return 0;

            Platform::Log::Log *log = executionState().mLog;

            if (log) {
                log->log(text, Platform::Log::MessageType::INFO_TYPE);
                return text.size();
            } else {
                LOG(text);
                return text.size();
            }
        }

    }
}
}
