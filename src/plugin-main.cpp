/*
Echo
Copyright (C) 2025 Voidscape Development

Derived from the atkAudio Plugin for OBS (https://github.com/atkAudio/PluginForObsRelease),
Copyright (C) atkAudio.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "core/CompareVersionStrings.h"
#include "config.h"
#include "core/atkaudio/GlobalSettings.h"
#include "core/atkaudio/Logging.h"
#include "core/atkaudio/atkaudio.h"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <QtWidgets>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR)

MODULE_EXPORT const char* obs_module_name(void)
{
    return PLUGIN_DISPLAY_NAME;
}

extern struct obs_source_info device_io_filter;
extern struct obs_source_info device_io2_filter;

namespace
{
bool g_loadConflictAlertShown = false;

// Lifecycle logging, prefixed with the module name the way obs-plugintemplate does it.
void obs_log(int log_level, const char* format, ...)
{
    size_t length = 4 + strlen(PLUGIN_NAME) + strlen(format);

    char* templ = (char*)malloc(length + 1);
    if (templ == nullptr)
        return;

    snprintf(templ, length, "[%s] %s", PLUGIN_NAME, format);

    va_list args;
    va_start(args, format);
    blogva(log_level, templ, args);
    va_end(args);

    free(templ);
}

obs_module_t* findLoadedModuleConflictByName()
{
    auto* currentModule = obs_current_module();
    auto* loadedModuleByName = obs_get_module(PLUGIN_NAME);

    if (loadedModuleByName == nullptr || loadedModuleByName == currentModule)
        return nullptr;

    return loadedModuleByName;
}

void showDuplicateInstallAlert(obs_module_t* loadedModule)
{
    if (g_loadConflictAlertShown)
        return;

    g_loadConflictAlertShown = true;

    auto* parent = static_cast<QWidget*>(obs_frontend_get_main_window());

    const char* loadedBinaryPath = loadedModule ? obs_get_module_binary_path(loadedModule) : nullptr;
    const char* loadedFileName = loadedModule ? obs_get_module_file_name(loadedModule) : nullptr;

    const QString message = QString::fromUtf8("Another %1 installation is already loaded in this OBS session.\n\n"
                                              "Attempted version: %2\n"
                                              "Loaded module file: %3\n"
                                              "Loaded module path: %4\n"
                                              "Attempted module path: %5\n\n"
                                              "OBS will skip loading this plugin to avoid duplicate registration.\n"
                                              "Please keep only one installation/version and restart OBS.")
                                .arg(QString::fromUtf8(PLUGIN_DISPLAY_NAME))
                                .arg(QString::fromUtf8(PLUGIN_VERSION))
                                .arg(QString::fromUtf8(loadedFileName ? loadedFileName : "(unknown)"))
                                .arg(QString::fromUtf8(loadedBinaryPath ? loadedBinaryPath : "(unknown)"))
                                .arg(QString::fromUtf8(obs_get_module_binary_path(obs_current_module())));

    QMessageBox::critical(parent, PLUGIN_DISPLAY_NAME " Plugin Load Error", message, QMessageBox::Ok);
}

void onFrontendEvent(enum obs_frontend_event event, void* private_data)
{
    UNUSED_PARAMETER(private_data);

    // Re-reads the OBS palette and mirrors it into the JUCE LookAndFeel.
    if (event == OBS_FRONTEND_EVENT_THEME_CHANGED || event == OBS_FRONTEND_EVENT_FINISHED_LOADING)
        atk::getQtMainWindowHandle();
}

std::string getAboutRichText()
{
    return std::string("<p><b>") + PLUGIN_DISPLAY_NAME + " " + PLUGIN_VERSION
        + "</b><br/>"
          "Audio device input/output filters for OBS Studio.</p>"
          "<p>Derived from the <a href=\"https://github.com/atkAudio/PluginForObsRelease\">atkAudio "
          "Plugin for OBS</a> by atkAudio, used under the AGPLv3. "
          "Built with the <a href=\"https://juce.com/\">JUCE framework</a>.</p>"
          "<p><a href=\""
        + std::string(PLUGIN_WEBSITE) + "\">" + PLUGIN_WEBSITE + "</a></p>";
}

void openGlobalSettingsDialog(void* private_data)
{
    UNUSED_PARAMETER(private_data);

    auto* parent = static_cast<QWidget*>(obs_frontend_get_main_window());

    QDialog dialog(parent);
    dialog.setWindowTitle(PLUGIN_DISPLAY_NAME);

    QVBoxLayout layout(&dialog);

    QLabel settingsHeading("Settings");
    settingsHeading.setStyleSheet("font-weight: bold;");
    layout.addWidget(&settingsHeading);

    QCheckBox enableLoggingCheckBox("Enable logging");
    enableLoggingCheckBox.setChecked(atk::settings::isLoggingEnabled());
    layout.addWidget(&enableLoggingCheckBox);

    auto* divider = new QFrame(&dialog);
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    layout.addWidget(divider);

    QLabel aboutHeading("About");
    aboutHeading.setStyleSheet("font-weight: bold;");
    layout.addWidget(&aboutHeading);

    QLabel aboutText(QString::fromUtf8(getAboutRichText().c_str()));
    aboutText.setTextFormat(Qt::RichText);
    aboutText.setOpenExternalLinks(true);
    aboutText.setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutText.setWordWrap(true);
    layout.addWidget(&aboutText);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttons);

    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        const bool loggingEnabled = enableLoggingCheckBox.isChecked();
        atk::settings::setLoggingEnabled(loggingEnabled);
        obs_log(LOG_INFO, "logging %s", loggingEnabled ? "enabled" : "disabled");
    }
}
} // namespace

bool obs_module_load(void)
{
    atk::settings::initialize();
    atk::logging::info("OBS_API", "obs_module_load called");

    auto* loadedModuleConflict = findLoadedModuleConflictByName();
    if (loadedModuleConflict != nullptr)
    {
        const char* loadedBinaryPath = obs_get_module_binary_path(loadedModuleConflict);
        const char* loadedFileName = obs_get_module_file_name(loadedModuleConflict);

        obs_log(
            LOG_ERROR,
            "Detected existing loaded module for plugin name '%s' (file='%s', path='%s'). "
            "Refusing to load version %s to prevent duplicate installations.",
            PLUGIN_NAME,
            loadedFileName ? loadedFileName : "(unknown)",
            loadedBinaryPath ? loadedBinaryPath : "(unknown)",
            PLUGIN_VERSION
        );

        showDuplicateInstallAlert(loadedModuleConflict);

        atk::logging::error("OBS_API", "obs_module_load failed: duplicate installation detected");
        atk::settings::shutdown();
        return false;
    }

    std::string obsCurrentVersion = obs_get_version_string();
    std::string requiredVersion = PLUGIN_OBS_VERSION_REQUIRED;

    if (CompareVersionStrings(obsCurrentVersion, requiredVersion) < 0)
    {
        obs_log(
            LOG_ERROR,
            "Incompatible OBS version: %s (required: %s)",
            obsCurrentVersion.c_str(),
            requiredVersion.c_str()
        );
        atk::logging::error("OBS_API", "obs_module_load failed: incompatible OBS version");
        atk::settings::shutdown();
        return false;
    }

    if (!atk::create())
    {
        obs_log(LOG_ERROR, "Failed to initialize the JUCE runtime");
        atk::logging::error("OBS_API", "obs_module_load failed while initializing lifecycle");
        atk::settings::shutdown();
        return false;
    }

    auto* mainWindow = (QObject*)obs_frontend_get_main_window();
    if (!atk::startMessagePump(mainWindow))
    {
        obs_log(LOG_ERROR, "Failed to start the JUCE message pump");
        atk::settings::shutdown();
        atk::destroy();
        atk::logging::error("OBS_API", "obs_module_load failed while starting message pump");
        return false;
    }

    obs_frontend_add_event_callback(onFrontendEvent, nullptr);

    // The OBS frontend API cannot extend File->Settings, so the Tools menu is the
    // supported entry point for plugin-level settings.
    obs_frontend_add_tools_menu_item(PLUGIN_DISPLAY_NAME, openGlobalSettingsDialog, nullptr);

    obs_register_source(&device_io_filter);
    obs_register_source(&device_io2_filter);

    obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
    atk::logging::info("OBS_API", "obs_module_load completed");

    return true;
}

void obs_module_unload(void)
{
    atk::logging::info("OBS_API", "obs_module_unload called");

    obs_frontend_remove_event_callback(onFrontendEvent, nullptr);

    // PropertiesFile owns a JUCE timer; release it before JUCE runtime teardown.
    atk::settings::shutdown();

    atk::destroy();

    obs_log(LOG_INFO, "plugin unloaded");
}
