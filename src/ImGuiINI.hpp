/* Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu,
 * University of Antwerp - All Rights Reserved.
 * You may use, distribute and modify
 * this code under the terms of the GPL3 license.
 * You should have received a copy of the GPL3 license with
 * this file. If not, please visit:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Authors:
 *   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
 *   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
 */

#include "imgui.h"
#include "ini.h"
#include <string>

namespace ImGuiINI
{
    // Styles
    void set_style(int style_idx)
    {
        switch (style_idx)
        {
        case 0:
            ImGui::StyleColorsDark();
            break;
        case 1:
            ImGui::StyleColorsLight();
            break;
        case 2:
            ImGui::StyleColorsClassic();
            break;
        }
    }

    bool ShowStyleSelector(const char *label, int &style_idx, mINI::INIStructure &ini_cfg)
    {
        if (ImGui::Combo(label, &style_idx, "Dark\0Light\0Classic\0"))
        {
            set_style(style_idx);
            ini_cfg["Appearance"]["Style"] = std::to_string(style_idx);
            return true;
        }
        return false;
    }

    // Fonts
    const char *font_names[6] = {
        "Karla-Regular",
        "Roboto-Medium",
        "Cousine-Regular",
        "DroidSans",
        "ProggyClean",
        "ProggyTiny"};

    bool ShowFontSelector(const char *label, int &selectedFont, mINI::INIStructure &ini_cfg)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImFont *font_current = ImGui::GetFont();
        if (ImGui::BeginCombo(label, font_names[selectedFont]))
        {
            for (int n = 0; n < io.Fonts->Fonts.Size; n++)
            {
                ImFont *font = io.Fonts->Fonts[n];
                ImGui::PushID((void *)font);
                if (ImGui::Selectable(font_names[n], font == font_current))
                {
                    io.FontDefault = font;
                    selectedFont = n;
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
            ini_cfg["Appearance"]["Font"] = std::to_string(selectedFont);
            return true;
        }
        return false;
    }

    void set_font(int font_idx)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImFont *font = io.Fonts->Fonts[font_idx];
        io.FontDefault = font;
    }

    // Helper functions for conversions
    template <typename T>
    std::vector<T> split2T(std::string &s, char delimiter)
    {
        std::vector<T> values;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            values.push_back((T)std::stod(token));
        }
        return values;
    }

    std::string ImVec2string(const ImVec4 &v)
    {
        std::string s = std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w);
        return s;
    }

    std::string ImVec2string(const ImVec2 &v)
    {
        std::string s = std::to_string(v.x) + "," + std::to_string(v.y);
        return s;
    }

    // Check if INI has a stored value for a given key and update the value if it does
    // Strings
    void check_ini_setting(mINI::INIStructure &ini_cfg, std::string section, std::string key, char *value)
    {
        if (ini_cfg.has(section))
        {
            if (ini_cfg[section].has(key))
            {
                value = &ini_cfg[section][key][0];
            }
            ini_cfg[section][key] = std::string(value);
        }
    }

    void check_ini_setting(mINI::INIStructure &ini_cfg, std::string section, std::string key, std::string value)
    {
        if (ini_cfg.has(section))
        {
            if (ini_cfg[section].has(key))
            {
                value = ini_cfg[section][key];
            }
            ini_cfg[section][key] = value;
        }
    }

    // General Numeric types
    template <typename T>
    void check_ini_setting(mINI::INIStructure &ini_cfg, std::string section, std::string key, T &value)
    {
        if (ini_cfg.has(section))
        {
            if (ini_cfg[section].has(key))
            {
                value = (T)stod(ini_cfg[section][key]);
            }
            ini_cfg[section][key] = std::to_string(value);
        }
    }

    // ImVec4
    void check_ini_setting(mINI::INIStructure &ini_cfg, std::string section, std::string key, ImVec4 &value)
    {
        if (ini_cfg.has(section))
        {
            if (ini_cfg[section].has(key))
            {
                std::vector<float> val_vec = split2T<float>(ini_cfg[section][key], ',');
                value = ImVec4(val_vec[0], val_vec[1], val_vec[2], val_vec[3]);
            }
            ini_cfg[section][key] = ImVec2string(value);
        }
    }

    // ImVec2
    void check_ini_setting(mINI::INIStructure &ini_cfg, std::string section, std::string key, ImVec2 &value)
    {
        if (ini_cfg.has(section))
        {
            if (ini_cfg[section].has(key))
            {
                std::vector<float> val_vec = split2T<float>(ini_cfg[section][key], ',');
                value = ImVec2(val_vec[0], val_vec[1]);
            }
            ini_cfg[section][key] = ImVec2string(value);
        }
    }
}