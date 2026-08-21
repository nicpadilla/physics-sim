# Continuous Integration

The required pull-request check is the `verify` job in the **Windows verification** workflow.

Every pull request runs the Standard verification tier. This is intentionally broader than the local Fast loop: changes to runtime state, persistence, replay, scenes, Lab, presentation, content, tests, or build files must fail before merge rather than after they reach `main`. Documentation-only pull requests also run Standard for a simple and predictable required-check policy.

Superseded runs for the same pull request cancel automatically. The workflow uses read-only repository permissions for verification and uploads logs and structured evidence even when a check fails.

Local equivalents:

```powershell
.\scripts\build.ps1
.\scripts\check-hygiene.ps1
.\scripts\test.ps1 -Tier Standard
```

`check-hygiene.ps1` formats every changed tracked C++ source or header and runs the selected production sources through clang-tidy. Use the full-tree format backstop explicitly with:

```powershell
.\scripts\check-hygiene.ps1 -All
```

Scheduled, manually dispatched, and version-tag workflows retain the Full verification path. Version tags also retain prerelease packaging.
