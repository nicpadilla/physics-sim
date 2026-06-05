#pragma once

#include <physics_sim/scene_document.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace physics_sim
{
enum class SaveBrowserEntryKind
{
    Autosave,
    NamedSave,
    Back,
};

struct SaveBrowserEntry
{
    SaveBrowserEntryKind kind = SaveBrowserEntryKind::NamedSave;
    std::filesystem::path path{};
    std::string label{};
    bool loadable = true;
};

[[nodiscard]] inline std::string sanitize_save_name(std::string_view text)
{
    std::string slug;
    slug.reserve(text.size());
    bool last_was_hyphen = false;

    for (const unsigned char ch : text)
    {
        if (std::isalnum(ch))
        {
            slug.push_back(static_cast<char>(std::tolower(ch)));
            last_was_hyphen = false;
            continue;
        }

        if (!slug.empty() && !last_was_hyphen)
        {
            slug.push_back('-');
            last_was_hyphen = true;
        }
    }

    while (!slug.empty() && slug.front() == '-')
    {
        slug.erase(slug.begin());
    }

    while (!slug.empty() && slug.back() == '-')
    {
        slug.pop_back();
    }

    if (slug.empty())
    {
        return "save";
    }

    return slug;
}

[[nodiscard]] inline std::string save_slot_name(const std::filesystem::path& scene_path, const SceneMetadata& metadata)
{
    if (!metadata.title.empty())
    {
        return sanitize_save_name(metadata.title);
    }

    if (!scene_path.empty())
    {
        return sanitize_save_name(scene_path.stem().string());
    }

    return "save";
}

[[nodiscard]] inline std::filesystem::path named_save_path(
    const std::filesystem::path& save_directory,
    const std::filesystem::path& scene_path,
    const SceneMetadata& metadata)
{
    return save_directory / (save_slot_name(scene_path, metadata) + ".pscene");
}

[[nodiscard]] inline std::filesystem::path backup_scene_path(const std::filesystem::path& path)
{
    return path.parent_path() / (path.stem().string() + ".bak" + path.extension().string());
}

namespace detail
{
[[nodiscard]] inline std::string loadable_scene_label(
    const std::filesystem::path& path,
    const SceneDocument& document,
    bool is_autosave)
{
    std::string label = !document.metadata.title.empty() ? document.metadata.title : path.stem().string();
    if (is_autosave)
    {
        label += " (Autosave)";
    }

    return label;
}

[[nodiscard]] inline std::string corrupt_scene_label(const std::filesystem::path& path, bool is_autosave)
{
    std::string label = std::string{"CORRUPT: "} + path.stem().string();
    if (is_autosave)
    {
        label += " (Autosave)";
    }

    return label;
}
} // namespace detail

[[nodiscard]] inline std::vector<SaveBrowserEntry> build_save_browser_entries(
    const std::filesystem::path& save_directory,
    const std::filesystem::path& autosave_path,
    bool include_back_entry = true)
{
    std::vector<SaveBrowserEntry> entries;

    if (std::filesystem::exists(autosave_path))
    {
        const auto document = load_scene(autosave_path);
        if (document.has_value())
        {
            entries.push_back({
                SaveBrowserEntryKind::Autosave,
                autosave_path,
                detail::loadable_scene_label(autosave_path, *document, true),
                true,
            });
        }
        else
        {
            entries.push_back({
                SaveBrowserEntryKind::Autosave,
                autosave_path,
                detail::corrupt_scene_label(autosave_path, true),
                false,
            });
        }
    }

    std::vector<SaveBrowserEntry> named_entries;
    std::error_code ec;
    if (std::filesystem::exists(save_directory, ec))
    {
        for (const auto& directory_entry : std::filesystem::directory_iterator(save_directory, ec))
        {
            if (ec || !directory_entry.is_regular_file())
            {
                continue;
            }

            const auto path = directory_entry.path();
            if (path == autosave_path || path.extension() != ".pscene")
            {
                continue;
            }

            const auto document = load_scene(path);
            if (document.has_value())
            {
                named_entries.push_back({
                    SaveBrowserEntryKind::NamedSave,
                    path,
                    detail::loadable_scene_label(path, *document, false),
                    true,
                });
            }
            else
            {
                named_entries.push_back({
                    SaveBrowserEntryKind::NamedSave,
                    path,
                    detail::corrupt_scene_label(path, false),
                    false,
                });
            }
        }
    }

    std::sort(named_entries.begin(), named_entries.end(), [](const SaveBrowserEntry& lhs, const SaveBrowserEntry& rhs)
    {
        return lhs.label < rhs.label;
    });

    entries.insert(entries.end(), named_entries.begin(), named_entries.end());

    if (include_back_entry)
    {
        entries.push_back({
            SaveBrowserEntryKind::Back,
            {},
            "Back",
            true,
        });
    }

    return entries;
}
} // namespace physics_sim
