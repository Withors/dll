#pragma once

#include "../Module.hpp"
#include "../../../Events/EventHandler.hpp"
#include "HiveStatListener.hpp"


class HiveStat : public Module {

public:


    HiveStat() : Module("HiveOverlay", "",
                      IDR_RE_Q_PNG, "O") {

        Module::setup();

    };

    void onEnable() override {
        EventHandler::registerListener(new HiveStatListener("HiveOverlay", this));
        Module::onEnable();
    }

    void onDisable() override {
        EventHandler::unregisterListener("HiveOverlay");
        Module::onDisable();
    }

    void defaultConfig() override {
        if (settings.getSettingByName<std::string>("Overlay") == nullptr)
            settings.addSetting("Overlay", (std::string) "O");

    }

    void settingsRender() override {
        float toggleX = Constraints::PercentageConstraint(0.019, "left");
        float toggleY = Constraints::PercentageConstraint(0.10, "top");

        FlarialGUI::KeybindSelector(0, toggleX + FlarialGUI::SettingsTextWidth("Overlay"), toggleY,
                                    settings.getSettingByName<std::string>("Overlay")->value);
    }
};

