#pragma once

#include "../streams/formatter.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT XMLFormatter : Formatter {

        XMLFormatter();

        void setupStream(std::iostream &) override;

        void beginExtendedWrite(const char *name, size_t count) override;

        void beginCompoundWrite(const char *name) override;
        void endCompoundWrite(const char *name) override;

        StreamResult beginExtendedRead(const char *name, size_t count) override;

        StreamResult beginCompoundRead(const char *name) override;
        StreamResult endCompoundRead(const char *name) override;

        void beginPrimitiveWrite(const char *name, uint8_t typeId) override;
        void endPrimitiveWrite(const char *name, uint8_t typeId) override;

        StreamResult beginPrimitiveRead(const char *name, uint8_t typeId) override;
        StreamResult endPrimitiveRead(const char *name, uint8_t typeId) override;

        StreamResult lookupFieldName(std::string &name) override;

        void beginContainerWrite(const char *name, uint32_t size) override;
        void endContainerWrite(const char *name) override;
        StreamResult beginContainerRead(const char *name, bool sized) override;
        StreamResult endContainerRead(const char *name) override;
        bool hasContainerItem() override;

        StreamResult readString(std::string &s) override;
        void writeString(const std::string &s) override;

        std::string indent();

    private:
        StreamResult prefetchAttributes(const char *name = nullptr);
        StreamResult skipValue();

    private:
        size_t mLevel = 0;
        bool mCurrentExtended = false;
        size_t mCurrentExtendedCount = 0;
        std::istream::pos_type mExtendedLookupPos = -1;
        std::map<std::string, std::istream::pos_type, std::less<>> mPrefetchedAttributes;
    };

}
}