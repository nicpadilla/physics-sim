# Fluid Quality And Backlog Reconciliation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the fluid-realism backlog with deterministic scenario coverage, then reconcile the finished-game polish backlog by closing the items that are already verified and deferring the larger shell/audio/tutorial work with documented rationale.

**Architecture:** Add a reusable fluid-quality snapshot harness on top of `WaterSimulation2D`, then use it from a focused test target and a manifest-driven runner script. Keep the current SDL app architecture intact; for the remaining polished shell items, record explicit deferrals rather than inventing a half-finished menu system that would destabilize the verified runtime loop.

**Tech Stack:** C++20, SDL2, CMake/CTest, PowerShell, Markdown docs.

---

### Task 1: Add the fluid-quality snapshot harness

**Files:**
- Create: `include/physics_sim/fluid_quality.hpp`
- Create: `tests/fluid_quality_tests.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

Add a new focused test target that references `physics_sim/fluid_quality.hpp` and fails to compile until the snapshot helpers exist.

- [ ] **Step 2: Run test to verify it fails**

Run: `.\scripts\build.ps1`
Expected: build fails because the new harness header and/or symbols are missing.

- [ ] **Step 3: Write minimal implementation**

Add snapshot helpers for particle speed, kinetic energy, bounds, center of mass, wall penetration, and pool-height estimation.

- [ ] **Step 4: Run test to verify it passes**

Run: `.\scripts\build.ps1`
Expected: the new test target builds successfully.

- [ ] **Step 5: Commit**

Stage and commit the harness files together once the scenario tests pass.

### Task 2: Add the deterministic fluid-quality suite and runner

**Files:**
- Create: `regression/fluid_quality_suite.psd1`
- Create: `scripts/verify-fluid-quality-suite.ps1`
- Modify: `docs/fluid-realism-roadmap.md`
- Modify: `docs/performance-budget.md`

- [ ] **Step 1: Write the failing test**

Add a script invocation path in the new runner that references the manifest before the test target exists.

- [ ] **Step 2: Run test to verify it fails**

Run: `.\scripts\verify-fluid-quality-suite.ps1`
Expected: fail because the suite executable or manifest-driven scenario wiring is not present yet.

- [ ] **Step 3: Write minimal implementation**

Teach the runner to execute the new suite target and document the scenario catalog and live/offline budgets.

- [ ] **Step 4: Run test to verify it passes**

Run: `.\scripts\verify-fluid-quality-suite.ps1`
Expected: all configured fluid-quality scenarios complete deterministically.

- [ ] **Step 5: Commit**

Record the suite script, manifest, and docs together.

### Task 3: Reconcile the remaining Epic 10 backlog

**Files:**
- Modify: `ISSUES.md`
- Modify: `README.md` if needed
- Modify: `docs/finished-game-polish-roadmap.md` only if a limitation note is required

- [ ] **Step 1: Write the failing test**

No code test here; the failure condition is the tracker showing still-open issues that are already verified or should be explicitly deferred.

- [ ] **Step 2: Run test to verify it fails**

Run: `.\scripts\check-tracking.ps1`
Expected: tracker still shows unresolved Epic 10 items before reconciliation.

- [ ] **Step 3: Write minimal implementation**

Close the already-verified polish items, and defer the menu/tutorial/audio/settings/save-load/error-handling items with a short rationale that the project is still a direct-launch deterministic simulator.

- [ ] **Step 4: Run test to verify it passes**

Run: `.\scripts\check-tracking.ps1`
Expected: tracker remains structurally valid after the backlog status updates.

- [ ] **Step 5: Commit**

Update the issue notes and the documentation note together with the tracker changes.
