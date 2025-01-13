#pragma once

#include "Generic/container/atomiccontaineroperation.h"
#include "Generic/container/container_api.h"
#include "Generic/container/containerevent.h"
#include "Generic/functor.h"
#include "Generic/memberoffsetptr.h"
#include "serializable.h"
#include "../operations.h"

namespace Engine {
namespace Serialize {

    template <typename C, typename Observer = NoOpFunctor, typename OffsetPtr = TaggedPlaceholder<MemberOffsetPtrTag, 0>>
    struct SerializableContainerImpl : Serializable<OffsetPtr>,
                                       C,
                                       Observer {
    private:
        using _traits = container_traits<C>;

    public:
        using container = SerializableContainerImpl<C, Observer, OffsetPtr>;

        using Base = C;

        typedef typename _traits::iterator iterator;
        typedef typename _traits::const_iterator const_iterator;
        typedef typename _traits::position_handle position_handle;        

    public:
        SerializableContainerImpl()
            : mActiveIterator(_traits::toPositionHandle(*this, Base::begin()))
        {
        }

        SerializableContainerImpl(const Base &c)
            : Base(c)
            , mActiveIterator(_traits::toPositionHandle(*this, Base::begin()))
        {
        }

        SerializableContainerImpl(const SerializableContainerImpl &other)
            : Base(other)
            , mActiveIterator(_traits::toPositionHandle(*this, Base::begin()))
        {
        }

        SerializableContainerImpl(SerializableContainerImpl &&other)
            : SerializableContainerImpl(std::move(other), other.mActiveIterator == _traits::toPositionHandle(other, other.Base::end()))
        {
        }

        SerializableContainerImpl(SerializableContainerImpl &&other, bool hasEndIterator)
            : Base(std::move(other))
            , mActiveIterator(hasEndIterator ? _traits::toPositionHandle(*this, Base::end()) : std::move(other.mActiveIterator))
        {
            set_parent(*this, OffsetPtr::parent(this));
            other.Base::clear();
            other.mActiveIterator = _traits::toPositionHandle(other, other.Base::begin());
        }

        SerializableContainerImpl<C, Observer, OffsetPtr> &operator=(const Base &other)
        {
            ResetOperation { *this, false } = other;
            return *this;
        }

        SerializableContainerImpl<C, Observer, OffsetPtr> &operator=(SerializableContainerImpl &&other)
        {
            bool hasEndIterator = other.mActiveIterator == _traits::toPositionHandle(other, other.Base::end());
            Base::operator=(std::move(other));
            mActiveIterator = hasEndIterator ? _traits::toPositionHandle(*this, Base::end()) : std::move(other.mActiveIterator);
            set_parent(*this, OffsetPtr::parent(this));
            other.Base::clear();
            other.mActiveIterator = _traits::toPositionHandle(other, other.Base::begin());
            return *this;
        }

        Observer &observer()
        {
            return *this;
        }

        iterator begin()
        {
            return Base::begin();
        }

        const_iterator begin() const
        {
            return Base::begin();
        }

        iterator end()
        {
            return _traits::toIterator(*this, mActiveIterator);
        }

        const_iterator end() const
        {
            return _traits::toIterator(*this, mActiveIterator);
        }

        /* reverse_iterator rbegin()
        {
            return std::make_reverse_iterator(_traits::toIterator(*this, mActiveIterator));
        }

        const_reverse_iterator rbegin() const
        {
            return std::make_reverse_iterator(_traits::toIterator(*this, mActiveIterator));
        }

        reverse_iterator rend()
        {
            return Base::rend();
        }

        const_reverse_iterator rend() const
        {
            return Base::rend();
        }*/

        size_t size() const
        {
            return std::distance(Base::begin(), _traits::toIterator(*this, mActiveIterator));
        }

        void clear()
        {
            ResetOperation { *this, false }.clear();
        }

        template <typename... Ty>
        friend iterator tag_invoke(emplace_t, bool &success, SerializableContainerImpl<C, Observer, OffsetPtr> &self, const iterator &where, Ty &&...args)
        {
            InsertOperation op { self, where };
            return Engine::emplace(success, op, where, std::forward<Ty>(args)...);
        }

        template <typename Init, typename... _Ty>
        iterator emplace_init(const iterator &where, Init &&init, _Ty &&...args)
        {
            bool success;
            return emplace_init(success, where, std::forward<Init>(init), std::forward<_Ty>(args)...);
        }

        template <typename Init, typename... _Ty>
        iterator emplace_init(bool &success, const iterator &where, Init &&init, _Ty &&...args)
        {
            InsertOperation op { *this, where };
            iterator it = emplace(success, op, where, std::forward<_Ty>(args)...);
            if (success) {
                init(*it);
            }
            return it;
        }

        iterator erase(const iterator &where)
        {
            return RemoveOperation { *this, where }.erase(where);
        }

        iterator erase(const iterator &from, const iterator &to)
        {
            return RemoveRangeOperation { *this, from, to }.erase(from, to);
        }

        std::ranges::range_value_t<Base> extract(const iterator &which)
        {
            RemoveOperation op { *this, which };
            std::ranges::range_value_t<Base> temp = std::move(*which);
            op.erase(which);
            return temp;
        }

        void setActive(bool active, bool existenceChange, bool controlled)
        {
            if (active) {
                while (mActiveIterator != _traits::toPositionHandle(*this, Base::end())) {
                    iterator it = _traits::toIterator(*this, mActiveIterator);
                    mActiveIterator = _traits::next(*this, mActiveIterator);
                    if (existenceChange) {
                        Observer::operator()(it, BEFORE | EMPLACE);
                        Observer::operator()(it, AFTER | EMPLACE);
                    }
                    Observer::operator()(it, BEFORE | ACTIVATE_ITEM);
                    Serialize::setActive(*it, active, existenceChange || !controlled, CallerHierarchyPtr { OffsetPtr::parent(this) });
                    Observer::operator()(it, AFTER | ACTIVATE_ITEM);
                }
            } else {
                while (mActiveIterator != _traits::toPositionHandle(*this, Base::begin())) {
                    mActiveIterator = _traits::prev(*this, mActiveIterator);
                    iterator it = _traits::toIterator(*this, mActiveIterator);
                    Observer::operator()(it, BEFORE | DEACTIVATE_ITEM);
                    Serialize::setActive(*it, active, existenceChange || !controlled, CallerHierarchyPtr { OffsetPtr::parent(this) });
                    Observer::operator()(it, AFTER | DEACTIVATE_ITEM);
                    if (existenceChange) {
                        Observer::operator()(it, BEFORE | ERASE);
                        Observer::operator()(it, AFTER | ERASE);
                    }
                }
            }
        }

        
        template <typename T>
        friend void tag_invoke(set_synced_t cpo, SerializableContainerImpl& c, bool b, const CallerHierarchyBasePtr &hierarchy)
        {
            for (auto &e : c.physical()) {
                cpo(e, b, CallerHierarchyPtr { OffsetPtr::parent(&c) });
            }
        }

        const C &physical() const
        {
            return *this;
        }

        C &physical()
        {
            return *this;
        }

        struct Operation : AtomicContainerOperationBase<container> {

            using AtomicContainerOperationBase<container>::AtomicContainerOperationBase;

            size_t size()
            {
                return this->mContainer.Base::size();
            }

            template <typename... Ty>
            auto emplace_local(bool& success, const const_iterator& it, Ty &&...args)
            {
                return this->mContainer.emplace_intern(success, it, std::forward<Ty>(args)...);
            }

            template <typename... Ty>
            friend auto tag_invoke(emplace_t, bool &success, Operation &self, const const_iterator &it, Ty &&...args)
            {
                return self.emplace_local(success, it, std::forward<Ty>(args)...);
            }

            decltype(auto) physical()
            {
                return Serialize::physical(this->mContainer);
            }
        };

        struct _InsertOperation : Operation {
            _InsertOperation(container &c, const iterator &it)
                : Operation(c)
            {
                if constexpr (!_traits::sorted) {
                    if (this->mContainer.isItemActive(it)) {
                        this->mContainer.Observer::operator()(it, BEFORE | EMPLACE);
                    }
                }
            }

            template <typename... Ty>
            friend auto tag_invoke(emplace_t, bool &success, _InsertOperation &self, const const_iterator &it, Ty &&...args)
            {
                assert(!self.mCalled);
                self.mCalled = true;
                auto retIt = Engine::emplace(success, static_cast<Operation &>(self), it, std::forward<Ty>(args)...);
                self.mIt = retIt;
                self.mInserted = success;
                return retIt;
            }

            ~_InsertOperation()
            {
                assert(mCalled);
                if constexpr (_traits::sorted) {
                    if (this->mContainer.isItemActive(mIt)) {
                        this->mContainer.Observer::operator()(std::next(mIt), BEFORE | EMPLACE);
                    }
                }
                if (mInserted) {
                    if (this->mContainer.isSynced())
                        Serialize::set_synced(*mIt, true, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                }
                if (this->mContainer.isItemActive(mIt)) {
                    this->mContainer.Observer::operator()(mIt, (mInserted ? AFTER : ABORTED) | EMPLACE);
                    if (mInserted) {
                        this->mContainer.Observer::operator()(mIt, BEFORE | ACTIVATE_ITEM);
                        Serialize::setActive(*mIt, true, true, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                        this->mContainer.Observer::operator()(mIt, AFTER | ACTIVATE_ITEM);
                    }
                }
            }

        protected:
            bool mCalled = false;
            bool mInserted = false;
            iterator mIt;
        };

        struct _MultiInsertOperation : Operation {
            _MultiInsertOperation(container &c)
                : Operation(c)
            {
            }

            template <typename... Ty>
            friend auto tag_invoke(emplace_t, bool &success, _MultiInsertOperation &self, const const_iterator &it, Ty &&...args)
            {
                if constexpr (!_traits::sorted) {
                    if (self.mContainer.isItemActive(it)) {
                        self.mContainer.Observer::operator()(it, BEFORE | EMPLACE);
                    }
                }
                auto retIt = Engine::emplace(success, static_cast<Operation &>(self), it, std::forward<Ty>(args)...);
                if (success) {
                    for (std::tuple<position_handle, bool, const_iterator> &handle : self.mIterators) {
                        _traits::revalidateHandleAfterInsert(std::get<0>(handle), self.mContainer, static_cast<iterator>(retIt));
                    }
                }
                self.mIterators.emplace_back(_traits::toPositionHandle(self.mContainer, retIt), success, it);
                return retIt;
            }

            ~_MultiInsertOperation()
            {
                if constexpr (_traits::sorted) {
                    for (std::tuple<position_handle, bool, const_iterator> &handle : mIterators) {
                        iterator it = _traits::toIterator(this->mContainer, std::get<0>(handle));
                        if (this->mContainer.isItemActive(it)) {
                            this->mContainer.Observer::operator()(std::get<2>(handle), BEFORE | EMPLACE);
                        }
                    }
                }
                for (std::tuple<position_handle, bool, const_iterator> &handle : mIterators) {
                    iterator it = _traits::toIterator(this->mContainer, std::get<0>(handle));
                    bool inserted = std::get<1>(handle);
                    if (inserted) {
                        if (this->mContainer.isSynced())
                            Serialize::set_synced(*it, true, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                    }
                    if (this->mContainer.isItemActive(it)) {
                        this->mContainer.Observer::operator()(it, (inserted ? AFTER : ABORTED) | EMPLACE);
                        if (inserted) {
                            this->mContainer.Observer::operator()(it, BEFORE | ACTIVATE_ITEM);
                            Serialize::setActive(*it, true, true, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                            this->mContainer.Observer::operator()(it, AFTER | ACTIVATE_ITEM);
                        }
                    }
                }
            }

        protected:
            std::vector<std::tuple<position_handle, bool, const_iterator>> mIterators;
        };

        struct _RemoveOperation : Operation {
            _RemoveOperation(container &c, const iterator &it)
                : Operation(c)
            {
                if (this->mContainer.isSynced()) {
                    Serialize::set_synced(*it, false, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                }
                if (this->mContainer.isItemActive(it)) {
                    this->mContainer.Observer::operator()(it, BEFORE | DEACTIVATE_ITEM);
                    Serialize::setActive(*it, false, true, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                    this->mContainer.Observer::operator()(it, AFTER | DEACTIVATE_ITEM);
                    this->mContainer.Observer::operator()(it, BEFORE | ERASE);
                    mWasActive = true;
                }
            }

            ~_RemoveOperation()
            {
                assert(mTriggered);
                if (mWasActive) {
                    this->mContainer.Observer::operator()(mIt, AFTER | ERASE);
                }
            }

            auto erase(const iterator &it)
            {
                assert(!mTriggered);
                mTriggered = true;
                mIt = this->mContainer.erase_intern(it);
                return mIt;
            }

        private:
            bool mWasActive = false;
            bool mTriggered = false;
            iterator mIt;
        };

        struct _RemoveRangeOperation : Operation {
            _RemoveRangeOperation(container &c, const iterator &from, const iterator &to)
                : Operation(c)
            {
                if (this->mContainer.isSynced()) {
                    for (iterator it = from; it != to; ++it) {
                        Serialize::set_synced(*it, false, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                    }
                }
                for (iterator it = from; it != to && this->mContainer.isItemActive(it); ++it) {
                    this->mContainer.Observer::operator()(it, BEFORE | DEACTIVATE_ITEM);
                    Serialize::setActive(*it, false, true, CallerHierarchyPtr { OffsetPtr::parent(&this->mContainer) });
                    this->mContainer.Observer::operator()(it, AFTER | DEACTIVATE_ITEM);
                    this->mContainer.Observer::operator()(it, BEFORE | ERASE);
                    ++mCount;
                }
            }

            ~_RemoveRangeOperation()
            {
                assert(mTriggered);
                for (size_t i = 0; i < mCount; ++i) {
                    this->mContainer.Observer::operator()(mIt, AFTER | ERASE);
                }
            }

            auto erase(const iterator &from, const iterator &to)
            {
                assert(!mTriggered);
                mTriggered = true;
                mIt = this->mContainer.erase_intern(from, to);
                return mIt;
            }

        private:
            size_t mCount = 0;
            bool mTriggered = false;
            iterator mIt;
        };

        struct _ResetOperation : Operation {
            _ResetOperation(container &c, bool controlled)
                : Operation(c)
                , mControlled(controlled)
            {
                if (this->mContainer.isSynced()) {
                    set_synced(this->mContainer, false);
                }

                if (this->mContainer.isActive()) {
                    assert(this->mContainer.mActiveIterator == _traits::toPositionHandle(this->mContainer, this->mContainer.Base::end()));
                    this->mContainer.Observer::operator()(this->mContainer.end(), BEFORE | RESET);
                    this->mContainer.setActive(false, !mControlled, mControlled);
                    mWasActive = true;
                }
            }
            ~_ResetOperation()
            {
                if (this->mContainer.isSynced()) {
                    set_synced(this->mContainer, true);
                }
                if (mWasActive) {
                    this->mContainer.setActive(true, !mControlled, mControlled);
                    this->mContainer.Observer::operator()(this->mContainer.end(), AFTER | RESET);
                }
            }

            void clear()
            {
                this->mContainer.clear_intern();
            }

            template <typename T>
            void operator=(T &&other)
            {
                this->mContainer.Base::operator=(std::forward<T>(other));
            }

        private:
            bool mWasActive = false;
            bool mControlled;
        };

        using InsertOperation = AtomicContainerOperation<_InsertOperation>;
        using MultiInsertOperation = AtomicContainerOperation<_MultiInsertOperation>;
        using RemoveOperation = AtomicContainerOperation<_RemoveOperation>;
        using RemoveRangeOperation = AtomicContainerOperation<_RemoveRangeOperation>;
        using ResetOperation = AtomicContainerOperation<_ResetOperation>;

    protected:
        bool isItemActive(const iterator &it)
        {
            typename Base::iterator active = _traits::toIterator(*this, mActiveIterator);
            if (active == Base::end())
                return this->isActive();
            for (auto it2 = Base::begin(); it2 != active; ++it2) {
                if (it2 == it)
                    return true;
            }
            return false;
        }

        template <typename... _Ty>
        iterator emplace_intern(bool &success, const const_iterator &where, _Ty &&...args)
        {
            iterator it = Engine::emplace(success, static_cast<Base&>(*this), where, std::forward<_Ty>(args)...);
            if (success) {
                _traits::revalidateHandleAfterInsert(mActiveIterator, *this, it);
                position_handle newHandle = _traits::toPositionHandle(*this, it);
                if (_traits::next(*this, newHandle) == mActiveIterator && !this->isActive())
                    mActiveIterator = newHandle;
                set_parent(*it, OffsetPtr::parent(this));
            }
            return it;
        }

        iterator erase_intern(const iterator &it)
        {
            bool test = mActiveIterator == _traits::toPositionHandle(*this, it);
            iterator newIt = Base::erase(it);
            _traits::revalidateHandleAfterRemove(mActiveIterator, *this, newIt, test);
            return newIt;
        }

        iterator erase_intern(const iterator &from, const iterator &to)
        {
            bool test = false;
            size_t count = 0;
            for (iterator it = from; it != to; ++it) {
                if (mActiveIterator == _traits::toPositionHandle(*this, it))
                    test = true;
                ++count;
            }
            iterator newIt = Base::erase(from, to);
            _traits::revalidateHandleAfterRemove(mActiveIterator, *this, newIt, test, count);
            return newIt;
        }

        void clear_intern()
        {
            //size_t oldSize = size();
            Base::clear();
            mActiveIterator = _traits::toPositionHandle(*this, Base::begin());
            //_traits::revalidateHandleAfterRemove(mActiveIterator, *this, Base::begin(), true, oldSize);
        }
        
    private:
        void emplace() { } //only used to hide any potential emplace() in base class;

    protected:
        position_handle mActiveIterator;
    };

    template <typename C, typename Observer = NoOpFunctor, typename OffsetPtr = TaggedPlaceholder<MemberOffsetPtrTag, 0>>
    using SerializableContainer = container_api<SerializableContainerImpl<C, Observer, OffsetPtr>>;

#define SERIALIZABLE_CONTAINER(Name, ...) MEMBER_OFFSET_CONTAINER(Name, , ::Engine::Serialize::SerializableContainer<__VA_ARGS__>)

}

template <typename C, typename Observer, typename _OffsetPtr>
struct underlying_container<Serialize::SerializableContainerImpl<C, Observer, _OffsetPtr>, void> {
    typedef C type;
};

}
