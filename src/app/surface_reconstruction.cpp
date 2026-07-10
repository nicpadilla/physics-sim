#include "physics_sim/surface_reconstruction.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <queue>

namespace physics_sim
{
namespace
{

struct Sample
{
    SurfacePoint point{};
    float value = 0.0f;
};

[[nodiscard]] Sample intersection(const Sample& inside, const Sample& outside, float threshold)
{
    const float denominator = outside.value - inside.value;
    const float t = denominator == 0.0f ? 0.5f : std::clamp((threshold - inside.value) / denominator, 0.0f, 1.0f);
    return {
        {inside.point.x + (outside.point.x - inside.point.x) * t,
         inside.point.y + (outside.point.y - inside.point.y) * t},
        threshold};
}

void append_clipped_triangle(
    std::vector<SurfaceTriangle>& output,
    const std::array<Sample, 3>& input,
    float threshold)
{
    std::array<Sample, 6> polygon{};
    std::size_t count = 0;
    for (std::size_t index = 0; index < input.size(); ++index)
    {
        const Sample& current = input[index];
        const Sample& next = input[(index + 1) % input.size()];
        const bool current_inside = current.value > threshold;
        const bool next_inside = next.value > threshold;
        if (current_inside)
        {
            polygon[count++] = current;
        }
        if (current_inside != next_inside)
        {
            polygon[count++] = current_inside
                ? intersection(current, next, threshold)
                : intersection(next, current, threshold);
        }
    }

    for (std::size_t index = 1; index + 1 < count; ++index)
    {
        const float coverage = std::clamp(
            (polygon[0].value + polygon[index].value + polygon[index + 1].value) / 3.0f,
            threshold,
            1.0f);
        output.push_back({polygon[0].point, polygon[index].point, polygon[index + 1].point, coverage});
    }
}

} // namespace

std::vector<SurfaceTriangle> reconstruct_surface(
    std::span<const float> cell_volume_fractions,
    std::span<const std::uint8_t> solid_cells,
    std::size_t width,
    std::size_t height,
    float cell_size,
    float threshold)
{
    if (width == 0 || height == 0 || cell_volume_fractions.size() != width * height ||
        (!solid_cells.empty() && solid_cells.size() != width * height))
    {
        return {};
    }

    std::vector<float> vertices((width + 1) * (height + 1), 0.0f);
    for (std::size_t vertex_y = 0; vertex_y <= height; ++vertex_y)
    {
        for (std::size_t vertex_x = 0; vertex_x <= width; ++vertex_x)
        {
            float sum = 0.0f;
            std::size_t count = 0;
            for (int offset_y = -1; offset_y <= 0; ++offset_y)
            {
                for (int offset_x = -1; offset_x <= 0; ++offset_x)
                {
                    const int cell_x = static_cast<int>(vertex_x) + offset_x;
                    const int cell_y = static_cast<int>(vertex_y) + offset_y;
                    if (cell_x < 0 || cell_y < 0 || cell_x >= static_cast<int>(width) || cell_y >= static_cast<int>(height))
                    {
                        continue;
                    }
                    const std::size_t index = static_cast<std::size_t>(cell_y) * width + static_cast<std::size_t>(cell_x);
                    if (!solid_cells.empty() && solid_cells[index])
                    {
                        continue;
                    }
                    sum += std::clamp(cell_volume_fractions[index], 0.0f, 1.0f);
                    ++count;
                }
            }
            vertices[vertex_y * (width + 1) + vertex_x] = count == 0 ? 0.0f : sum / static_cast<float>(count);
        }
    }

    std::vector<SurfaceTriangle> triangles;
    triangles.reserve(width * height * 3);
    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const std::size_t cell_index = y * width + x;
            if (!solid_cells.empty() && solid_cells[cell_index])
            {
                continue;
            }
            const Sample top_left{{x * cell_size, y * cell_size}, vertices[y * (width + 1) + x]};
            const Sample top_right{{(x + 1) * cell_size, y * cell_size}, vertices[y * (width + 1) + x + 1]};
            const Sample bottom_right{{(x + 1) * cell_size, (y + 1) * cell_size}, vertices[(y + 1) * (width + 1) + x + 1]};
            const Sample bottom_left{{x * cell_size, (y + 1) * cell_size}, vertices[(y + 1) * (width + 1) + x]};
            append_clipped_triangle(triangles, {top_left, top_right, bottom_right}, threshold);
            append_clipped_triangle(triangles, {top_left, bottom_right, bottom_left}, threshold);
        }
    }
    return triangles;
}

SurfaceSemantics analyze_surface(
    std::span<const float> cell_volume_fractions,
    std::span<const std::uint8_t> solid_cells,
    std::size_t width,
    std::size_t height,
    float threshold,
    std::size_t allowed_min_x,
    std::size_t allowed_max_x,
    std::size_t allowed_min_y,
    std::size_t allowed_max_y)
{
    SurfaceSemantics result{};
    result.min_x = width;
    result.min_y = height;
    if (cell_volume_fractions.size() != width * height || (!solid_cells.empty() && solid_cells.size() != width * height))
    {
        return result;
    }
    allowed_max_x = std::min(allowed_max_x, width == 0 ? 0 : width - 1);
    allowed_max_y = std::min(allowed_max_y, height == 0 ? 0 : height - 1);
    std::vector<bool> occupied(width * height, false);
    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const std::size_t index = y * width + x;
            occupied[index] = (solid_cells.empty() || !solid_cells[index]) && cell_volume_fractions[index] > threshold;
            if (!occupied[index])
            {
                continue;
            }
            ++result.occupied_cells;
            result.min_x = std::min(result.min_x, x);
            result.max_x = std::max(result.max_x, x);
            result.min_y = std::min(result.min_y, y);
            result.max_y = std::max(result.max_y, y);
            if (x < allowed_min_x || x > allowed_max_x || y < allowed_min_y || y > allowed_max_y)
            {
                ++result.outside_allowed_cells;
            }
        }
    }

    std::vector<bool> visited(width * height, false);
    constexpr std::array<std::array<int, 2>, 4> neighbors{{{{-1, 0}}, {{1, 0}}, {{0, -1}}, {{0, 1}}}};
    for (std::size_t start = 0; start < occupied.size(); ++start)
    {
        if (!occupied[start] || visited[start])
        {
            continue;
        }
        ++result.connected_components;
        std::size_t component_size = 0;
        std::queue<std::size_t> pending;
        pending.push(start);
        visited[start] = true;
        while (!pending.empty())
        {
            const std::size_t index = pending.front();
            pending.pop();
            ++component_size;
            const int x = static_cast<int>(index % width);
            const int y = static_cast<int>(index / width);
            for (const auto& offset : neighbors)
            {
                const int nx = x + offset[0];
                const int ny = y + offset[1];
                if (nx < 0 || ny < 0 || nx >= static_cast<int>(width) || ny >= static_cast<int>(height))
                {
                    continue;
                }
                const std::size_t neighbor = static_cast<std::size_t>(ny) * width + static_cast<std::size_t>(nx);
                if (occupied[neighbor] && !visited[neighbor])
                {
                    visited[neighbor] = true;
                    pending.push(neighbor);
                }
            }
        }
        if (component_size == 1)
        {
            ++result.isolated_cells;
        }
    }
    if (result.occupied_cells == 0)
    {
        result.min_x = result.min_y = result.max_x = result.max_y = 0;
    }
    return result;
}

} // namespace physics_sim
