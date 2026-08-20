from __future__ import annotations

from pathlib import Path
import json
import re
import textwrap

ROOT = Path(__file__).resolve().parents[1]
CHANGED: list[str] = []


def path_for(relative: str) -> Path:
    return ROOT / relative


def read(relative: str) -> str:
    return path_for(relative).read_text(encoding="utf-8")


def write(relative: str, content: str) -> None:
    path = path_for(relative)
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_text(encoding="utf-8") == content:
        return
    path.write_text(content, encoding="utf-8", newline="\n")
    CHANGED.append(relative)


def replace_once(relative: str, old: str, new: str) -> None:
    text = read(relative)
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{relative}: expected one occurrence, found {count}: {old[:80]!r}")
    write(relative, text.replace(old, new, 1))


def replace_all(relative: str, old: str, new: str, minimum: int = 1) -> None:
    text = read(relative)
    count = text.count(old)
    if count < minimum:
        raise RuntimeError(f"{relative}: expected at least {minimum} occurrences, found {count}: {old[:80]!r}")
    write(relative, text.replace(old, new))


def function_range(text: str, signature: str) -> tuple[int, int]:
    start = text.find(signature)
    if start < 0:
        raise RuntimeError(f"function signature not found: {signature}")
    brace = text.find("{", start + len(signature))
    if brace < 0:
        raise RuntimeError(f"opening brace not found for: {signature}")
    depth = 0
    in_string = False
    in_char = False
    escaped = False
    line_comment = False
    block_comment = False
    i = brace
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if line_comment:
            if ch == "\n":
                line_comment = False
            i += 1
            continue
        if block_comment:
            if ch == "*" and nxt == "/":
                block_comment = False
                i += 2
                continue
            i += 1
            continue
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            i += 1
            continue
        if in_char:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == "'":
                in_char = False
            i += 1
            continue
        if ch == "/" and nxt == "/":
            line_comment = True
            i += 2
            continue
        if ch == "/" and nxt == "*":
            block_comment = True
            i += 2
            continue
        if ch == '"':
            in_string = True
        elif ch == "'":
            in_char = True
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return start, i + 1
        i += 1
    raise RuntimeError(f"closing brace not found for: {signature}")


def replace_function(relative: str, signature: str, replacement: str) -> None:
    text = read(relative)
    start, end = function_range(text, signature)
    write(relative, text[:start] + replacement.rstrip() + text[end:])


def insert_after_function(relative: str, signature: str, addition: str) -> None:
    text = read(relative)
    _, end = function_range(text, signature)
    write(relative, text[:end] + "\n\n" + addition.strip("\n") + text[end:])


def append_before_last(relative: str, marker: str, addition: str) -> None:
    text = read(relative)
    index = text.rfind(marker)
    if index < 0:
        raise RuntimeError(f"{relative}: final marker not found: {marker!r}")
    write(relative, text[:index] + addition.rstrip() + "\n\n" + text[index:])


# ---------------------------------------------------------------------------
# GitHub-native tracking and contributor workflow (#9)
# ---------------------------------------------------------------------------

write(
    "CONTRIBUTING.md",
    textwrap.dedent(
        """\
        # Contributing to Physics Sim

        ## Active work

        GitHub Issues are the source of truth for active work. `ISSUES.md` and `PROGRESS.md` preserve the completed recovery program; do not add new task status to those ledgers.

        Before starting code, make the issue state the problem, desired contract, acceptance criteria, verification commands, dependencies, and explicit out-of-scope work.

        ## Branches and pull requests

        - Create a focused branch from `main`.
        - Keep one coherent issue or tightly coupled dependency group in a pull request.
        - Link issues with `Fixes #<number>` only when every acceptance criterion is met. Use `Refs #<number>` for partial work.
        - Do not mix solver tuning, refactoring, baseline regeneration, and product changes without an issue that requires the combined change.
        - Explain every changed threshold, replay digest, or visual baseline. A changed hash alone is not acceptance evidence.

        ## Verification tiers

        - **Fast:** documentation helpers and narrow logic changes.
        - **Standard:** runtime, scene, replay, UI, content, device, challenge, Lab, and ordinary solver changes.
        - **Full:** solver behavior, digest contracts, baselines, packaging, release workflow, or broad architecture changes.

        Common commands:

        ```powershell
        .\\scripts\\check-hygiene.ps1
        .\\scripts\\test.ps1 -Tier Fast
        .\\scripts\\test.ps1 -Tier Standard
        .\\scripts\\verify-all.ps1
        ```

        ## Commit and evidence rules

        - Keep commits reviewable and state the issue or contract they implement.
        - Record exact commands, results, timings, digests, benchmark values, and artifact paths in the pull request.
        - Preserve failure artifacts before changing code or baselines.
        - Reserve **Human Accepted** for a named, dated human review. Record automated and agent review as separate evidence.
        - Never weaken a gate merely to make CI pass. Explain the physical or product reason for every gate change.
        """
    ),
)

write(
    ".github/ISSUE_TEMPLATE/implementation.md",
    textwrap.dedent(
        """\
        ---
        name: Implementation work
        about: Define a bug, refactor, validation task, or feature with testable completion rules
        title: ""
        labels: ""
        assignees: ""
        ---

        ## Tracking metadata

        - **Priority:** P0 / P1 / P2
        - **Milestone:**
        - **Area:**
        - **Type:** Bug / Refactor / Validation / Feature / Documentation / Tooling
        - **Size:** S / M / L / XL

        ## Problem

        ## Desired contract

        ## Scope

        ## Acceptance criteria

        - [ ]

        ## Verification

        ```powershell
        # Exact commands
        ```

        ## Dependencies

        ## Out of scope

        ## Evidence

        Add commands, results, timings, digests, benchmark values, captures, and review records during implementation.
        """
    ),
)

write(
    ".github/pull_request_template.md",
    textwrap.dedent(
        """\
        ## Linked work

        Fixes #

        ## Summary

        ## Contract and risk

        - Behavior changed:
        - Determinism or persistence impact:
        - Compatibility impact:
        - Main risks:

        ## Verification

        | Command | Result | Duration / evidence |
        | --- | --- | --- |
        |  |  |  |

        ## Baselines and thresholds

        - [ ] No baseline, digest, or threshold changed.
        - [ ] Changes are listed below with the reason the old evidence became invalid.

        ## Review evidence

        - Automated:
        - Agent review:
        - Named human review, when required:
        """
    ),
)

for relative, heading, notice in [
    (
        "ISSUES.md",
        "# Recovery Issues\n",
        "\n> **Completed recovery ledger.** GitHub Issues now hold active work. Preserve this file as the implementation and acceptance record for the recovery program.\n",
    ),
    (
        "PROGRESS.md",
        "# Recovery Progress\n",
        "\n> **Completed recovery ledger.** GitHub Issues now hold active work. The statuses below remain the dated recovery evidence record.\n",
    ),
    (
        "ROADMAP.md",
        "# Recovery Roadmap\n",
        "\n> **Completed recovery roadmap.** Current product direction belongs in GitHub milestones and focused roadmap documents; active task status belongs in GitHub Issues.\n",
    ),
]:
    text = read(relative)
    if notice.strip() not in text:
        if not text.startswith(heading):
            raise RuntimeError(f"{relative}: unexpected heading")
        write(relative, text.replace(heading, heading + notice, 1))

tracking = read("docs/TRACKING.md")
tracking_heading_end = tracking.find("\n", tracking.find("# ")) + 1
tracking_section = textwrap.dedent(
    """

    ## Active work after recovery

    GitHub Issues are the source of truth for active implementation work. `ISSUES.md`, `PROGRESS.md`, and the recovery sections of `ROADMAP.md` are completed historical ledgers and remain subject to this file's evidence checks. New work must not be copied into those ledgers as a second status system.

    Pull requests link active issues and record exact verification evidence. **Human Accepted** requires a named, dated human review; automated checks and agent review use their own evidence labels.
    """
)
if "## Active work after recovery" not in tracking:
    write("docs/TRACKING.md", tracking[:tracking_heading_end] + tracking_section + tracking[tracking_heading_end:])

agents = read("AGENTS.md")
if "## Active GitHub tracking" not in agents:
    write(
        "AGENTS.md",
        agents.rstrip()
        + textwrap.dedent(
            """


            ## Active GitHub tracking

            - GitHub Issues are the source of truth for new work.
            - `ISSUES.md`, `PROGRESS.md`, and the recovery roadmap are completed recovery ledgers; update them only to correct their historical evidence.
            - Link pull requests to issues and record exact verification, timing, digest, benchmark, and baseline rationale.
            - Do not mark agent review as `Human Accepted`.
            """
        ),
    )

readme = read("README.md")
readme = readme.replace(
    "Version `0.2.0-alpha.2` is the complete-water-sandbox recovery prerelease.",
    "Version `0.2.0-alpha.3` is the runtime-correctness and tracking prerelease.",
)
readme = readme.replace(
    "- `ROADMAP.md`: recovery product contract.\n- `PROGRESS.md`: current implementation, automation, and human-acceptance status.\n- `ISSUES.md`: implementation-ready recovery queue.",
    "- GitHub Issues: active implementation work and release milestones.\n- `ROADMAP.md`: completed recovery product contract and retained direction record.\n- `PROGRESS.md`: completed recovery implementation, automation, and acceptance ledger.\n- `ISSUES.md`: completed implementation-ready recovery ledger.",
)
write("README.md", readme)

# ---------------------------------------------------------------------------
# Runtime-state contract and deterministic checkpoint history (#10-#12)
# ---------------------------------------------------------------------------

write(
    "docs/runtime-state-contract.md",
    textwrap.dedent(
        """\
        # Runtime State Contract

        Status: Accepted for the post-recovery correctness workstream.

        ## State domains

        **Authored scene state** contains the grid, cell size, solver profile, walls, devices, descriptive metadata, and challenge definition. It does not contain transient sensor activity, fluid, counters, current tick, pause state, or challenge progress.

        **Runtime state** contains every value needed to continue one deterministic run: particles, grid fields and solver workspaces, solver settings, tick, lifecycle counters, device runtime state, emitter accumulation and phase, simulation clock, fixed-step driver state, and challenge progress.

        **Presentation state** contains camera, open menus, selection, hover, temporary messages, and derived audio or visual effects. It is not physics identity unless a feature explicitly makes it affect future commands.

        ## Operation table

        | Operation | Authored scene | Fluid and solver state | Tick and counters | Emitter runtime | Sensor/challenge runtime | Driver state | Undo history |
        | --- | --- | --- | --- | --- | --- | --- | --- |
        | Load authored scene | Replace | Clear | Reset to zero | Reset | Recompute/reset | Reset accumulator; caller controls initial pause | Replace with one initial checkpoint |
        | Retry authored scene | Restore initial definition | Clear | Reset to zero | Reset | Recompute/reset | Reset accumulator | Replace with one initial checkpoint |
        | Clear fluid | Preserve | Clear fluid fields/particles | Reset fluid accounting and tick | Reset accumulator/phase | Recompute/reset | Preserve pause, clear pending step | Preserve scene-edit history only when explicitly requested |
        | Clear scene | Clear | Clear | Reset | Clear | Clear/reset | Reset accumulator | Replace with one empty checkpoint |
        | Undo/redo edit | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Move within bounded history |
        | Failed load | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve |

        ## Checkpoint policy

        Scene-edit undo and redo store complete in-memory runtime checkpoints. Checkpoints are not authored scene files and have no cross-version persistence guarantee. History is bounded to 64 entries and an estimated 64 MiB. The current checkpoint is retained even when one state exceeds the estimate.

        Restoring a checkpoint must satisfy deterministic continuation: after restore, the same future commands and fixed steps produce the same versioned runtime digests.

        ## Scene-format compatibility

        Scene v2 retains the existing sensor activity token for parser compatibility, but writers emit `0` and loaders ignore the token. Sensor activity is derived from the newly created runtime. This avoids a format-number change while removing transient state from authored semantics.
        """
    ),
)

replace_once(
    "include/physics_sim/challenge_objective.hpp",
    "    [[nodiscard]] const ChallengeProgress& progress() const noexcept { return progress_; }\n",
    "    [[nodiscard]] const ChallengeProgress& progress() const noexcept { return progress_; }\n\n"
    "    void restore(const ChallengeProgress& progress) noexcept { progress_ = progress; }\n",
)

scene_controller = read("include/physics_sim/scene_controller.hpp")
for include_line in [
    "#include <physics_sim/challenge_objective.hpp>\n",
    "#include <physics_sim/fixed_timestep.hpp>\n",
    "#include <physics_sim/simulation_state.hpp>\n",
]:
    if include_line not in scene_controller:
        scene_controller = scene_controller.replace(
            "#include <physics_sim/math.hpp>\n",
            "#include <physics_sim/math.hpp>\n" + include_line,
            1,
        )
write("include/physics_sim/scene_controller.hpp", scene_controller)

insert_after_function(
    "include/physics_sim/scene_controller.hpp",
    "    explicit SceneController(WaterSimulation2D& simulation) noexcept",
    textwrap.dedent(
        """
            void attach_runtime_state(
                SimulationState* simulation_state,
                FixedStepDriver* step_driver,
                ChallengeEvaluator* challenge_evaluator) noexcept
            {
                simulation_state_ = simulation_state;
                step_driver_ = step_driver;
                challenge_evaluator_ = challenge_evaluator;
                sync_history();
            }

            [[nodiscard]] std::size_t history_size() const noexcept { return history_.size(); }
            [[nodiscard]] std::size_t history_estimated_bytes() const noexcept { return history_estimated_bytes_; }
            [[nodiscard]] static constexpr std::size_t history_entry_limit() noexcept { return max_history_entries_; }
            [[nodiscard]] static constexpr std::size_t history_byte_limit() noexcept { return max_history_bytes_; }
        """
    ).rstrip(),
)

replace_function(
    "include/physics_sim/scene_controller.hpp",
    "    void reset_scene() noexcept",
    textwrap.dedent(
        """
            void reset_scene() noexcept
            {
                if (simulation_ != nullptr)
                {
                    simulation_->clear_scene();
                }
                if (simulation_state_ != nullptr)
                {
                    simulation_state_->reset();
                }
                if (step_driver_ != nullptr)
                {
                    step_driver_->reset();
                }
                if (challenge_evaluator_ != nullptr)
                {
                    challenge_evaluator_->reset();
                }

                stroke_active_ = false;
                clear_selection();
                sync_history();
            }
        """
    ),
)

replace_function(
    "include/physics_sim/scene_controller.hpp",
    "    void sync_history() noexcept",
    textwrap.dedent(
        """
            void sync_history() noexcept
            {
                history_.clear();
                history_bytes_.clear();
                history_index_ = 0;
                history_estimated_bytes_ = 0;
                pending_history_before_edit_.reset();

                if (simulation_ != nullptr)
                {
                    history_.push_back(capture_history_entry());
                    history_bytes_.push_back(estimate_history_entry_bytes(history_.back()));
                    history_estimated_bytes_ = history_bytes_.back();
                }
            }
        """
    ),
)

private_marker = "private:\n    [[nodiscard]] bool is_wall_tool() const noexcept"
private_helpers = textwrap.dedent(
    """
    private:
        struct RuntimeHistoryEntry
        {
            WaterSimulation2D simulation;
            std::optional<SimulationState> simulation_state{};
            std::optional<FixedStepDriver> step_driver{};
            std::optional<ChallengeProgress> challenge_progress{};
        };

        [[nodiscard]] RuntimeHistoryEntry capture_history_entry() const
        {
            RuntimeHistoryEntry entry{*simulation_};
            if (simulation_state_ != nullptr)
            {
                entry.simulation_state = *simulation_state_;
            }
            if (step_driver_ != nullptr)
            {
                entry.step_driver = *step_driver_;
            }
            if (challenge_evaluator_ != nullptr)
            {
                entry.challenge_progress = challenge_evaluator_->progress();
            }
            return entry;
        }

        void begin_history_edit()
        {
            if (simulation_ != nullptr)
            {
                pending_history_before_edit_ = capture_history_entry();
            }
        }

        [[nodiscard]] static std::size_t estimate_history_entry_bytes(const RuntimeHistoryEntry& entry) noexcept
        {
            const auto& simulation = entry.simulation;
            const auto& grid = simulation.grid();
            std::size_t bytes = sizeof(RuntimeHistoryEntry);
            bytes += simulation.particles().capacity() * sizeof(FluidParticle);
            bytes += simulation.emitters().capacity() * sizeof(WaterEmitter);
            bytes += simulation.gates().capacity() * sizeof(WaterGate);
            bytes += simulation.sensors().capacity() * sizeof(WaterSensor);
            bytes += simulation.drains().capacity() * sizeof(WaterDrain);
            bytes += simulation.pumps().capacity() * sizeof(WaterPump);
            bytes += simulation.valves().capacity() * sizeof(WaterValve);
            for (const auto& sensor : simulation.sensors())
            {
                bytes += sensor.label.capacity();
            }
            bytes += grid.pressure_values().capacity() * sizeof(float);
            bytes += grid.divergence_values().capacity() * sizeof(float);
            bytes += grid.u_values().capacity() * sizeof(float);
            bytes += grid.v_values().capacity() * sizeof(float);
            bytes += grid.cell_count() * (sizeof(std::uint8_t) + sizeof(float) * 4U);
            bytes += grid.u_count() * sizeof(float) * 2U;
            bytes += grid.v_count() * sizeof(float) * 2U;
            return bytes;
        }

        void trim_history_to_budget() noexcept
        {
            while (history_.size() > 1
                && (history_.size() > max_history_entries_ || history_estimated_bytes_ > max_history_bytes_))
            {
                history_estimated_bytes_ -= history_bytes_.front();
                history_.erase(history_.begin());
                history_bytes_.erase(history_bytes_.begin());
                if (history_index_ > 0)
                {
                    --history_index_;
                }
            }
        }

        [[nodiscard]] bool is_wall_tool() const noexcept
    """
)
scene_controller = read("include/physics_sim/scene_controller.hpp")
if private_marker not in scene_controller:
    raise RuntimeError("scene_controller.hpp: private insertion marker missing")
write("include/physics_sim/scene_controller.hpp", scene_controller.replace(private_marker, private_helpers, 1))

replace_function(
    "include/physics_sim/scene_controller.hpp",
    "    void record_history_snapshot() noexcept",
    textwrap.dedent(
        """
            void record_history_snapshot() noexcept
            {
                if (simulation_ == nullptr)
                {
                    pending_history_before_edit_.reset();
                    return;
                }

                if (history_.empty())
                {
                    const RuntimeHistoryEntry initial = pending_history_before_edit_.has_value()
                        ? *pending_history_before_edit_
                        : capture_history_entry();
                    history_.push_back(initial);
                    history_bytes_.push_back(estimate_history_entry_bytes(history_.back()));
                    history_estimated_bytes_ = history_bytes_.back();
                    history_index_ = 0;
                }
                else if (pending_history_before_edit_.has_value())
                {
                    history_estimated_bytes_ -= history_bytes_[history_index_];
                    history_[history_index_] = *pending_history_before_edit_;
                    history_bytes_[history_index_] = estimate_history_entry_bytes(history_[history_index_]);
                    history_estimated_bytes_ += history_bytes_[history_index_];
                }

                if (history_index_ + 1 < history_.size())
                {
                    for (std::size_t index = history_index_ + 1; index < history_bytes_.size(); ++index)
                    {
                        history_estimated_bytes_ -= history_bytes_[index];
                    }
                    history_.erase(history_.begin() + static_cast<std::ptrdiff_t>(history_index_ + 1), history_.end());
                    history_bytes_.erase(history_bytes_.begin() + static_cast<std::ptrdiff_t>(history_index_ + 1), history_bytes_.end());
                }

                history_.push_back(capture_history_entry());
                history_bytes_.push_back(estimate_history_entry_bytes(history_.back()));
                history_estimated_bytes_ += history_bytes_.back();
                history_index_ = history_.size() - 1;
                pending_history_before_edit_.reset();
                trim_history_to_budget();
            }
        """
    ),
)

replace_function(
    "include/physics_sim/scene_controller.hpp",
    "    void restore_history_entry() noexcept",
    textwrap.dedent(
        """
            void restore_history_entry() noexcept
            {
                if (simulation_ == nullptr || history_.empty() || history_index_ >= history_.size())
                {
                    return;
                }

                pending_history_before_edit_.reset();
                const RuntimeHistoryEntry& entry = history_[history_index_];
                *simulation_ = entry.simulation;
                if (simulation_state_ != nullptr && entry.simulation_state.has_value())
                {
                    *simulation_state_ = *entry.simulation_state;
                }
                if (step_driver_ != nullptr && entry.step_driver.has_value())
                {
                    *step_driver_ = *entry.step_driver;
                }
                if (challenge_evaluator_ != nullptr && entry.challenge_progress.has_value())
                {
                    challenge_evaluator_->restore(*entry.challenge_progress);
                }

                clear_selection();
            }
        """
    ),
)

replace_once(
    "include/physics_sim/scene_controller.hpp",
    "    std::vector<SceneDocument> history_{};\n    std::size_t history_index_ = 0;\n",
    "    SimulationState* simulation_state_ = nullptr;\n"
    "    FixedStepDriver* step_driver_ = nullptr;\n"
    "    ChallengeEvaluator* challenge_evaluator_ = nullptr;\n"
    "    std::vector<RuntimeHistoryEntry> history_{};\n"
    "    std::vector<std::size_t> history_bytes_{};\n"
    "    std::optional<RuntimeHistoryEntry> pending_history_before_edit_{};\n"
    "    std::size_t history_index_ = 0;\n"
    "    std::size_t history_estimated_bytes_ = 0;\n"
    "    static constexpr std::size_t max_history_entries_ = 64;\n"
    "    static constexpr std::size_t max_history_bytes_ = 64U * 1024U * 1024U;\n",
)

# Capture the complete current runtime immediately before every edit. This
# updates the undo baseline after simulation time has advanced.
for signature in [
    "    [[nodiscard]] bool set_selected_fixture_direction(Vec2 direction) noexcept",
    "    [[nodiscard]] bool rotate_selected_fixture(float radians) noexcept",
    "    [[nodiscard]] bool move_selected_fixture(Vec2 delta) noexcept",
    "    [[nodiscard]] bool set_selected_fixture_speed(float speed) noexcept",
    "    [[nodiscard]] bool set_selected_fixture_emission_rate(float rate) noexcept",
    "    [[nodiscard]] bool set_selected_fixture_enabled(bool enabled) noexcept",
    "    void begin_stroke(Vec2 world_position) noexcept",
    "    bool place_fixture(Vec2 world_position) noexcept",
    "    bool place_gate(Vec2 world_position) noexcept",
    "    bool place_sensor(Vec2 world_position) noexcept",
    "    bool place_drain(Vec2 world_position) noexcept",
    "    bool place_pump(Vec2 world_position) noexcept",
    "    bool place_valve(Vec2 world_position) noexcept",
    "    [[nodiscard]] bool delete_selected_fixture() noexcept",
    "    [[nodiscard]] bool delete_selected_gate() noexcept",
    "    [[nodiscard]] bool delete_selected_sensor() noexcept",
    "    [[nodiscard]] bool delete_selected_valve() noexcept",
    "    [[nodiscard]] bool delete_selected_drain() noexcept",
    "    [[nodiscard]] bool delete_selected_pump() noexcept",
    "    [[nodiscard]] bool toggle_selected_drain_enabled() noexcept",
    "    [[nodiscard]] bool toggle_selected_pump_enabled() noexcept",
    "    [[nodiscard]] bool rotate_selected_pump(float radians) noexcept",
    "    [[nodiscard]] bool set_selected_pump_direction(Vec2 direction) noexcept",
    "    [[nodiscard]] bool set_selected_pump_strength(float strength) noexcept",
    "    [[nodiscard]] bool adjust_selected_pump_strength(float delta) noexcept",
    "    [[nodiscard]] bool toggle_selected_gate_open() noexcept",
    "    [[nodiscard]] bool toggle_selected_sensor_enabled() noexcept",
    "    [[nodiscard]] bool toggle_selected_valve_open() noexcept",
]:
    text = read("include/physics_sim/scene_controller.hpp")
    function_start, _ = function_range(text, signature)
    brace = text.find("{", function_start + len(signature))
    insertion = "\n        begin_history_edit();"
    if text.startswith(insertion, brace + 1):
        continue
    write("include/physics_sim/scene_controller.hpp", text[: brace + 1] + insertion + text[brace + 1 :])

replace_once(
    "src/app/application.cpp",
    "    physics_sim::ChallengeEvaluator challengeEvaluator;\n    const auto sync_solver_profile_from_simulation = [&]()\n",
    "    physics_sim::ChallengeEvaluator challengeEvaluator;\n"
    "    controller.attach_runtime_state(&simulationState, &stepDriver, &challengeEvaluator);\n"
    "    const auto sync_solver_profile_from_simulation = [&]()\n",
)

# Scene v2 keeps the active token for compatibility but treats it as reserved/derived.
replace_once(
    "include/physics_sim/scene_document.hpp",
    "    bool active = false;\n    bool objective = false;\n",
    "    bool active = false; // Reserved runtime field; authored loads always recompute it.\n    bool objective = false;\n",
)
replace_all(
    "include/physics_sim/scene_document.hpp",
    "            sensor.active,\n            sensor.objective,\n",
    "            false,\n            sensor.objective,\n",
    minimum=2,
)
replace_once(
    "include/physics_sim/scene_document.hpp",
    "            sensor.active = active_value != 0;\n            sensor.objective = objective_value != 0;\n",
    "            static_cast<void>(active_value); // Scene v2 token retained for compatibility.\n"
    "            sensor.active = false;\n"
    "            sensor.objective = objective_value != 0;\n",
)
replace_once(
    "include/physics_sim/scene_document.hpp",
    "             << (sensor.active ? 1 : 0) << ' ' \n",
    "             << 0 << ' ' \n",
)

# ---------------------------------------------------------------------------
# Complete versioned runtime digest while retaining legacy replay identity (#13)
# ---------------------------------------------------------------------------

water = read("include/physics_sim/water_simulation.hpp")
water = water.replace(
    "    float speed = 6.0f;\n",
    "    float speed = 6.0f; // Cells per second; converted to world units at emission.\n",
    1,
)
water = water.replace(
    "    float strength = 8.0f;\n",
    "    float strength = 8.0f; // World-velocity units per second inside the pump region.\n",
    1,
)
water = water.replace(
    "    float gravity_acceleration = 9.8f;\n",
    "    float gravity_acceleration = 9.8f; // Cells per second squared; converted at the grid boundary.\n",
    1,
)
write("include/physics_sim/water_simulation.hpp", water)

# Reset tick as part of resize/new-runtime semantics.
water = read("include/physics_sim/water_simulation.hpp")
start, end = function_range(water, "    void resize(size_type width, size_type height, float cell_size = 1.0f)")
resize_text = water[start:end]
if "simulation_tick_ = 0;" not in resize_text:
    if resize_text.count("        metrics_ = {};\n") != 1:
        raise RuntimeError("resize: metrics marker missing")
    resize_text = resize_text.replace("        metrics_ = {};\n", "        metrics_ = {};\n        simulation_tick_ = 0;\n", 1)
    write("include/physics_sim/water_simulation.hpp", water[:start] + resize_text + water[end:])

runtime_digest_method = textwrap.dedent(
    """
        static constexpr std::uint32_t RuntimeStateDigestVersion = 2;

        [[nodiscard]] std::string runtime_state_digest() const
        {
            std::uint64_t hash = 14695981039346656037ULL;
            const auto mix = [&hash](std::uint64_t value)
            {
                for (int byte = 0; byte < 8; ++byte)
                {
                    hash ^= static_cast<std::uint8_t>((value >> (byte * 8)) & 0xffU);
                    hash *= 1099511628211ULL;
                }
            };
            const auto mix_float = [&mix](float value) { mix(std::bit_cast<std::uint32_t>(value)); };
            const auto mix_double = [&mix](double value) { mix(std::bit_cast<std::uint64_t>(value)); };
            const auto mix_string = [&mix](const std::string& value)
            {
                mix(value.size());
                for (const unsigned char character : value)
                {
                    mix(character);
                }
            };
            const auto mix_float_vector = [&mix, &mix_float](const std::vector<float>& values)
            {
                mix(values.size());
                for (const float value : values) mix_float(value);
            };
            const auto mix_double_vector = [&mix, &mix_double](const std::vector<double>& values)
            {
                mix(values.size());
                for (const double value : values) mix_double(value);
            };

            mix(0x5053494D52554E32ULL); // "PSIMRUN2"
            mix(RuntimeStateDigestVersion);
            mix(simulation_tick_);
            mix(grid_.width());
            mix(grid_.height());
            mix_float(grid_.cell_size());

            const FluidSolverSettings& settings = solver_settings_;
            mix(static_cast<std::uint64_t>(settings.profile));
            mix(static_cast<std::uint64_t>(settings.tier));
            mix_float(settings.gravity_acceleration);
            mix(static_cast<std::uint64_t>(settings.pressure_max_iterations));
            mix_float(settings.pressure_relative_residual_target);
            mix_float(settings.rest_density);
            mix(settings.particles_per_full_cell);
            mix_float(settings.density_kernel_radius_cells);
            mix(static_cast<std::uint64_t>(settings.density_correction_iterations));
            mix_float(settings.max_density_correction_fraction);
            mix_float(settings.density_correction_velocity_ratio);
            mix_float(settings.flip_blend);
            mix_float(settings.velocity_retention);
            mix_float(settings.apic_affine_ratio);
            mix(static_cast<std::uint64_t>(settings.wall_material));
            mix_float(settings.wall_tangential_velocity_retention);
            mix_float(settings.viscosity_coefficient);
            mix_float(settings.surface_tension_coefficient);
            mix_float(settings.max_surface_velocity_delta_fraction);
            mix(settings.resampling.enabled);
            mix(settings.resampling.min_particles_per_fluid_cell);
            mix(settings.resampling.target_particles_per_fluid_cell);
            mix(settings.resampling.max_particles_per_fluid_cell);
            mix(settings.resampling.max_resampling_operations_per_step);
            mix_float(settings.resampling.split_offset_fraction);
            mix_float(settings.resampling.min_split_particle_mass);
            mix(settings.regularization.enabled);
            mix(settings.regularization.minimum_particle_count);
            mix(settings.regularization.interval_ticks);
            mix(static_cast<std::uint64_t>(settings.regularization.iterations));
            mix_float(settings.regularization.support_radius_cells);
            mix_float(settings.regularization.strength);
            mix_float(settings.regularization.max_displacement_fraction);
            mix(settings.density_metrics_interval_ticks);

            mix_float_vector(grid_.pressure_values());
            mix_float_vector(grid_.divergence_values());
            mix_float_vector(grid_.u_values());
            mix_float_vector(grid_.v_values());
            for (std::size_t y = 0; y < grid_.height(); ++y)
                for (std::size_t x = 0; x < grid_.width(); ++x) mix(grid_.solid(x, y));

            mix(particles_.size());
            for (const FluidParticle& particle : particles_)
            {
                mix_float(particle.position.x); mix_float(particle.position.y);
                mix_float(particle.velocity.x); mix_float(particle.velocity.y);
                mix_float(particle.mass); mix_float(particle.volume); mix_float(particle.density);
                mix_float(particle.affine_velocity.m00); mix_float(particle.affine_velocity.m01);
                mix_float(particle.affine_velocity.m10); mix_float(particle.affine_velocity.m11);
                mix(particle.neighbor_count);
            }

            mix(emitters_.size());
            for (const WaterEmitter& emitter : emitters_)
            {
                mix(static_cast<std::uint64_t>(emitter.kind));
                mix_float(emitter.position.x); mix_float(emitter.position.y);
                mix_float(emitter.direction.x); mix_float(emitter.direction.y);
                mix_float(emitter.speed); mix_float(emitter.emission_rate); mix(emitter.enabled);
                mix_double(emitter.emission_accumulator); mix(emitter.emitted_particles);
            }
            mix(gates_.size());
            for (const WaterGate& gate : gates_) { mix(gate.x); mix(gate.y); mix(gate.open); }
            mix(sensors_.size());
            for (const WaterSensor& sensor : sensors_)
            {
                mix(sensor.x); mix(sensor.y); mix(sensor.width); mix(sensor.height);
                mix(sensor.enabled); mix(sensor.active); mix(sensor.objective); mix_string(sensor.label);
            }
            mix(drains_.size());
            for (const WaterDrain& drain : drains_) { mix(drain.x); mix(drain.y); mix(drain.width); mix(drain.height); mix(drain.enabled); }
            mix(pumps_.size());
            for (const WaterPump& pump : pumps_)
            {
                mix(pump.x); mix(pump.y); mix(pump.width); mix(pump.height); mix(pump.enabled);
                mix_float(pump.direction.x); mix_float(pump.direction.y); mix_float(pump.strength);
            }
            mix(valves_.size());
            for (const WaterValve& valve : valves_) { mix(valve.x); mix(valve.y); mix(valve.open); }

            mix(fluid_cells_.size());
            for (const std::uint8_t value : fluid_cells_) mix(value);
            mix(cell_states_.size());
            for (const FluidCellState value : cell_states_) mix(static_cast<std::uint64_t>(value));
            mix_float_vector(cell_volume_fractions_);
            mix_float_vector(cell_densities_);
            mix_float_vector(u_weights_);
            mix_float_vector(v_weights_);
            mix_float_vector(u_previous_);
            mix_float_vector(v_previous_);
            mix_float_vector(pressure_next_);
            mix_double_vector(pressure_workspace_);
            mix_double_vector(pressure_rhs_workspace_);
            mix_double_vector(pressure_residual_workspace_);
            mix_double_vector(pressure_direction_workspace_);
            mix_double_vector(pressure_preconditioned_workspace_);
            mix_double_vector(pressure_applied_workspace_);
            mix_double_vector(pressure_inverse_diagonal_workspace_);
            mix(pressure_cell_to_system_workspace_.size());
            for (const int value : pressure_cell_to_system_workspace_) mix(static_cast<std::uint64_t>(value));
            mix(pressure_system_to_cell_workspace_.size());
            for (const size_type value : pressure_system_to_cell_workspace_) mix(value);

            mix(total_emitted_); mix_double(total_emitted_mass_);
            mix(total_removed_); mix_double(total_removed_mass_);
            mix(total_outflow_); mix_double(total_outflow_mass_);

            const WaterSimulationMetrics& metrics = metrics_;
            mix(metrics.total_emitted); mix_double(metrics.total_emitted_mass);
            mix(metrics.total_removed); mix_double(metrics.total_removed_mass);
            mix(metrics.total_outflow); mix_double(metrics.total_outflow_mass);
            mix(metrics.active_particles); mix(metrics.active_cells); mix(metrics.visible_fluid_cells);
            mix(metrics.pressure_active_cells); mix_double(metrics.active_cell_overreach_ratio);
            mix(metrics.active_sensors); mix(metrics.objective_sensors); mix(metrics.objective_completed);
            mix_double(metrics.average_divergence_after_projection); mix_double(metrics.max_divergence_after_projection);
            mix_double(metrics.min_density); mix_double(metrics.max_density); mix_double(metrics.average_density);
            mix_double(metrics.average_density_error); mix_double(metrics.max_density_error);
            mix_double(metrics.average_neighbor_count); mix(metrics.max_neighbor_count); mix_double(metrics.kinetic_energy);
            const PressureSolveResult& pressure = metrics.pressure_solve;
            mix(static_cast<std::uint64_t>(pressure.iterations)); mix(static_cast<std::uint64_t>(pressure.max_iterations));
            mix_float(pressure.target_relative_residual); mix_float(pressure.target_absolute_residual);
            mix_float(pressure.initial_residual); mix_float(pressure.final_residual);
            mix_float(pressure.absolute_residual); mix_float(pressure.relative_residual);
            mix_float(pressure.rhs_l2); mix_float(pressure.solution_l2);
            mix_float(pressure.pressure_dt); mix_float(pressure.rest_density); mix(pressure.converged);
            mix(pressure.visible_cells); mix(pressure.active_cells); mix(pressure.pressure_active_cells);
            mix_double(pressure.active_cell_overreach_ratio);

            std::ostringstream stream;
            stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash;
            return stream.str();
        }
    """
).rstrip()
water = read("include/physics_sim/water_simulation.hpp")
if "runtime_state_digest() const" not in water:
    _, legacy_end = function_range(water, "    [[nodiscard]] std::string state_digest() const")
    write("include/physics_sim/water_simulation.hpp", water[:legacy_end] + "\n\n" + runtime_digest_method + water[legacy_end:])

simulation_hpp = read("include/physics_sim/simulation.hpp")
simulation_hpp = simulation_hpp.replace("    std::uint64_t deterministic_seed = 0;\n", "")
simulation_hpp = simulation_hpp.replace(
    "    [[nodiscard]] std::string state_digest() const;\n",
    "    [[nodiscard]] std::string state_digest() const;\n"
    "    [[nodiscard]] std::string runtime_state_digest() const;\n"
    "    static constexpr std::uint32_t RuntimeStateDigestVersion = 2;\n",
    1,
)
write("include/physics_sim/simulation.hpp", simulation_hpp)

simulation_cpp = read("src/core/simulation.cpp")
if "Simulation::runtime_state_digest() const" not in simulation_cpp:
    _, end = function_range(simulation_cpp, "std::string Simulation::state_digest() const")
    addition = textwrap.dedent(
        """

        std::string Simulation::runtime_state_digest() const
        {
            std::uint64_t hash = 14695981039346656037ULL;
            const auto mix = [&hash](std::uint64_t value)
            {
                for (int byte = 0; byte < 8; ++byte)
                {
                    hash ^= static_cast<std::uint8_t>((value >> (byte * 8)) & 0xffU);
                    hash *= 1099511628211ULL;
                }
            };
            mix(0x5053494D46414332ULL); // "PSIMFAC2"
            mix(RuntimeStateDigestVersion);
            for (const unsigned char character : impl_->water.runtime_state_digest()) mix(character);
            mix(std::bit_cast<std::uint64_t>(impl_->config.fixed_timestep));
            mix(impl_->paused);
            mix(impl_->single_step_pending);
            std::ostringstream stream;
            stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash;
            return stream.str();
        }
        """
    )
    write("src/core/simulation.cpp", simulation_cpp[:end] + addition + simulation_cpp[end:])

# ---------------------------------------------------------------------------
# Explicit unit contract and scale checks (#14)
# ---------------------------------------------------------------------------

write(
    "docs/units.md",
    textwrap.dedent(
        """\
        # Simulation Units

        The current solver uses a **cell-relative configuration boundary** and world-coordinate storage.

        - Grid positions and particle positions are stored in world units.
        - `cell_size` is world units per grid cell.
        - Fixed timestep is seconds.
        - Configured gravity is cells per second squared and is converted to world units per second squared by multiplying by `cell_size` once at the solver boundary.
        - Emitter speed is cells per second and is converted to world units per second by multiplying by `cell_size` once when a particle is emitted.
        - Emission rate is particles per second.
        - Particle velocity and MAC-grid velocity are stored in world units per second.
        - Pump strength is currently world-velocity units per second inside the pump region; it is not cell-scaled. This implementation-specific unit must be reconsidered before claiming scale-independent device behavior.
        - Density, pressure, viscosity, and surface-tension values remain solver units described by `docs/physics-accuracy-contract.md`; they are not SI-calibrated claims.

        Equivalent scenes with the same grid dimensions and cell-relative configuration should produce equivalent normalized motion when world positions are divided by `cell_size`. New code must not add a second hidden cell-size multiplier.
        """
    ),
)

accuracy = read("docs/physics-accuracy-contract.md")
accuracy = accuracy.replace(
    "- Velocity `v_p` and grid velocity `u`: world units per second.\n- Gravity: current implementation uses `9.8` world units per second squared in the positive Y direction.",
    "- Velocity `v_p` and grid velocity `u`: stored in world units per second.\n"
    "- Configured gravity: cells per second squared. The solver converts it once to world units per second squared by multiplying by `dx`. The default is `9.8` cells per second squared in the positive Y direction.\n"
    "- Configured emitter speed: cells per second, converted once to world units per second when particles are emitted.",
)
accuracy = accuracy.replace(
    "- `dt`: fixed simulation step, currently `1 / 120` seconds in the app and quality harness.",
    "- `dt`: fixed simulation step, currently `1 / 120` seconds in the app and quality harness.\n"
    "- The authoritative unit boundary and device exceptions are recorded in [units.md](units.md).",
)
write("docs/physics-accuracy-contract.md", accuracy)

known = read("docs/physics-known-limitations.md")
unit_bullet = "- Configuration is cell-relative for gravity and emitter speed, while particles and grid velocities are stored in world units; pump strength remains an implementation-specific world-velocity acceleration. See [units.md](units.md).\n"
if unit_bullet not in known:
    known = known.replace("## Scope Constraints\n\n", "## Scope Constraints\n\n" + unit_bullet, 1)
write("docs/physics-known-limitations.md", known)

replace_once(
    "src/lab/lab_module.cpp",
    '        parameters_changed |= ImGui::SliderFloat("Gravity", &gravity, 0.0f, 30.0f, "%.2f");\n',
    '        parameters_changed |= ImGui::SliderFloat("Gravity (cells/s^2)", &gravity, 0.0f, 30.0f, "%.2f");\n',
)

# ---------------------------------------------------------------------------
# Tests for checkpoints, load reset, digests, and scale semantics
# ---------------------------------------------------------------------------

replace_once(
    "tests/scene_persistence_tests.cpp",
    '        REQUIRE(device_restored.sensors().front().active, "load_scene lost the sensor active state");\n',
    '        REQUIRE(!device_restored.sensors().front().active, "load_scene trusted transient authored sensor activity");\n',
)

append_before_last(
    "tests/scene_persistence_tests.cpp",
    "    fs::remove(temp_path);",
    textwrap.dedent(
        """
            {
                physics_sim::WaterSimulation2D active{8, 8, 1.0f};
                active.add_emitter(physics_sim::WaterEmitter{
                    physics_sim::WaterEmitterKind::Directional,
                    {4.0f, 1.0f},
                    {0.0f, 1.0f},
                    2.0f,
                    240.0f,
                    true});
                for (int step = 0; step < 20; ++step)
                {
                    active.step(1.0 / 120.0);
                }
                REQUIRE(active.simulation_tick() == 20, "active scene did not advance before reload test");
                REQUIRE(!active.particles().empty(), "active scene emitted no particles before reload test");

                auto clean_document = snapshot;
                clean_document.sensors.push_back({1, 1, 2, 2, true, true, true, "Derived goal"});
                physics_sim::apply_scene(clean_document, active);
                REQUIRE(active.simulation_tick() == 0, "apply_scene retained the previous runtime tick");
                REQUIRE(active.particles().empty(), "apply_scene retained previous fluid");
                REQUIRE(active.metrics().total_emitted == 0, "apply_scene retained lifecycle counters");
                REQUIRE(active.sensors().size() == 1 && !active.sensors().front().active,
                    "apply_scene retained transient sensor activity");

                const std::string before_failed_load = active.runtime_state_digest();
                const fs::path missing_path = fs::temp_directory_path() / "physics-sim-atomic-missing.pscene";
                fs::remove(missing_path);
                REQUIRE(!physics_sim::load_scene(missing_path, active), "missing scene load unexpectedly succeeded");
                REQUIRE(active.runtime_state_digest() == before_failed_load, "failed scene load mutated the valid runtime");
            }
        """
    ),
)

append_before_last(
    "tests/editor_controls_tests.cpp",
    "    return 0;",
    textwrap.dedent(
        """
            {
                physics_sim::WaterSimulation2D simulation{20, 20, 1.0f};
                simulation.add_emitter(physics_sim::WaterEmitter{
                    physics_sim::WaterEmitterKind::Directional,
                    {10.0f, 2.0f},
                    {0.0f, 1.0f},
                    1.0f,
                    240.0f,
                    true});
                physics_sim::FixedStepDriver driver;
                physics_sim::SimulationState state;
                physics_sim::ChallengeEvaluator challenge;
                physics_sim::SceneController controller{simulation};
                controller.attach_runtime_state(&state, &driver, &challenge);
                for (int step = 0; step < 10; ++step)
                {
                    simulation.step(1.0 / 120.0);
                    state.advance(physics_sim::SimulationState::duration{1.0 / 120.0});
                }
                const std::string pre_edit_digest = simulation.runtime_state_digest();
                const std::uint64_t pre_edit_tick = state.tick_count;
                controller.set_tool(physics_sim::SceneTool::PaintWall);
                controller.begin_stroke({2.0f, 10.0f});
                controller.end_stroke({4.0f, 10.0f});
                REQUIRE(controller.undo_scene_edit(), "first runtime checkpoint undo failed");
                REQUIRE(simulation.runtime_state_digest() == pre_edit_digest,
                    "first undo returned to controller construction instead of the pre-edit runtime");
                REQUIRE(state.tick_count == pre_edit_tick, "first undo did not restore the pre-edit simulation clock");
            }

            {
                physics_sim::WaterSimulation2D simulation{20, 20, 1.0f};
                auto settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Balanced);
                settings.gravity_acceleration = 0.0f;
                simulation.set_solver_settings(settings);
                simulation.add_emitter(physics_sim::WaterEmitter{
                    physics_sim::WaterEmitterKind::Directional,
                    {10.0f, 2.0f},
                    {0.0f, 1.0f},
                    1.0f,
                    240.0f,
                    true});

                physics_sim::FixedStepDriver driver;
                physics_sim::SimulationState state;
                physics_sim::ChallengeEvaluator challenge;
                physics_sim::SceneController controller{simulation};
                controller.attach_runtime_state(&state, &driver, &challenge);

                for (int step = 0; step < 10; ++step)
                {
                    simulation.step(1.0 / 120.0);
                    state.advance(physics_sim::SimulationState::duration{1.0 / 120.0});
                }
                physics_sim::ChallengeProgress first_progress;
                first_progress.status = physics_sim::ChallengeStatus::Running;
                first_progress.held_ticks = 7;
                challenge.restore(first_progress);
                driver.set_paused(true);
                controller.set_tool(physics_sim::SceneTool::PaintWall);
                controller.begin_stroke({2.0f, 10.0f});
                controller.end_stroke({4.0f, 10.0f});
                const std::string first_digest = simulation.runtime_state_digest();
                const std::uint64_t first_tick = state.tick_count;
                const std::size_t first_particles = simulation.particles().size();

                driver.set_paused(false);
                for (int step = 0; step < 5; ++step)
                {
                    simulation.step(1.0 / 120.0);
                    state.advance(physics_sim::SimulationState::duration{1.0 / 120.0});
                }
                physics_sim::ChallengeProgress second_progress = first_progress;
                second_progress.held_ticks = 11;
                challenge.restore(second_progress);
                controller.begin_stroke({6.0f, 10.0f});
                controller.end_stroke({8.0f, 10.0f});
                const std::string second_digest = simulation.runtime_state_digest();

                REQUIRE(controller.undo_scene_edit(), "runtime checkpoint undo failed");
                REQUIRE(simulation.runtime_state_digest() == first_digest, "undo did not restore the full simulation checkpoint");
                REQUIRE(state.tick_count == first_tick, "undo did not restore the simulation clock");
                REQUIRE(simulation.particles().size() == first_particles, "undo did not restore moving water");
                REQUIRE(driver.paused(), "undo did not restore fixed-step pause state");
                REQUIRE(challenge.progress().held_ticks == 7, "undo did not restore challenge progress");

                REQUIRE(controller.redo_scene_edit(), "runtime checkpoint redo failed");
                REQUIRE(simulation.runtime_state_digest() == second_digest, "redo did not restore the full simulation checkpoint");
                REQUIRE(!driver.paused(), "redo did not restore running driver state");
                REQUIRE(challenge.progress().held_ticks == 11, "redo did not restore challenge progress");

                for (int edit = 0; edit < 80; ++edit)
                {
                    const float x = static_cast<float>(1 + edit % 18);
                    const float y = static_cast<float>(1 + (edit / 18) % 18);
                    controller.begin_stroke({x, y});
                    controller.end_stroke({x, y});
                }
                REQUIRE(controller.history_size() <= physics_sim::SceneController::history_entry_limit(),
                    "runtime checkpoint history exceeded its entry limit");
                REQUIRE(controller.history_estimated_bytes() <= physics_sim::SceneController::history_byte_limit()
                        || controller.history_size() == 1,
                    "runtime checkpoint history exceeded its byte limit");
            }
        """
    ),
)

append_before_last(
    "tests/water_simulation_tests.cpp",
    "    return 0;",
    textwrap.dedent(
        """
            {
                physics_sim::WaterSimulation2D simulation{8, 8, 1.0f};
                simulation.add_particle({{4.0f, 2.0f}, {0.0f, 0.0f}});
                simulation.step(1.0 / 120.0);
                REQUIRE(simulation.simulation_tick() == 1, "simulation did not advance before resize reset test");
                simulation.resize(10, 10, 2.0f);
                REQUIRE(simulation.simulation_tick() == 0, "resize retained the previous runtime tick");
                REQUIRE(simulation.metrics().total_emitted == 0, "resize retained lifecycle metrics");
            }

            {
                physics_sim::WaterSimulation2D first{8, 8, 1.0f};
                physics_sim::WaterEmitter emitter;
                emitter.position = {4.0f, 2.0f};
                emitter.emission_rate = 1.0f;
                emitter.emission_accumulator = 0.25;
                emitter.emitted_particles = 7;
                first.add_emitter(emitter);
                physics_sim::WaterSimulation2D second = first;
                second.emitters().front().emission_accumulator = 0.75;
                REQUIRE(first.state_digest() == second.state_digest(), "legacy digest unexpectedly changed compatibility behavior");
                REQUIRE(first.runtime_state_digest() != second.runtime_state_digest(),
                    "runtime digest omitted emitter fractional accumulation");
                second = first;
                second.emitters().front().emitted_particles = 8;
                REQUIRE(first.runtime_state_digest() != second.runtime_state_digest(),
                    "runtime digest omitted emitter deterministic phase");
                second = first;
                auto changed_settings = second.solver_settings();
                changed_settings.gravity_acceleration += 1.0f;
                second.set_solver_settings(changed_settings);
                REQUIRE(first.runtime_state_digest() != second.runtime_state_digest(),
                    "runtime digest omitted solver configuration");
            }

            {
                physics_sim::WaterSimulation2D unit_grid{8, 8, 1.0f};
                physics_sim::WaterSimulation2D scaled_grid{8, 8, 2.0f};
                unit_grid.add_particle({{4.0f, 2.0f}, {0.0f, 0.0f}});
                scaled_grid.add_particle({{8.0f, 4.0f}, {0.0f, 0.0f}});
                unit_grid.step(1.0 / 120.0);
                scaled_grid.step(1.0 / 120.0);
                REQUIRE(!unit_grid.particles().empty() && !scaled_grid.particles().empty(), "scale test lost its particles");
                const auto& unit_particle = unit_grid.particles().front();
                const auto& scaled_particle = scaled_grid.particles().front();
                REQUIRE(std::fabs(unit_particle.position.x - scaled_particle.position.x / 2.0f) < 0.001f,
                    "cell-relative units changed normalized x motion");
                REQUIRE(std::fabs(unit_particle.position.y - scaled_particle.position.y / 2.0f) < 0.001f,
                    "cell-relative units changed normalized y motion");
                REQUIRE(std::fabs(unit_particle.velocity.y - scaled_particle.velocity.y / 2.0f) < 0.001f,
                    "cell-relative gravity changed normalized velocity");
            }
        """
    ),
)

append_before_last(
    "tests/simulation_api_tests.cpp",
    "    std::puts(\"simulation API tests passed\");",
    textwrap.dedent(
        """
            {
                physics_sim::Simulation running{config};
                physics_sim::Simulation paused{config};
                require(running.state_digest() == paused.state_digest(), "legacy facade digests differed before control-state change");
                paused.apply(physics_sim::SetPausedCommand{true});
                require(running.runtime_state_digest() != paused.runtime_state_digest(),
                    "runtime facade digest omitted pause state");
            }
        """
    ),
)

# ---------------------------------------------------------------------------
# Version source, validated mirrors, and manifest-driven packaging (#26)
# ---------------------------------------------------------------------------

write("VERSION.txt", "0.2.0-alpha.3\n")
write(
    "include/physics_sim/version.hpp.in",
    textwrap.dedent(
        """\
        #pragma once

        #include <string_view>

        namespace physics_sim
        {
        inline constexpr std::string_view project_version = "@PHYSICS_SIM_VERSION@";
        inline constexpr std::string_view project_version_core = "@PHYSICS_SIM_VERSION_CORE@";
        } // namespace physics_sim
        """
    ),
)
write(
    "tests/version_tests.cpp",
    textwrap.dedent(
        """\
        #include <physics_sim/version.hpp>

        #include <cstdio>
        #include <cstdlib>

        int main()
        {
            if (physics_sim::project_version != "0.2.0-alpha.3")
            {
                std::fprintf(stderr, "unexpected configured version: %.*s\\n",
                    static_cast<int>(physics_sim::project_version.size()), physics_sim::project_version.data());
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        """
    ),
)
write(
    "regression/replays/package.manifest",
    textwrap.dedent(
        """\
        physics-sim-replay-package\t1
        replay\tchallenge_fill.replay
        replay\tchallenge_gate.replay
        replay\tchallenge_pump_valve.replay
        """
    ),
)
write(
    "docs/release-notes-0.2.0-alpha.3.md",
    textwrap.dedent(
        """\
        # Physics Sim 0.2.0-alpha.3

        This prerelease begins the post-recovery correctness workstream.

        ## Highlights

        - GitHub Issues now hold active work while the recovery Markdown files remain completed evidence ledgers.
        - Scene loading resets runtime identity and ignores transient authored sensor activity.
        - Scene-edit undo and redo use bounded complete runtime checkpoints, including water, timing, emitter phase, driver state, and challenge progress.
        - A versioned runtime digest covers hidden continuation state while the legacy digest remains available for replay compatibility.
        - Simulation units now have an explicit cell-relative configuration contract.
        - Pull requests run Standard verification, and changed C++ files receive broader style checks.
        - Packaging reads one authoritative version and derives scenes, thumbnails, and challenge replays from manifests.

        ## Compatibility

        - Scene format remains version 2. The sensor activity token is retained for parsing compatibility but is written as zero and recomputed at load.
        - Legacy replay digests remain unchanged. Runtime digest version 2 is additive and intended for checkpoints and future replay migration.
        - Runtime checkpoints are in-memory implementation details and are not cross-version save files.
        """
    ),
)

cmake = read("CMakeLists.txt")
cmake = cmake.replace(
    "project(physics_sim VERSION 0.1.0 LANGUAGES CXX)\n",
    "file(STRIP \"${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt\" PHYSICS_SIM_VERSION)\n"
    "string(REGEX MATCH \"^[0-9]+\\\\.[0-9]+\\\\.[0-9]+\" PHYSICS_SIM_VERSION_CORE \"${PHYSICS_SIM_VERSION}\")\n"
    "if(NOT PHYSICS_SIM_VERSION_CORE)\n"
    "    message(FATAL_ERROR \"VERSION.txt must begin with a semantic numeric version\")\n"
    "endif()\n\n"
    "project(physics_sim VERSION ${PHYSICS_SIM_VERSION_CORE} LANGUAGES CXX)\n\n"
    "configure_file(\n"
    "    ${CMAKE_CURRENT_SOURCE_DIR}/include/physics_sim/version.hpp.in\n"
    "    ${CMAKE_CURRENT_BINARY_DIR}/generated/physics_sim/version.hpp\n"
    "    @ONLY\n"
    ")\n",
    1,
)
cmake = cmake.replace(
    "target_include_directories(physics_sim_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)\n",
    "target_include_directories(physics_sim_core PUBLIC\n"
    "    ${CMAKE_CURRENT_SOURCE_DIR}/include\n"
    "    ${CMAKE_CURRENT_BINARY_DIR}/generated\n"
    ")\n",
    1,
)
version_test_block = textwrap.dedent(
    """
        add_executable(physics_sim_version_tests tests/version_tests.cpp)
        target_link_libraries(physics_sim_version_tests PRIVATE physics_sim_core)
        target_compile_features(physics_sim_version_tests PRIVATE cxx_std_20)
        if(MSVC)
            target_compile_options(physics_sim_version_tests PRIVATE /W4 /permissive- /utf-8)
            target_compile_definitions(physics_sim_version_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
        endif()
        add_test(NAME physics_sim_version_tests COMMAND physics_sim_version_tests)
    """
)
if "physics_sim_version_tests" not in cmake:
    marker = "    add_executable(physics_sim_simulation_api_tests\n"
    if marker not in cmake:
        raise RuntimeError("CMakeLists.txt: simulation API test marker missing")
    cmake = cmake.replace(marker, version_test_block + "\n" + marker, 1)
    label_marker = "        physics_sim_simulation_api_tests\n"
    cmake = cmake.replace(label_marker, "        physics_sim_version_tests\n" + label_marker, 1)
write("CMakeLists.txt", cmake)

vcpkg = json.loads(read("vcpkg.json"))
vcpkg["version-string"] = "0.2.0-alpha.3"
write("vcpkg.json", json.dumps(vcpkg, indent=2) + "\n")

write(
    "scripts/check-version.ps1",
    textwrap.dedent(
        """\
        $ErrorActionPreference = 'Stop'
        $repoRoot = Split-Path -Parent $PSScriptRoot
        $versionPath = Join-Path $repoRoot 'VERSION.txt'
        if (-not (Test-Path -LiteralPath $versionPath)) { throw '[version] VERSION.txt is missing.' }
        $version = (Get-Content -LiteralPath $versionPath -Raw).Trim()
        if ($version -notmatch '^\\d+\\.\\d+\\.\\d+(?:-[0-9A-Za-z.-]+)?$') { throw "[version] invalid version: $version" }

        $vcpkg = Get-Content -LiteralPath (Join-Path $repoRoot 'vcpkg.json') -Raw | ConvertFrom-Json
        if ($vcpkg.'version-string' -ne $version) { throw "[version] vcpkg version '$($vcpkg.'version-string')' differs from '$version'." }

        $notes = Join-Path $repoRoot "docs\\release-notes-$version.md"
        if (-not (Test-Path -LiteralPath $notes)) { throw "[version] missing release notes: $notes" }

        $readme = Get-Content -LiteralPath (Join-Path $repoRoot 'README.md') -Raw
        if ($readme -notmatch [regex]::Escape("Version ``$version``")) { throw '[version] README does not identify the authoritative version.' }

        if ($env:GITHUB_REF_TYPE -eq 'tag')
        {
            $expectedTag = "v$version"
            if ($env:GITHUB_REF_NAME -ne $expectedTag) { throw "[version] tag '$($env:GITHUB_REF_NAME)' must equal '$expectedTag'." }
        }

        Write-Host "[version] OK version=$version notes=$notes"
        """
    ),
)

package = read("scripts/package-release.ps1")
package = package.replace("$version = '0.2.0-alpha.2'\n", "", 1)
package = package.replace(
    "$repoRoot = Split-Path -Parent $PSScriptRoot\n",
    "$repoRoot = Split-Path -Parent $PSScriptRoot\n"
    "& (Join-Path $PSScriptRoot 'check-version.ps1')\n"
    "$version = (Get-Content -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Raw).Trim()\n",
    1,
)
package = re.sub(
    r"\$packageScenes = Join-Path \$packageRoot 'scenes'.*?\$packageGallery = Join-Path \$packageRoot 'gallery'",
    textwrap.dedent(
        """
        $galleryManifest = Join-Path $repoRoot 'gallery\\gallery.manifest'
        $galleryEntries = Get-Content -LiteralPath $galleryManifest | Where-Object { $_ -like "entry`t*" } | ForEach-Object {
            $fields = $_ -split "`t"
            if ($fields.Count -lt 7) { throw "Malformed gallery entry: $_" }
            [pscustomobject]@{ Scene = $fields[4]; Thumbnail = $fields[5] }
        }
        if (-not $galleryEntries) { throw 'Gallery manifest contains no package entries.' }

        $packageScenes = Join-Path $packageRoot 'scenes'
        New-Item -ItemType Directory -Path $packageScenes -Force | Out-Null
        foreach ($scenePath in ($galleryEntries.Scene | Sort-Object -Unique))
        {
            $source = Join-Path $repoRoot $scenePath
            if (-not (Test-Path -LiteralPath $source)) { throw "Gallery scene is missing: $scenePath" }
            Copy-Item -LiteralPath $source -Destination $packageScenes
        }

        $packageGallery = Join-Path $packageRoot 'gallery'
        """
    ).rstrip(),
    package,
    count=1,
    flags=re.S,
)
package = re.sub(
    r"\$packageThumbnails = Join-Path \$packageGallery 'thumbnails'.*?\$packageReplays = Join-Path \$packageRoot 'replays'",
    textwrap.dedent(
        """
        $packageThumbnails = Join-Path $packageGallery 'thumbnails'
        New-Item -ItemType Directory -Path $packageThumbnails -Force | Out-Null
        Copy-Item -LiteralPath $galleryManifest -Destination $packageGallery
        foreach ($thumbnailPath in ($galleryEntries.Thumbnail | Sort-Object -Unique))
        {
            $source = Join-Path $repoRoot $thumbnailPath
            if (-not (Test-Path -LiteralPath $source)) { throw "Gallery thumbnail is missing: $thumbnailPath" }
            Copy-Item -LiteralPath $source -Destination $packageThumbnails
        }

        $packageReplays = Join-Path $packageRoot 'replays'
        """
    ).rstrip(),
    package,
    count=1,
    flags=re.S,
)
package = re.sub(
    r"New-Item -ItemType Directory -Path \$packageReplays -Force \| Out-Null\nforeach \(\$replay in @\(.*?\n\}\nCopy-Item -LiteralPath \(Join-Path \$repoRoot 'README.md'\) -Destination \$packageRoot",
    textwrap.dedent(
        """
        New-Item -ItemType Directory -Path $packageReplays -Force | Out-Null
        $replayManifest = Join-Path $repoRoot 'regression\\replays\\package.manifest'
        $replayEntries = Get-Content -LiteralPath $replayManifest | Where-Object { $_ -like "replay`t*" } | ForEach-Object {
            ($_ -split "`t", 2)[1]
        }
        if (-not $replayEntries) { throw 'Replay package manifest contains no entries.' }
        foreach ($replay in $replayEntries)
        {
            $source = Join-Path $repoRoot "regression\\replays\\$replay"
            if (-not (Test-Path -LiteralPath $source)) { throw "Packaged replay is missing: $replay" }
            Copy-Item -LiteralPath $source -Destination $packageReplays
        }
        Copy-Item -LiteralPath $replayManifest -Destination $packageReplays
        Copy-Item -LiteralPath (Join-Path $repoRoot 'README.md') -Destination $packageRoot
        """
    ).rstrip(),
    package,
    count=1,
    flags=re.S,
)
package = package.replace(
    "Copy-Item -LiteralPath (Join-Path $repoRoot 'docs\\release-notes-0.2.0-alpha.2.md') -Destination $packageRoot\n",
    "Copy-Item -LiteralPath (Join-Path $repoRoot \"docs\\release-notes-$version.md\") -Destination $packageRoot\n"
    "Copy-Item -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Destination $packageRoot\n",
    1,
)
package = package.replace(
    "    replay_format = 2\n",
    "    replay_format = 2\n    runtime_digest_version = 2\n",
    1,
)
write("scripts/package-release.ps1", package)

# ---------------------------------------------------------------------------
# CI and hygiene (#15-#16, release automation portion of #27)
# ---------------------------------------------------------------------------

write(
    ".github/workflows/windows.yml",
    textwrap.dedent(
        """\
        name: Windows verification

        on:
          pull_request:
          push:
            branches: [main]
            tags: ['v*']
          schedule:
            - cron: '23 7 * * *'
          workflow_dispatch:

        permissions:
          contents: read

        concurrency:
          group: physics-sim-${{ github.workflow }}-${{ github.event.pull_request.number || github.ref }}
          cancel-in-progress: true

        jobs:
          verify:
            runs-on: windows-2022
            timeout-minutes: 30
            steps:
              - uses: actions/checkout@v4
                with:
                  fetch-depth: 0
              - uses: lukka/run-vcpkg@v11
                with:
                  vcpkgJsonGlob: vcpkg.json
              - name: Configure and build
                shell: pwsh
                run: .\\scripts\\build.ps1
              - name: Tracking, version, and hygiene
                shell: pwsh
                run: .\\scripts\\check-hygiene.ps1
              - name: Standard pull-request verification
                if: github.event_name == 'pull_request'
                shell: pwsh
                run: .\\scripts\\test.ps1 -Tier Standard
              - name: Standard main verification
                if: github.event_name == 'push' && !startsWith(github.ref, 'refs/tags/')
                shell: pwsh
                run: .\\scripts\\test.ps1 -Tier Standard
              - name: Full verification
                if: github.event_name == 'schedule' || github.event_name == 'workflow_dispatch' || startsWith(github.ref, 'refs/tags/')
                shell: pwsh
                run: .\\scripts\\verify-all.ps1
              - name: Package prerelease
                if: startsWith(github.ref, 'refs/tags/')
                shell: pwsh
                run: .\\scripts\\package-release.ps1
              - name: Upload verification evidence
                if: always()
                uses: actions/upload-artifact@v4
                with:
                  name: physics-sim-verification-${{ github.run_id }}
                  path: |
                    build/windows-x64/*.log
                    build/windows-x64/fluid-quality-suite/**
                    build/windows-x64/lab-smoke/**
                    dist/*.zip*
                  if-no-files-found: ignore

          publish:
            if: startsWith(github.ref, 'refs/tags/v')
            needs: verify
            runs-on: windows-2022
            permissions:
              contents: write
            steps:
              - uses: actions/checkout@v4
              - uses: actions/download-artifact@v4
                with:
                  name: physics-sim-verification-${{ github.run_id }}
              - name: Publish verified prerelease
                shell: pwsh
                env:
                  GH_TOKEN: ${{ github.token }}
                run: |
                  $version = (Get-Content -LiteralPath VERSION.txt -Raw).Trim()
                  $tag = $env:GITHUB_REF_NAME
                  if ($tag -ne "v$version") { throw "Tag $tag does not match v$version" }
                  $notes = "docs\\release-notes-$version.md"
                  $archive = Get-ChildItem -LiteralPath dist -Filter '*.zip' -File | Select-Object -First 1
                  $checksum = Get-ChildItem -LiteralPath dist -Filter '*.zip.sha256' -File | Select-Object -First 1
                  if ($null -eq $archive -or $null -eq $checksum) { throw 'Release archive or checksum is missing.' }
                  & gh release view $tag --repo $env:GITHUB_REPOSITORY *> $null
                  if ($LASTEXITCODE -eq 0) {
                    & gh release edit $tag --repo $env:GITHUB_REPOSITORY --title "Physics Sim $version" --notes-file $notes
                    if ($LASTEXITCODE -ne 0) { throw 'Failed to update release metadata.' }
                    & gh release upload $tag $archive.FullName $checksum.FullName --repo $env:GITHUB_REPOSITORY --clobber
                    if ($LASTEXITCODE -ne 0) { throw 'Failed to upload release artifacts.' }
                  } else {
                    $arguments = @('release', 'create', $tag, $archive.FullName, $checksum.FullName,
                      '--repo', $env:GITHUB_REPOSITORY, '--title', "Physics Sim $version", '--notes-file', $notes)
                    if ($version.Contains('-')) { $arguments += '--prerelease' }
                    & gh @arguments
                    if ($LASTEXITCODE -ne 0) { throw 'Failed to create release.' }
                  }
        """
    ),
)

write(
    "scripts/check-hygiene.ps1",
    textwrap.dedent(
        """\
        param([switch]$All)

        $ErrorActionPreference = 'Stop'
        $repoRoot = Split-Path -Parent $PSScriptRoot
        $llvmBin = 'C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Tools\\Llvm\\x64\\bin'
        $clangFormat = (Get-Command clang-format -ErrorAction SilentlyContinue).Path
        $clangTidy = (Get-Command clang-tidy -ErrorAction SilentlyContinue).Path
        if (-not $clangFormat) { $clangFormat = Join-Path $llvmBin 'clang-format.exe' }
        if (-not $clangTidy) { $clangTidy = Join-Path $llvmBin 'clang-tidy.exe' }
        if (-not (Test-Path -LiteralPath $clangFormat) -or -not (Test-Path -LiteralPath $clangTidy))
        {
            throw '[hygiene] clang-format and clang-tidy are required.'
        }

        $allCppFiles = & git -C $repoRoot ls-files -- 'src/*.cpp' 'src/**/*.cpp' 'include/*.hpp' 'include/**/*.hpp' 'tests/*.cpp'
        if ($All)
        {
            $formatRelative = @($allCppFiles)
        }
        else
        {
            $base = if ($env:GITHUB_BASE_REF) { "origin/$($env:GITHUB_BASE_REF)" } else { 'HEAD~1' }
            $changed = @()
            & git -C $repoRoot rev-parse --verify $base *> $null
            if ($LASTEXITCODE -eq 0)
            {
                $changed += & git -C $repoRoot diff --name-only --diff-filter=ACMRT "$base...HEAD" -- '*.cpp' '*.hpp'
            }
            $changed += & git -C $repoRoot diff --name-only --diff-filter=ACMRT -- '*.cpp' '*.hpp'
            $changed += & git -C $repoRoot diff --cached --name-only --diff-filter=ACMRT -- '*.cpp' '*.hpp'
            $formatRelative = @($changed | Where-Object { $_ -and ($allCppFiles -contains $_) } | Sort-Object -Unique)
        }

        if ($formatRelative.Count -gt 0)
        {
            $formatFiles = $formatRelative | ForEach-Object { Join-Path $repoRoot $_ }
            & $clangFormat --dry-run --Werror @formatFiles
            if ($LASTEXITCODE -ne 0) { throw '[hygiene] clang-format check failed.' }
        }

        $tidySources = @(
            'src\\core\\simulation.cpp',
            'src\\core\\water_feel_metrics.cpp',
            'src\\app\\surface_reconstruction.cpp',
            'src\\app\\water_visual_effects.cpp'
        )
        $tidyRoot = Join-Path $repoRoot 'build\\windows-x64\\clang-tidy'
        New-Item -ItemType Directory -Path $tidyRoot -Force | Out-Null
        foreach ($relative in $tidySources)
        {
            $source = Join-Path $repoRoot $relative
            $log = Join-Path $tidyRoot (($relative -replace '[\\/]', '-') + '.log')
            $previousErrorAction = $ErrorActionPreference
            $ErrorActionPreference = 'Continue'
            & $clangTidy $source '-checks=-*,bugprone-*,performance-*' '--' '-std=c++20' "-I$(Join-Path $repoRoot 'include')" '-DNOMINMAX' '-DWIN32_LEAN_AND_MEAN' 2>&1 |
                Set-Content -LiteralPath $log
            $tidyExit = $LASTEXITCODE
            $ErrorActionPreference = $previousErrorAction
            if ($tidyExit -ne 0) { throw "[hygiene] clang-tidy failed for $relative; log=$log" }
        }

        $secretPattern = '(?i)(-----BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY-----|ghp_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,}|AKIA[0-9A-Z]{16})'
        $trackedFiles = & git -C $repoRoot ls-files
        foreach ($relativePath in $trackedFiles)
        {
            $path = Join-Path $repoRoot $relativePath
            if (Test-Path -LiteralPath $path -PathType Leaf)
            {
                $match = Select-String -LiteralPath $path -Pattern $secretPattern -Quiet -ErrorAction SilentlyContinue
                if ($match) { throw "[hygiene] possible secret in $relativePath" }
            }
        }

        & (Join-Path $PSScriptRoot 'check-dependencies.ps1')
        & (Join-Path $PSScriptRoot 'check-tracking.ps1')
        & (Join-Path $PSScriptRoot 'check-version.ps1')
        Write-Host "[hygiene] format_files=$($formatRelative.Count) tidy_files=$($tidySources.Count); dependency, tracking, version, secret, and path checks passed"
        """
    ),
)

# Ensure the package and docs refer to the new release note.
readme = read("README.md")
readme = readme.replace(
    "See `docs/release-notes-0.2.0-alpha.2.md` for this prerelease's scope and limitations.",
    "See `docs/release-notes-0.2.0-alpha.3.md` for this prerelease's scope and limitations.",
)
write("README.md", readme)

# Save a deterministic change list for the workflow's explicit staging step.
changed = sorted(set(CHANGED))
write(".agent/changed_paths.txt", "\n".join(changed) + "\n")
print(f"Applied {len(changed)} files")
for item in changed:
    print(item)
