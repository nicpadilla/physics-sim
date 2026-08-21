# Packaged-Build Usability Test Protocol

This protocol prepares the manual evidence required by GitHub issue #28. It does not replace independent human sessions.

## Goal

Observe whether a new user can launch the packaged build, learn the basic interaction loop, create and edit a water scene, understand one challenge, recover work through save/load, and inspect the same concepts in Lab mode without source-code context.

## Participant criteria

Use at least three independent participants who:

- did not implement the feature set under review;
- have not received a detailed walkthrough of Physics Sim;
- can use a Windows desktop application with a mouse and keyboard;
- agree to observation and privacy-safe note taking.

Record only an anonymized participant identifier. Do not commit names, email addresses, recordings, or other personal data unless the participant gave explicit consent and the evidence genuinely needs it.

## Test package and environment

Each session must use:

- the same clean release ZIP or an explicitly recorded replacement;
- a verified package SHA-256, version, commit, and release-manifest identity;
- a clean extraction directory;
- a fresh application preference/data directory;
- no source checkout, debugger, developer overlay, or advance explanation of controls;
- the default packaged settings unless a task explicitly changes them.

Run `scripts/capture-usability-package.ps1` before the first session and store its JSON output with the session records. If the package changes, start a new evidence set.

## Moderator rules

- Read each task prompt as written.
- Do not point at controls or reveal shortcuts unless the participant asks for help.
- Record every request for help and the smallest hint given.
- Ask the participant to think aloud, but do not interpret silence as success or failure.
- Stop a task only for a crash, data-loss risk, participant request, or a ten-minute hard limit.
- After each task, ask one neutral question: “What did you expect to happen?”

## Core task script

### Task 1: First launch and orientation

Prompt: “Launch Physics Sim and get to a point where you can interact with water.”

Observe:

- whether the participant understands the first screen;
- time to begin interacting;
- tutorial completion or abandonment;
- unclear labels, controls, or visual state.

Success: the participant reaches a playable scene and pours water without a moderator hint.

### Task 2: Create a simple basin

Prompt: “Make a container that holds water, then add enough water to show that it works.”

Observe tool discovery, wall drawing, correction of mistakes, pause/resume use, and whether the water behavior matches expectations.

Success: water remains visibly contained for at least five seconds.

### Task 3: Add and edit a device

Prompt: “Add a device that moves or controls water, then change one of its settings.”

Do not name a specific tool. Observe Advanced Tools discovery, placement feedback, selection, editing, and deletion or undo behavior.

Success: the participant places a valid device and changes a visible or measurable setting.

### Task 4: Complete one packaged challenge

Prompt: “Open a challenge and complete its stated goal.”

Observe gallery navigation, objective comprehension, progress feedback, budget/failure feedback, retry behavior, and whether completion is clear.

Success: the challenge reaches its completion state, with no more than one moderator hint.

### Task 5: Save, change, and recover

Prompt: “Save your current scene, make an obvious change, then restore the saved version.”

Observe whether save/load locations and effects are understood and whether the participant fears losing work.

Success: the restored scene visibly matches the saved authored state.

### Task 6: Inspect the simulation in Lab mode

Prompt: “Open the engineering view and find information about the water’s motion or pressure, then return to the sandbox.”

Observe mode discovery, field selection, terminology, navigation, keyboard support, and return expectations.

Success: the participant identifies one relevant field or metric and returns without losing the sandbox state expected by the tested build.

### Task 7: Free-form use

Prompt: “Use the simulator however you like for five minutes.”

Observe spontaneous feature discovery, repeated friction, performance issues, visual expectations, and whether the participant chooses to keep experimenting.

## Measures

For every task record:

- start and end time or elapsed seconds;
- completion: success, partial, failed, or not attempted;
- errors and recoveries;
- requests for help;
- hints given;
- observed hesitation or confusion;
- participant expectation in their own words;
- product defect versus preference classification;
- linked screenshot or log only when privacy-safe and useful.

## Severity rules

- **Blocking:** crash, unrecoverable data loss, inability to launch, or no participant can complete a core task.
- **Major:** most participants fail a core task, need direct instruction, or form a wrong model that causes repeated failure.
- **Moderate:** clear delay, avoidable error, misleading feedback, or recurring confusion that participants can recover from.
- **Minor:** cosmetic issue, wording preference, or low-cost friction that does not impair task completion.

Every blocking or major finding must become a linked GitHub issue. Moderate findings should be grouped only when they have the same cause and acceptance criteria.

## Completion criteria

Issue #28 can be closed only after:

1. at least three completed independent sessions use a recorded package identity;
2. the session records use the same core task set;
3. a summary reports completion, errors, help requests, and severity-ranked findings;
4. every blocking or major finding has a linked issue;
5. the summary states what was not tested;
6. no agent or automated review is labeled `Human Accepted`.
