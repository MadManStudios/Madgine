#include "../widgetslib.h"

#include "properties.h"

#include "Meta/serialize/operations.h"

#include "Meta/serialize/helper/typedobjectserialize.h"

namespace Engine {
namespace Widgets {

    static constexpr size_t sIncrementSteps[] = {
        1,
        1,
        0
    };

    std::strong_ordering PropertyDescriptor::operator<=>(const PropertyDescriptor &other) const
    {
        if (std::strong_ordering o = mType <=> other.mType; o != 0)
            return o;
        if (std::strong_ordering o = mAnnotator1 <=> other.mAnnotator1; o != 0)
            return o;
        return std::strong_ordering::equal;
    }

    bool PropertyDescriptor::operator==(const PropertyDescriptor &other) const
    {
        return mType == other.mType && mAnnotator1 == other.mAnnotator1;
    }

    const PropertyDescriptor &PropertyIterator::operator*() const
    {
        return mIt->mDescriptor;
    }

    const PropertyDescriptor *PropertyIterator::operator->() const
    {
        return &mIt->mDescriptor;
    }

    bool PropertyIterator::operator!=(const PropertyIterator &other) const
    {
        return mIt != other.mIt;
    }

    bool PropertyIterator::operator==(const PropertyIterator &other) const
    {
        return mIt == other.mIt;
    }

    PropertyIterator &PropertyIterator::operator++()
    {
        if (mIt->mDescriptor.mType == PropertyType::CONDITIONAL) {
            mIt += mIt->mDescriptor.mAnnotator0 + 1;
        } else {
            mIt += sIncrementSteps[mIt->mDescriptor.mType] + 1;
        }
        return *this;
    }

    PropertyIterator PropertyIterator::operator++(int)
    {
        PropertyIterator copy = *this;
        ++*this;
        return copy;
    }

    PropertyRange PropertyIterator::conditionalRange() const
    {
        assert(mIt->mDescriptor.mType == PropertyType::CONDITIONAL);
        return { std::next(mIt), std::next(mIt, 1 + mIt->mDescriptor.mAnnotator0) };
    }

    float PropertyIterator::value(size_t index) const
    {
        assert(index < sIncrementSteps[mIt->mDescriptor.mType]);
        return std::next(mIt, index + 1)->mValue;
    }

    PropertyIterator PropertyRange::begin() const
    {
        return mBegin;
    }

    PropertyIterator PropertyRange::end() const
    {
        return mEnd;
    }

    size_t PropertyRange::size() const
    {
        return std::distance(mBegin, mEnd);
    }

    PropertyIterator PropertyList::begin() const
    {
        return { mList.begin() };
    }

    PropertyIterator PropertyList::end() const
    {
        return { mList.end() };
    }

    void PropertyList::clear()
    {
        mList.clear();
    }

    void PropertyList::set(PropertyDescriptor desc, std::vector<float> params)
    {
        set(PropertyRange { begin(), end() }, desc, params);
    }

    void PropertyList::setConditional(uint16_t mask, PropertyDescriptor desc, std::vector<float> params)
    {
        assert(desc.mType != PropertyType::CONDITIONAL);
        auto it = set(*this, { PropertyType::CONDITIONAL, 0, mask }).first;
        size_t pos = std::distance(mList.cbegin(), it.mIt);
        bool inserted = set(it.conditionalRange(), desc, params).second;
        if (inserted) {
            std::next(mList.begin(), pos)->mDescriptor.mAnnotator0 += 1 + params.size();
        }
    }

    void PropertyList::unsetConditional(uint16_t mask, PropertyDescriptor desc)
    {
        auto it = std::ranges::find(*this, PropertyDescriptor { PropertyType::CONDITIONAL, 0, mask });
        if (it != end()) {
            size_t pos = std::distance(mList.cbegin(), it.mIt);
            if (unset(it.conditionalRange(), desc))
                std::next(mList.begin(), pos)->mDescriptor.mAnnotator0 -= 1 + sIncrementSteps[desc.mType];
        }
    }

    PropertyList::operator PropertyRange() const
    {
        return { begin(), end() };
    }

    std::pair<PropertyIterator, bool> PropertyList::set(PropertyRange range, PropertyDescriptor desc, std::vector<float> params)
    {
        assert(sIncrementSteps[desc.mType] == params.size());

        auto it = std::ranges::lower_bound(range, desc);

        bool inserted = false;

        std::deque<PropertyValue>::iterator innerIt;

        if (it == range.mEnd || *it != desc) {
            innerIt = mList.insert(it.mIt, params.size() + 1, {});
            innerIt->mDescriptor = desc;
            it = { innerIt };
            inserted = true;
        } else {
            innerIt = std::next(mList.begin(), std::distance(mList.cbegin(), it.mIt));
            if (desc.mType != PropertyType::CONDITIONAL)
                innerIt->mDescriptor.mAnnotator0 = desc.mAnnotator0;
        }

        for (float param : params) {
            ++innerIt;
            innerIt->mValue = param;
        }

        return { it, inserted };
    }

    bool PropertyList::unset(PropertyRange range, PropertyDescriptor desc)
    {
        auto it = std::ranges::find(range, desc);
        if (it == range.mEnd)
            return false;
        mList.erase(it.mIt, std::next(it.mIt, 1 + sIncrementSteps[desc.mType]));
        return true;
    }

}

namespace Serialize {

    static constexpr const char *sTags[] = {
        "Position",
        "Size",
        "Conditional"
    };

    static constexpr const char *sAnnotator1Names[] = {
        "index",
        "index",
        "mask"
    };

    static StreamResult readPropertyDescriptor(Serialize::FormattedSerializeStream &in, Widgets::PropertyDescriptor &desc, std::vector<float> &values)
    {
        STREAM_PROPAGATE_ERROR(Serialize::beginExtendedTypedRead(in, desc.mType, sTags));
        const char *annotator1Name = sAnnotator1Names[desc.mType];
        if (annotator1Name) {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(nullptr, 1));
            STREAM_PROPAGATE_ERROR(Serialize::read(in, desc.mAnnotator1, annotator1Name));
        }
        if (desc.mType != Widgets::PropertyType::CONDITIONAL) {
            STREAM_PROPAGATE_ERROR(in.readPrimitive(values.emplace_back(), nullptr));
        }
        return {};
    }

    StreamResult Operations<Widgets::PropertyList>::read(Serialize::FormattedSerializeStream &in, Widgets::PropertyList &list, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, true));

        list.clear();

        while (in.hasContainerItem()) {
            Widgets::PropertyDescriptor desc;
            std::vector<float> values;
            STREAM_PROPAGATE_ERROR(readPropertyDescriptor(in, desc, values));
            if (desc.mType == Widgets::PropertyType::CONDITIONAL) {
                uint16_t mask = desc.mAnnotator1;
                STREAM_PROPAGATE_ERROR(in.beginContainerRead("Conditional", true));
                while (in.hasContainerItem()) {
                    STREAM_PROPAGATE_ERROR(readPropertyDescriptor(in, desc, values));
                    list.setConditional(mask, desc, std::move(values));
                }
                STREAM_PROPAGATE_ERROR(in.endContainerRead("Conditional"));
            } else {
                list.set(desc, std::move(values));
            }
        }

        return in.endContainerRead(name);
    }

    void Operations<Widgets::PropertyList>::write(Serialize::FormattedSerializeStream &out, Widgets::PropertyRange list, const char *name)
    {
        out.beginContainerWrite(name, list.size());
        for (auto it = list.begin(); it != list.end(); ++it) {
            const Widgets::PropertyDescriptor &desc = *it;

            const char *tag = Serialize::beginExtendedTypedWrite(out, desc.mType, sTags);

            const char *annotator1Name = sAnnotator1Names[desc.mType];
            if (annotator1Name) {
                out.beginExtendedWrite(tag, 1);
                Serialize::write(out, desc.mAnnotator1, annotator1Name);
            }

            if (desc.mType == Widgets::PropertyType::CONDITIONAL) {
                write(out, it.conditionalRange(), tag);
            } else {
                out.writePrimitive(it.value(0), tag);
            }
        }
        out.endContainerWrite(name);
    }

    StreamResult Operations<Widgets::PropertyList>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, true));

        while (in.hasContainerItem()) {
            Widgets::PropertyDescriptor desc;
            std::vector<float> values;
            STREAM_PROPAGATE_ERROR(readPropertyDescriptor(in, desc, values));
            if (desc.mType == Widgets::PropertyType::CONDITIONAL) {
                uint16_t mask = desc.mAnnotator1;
                STREAM_PROPAGATE_ERROR(in.beginContainerRead("Conditional", true));
                while (in.hasContainerItem()) {
                    STREAM_PROPAGATE_ERROR(readPropertyDescriptor(in, desc, values));
                }
                STREAM_PROPAGATE_ERROR(in.endContainerRead("Conditional"));
            }
        }

        return in.endContainerRead(name);
    }

}

}