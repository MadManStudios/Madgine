#pragma once

#include "../streams/formatter.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT JSONFormatter : Formatter {

        JSONFormatter();

        virtual void setupStream(std::iostream &) override;

        virtual void beginExtendedWrite(const char *name, size_t count) override;

        virtual void beginCompoundWrite(const char *name) override;
        virtual void endCompoundWrite(const char *name) override;

        virtual StreamResult beginExtendedRead(const char *name, size_t count) override;

        virtual StreamResult beginCompoundRead(const char *name) override;
        virtual StreamResult endCompoundRead(const char *name) override;

        virtual void beginPrimitiveWrite(const char *name, uint8_t typeId) override;
        virtual void endPrimitiveWrite(const char *name, uint8_t typeId) override;

        virtual StreamResult beginPrimitiveRead(const char *name, uint8_t typeId) override;
        virtual StreamResult endPrimitiveRead(const char *name, uint8_t typeId) override;

        virtual StreamResult lookupFieldName(std::string &name) override;

        virtual void beginContainerWrite(const char *name, uint32_t size) override;
        virtual void endContainerWrite(const char *name) override;
        virtual StreamResult beginContainerRead(const char *name, bool sized) override;
        virtual StreamResult endContainerRead(const char *name) override;
        virtual bool hasContainerItem() override;

        virtual void beginMessageWrite() override;
        virtual void endMessageWrite() override;
        virtual StreamResult beginMessageRead() override;
        virtual StreamResult endMessageRead() override;

        std::string indent();

    private:
        StreamResult readFieldName(const char *name);

        StreamResult prefetchFields(const char *name = nullptr);

        StreamResult skipObject();

    private:
        size_t mLevel = 0;
        bool mCurrentExtended = false;
        size_t mCurrentExtendedCount = 0;
        bool mAfterItemRead = false, mAfterItemWrite = false;
        bool mLastPrimitive;

        struct ParseLevel {

            ParseLevel(bool isContainer)
                : mIsContainer(isContainer)
            {
            }

            bool mIsContainer;
            std::map<std::string, std::istream::pos_type, std::less<>> mPrefetchedFields;
            std::istream::pos_type mLookupPos = -1;
        };

        std::stack<ParseLevel> mParseLevel;
    };

}
}