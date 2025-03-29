#include "python3lib.h"

#include "python3streamredirect.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "util/pyobjectutil.h"

#include "Interfaces/log/log.h"

METATABLE_BEGIN(Engine::Scripting::Python3::Python3StreamRedirect)
FUNCTION(write, text)
METATABLE_END(Engine::Scripting::Python3::Python3StreamRedirect)

namespace Engine {
namespace Scripting {
    namespace Python3 {

        Python3StreamRedirect::Python3StreamRedirect(Log::Log *log)
            : mLog(log)
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

            PySys_SetObject(name.data(), Scripting::Python3::toPyObject(ScopePtr { this }));
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
            if (text == "\n")
                return 0;

            if (mLog) {
                mLog->log(text, Log::MessageType::INFO_TYPE);
                return text.size();
            } else {
                LOG(text);
                return text.size();
            }
        }

        void Python3StreamRedirect::setLog(Log::Log *log)
        {
            mLog = log;
        }

        Log::Log *Python3StreamRedirect::log()
        {
            return mLog;
        }

    }
}
}
