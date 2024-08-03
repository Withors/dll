#include "Module.hpp"
#include "../../Client.hpp"
#include "ClickGUI/ClickGUIRenderer.hpp"

std::map<std::string, DWRITE_TEXT_ALIGNMENT> alignments = {
        {"Left", DWRITE_TEXT_ALIGNMENT_LEADING},
        {"Center", DWRITE_TEXT_ALIGNMENT_CENTER},
        {"Right", DWRITE_TEXT_ALIGNMENT_TRAILING}
};

static std::string Lname = "";

void Module::normalRender(int index, std::string &value) {
    if(!isEnabled() || SDK::currentScreen != "hud_screen") return;

    std::string text{};
    if(this->settings.getSettingByName<std::string>("text")!=nullptr)
        text = this->settings.getSettingByName<std::string>("text")->value;

    float rotation = this->settings.getSettingByName<float>("rotation")->value;
    DWRITE_TEXT_ALIGNMENT alignment = alignments[this->settings.getSettingByName<std::string>(
            "textalignment")->value];
    bool responsivewidth = this->settings.getSettingByName<bool>("responsivewidth")->value;
    float paddingX = this->settings.getSettingByName<float>("padx")->value;
    float paddingY = this->settings.getSettingByName<float>("pady")->value;


    if (this->settings.getSettingByName<bool>("reversepaddingx")->value)
        paddingX = -(this->settings.getSettingByName<float>("padx")->value);
    if (this->settings.getSettingByName<bool>("reversepaddingy")->value)
        paddingY = -(this->settings.getSettingByName<float>("pady")->value);

    std::string uppercaseSentence;
    std::string search = "{VALUE}";

    for (char c: text) {
        uppercaseSentence += (char)std::toupper(c);
    }

    size_t pos = uppercaseSentence.find(search);
    if (pos != std::string::npos) {
        text.replace(pos, search.length(), value);
    }

    float textWidth = Constraints::RelativeConstraint(0.7f * settings.getSettingByName<float>("uiscale")->value);
    float textHeight = Constraints::RelativeConstraint(0.1f * settings.getSettingByName<float>("uiscale")->value);

    float textSize = Constraints::SpacingConstraint(3.2f, textHeight) *
                     this->settings.getSettingByName<float>("textscale")->value;

    Vec2<float> settingperc = Vec2<float>(
            this->settings.getSettingByName<float>("percentageX")->value,
            this->settings.getSettingByName<float>("percentageY")->value
    );

    float realspacing = Constraints::SpacingConstraint(0.05f, textWidth);

    std::string rname = FlarialGUI::FlarialTextWithFont(0,0,
        FlarialGUI::to_wide(text).c_str(),
        1000000,
        textHeight,
        alignment,
        textSize, DWRITE_FONT_WEIGHT_NORMAL,
        D2D1::ColorF(0, 0, 0 ,0),
        true
);

    Lname = rname;

    float rectWidth = (!responsivewidth
                       ? (Constraints::RelativeConstraint(
                    0.225f * settings.getSettingByName<float>("uiscale")->value) *
                          this->settings.getSettingByName<float>("rectwidth")->value)
                       : (FlarialGUI::TextSizes[Lname] + Constraints::SpacingConstraint(2.0, realspacing)) *
                         this->settings.getSettingByName<float>("rectwidth")->value);

    Vec2<float> realcenter;

    if (settingperc.x != 0)
        realcenter = Vec2<float>(settingperc.x * MC::windowSize.x, settingperc.y * MC::windowSize.y);
    else
        realcenter = Constraints::CenterConstraint(rectWidth, textHeight * this->settings.getSettingByName<float>(
                "rectheight")->value);

    if (ModuleManager::getModule("ClickGUI")->isEnabled() ||
        ClickGUIRenderer::editmenu) {
        FlarialGUI::SetWindowRect(realcenter.x, realcenter.y, rectWidth,
                                  textHeight * this->settings.getSettingByName<float>("rectheight")->value, index);

        Vec2<float> vec2 = FlarialGUI::CalculateMovedXY(realcenter.x, realcenter.y, index, rectWidth, textHeight *
                                                                                                      this->settings.getSettingByName<float>(
                                                                                                              "rectheight")->value);

        realcenter.x = vec2.x;
        realcenter.y = vec2.y;

        realcenter = realcenter;

        Vec2<float> percentages = Constraints::CalculatePercentage(realcenter.x, realcenter.y);

        this->settings.setValue("percentageX", percentages.x);
        this->settings.setValue("percentageY", percentages.y);
    }

    Vec2<float> rounde = Constraints::RoundingConstraint(this->settings.getSettingByName<float>("rounding")->value *
                                                         settings.getSettingByName<float>("uiscale")->value,
                                                         this->settings.getSettingByName<float>("rounding")->value *
                                                         settings.getSettingByName<float>("uiscale")->value);

    D2D1_COLOR_F bgColor = settings.getSettingByName<bool>("bgRGB")->value ? FlarialGUI::rgbColor
                                                                           : FlarialGUI::HexToColorF(
                    settings.getSettingByName<std::string>("bgColor")->value);
    D2D1_COLOR_F textColor = settings.getSettingByName<bool>("textRGB")->value ? FlarialGUI::rgbColor
                                                                               : FlarialGUI::HexToColorF(
                    settings.getSettingByName<std::string>("textColor")->value);
    D2D1_COLOR_F borderColor = settings.getSettingByName<bool>("borderRGB")->value ? FlarialGUI::rgbColor
                                                                                   : FlarialGUI::HexToColorF(
                    settings.getSettingByName<std::string>("borderColor")->value);

    bgColor.a = settings.getSettingByName<float>("bgOpacity")->value;
    textColor.a = settings.getSettingByName<float>("textOpacity")->value;
    borderColor.a = settings.getSettingByName<float>("borderOpacity")->value;

    ImVec2 rotationCenter = ImVec2(
    realcenter.x + rectWidth / 2.0f,
    realcenter.y + textHeight * this->settings.getSettingByName<float>("rectheight")->value / 2.0f);

    if (rotation > 0.0f) {

        FlarialGUI::ImRotateStart();
    }



    if (settings.getSettingByName<bool>("BlurEffect")->value)
        FlarialGUI::BlurRect(D2D1::RoundedRect(D2D1::RectF(realcenter.x, realcenter.y,
                                                           realcenter.x + rectWidth,
                                                           realcenter.y + (textHeight) *
                                                                          this->settings.getSettingByName<float>(
                                                                                  "rectheight")->value), rounde.x,
                                               rounde.x));

    FlarialGUI::RoundedRect(
            realcenter.x,
            realcenter.y,
            bgColor,
            rectWidth,
            textHeight * this->settings.getSettingByName<float>("rectheight")->value,
            rounde.x,
            rounde.x
    );

    FlarialGUI::FlarialTextWithFont(
            realcenter.x + Constraints::SpacingConstraint(paddingX, textWidth),
            realcenter.y + Constraints::SpacingConstraint(paddingY, textWidth),
            FlarialGUI::to_wide(text).c_str(),
            rectWidth,
            textHeight,
            alignment,
            textSize, DWRITE_FONT_WEIGHT_NORMAL,
            textColor,
            true
    );

    if (this->settings.getSettingByName<bool>("border")->value) {
        FlarialGUI::RoundedHollowRect(
                realcenter.x,
                realcenter.y,
                Constraints::RelativeConstraint((this->settings.getSettingByName<float>("borderWidth")->value *
                                                 settings.getSettingByName<float>("uiscale")->value) / 100.0f,
                                                "height", true),
                borderColor,
                rectWidth,
                textHeight * this->settings.getSettingByName<float>("rectheight")->value,
                rounde.x,
                rounde.x
        );
    }

    if (rotation > 0.0f) {
        FlarialGUI::ImRotateEnd(rotation, rotationCenter);
    }

    if (ModuleManager::getModule("ClickGUI")->isEnabled() ||
        ClickGUIRenderer::editmenu)
        FlarialGUI::UnsetWindowRect();
}

void Module::resetPadding() {
    padding = 0;
    toggleIndex = 0;
    dropdownIndex = 0;
    sliderIndex = 0;
    colorPickerIndex = 0;
    keybindIndex = 0;
    textboxIndex = 0;
}

void Module::addToggle(std::string text, std::string subtext, bool& value) {
    float x = Constraints::PercentageConstraint(0.019, "left");
    float elementX = Constraints::PercentageConstraint(0.119f, "right");
    float y = Constraints::PercentageConstraint(0.10, "top") + padding;

    if (FlarialGUI::Toggle(toggleIndex, elementX, y, value, false)) value = !value;

    float textX = x;
    float textY = y; // For some reason, if you do not do this minus, the spacing is way bigger than expected.
    float subtextY;
    float fontSize = Constraints::RelativeConstraint(0.155f, "height", true);
    float fontSize2 = Constraints::RelativeConstraint(0.132f, "height", true);

    if (!subtext.empty()) {
        subtextY = textY;
        textY -= Constraints::RelativeConstraint(0.009f, "height", true);
        subtextY += Constraints::RelativeConstraint(0.009f, "height", true);
    }
    else {
        textY += Constraints::RelativeConstraint(0.0015f, "height", true);
    }

    FlarialGUI::FlarialTextWithFont(textX, textY, FlarialGUI::to_wide(text).c_str(), 200, 0, DWRITE_TEXT_ALIGNMENT_LEADING, fontSize, DWRITE_FONT_WEIGHT_MEDIUM, false);
    if (!subtext.empty()) FlarialGUI::FlarialTextWithFont(textX, subtextY, FlarialGUI::to_wide(subtext).c_str(), 200, 0, DWRITE_TEXT_ALIGNMENT_LEADING, fontSize2, DWRITE_FONT_WEIGHT_MEDIUM, FlarialGUI::HexToColorF("473b3d"), false);


    padding += Constraints::RelativeConstraint(0.05f, "height", true);
    toggleIndex++;
}

void Module::loadDefaults() {
    settings.reset();
    setup();
}

void Module::saveSettings() const {
    try {
        std::ofstream outputFile(settingspath);
        if (outputFile.is_open()) {
            std::string jsonString = settings.ToJson();
            outputFile << jsonString;
            outputFile.close();
        }
        else {
            Logger::error("Failed to open file. Maybe it doesn't exist?: " + settingspath);
        }
    }
    catch (const std::exception& ex) {
        Logger::error(ex.what());
    }
}

void Module::loadSettings() {
    std::ifstream inputFile(settingspath);
    std::stringstream ss;

    if (inputFile.is_open()) {
        ss << inputFile.rdbuf();
        inputFile.close();
    }
    else {
        Logger::error("File could not be opened. Maybe it doesn't exist?: " + settingspath);
        return;
    }

    std::string settingstring = ss.str();
    settings.FromJson(settingstring);
}

void Module::checkSettingsFile() const {

    if (!std::filesystem::exists(settingspath)) {

        std::filesystem::path filePath(settingspath);
        std::filesystem::create_directories(filePath.parent_path());

        HANDLE fileHandle = CreateFileA(settingspath.c_str(), GENERIC_WRITE | GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            Logger::error("Failed to create file: " + settingspath);
            return;
        }

        CloseHandle(fileHandle);
    }
}

void Module::toggle() {
    bool& enabled = settings.getSettingByName<bool>("enabled")->value;
    enabled = !enabled;
    if (enabled) {
        onEnable();
    }
    else {
        onDisable();
    }
}

void Module::setup() {
    defaultConfig();
    Module::defaultConfig();
    onSetup();
    // TODO: might call on enable twice
    if (settings.getSettingByName<bool>("enabled")->value)
        onEnable();
}

void Module::onSetup() { }

// TODO: rename to Enable/Disable?
void Module::onEnable() {
    settings.getSettingByName<bool>("enabled")->value = true;
    saveSettings();
}

void Module::onDisable() {
    active = false;
    if (!terminating) {
        settings.getSettingByName<bool>("enabled")->value = false;
    }
    saveSettings();
}

void Module::terminate() {
    terminating = true;
    onDisable();
    active = false;
    settings.getSettingByName<bool>("enabled")->value = false;
}

// TODO: find all getSettingByName<bool>("enabled")->value and replace!!!
bool Module::isEnabled() {
    return settings.getSettingByName<bool>("enabled")->value;
}

void Module::setEnabled(bool enabled) {
    settings.getSettingByName<bool>("enabled")->value = enabled;
}

void Module::setKeybind(const std::string& newKeybind) {
    auto key = settings.getSettingByName<std::string>("keybind");
    if (key == nullptr)
        settings.addSetting("keybind", newKeybind);
}

std::string& Module::getKeybind() {
    auto key = settings.getSettingByName<std::string>("keybind");
    if (key == nullptr)
        settings.addSetting("keybind", defaultKeybind);
    return settings.getSettingByName<std::string>("keybind")->value;
}

void Module::defaultConfig() {

    getKeybind();

    if (settings.getSettingByName<bool>("enabled") == nullptr) {
        settings.addSetting("enabled", false);
    }

    if (settings.getSettingByName<float>("percentageX") == nullptr) {
        settings.addSetting("percentageX", 0.0f);
        settings.addSetting("percentageY", 0.0f);
    }

    if (settings.getSettingByName<bool>("border") == nullptr) {
        settings.addSetting("border", false);
        settings.addSetting("borderWidth", 1.0f);
    }

    if (settings.getSettingByName<bool>("reversepaddingx") == nullptr)
        settings.addSetting("reversepaddingx", false);
    if (settings.getSettingByName<bool>("reversepaddingy") == nullptr)
        settings.addSetting("reversepaddingy", false);
    if (settings.getSettingByName<float>("padx") == nullptr) settings.addSetting("padx", 0.0f);
    if (settings.getSettingByName<float>("pady") == nullptr) settings.addSetting("pady", 0.0f);
    if (settings.getSettingByName<float>("rectwidth") == nullptr) settings.addSetting("rectwidth", 1.0f);
    if (settings.getSettingByName<float>("rectheight") == nullptr) settings.addSetting("rectheight", 1.0f);
    if (settings.getSettingByName<bool>("responsivewidth") == nullptr)
        settings.addSetting("responsivewidth", false);
    if (settings.getSettingByName<std::string>("textalignment") == nullptr)
        settings.addSetting("textalignment", (std::string)"Center");

    if (settings.getSettingByName<float>("rounding") == nullptr) settings.addSetting("rounding", 32.0f);

    if (settings.getSettingByName<std::string>("bgColor") == nullptr) {
        settings.addSetting("bgColor", (std::string)"000000");
        settings.addSetting("textColor", (std::string)"fafafa");
        settings.addSetting("borderColor", (std::string)"000000");
    }

    if (settings.getSettingByName<float>("bgOpacity") == nullptr) {
        settings.addSetting("bgOpacity", 0.55f);
        settings.addSetting("textOpacity", 1.0f);
        settings.addSetting("borderOpacity", 1.0f);
    }

    if (settings.getSettingByName<bool>("bgRGB") == nullptr) {
        settings.addSetting("bgRGB", false);
        settings.addSetting("textRGB", false);
        settings.addSetting("borderRGB", false);
    }

    if (settings.getSettingByName<float>("uiscale") == nullptr) {

        settings.addSetting("uiscale", 0.65f);
    }

    if (settings.getSettingByName<float>("rotation") == nullptr) {
        settings.addSetting("rotation", 0.0f);
    }

    if (settings.getSettingByName<bool>("BlurEffect") == nullptr) {
        settings.addSetting("BlurEffect", false);
    }

}

bool Module::isKeybind(const std::array<bool, 256>& keys) {

    std::vector<int> keyCodes = Utils::getStringAsKeys(getKeybind());

    for (int keyCode : keyCodes) {
        if (!keys[keyCode]) {
            // Key is not being held down
            return false;
        }
    }
    // TODO: in menu should also mean in Edit Mode, when edit mode all modules set to active(visible? add visible setting (for TAB))
    if (name != "ClickGUI")
        if (FlarialGUI::inMenu) return false;
    if (SDK::currentScreen == "chat_screen") return false;
    bool allInactive = std::ranges::all_of(FlarialGUI::TextBoxes, [](const TextBoxStruct& i) {
        return !i.isActive;
        });
    // All keys in the keybind are being held down
    return allInactive;
}

[[nodiscard]] bool Module::isAdditionalKeybind(const std::array<bool, 256>& keys, const std::string& bind) const {
    std::vector<int> keyCodes = Utils::getStringAsKeys(bind);

    for (int keyCode : keyCodes) {
        if (!keys[keyCode]) {

            // Key is not being held down
            return false;
        }
    }
    if (name != "ClickGUI")
        if (FlarialGUI::inMenu) return false;
    if (SDK::currentScreen == "chat_screen") return false;
    bool allInactive = std::ranges::all_of(FlarialGUI::TextBoxes, [](const TextBoxStruct& i) {
        return !i.isActive;
        });
    // All keys in the keybind are being held down
    return allInactive;
}

bool Module::isKeyPartOfKeybind(int keyCode) {
    std::vector<int> keyCodes = Utils::getStringAsKeys(getKeybind());
    return std::find(keyCodes.begin(), keyCodes.end(), keyCode) != keyCodes.end();
}

bool Module::isKeyPartOfAdditionalKeybind(int keyCode, const std::string& bind) {
    std::vector<int> keyCodes = Utils::getStringAsKeys(bind);
    return std::find(keyCodes.begin(), keyCodes.end(), keyCode) != keyCodes.end();
}