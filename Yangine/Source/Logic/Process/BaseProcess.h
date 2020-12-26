#pragma once

#include <Logic/Process/IProcess.h>
#include <Logic/Actor/Actor.h>
#include <Utils/Logger.h>
#include <memory>
#include <string_view>

namespace yang
{

namespace Detail
{
    template<typename, typename T>
    struct has_post_init {
        static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");

        static constexpr bool value = false;
    };

    // specialization that does the checking

    template<typename C, typename Ret, typename... Args>
    struct has_post_init<C, Ret(Args...)>
    {
    private:
        template<typename T>
        static constexpr auto check(T* t, Args... args)
            -> typename
            std::is_same<
            decltype(t->PostInit(args...)),
            Ret    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            >::type;  // attempt to call it and see if the return type is correct

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(nullptr, std::declval<Args>()...)) type;

    public:
        static constexpr bool value = type::value;
    };

    template <typename T, typename Ret, typename... Args>
    constexpr bool has_post_init_v = has_post_init<T, Ret (Args...)>::value;
}

template <class ProcessDescription, class... ComponentTypes>
class BaseProcess : public yang::IProcess
{
    static_assert(std::is_invocable_v<decltype(&ProcessDescription::Update), ProcessDescription, float, ComponentTypes*...>, "Process description doesn't match");
public:
    template<class... DescriptionConstructorArgs>
    BaseProcess(std::shared_ptr<yang::Actor> pOwner, DescriptionConstructorArgs&&... args);

    virtual ~BaseProcess() = default;

    virtual bool Init(tinyxml2::XMLElement* pData) override final;
    virtual bool PostInit() override final;

    virtual void Update(float deltaseconds) override final;

    static constexpr uint32_t GetHashName() { return ProcessDescription::GetHashName(); }
    static constexpr std::string_view GetName() { return ProcessDescription::GetName(); }
protected:
    template<class ComponentType>
    ComponentType* GetComponent();
private:
    std::tuple<ComponentTypes*...> m_components;
    ProcessDescription m_actualProcess;
};

template<class ProcessDescription, class ...ComponentTypes>
template<class ...DescriptionConstructorArgs>
inline BaseProcess<ProcessDescription, ComponentTypes...>::BaseProcess(std::shared_ptr<yang::Actor> pOwner, DescriptionConstructorArgs&& ...args)
    :IProcess(pOwner)
    , m_actualProcess(this, std::forward<DescriptionConstructorArgs>(args)...)
{
}

template<class ProcessDescription, class ...ComponentTypes>
inline bool BaseProcess<ProcessDescription, ComponentTypes...>::Init(tinyxml2::XMLElement* pData)
{
    return m_actualProcess.Init(pData);
}

template<class ProcessDescription, class ...ComponentTypes>
inline bool BaseProcess<ProcessDescription, ComponentTypes...>::PostInit()
{
    bool success = true;
    auto logErrorIfNull = [&success](auto pComp, const char* compName)
    {
        if (!pComp)
        {
            LOG(Error, "%s was nullptr", compName);
            success = false;
        }
        return pComp;
    };

    auto pOwner = m_pOwner.lock();
    if (!pOwner)
    {
        return false;
    }

    m_components = std::make_tuple(logErrorIfNull(pOwner->GetComponent<ComponentTypes>(), ComponentTypes::GetName())...);

    if constexpr (Detail::has_post_init_v<ProcessDescription, bool, ComponentTypes*...>)
    {
        success = success && std::apply(&ProcessDescription::PostInit, std::tuple_cat(std::make_tuple(m_actualProcess), m_components));
    }

    return success;
}

template<class ProcessDescription, class ...ComponentTypes>
inline void BaseProcess<ProcessDescription, ComponentTypes...>::Update(float deltaseconds)
{
    std::apply(&ProcessDescription::Update, std::tuple_cat(std::make_tuple(m_actualProcess, deltaseconds), m_components));
}

template<class ProcessDescription, class ...ComponentTypes>
template<class ComponentType>
inline ComponentType* BaseProcess<ProcessDescription, ComponentTypes...>::GetComponent()
{
    return std::get<ComponentType*>(m_components);
}

}