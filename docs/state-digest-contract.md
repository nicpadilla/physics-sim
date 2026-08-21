# Versioned State Digest Contract

## Purpose

Physics Sim uses state digests to identify deterministic inputs and runtime continuation state. A digest is a compact regression and replay diagnostic. It is not a cryptographic signature and does not prove physical or visual correctness.

## Format

The versioned digest format is currently `2`. Each digest begins with a fixed Physics Sim namespace, the format version, and a domain identifier before any state fields are mixed. The output remains a 16-character uppercase hexadecimal FNV-1a value for compact logs and compatibility with existing tooling.

Changing the field inventory, byte representation, ordering, or domain rules requires a new format version and an explicit baseline migration.

## Domains

### Scene definition

The scene-definition domain identifies durable authored intent:

- grid size and cell size;
- solver profile;
- execution-relevant metadata and challenge criteria;
- walls;
- configured emitters and devices;
- sensor region, enabled/objective state, and label.

Derived sensor activity is excluded because authored scene files do not preserve the water that produced it.

### Physics runtime

The physics-runtime domain identifies values exposed by the deterministic checkpoint and facade boundaries that can alter current state, future steps, device behavior, challenge accounting, or command sequencing:

- simulation tick and fixed timestep;
- pause and pending single-step state;
- solver profile and all exposed solver settings;
- particles, including affine velocity and density data;
- current grid pressure, divergence, velocity, solid, volume-fraction, and density fields;
- emitters, including fractional accumulation and emitted-particle phase;
- gates, sensors, drains, pumps, and valves;
- lifecycle, density, energy, sensor, and pressure-solve metrics.

Private scratch buffers that the solver fully overwrites before they can affect the next step are excluded from the public digest utility. The in-memory checkpoint still preserves those buffers for exact continuation.

### Rendered output

The rendered-output domain is reserved for image or presentation evidence. Rendered-output identity must not be substituted for physics-runtime identity.

## Determinism rule

Within the supported Windows/MSVC fixed-step contract:

1. equal physics-runtime digests must represent equal public continuation state;
2. equal future commands and fixed steps should produce equal later physics-runtime digests;
3. a difference in any included future-affecting field must change the digest;
4. scene-definition and physics-runtime digests are separate domains even when their textual output happens to use the same hash width.

## Compatibility

The existing unversioned `state_digest()` methods remain available for current replay and accepted baseline compatibility. New code should report the version and domain when it uses the v2 utilities. Replay migration and baseline changes must be explicit rather than silently changing existing expected digests.

The unused public `deterministic_seed` setting was removed because it did not affect simulation behavior. A future seeded feature must implement the seed, include it in runtime identity, and add negative tests before exposing it publicly.
