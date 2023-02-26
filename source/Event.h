/*
 * Event.h
 *
 *  Created on: 09.02.2014
 *      Author: NullPointer
 */

#pragma once

#include <map>
#include <list>
#include <memory>
#include <functional>
#include <utility>

namespace sable
{

template <typename... Args>
class Listener;

template <typename... Args>
class EventImpl;

template <typename... Args>
class Event
{
public:
    using ListenerPtr = std::unique_ptr<Listener<Args...>>;

    // Attach a callable.
    template <typename T>
    [[nodiscard]] ListenerPtr attach(T&& f) const;

    // Attach a member function with given instance or a callable with given user data.
    template <typename T, typename Instance>
    [[nodiscard]] ListenerPtr attach(T&& f, Instance instance) const;

    // Trigger this event.
    void operator()(Args... args);

    size_t getListenersCount() const;

private:
    std::shared_ptr<EventImpl<Args...>> eventImpl_ = std::make_shared<EventImpl<Args...>>();
};

template <typename... Args>
class Listener
{
public:
    Listener(Listener &&rhs);
    Listener &operator=(Listener &&rhs);
    ~Listener();
    void detach();

private:
    // EventImpl<Args...> has access to this ctor.
    Listener(std::weak_ptr<const EventImpl<Args...>> event, int id);
    Listener(const Listener &) = delete;
    void operator=(const Listener &) = delete;
    friend class EventImpl<Args...>;

    std::weak_ptr<const EventImpl<Args...>> event_;
    int id_;
};

template <typename... Args>
using ListenerPtr = std::unique_ptr<Listener<Args...>>;

// Implementation

template <typename... Args>
class EventImpl : public std::enable_shared_from_this<EventImpl<Args...>>
{
public:
    using F = std::function<void(Args...)>;
    using ListenerPtr = std::unique_ptr<Listener<Args...>>;

    EventImpl() = default;

    template <typename T>
    ListenerPtr attach(T&& f) const
    {
        removeErased();

        nextCallbackId_ += 1;
        callbacks_.emplace(nextCallbackId_, std::forward<T>(f));
        return ListenerPtr{ new Listener<Args...>(this->shared_from_this(), nextCallbackId_) };
    }

    void operator()(Args... args)
    {
        removeErased();

        for (auto &pair : callbacks_) {
            pair.second(std::forward<Args>(args)...);
        }
    }

    void remove(int id) const
    {
        erased_.push_back(id);
    }

    size_t getListenersCount() const
    {
        removeErased();

        return static_cast<size_t>(callbacks_.size());
    }

private:
    void removeErased() const
    {
        for (auto i : erased_) {
            callbacks_.erase(i);
        }
        erased_.clear();
    }

    mutable std::map<int, F> callbacks_;
    mutable std::list<int> erased_;
    mutable int nextCallbackId_{ 0 };
};

template <typename... Args>
template <typename T>
typename Event<Args...>::ListenerPtr Event<Args...>::attach(T&& f) const
{
    if (!eventImpl_) {
        return nullptr;
    }
    return eventImpl_->attach(std::forward<T>(f));
}

template <typename... Args>
template <typename T, typename Instance>
typename Event<Args...>::ListenerPtr Event<Args...>::attach(T&& f, Instance instance) const
{
    if (!eventImpl_) {
        return nullptr;
    }

    return eventImpl_->attach([f, instance](Args... args) {
        std::invoke(f, instance, std::forward<Args>(args)...);
    });
}

template <typename... Args>
void Event<Args...>::operator()(Args... args)
{
    if (!eventImpl_) {
        return;
    }

    return eventImpl_->operator()(std::forward<Args>(args)...);
}

template <typename... Args>
size_t Event<Args...>::getListenersCount() const
{
    if (!eventImpl_) {
        return 0;
    }

    return eventImpl_->getListenersCount();
}

template <typename... Args>
Listener<Args...>::Listener(Listener &&rhs) : Listener(rhs.event_, rhs.id_)
{
    rhs.id_ = 0;
    rhs.event_.reset();
}

template <typename... Args>
Listener<Args...> &Listener<Args...>::operator=(Listener &&rhs)
{
    detach();

    this->id_ = rhs.id_;
    this->event_ = rhs.event_;

    rhs.id_ = 0;
    rhs.event_.reset();

    return *this;
}

template <typename... Args>
Listener<Args...>::~Listener()
{
    detach();
}

template <typename... Args>
void Listener<Args...>::detach()
{
    if (!id_) {
        return;
    }

    if (auto e = event_.lock()) {
        e->remove(id_);
    }
    event_.reset();
    id_ = 0;
}

template <typename... Args>
Listener<Args...>::Listener(std::weak_ptr<const EventImpl<Args...>> event, int id)
    : event_(std::move(event))
    , id_(id)
{
}

}
