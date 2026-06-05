# ADR 0002: Scene Format

Status: Accepted

Context:

Scene files need to survive save/load, support metadata and device expansion, and fail safely when the file is malformed or from an unsupported future format.

Decision:

Use a versioned plain-text scene format with explicit `physics-sim-scene` headers, named fields for metadata, and keyword-driven records for walls, emitters, devices, and future scene extensions.

Consequences:

- Known format versions can be validated directly by the parser.
- Unknown or malformed versions fail cleanly instead of being guessed.
- New scene capabilities can be added by extending the keyword set while keeping the version policy explicit.
