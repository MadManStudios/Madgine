#include "../interfaceslib.h"

#if !WINDOWS

#    include "async.h"

#    include "fsapi.h"

namespace Engine {
namespace Filesystem {

    struct AsyncFileReadAuxiliaryData {

    };

    void checkAsyncIOCompletion()
    {
  
    }

    void cancelAllAsyncIO()
    {
        
    }

    size_t pendingIOOperationCount()
    {
        return 0;
    }

    AsyncFileReadState::AsyncFileReadState(Path path)
        : mPath(std::move(path))
    {
    }

    AsyncFileReadState::~AsyncFileReadState() = default;

    void AsyncFileReadState::start()
    {
        Stream file = openFileRead(mPath);
        if (!file)
            set_error(GenericResult::UNKNOWN_ERROR);
        else        
            set_value(std::vector<unsigned char> { file.iterator(), file.end() });
    }

}
}

#endif