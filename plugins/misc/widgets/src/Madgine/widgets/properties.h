#pragma once

#include "Generic/enum.h"

namespace Engine {
namespace Widgets {

    TYPED_ENUM(PropertyType, int8_t,
        POSITION,
        SIZE,
        CONDITIONAL);

    struct MADGINE_WIDGETS_EXPORT PropertyDescriptor {
        PropertyType mType;
        uint8_t mAnnotator0;
        uint16_t mAnnotator1;

        std::strong_ordering operator<=>(const PropertyDescriptor &) const;
        bool operator==(const PropertyDescriptor &) const;
    };

    union PropertyValue {
        PropertyDescriptor mDescriptor;
        float mValue;
    };

    struct PropertyRange;

    struct MADGINE_WIDGETS_EXPORT PropertyIterator {

        using difference_type = std::deque<PropertyValue>::const_iterator::difference_type;
        using value_type = PropertyDescriptor;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::forward_iterator_tag;

        const PropertyDescriptor &operator*() const;
        const PropertyDescriptor *operator->() const;
        bool operator!=(const PropertyIterator &other) const;
        bool operator==(const PropertyIterator &other) const;
        PropertyIterator &operator++();
        PropertyIterator operator++(int);


        PropertyRange conditionalRange() const;
        float value(size_t i) const;

        std::deque<PropertyValue>::const_iterator mIt;
    };

    struct MADGINE_WIDGETS_EXPORT PropertyRange {
        PropertyIterator begin() const;
        PropertyIterator end() const;

        size_t size() const;

        PropertyIterator mBegin;
        PropertyIterator mEnd;
    };

    struct PropertyList {

        PropertyIterator begin() const;
        PropertyIterator end() const;

        void clear();

        void set(PropertyDescriptor desc, std::vector<float> params = {});
        void setConditional(uint16_t mask, PropertyDescriptor desc, std::vector<float> params = {});

        void unsetConditional(uint16_t mask, PropertyDescriptor desc);

        operator PropertyRange() const;

    protected:
        std::pair<PropertyIterator, bool> set(PropertyRange range, PropertyDescriptor desc, std::vector<float> params = {});
        bool unset(PropertyRange range, PropertyDescriptor desc);

    private:
        std::deque<PropertyValue> mList;
    };
}

namespace Serialize {

    template <>
    struct Operations<Widgets::PropertyList> {
        static StreamResult read(Serialize::FormattedSerializeStream &in, Widgets::PropertyList &list, const char *name);
        static void write(Serialize::FormattedSerializeStream &out, Widgets::PropertyRange list, const char *name);
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);
    };
}

}