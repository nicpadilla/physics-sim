#define SDL_MAIN_HANDLED

#include <physics_sim/challenge_objective.hpp>
#include <physics_sim/replay_script.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace
{
[[noreturn]] void fail(const char* expression, int line)
{
    std::fprintf(stderr, "FAIL challenge_objective_tests.cpp:%d: %s\n", line, expression);
    std::exit(1);
}
}
#define REQUIRE(value) do { if (!(value)) fail(#value, __LINE__); } while (false)

int main()
{
    physics_sim::SceneChallenge challenge{"Hold both targets", 2, 3, 10.0, 2.0};
    physics_sim::WaterSimulationMetrics metrics;
    metrics.objective_sensors = 2;
    metrics.objective_completed = true;
    physics_sim::ChallengeEvaluator evaluator;
    REQUIRE(evaluator.update(challenge, metrics, 10).status == physics_sim::ChallengeStatus::Running);
    REQUIRE(evaluator.update(challenge, metrics, 11).held_ticks == 2);
    const auto complete = evaluator.update(challenge, metrics, 12);
    REQUIRE(complete.status == physics_sim::ChallengeStatus::Complete && complete.completion_tick == 12);
    evaluator.reset();
    metrics.objective_completed = false;
    REQUIRE(evaluator.update(challenge, metrics, 20).held_ticks == 0);
    metrics.total_emitted_mass = 11.0;
    REQUIRE(evaluator.update(challenge, metrics, 21).status == physics_sim::ChallengeStatus::Failed);
    evaluator.reset();
    REQUIRE(evaluator.progress().status == physics_sim::ChallengeStatus::Inactive);

    namespace fs = std::filesystem;
    for (const char* name : {"objective_fill.pscene", "challenge_gate.pscene", "challenge_pump_valve.pscene"})
    {
        const auto document = physics_sim::load_scene(fs::path{PHYSICS_SIM_SOURCE_DIR} / "scenes" / name);
        REQUIRE(document.has_value());
        REQUIRE(document->metadata.challenge.has_value());
        const auto objectives = std::count_if(document->sensors.begin(), document->sensors.end(), [](const auto& sensor)
        { return sensor.objective && sensor.enabled; });
        REQUIRE(objectives >= static_cast<std::ptrdiff_t>(document->metadata.challenge->required_objective_sensors));
    }
    for (const char* name : {"challenge_fill.replay", "challenge_gate.replay", "challenge_pump_valve.replay"})
    {
        REQUIRE(physics_sim::load_replay_script(fs::path{PHYSICS_SIM_SOURCE_DIR} / "regression" / "replays" / name).has_value());
    }
    return 0;
}
