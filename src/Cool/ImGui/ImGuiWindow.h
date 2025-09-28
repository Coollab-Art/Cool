#pragma once

namespace Cool {

struct ImGuiWindowConfig {
    bool is_modal   = false;
    bool start_open = false;
};

class ImGuiWindow {
public:
    explicit ImGuiWindow(std::string title, ImGuiWindowConfig const& config = {})
        : _title{std::move(title)}
        , _is_modal{config.is_modal}
        , _is_open{config.start_open}
    {
    }

    /**
     * @brief Shows the windows if it is open
     *
     * @param widgets A function that renders all the desired widgets
     */
    void show(std::function<void(bool is_opening)> const& widgets);
    void open();
    void close();

    void open_close_toggle();

private:
    std::string _title;
    bool        _is_modal;
    bool        _is_open;
    bool        _just_opened{true};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Is open", _is_open)
        );
    }
};

} // namespace Cool
