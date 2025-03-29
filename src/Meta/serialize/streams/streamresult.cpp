#include "../../metalib.h"

#include "streamresult.h"

#include "formattedserializestream.h"

namespace Engine {
namespace Serialize {

    StreamError::StreamError(SerializeStream *in, bool binary, const std::string &msg, const char *file, size_t sourceLine)
    {
        std::ostringstream ss;
        std::ostringstream notes;

        ss << "ERROR: (";
        
        if (in) {

            std::ios_base::iostate state = in->state();
            in->clear();

            mPosition = static_cast<int>(in->tell()) - in->gcount();

            if (!binary) {
                in->seek(0);
                pos_type lastNewLine = 0;
                mLineNumber = 1;

                std::istreambuf_iterator it = in->iterator();
                for (int i = 0; i < mPosition; ++i) {
                    char c = *it++;
                    if (c == '\n') {
                        ++mLineNumber;
                        lastNewLine = i;
                    }
                }
                mPosition -= lastNewLine;

                ss << mLineNumber << ", ";

                if (*in) {
                    std::string area;
                    in->Stream::operator>>(area);
                    notes << "Note: next to '" << area << "'\n";
                }
            }
            in->setState(state);
        }        

        notes << "Note: in file '" << file << "':" << sourceLine << "\n";

        ss << mPosition << "): " << msg;

        mMsg = ss.str();
        mNotes = notes.str();

        
    }

    std::ostream &operator<<(std::ostream &out, const StreamError &error)
    {
        return out << error.mMsg << "\n"
                   << error.mNotes;
    }

    std::ostream &operator<<(std::ostream &out, const StreamResult &result)
    {
        out << result.mState;
        if (result.mState != StreamState::OK)
            out << '\n'
                << *result.mError;
        return out;
    }

    StreamResultBuilder::StreamResultBuilder(StreamState type, FormattedSerializeStream &stream, const char *file, size_t line)
        : StreamResultBuilder(type, stream.stream(), stream.isBinary(), file, line)
    {
    }

    StreamResultBuilder::operator StreamResult()
    {
        assert(mType != StreamState::OK);
        return {
            mType,
            std::make_unique<StreamError>(mStream, mBinary, mMsg.str(), mFile, mLine)
        };
    }

    StreamState streamError(std::ios::iostate state, std::ostringstream &out)
    {

        if (state & std::ios_base::badbit) {
            out << "Stream corrupt";
            return StreamState::UNKNOWN_ERROR;
        } else if (state & std::ios_base::failbit) {
            out << "Operation failure";
            return StreamState::UNKNOWN_ERROR;
        } else if (state & std::ios_base::eofbit) {
            out << "Unexpected EOF";
            return StreamState::UNKNOWN_ERROR;
        } else {
            out << "Unknown error";
            return StreamState::UNKNOWN_ERROR;
        }
    }

}
}