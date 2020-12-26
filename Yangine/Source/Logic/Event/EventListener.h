#pragma once
#include <Logic/Event/EventDispatcher.h>
#include <Utils/Typedefs.h>

namespace yang
{

    template <class EventType>
    class EventListener
    {
    public:
        EventListener();

        template <class F>
        EventListener(F&& handler);

        template <class F>
        void Register(F&& handler);

        void Unregister();
        ~EventListener();
    private:
        size_t m_eventListenerIndex;
    };

    template<class EventType>
    inline EventListener<EventType>::EventListener()
        :m_eventListenerIndex(kInvalidValue<size_t>)
    {
    }

    template<class EventType>
    template<class F>
    inline EventListener<EventType>::EventListener(F&& handler)
        :m_eventListenerIndex(yang::EventDispatcher::Get()->AddEventListener(EventType::kEventId, std::forward<F>(handler)))
    {
    }

    template<class EventType>
    template <class F>
    inline void EventListener<EventType>::Register(F&& handler)
    {
        Unregister();
        m_eventListenerIndex = yang::EventDispatcher::Get()->AddEventListener(EventType::kEventId, std::forward<F>(handler));
    }

    template<class EventType>
    inline void EventListener<EventType>::Unregister()
    {
        if (m_eventListenerIndex != kInvalidValue<size_t>)
        {
            yang::EventDispatcher::Get()->RemoveEventListener(EventType::kEventId, m_eventListenerIndex);
            m_eventListenerIndex = kInvalidValue<size_t>;
        }
    }

    template<class EventType>
    inline EventListener<EventType>::~EventListener()
    {
        Unregister();
    }

}