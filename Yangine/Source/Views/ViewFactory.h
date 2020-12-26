#pragma once
#include <functional>
#include <unordered_map>
#include <memory>
#include <Views/IView.h>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{
    class Actor;
    class ApplicationLayer;

    class ViewFactory
    {
    public:
        using ViewFunction = std::function<std::unique_ptr<IView>()>;

        ViewFactory() = default;

        void Init(const ApplicationLayer& app);

        void RegisterViewCreator(uint32_t id, ViewFunction pFunction);

        template<class View, class... Args>
        void RegisterView(Args... args);

        std::unique_ptr<IView> CreateView(std::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData);
    private:
        std::unordered_map<uint32_t, ViewFunction> m_viewCreatorMap;

        const ApplicationLayer* m_pAppLayer = nullptr;
    };

    template<class View, class ...Args>
    inline void ViewFactory::RegisterView(Args ...args)
    {
        static_assert(std::is_base_of_v<IView, View>, "IView should be a base class of View");
        RegisterViewCreator(View::GetHashName(),
            [args...]()
        {
            return IView::CreateView<View::GetHashName()>(args...);
        });
    }
}
