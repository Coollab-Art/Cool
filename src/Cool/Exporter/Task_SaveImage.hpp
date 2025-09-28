#pragma once
#include "Cool/File/File.h"
#include "Cool/Task/TaskWithProgressBar.hpp"
#include "no_sleep/no_sleep.hpp"

namespace Cool {

class Task_SaveImage : public TaskWithProgressBar {
public:
    Task_SaveImage(std::filesystem::path const& file_path, img::Image image);

    auto name() const -> std::string override { return fmt::format("Exporting image \"{}\"", Cool::File::file_name(_file_path)); }

private:
    void execute() override;
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return true; }
    auto notification_after_execution_completes() const -> ImGuiNotify::Notification override;

private:
    std::filesystem::path           _file_path;
    img::Image                      _image;
    no_sleep::Scoped                _disable_sleep{COOL_APP_NAME, COOL_APP_NAME " is exporting an image", no_sleep::Mode::ScreenCanTurnOffButKeepComputing};
    tl::expected<void, std::string> _result;
};

} // namespace Cool