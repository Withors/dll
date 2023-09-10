#pragma once
#include "../../../../Hook/Hooks/Input/MouseHook.hpp"
#include "../../../../Events/Listener.hpp"
#include "../../../../Client.hpp"

class TextAliasListener : public Listener {

public:

    std::map<std::string, D2D1_TEXT_ANTIALIAS_MODE> aliases = {
        {"Default", D2D1_TEXT_ANTIALIAS_MODE_DEFAULT},
        {"Cleartype", D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE},
        {"Grayscale", D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE},
        {"Aliased", D2D1_TEXT_ANTIALIAS_MODE_ALIASED}
    };
    std::string curAliasMode;

    void onLocalTick(TickEvent& event) override {
        if (curAliasMode != Client::settings.getSettingByName<std::string>("aliasingMode")->value) {
            curAliasMode = Client::settings.getSettingByName<std::string>("aliasingMode")->value;
            D2D::context->SetTextAntialiasMode(aliases[curAliasMode]);
            return;
        }
    }

public:
    explicit TextAliasListener(const char string[5]) {
        this->name = string;
    }
};
