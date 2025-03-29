#include "../interfaceslib.h"

#ifndef NDEBUG

#include "traceback.h"

namespace Engine{
    namespace Debug{

        std::ostream &operator <<(std::ostream &out, const TraceBack &trace){
            out << trace.mFunction;
            if (strlen(trace.mFile) > 0){
                out << " { " << trace.mFile;
                if (trace.mLineNr != -1)
                    out << " [" << trace.mLineNr << "]";
                out << " } ";
            }
            return out;
        }

    }
}

namespace std {
	string to_string(const Engine::Debug::TraceBack &t) {
		stringstream ss;
		ss << t;
		return ss.str();
	}
}

#endif