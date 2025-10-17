#include "SaveImage.h"
#include "Cool/File/File.h"

namespace Cool::ImageU {

auto save(
    std::filesystem::path const& file_path,
    img::Image const&            image,
    stbiw_SaveOptions const&     options
) -> tl::expected<void, std::string>
{
    return Cool::ImageU::save(file_path, image.size().width(), image.size().height(), image.data(), image.channels_count(), image.row_order(), options);
}

auto save(
    std::filesystem::path const& file_path,
    img::Size::DataType          width,
    img::Size::DataType          height,
    const void*                  data,
    size_t                       channels_count,
    img::FirstRowIs              row_order,
    stbiw_SaveOptions const&     options
) -> tl::expected<void, std::string>
{
    if (!File::create_folders_for_file_if_they_dont_exist(file_path))
        return tl::make_unexpected("Maybe you are not allowed to save files in this folder?");

    std::string const extension = Cool::File::extension(file_path).string();

    bool success{};
    if (extension == ".png" || extension == ".PNG")
        success = img::save_png(file_path, width, height, data, channels_count, row_order, options);
    else if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG")
        success = img::save_jpeg(file_path, width, height, data, channels_count, row_order, options);
    else
        return tl::make_unexpected(fmt::format("Unsupported format \"{}\"\nPlease use \".png\" or \".jpg\"", extension));

    if (!success)
    {
        return tl::make_unexpected(
            (options.cancel_requested && options.cancel_requested())
                ? "Canceled"
                : "Maybe you are not allowed to save files in this folder?"
        );
    }
    return {};
}

} // namespace Cool::ImageU