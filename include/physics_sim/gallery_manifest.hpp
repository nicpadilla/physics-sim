#pragma once

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace physics_sim
{
enum class GalleryCategory
{
    Learn,
    Sandbox,
    Challenges,
};

struct GalleryEntry
{
    std::string id;
    std::string title;
    std::string description;
    GalleryCategory category = GalleryCategory::Sandbox;
    std::filesystem::path scene_path;
    std::filesystem::path thumbnail_path;
    std::vector<std::string> required_features;
    int sort_order = 0;
};

struct GalleryManifest
{
    int version = 0;
    std::vector<GalleryEntry> entries;
};

[[nodiscard]] inline constexpr std::string_view gallery_category_name(GalleryCategory category) noexcept
{
    switch (category)
    {
    case GalleryCategory::Learn: return "LEARN";
    case GalleryCategory::Sandbox: return "SANDBOX";
    case GalleryCategory::Challenges: return "CHALLENGES";
    }
    return "SANDBOX";
}

[[nodiscard]] inline bool load_gallery_manifest(
    const std::filesystem::path& path,
    GalleryManifest& manifest,
    std::string* error = nullptr)
{
    const auto fail = [&](std::string message)
    {
        if (error != nullptr) *error = std::move(message);
        return false;
    };
    std::ifstream input(path);
    if (!input) return fail("Gallery manifest could not be opened: " + path.string());

    GalleryManifest parsed;
    std::string line;
    if (!std::getline(input, line) || line != "physics-sim-gallery\t1")
    {
        return fail("Gallery manifest requires version 1.");
    }
    parsed.version = 1;
    std::set<std::string> ids;
    std::size_t line_number = 1;
    while (std::getline(input, line))
    {
        ++line_number;
        if (line.empty() || line.front() == '#') continue;
        std::vector<std::string> fields;
        std::stringstream stream(line);
        std::string field;
        while (std::getline(stream, field, '\t')) fields.push_back(field);
        if (fields.size() != 9 || fields[0] != "entry")
        {
            return fail("Malformed gallery entry at line " + std::to_string(line_number) + ".");
        }
        GalleryEntry entry;
        entry.id = fields[1];
        if (fields[2] == "learn") entry.category = GalleryCategory::Learn;
        else if (fields[2] == "sandbox") entry.category = GalleryCategory::Sandbox;
        else if (fields[2] == "challenges") entry.category = GalleryCategory::Challenges;
        else return fail("Unknown gallery category at line " + std::to_string(line_number) + ".");
        const auto [end, ec] = std::from_chars(fields[3].data(), fields[3].data() + fields[3].size(), entry.sort_order);
        if (ec != std::errc{} || end != fields[3].data() + fields[3].size())
        {
            return fail("Invalid gallery sort order at line " + std::to_string(line_number) + ".");
        }
        entry.scene_path = fields[4];
        entry.thumbnail_path = fields[5];
        std::stringstream feature_stream(fields[6]);
        while (std::getline(feature_stream, field, ',')) if (!field.empty()) entry.required_features.push_back(field);
        entry.title = fields[7];
        entry.description = fields[8];
        if (entry.id.empty() || entry.title.empty() || entry.description.empty()
            || entry.scene_path.empty() || entry.thumbnail_path.empty()
            || entry.scene_path.is_absolute() || entry.thumbnail_path.is_absolute()
            || entry.scene_path.string().find("..") != std::string::npos
            || entry.thumbnail_path.string().find("..") != std::string::npos)
        {
            return fail("Unsafe or incomplete gallery entry at line " + std::to_string(line_number) + ".");
        }
        if (!ids.insert(entry.id).second) return fail("Duplicate gallery id: " + entry.id);
        parsed.entries.push_back(std::move(entry));
    }
    if (parsed.entries.empty()) return fail("Gallery manifest contains no entries.");
    std::stable_sort(parsed.entries.begin(), parsed.entries.end(), [](const GalleryEntry& lhs, const GalleryEntry& rhs)
    {
        return lhs.sort_order < rhs.sort_order;
    });
    manifest = std::move(parsed);
    return true;
}
} // namespace physics_sim
