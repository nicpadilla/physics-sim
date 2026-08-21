#pragma once

#include <physics_sim/scene_document.hpp>
#include <physics_sim/state_digest.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace physics_sim
{
[[nodiscard]] inline std::string versioned_scene_definition_digest(const SceneDocument &document)
{
    StateDigestBuilder digest{StateDigestDomain::SceneDefinition};
    digest.mix_string("SceneDocument");
    const auto mix_size = [&digest](std::size_t value) { digest.mix_u64(static_cast<std::uint64_t>(value)); };

    mix_size(document.grid_width);
    mix_size(document.grid_height);
    digest.mix_float(document.cell_size);
    digest.mix_bool(document.solver_profile.has_value());
    if (document.solver_profile.has_value())
        digest.mix_enum(*document.solver_profile);

    digest.mix_string(document.metadata.title);
    digest.mix_string(document.metadata.description);
    digest.mix_string(document.metadata.author);
    mix_size(document.metadata.tags.size());
    for (const std::string &tag : document.metadata.tags)
        digest.mix_string(tag);
    mix_size(document.metadata.notes.size());
    for (const std::string &note : document.metadata.notes)
        digest.mix_string(note);
    digest.mix_bool(document.metadata.challenge.has_value());
    if (document.metadata.challenge.has_value())
    {
        const SceneChallenge &challenge = *document.metadata.challenge;
        digest.mix_string(challenge.title);
        mix_size(challenge.required_objective_sensors);
        digest.mix_u64(challenge.hold_ticks);
        digest.mix_bool(challenge.maximum_emitted_mass.has_value());
        if (challenge.maximum_emitted_mass.has_value())
            digest.mix_double(*challenge.maximum_emitted_mass);
        digest.mix_bool(challenge.maximum_outflow_mass.has_value());
        if (challenge.maximum_outflow_mass.has_value())
            digest.mix_double(*challenge.maximum_outflow_mass);
    }

    mix_size(document.solid_cells.size());
    for (const SceneCell &cell : document.solid_cells)
    {
        mix_size(cell.x);
        mix_size(cell.y);
    }

    mix_size(document.emitters.size());
    for (const SceneEmitter &emitter : document.emitters)
    {
        digest.mix_enum(emitter.kind);
        digest.mix_float(emitter.position.x);
        digest.mix_float(emitter.position.y);
        digest.mix_float(emitter.direction.x);
        digest.mix_float(emitter.direction.y);
        digest.mix_float(emitter.speed);
        digest.mix_float(emitter.emission_rate);
        digest.mix_bool(emitter.enabled);
    }

    mix_size(document.gates.size());
    for (const SceneGate &gate : document.gates)
    {
        mix_size(gate.x);
        mix_size(gate.y);
        digest.mix_bool(gate.open);
    }

    mix_size(document.sensors.size());
    for (const SceneSensor &sensor : document.sensors)
    {
        mix_size(sensor.x);
        mix_size(sensor.y);
        mix_size(sensor.width);
        mix_size(sensor.height);
        digest.mix_bool(sensor.enabled);
        digest.mix_bool(sensor.objective);
        digest.mix_string(sensor.label);
    }

    mix_size(document.drains.size());
    for (const SceneDrain &drain : document.drains)
    {
        mix_size(drain.x);
        mix_size(drain.y);
        mix_size(drain.width);
        mix_size(drain.height);
        digest.mix_bool(drain.enabled);
    }

    mix_size(document.pumps.size());
    for (const ScenePump &pump : document.pumps)
    {
        mix_size(pump.x);
        mix_size(pump.y);
        mix_size(pump.width);
        mix_size(pump.height);
        digest.mix_bool(pump.enabled);
        digest.mix_float(pump.direction.x);
        digest.mix_float(pump.direction.y);
        digest.mix_float(pump.strength);
    }

    mix_size(document.valves.size());
    for (const SceneValve &valve : document.valves)
    {
        mix_size(valve.x);
        mix_size(valve.y);
        digest.mix_bool(valve.open);
    }

    return digest.finish();
}
} // namespace physics_sim
