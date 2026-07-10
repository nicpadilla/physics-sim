# Pre-Recovery Snapshot

The pre-recovery project state is preserved by annotated Git tag `pre-recovery-2026-07-10` at commit `647153c` (`PSIM-0088 tune live fluid density`).

Inspect the former sources of truth without restoring them:

```powershell
git show pre-recovery-2026-07-10:ROADMAP.md
git show pre-recovery-2026-07-10:PROGRESS.md
git show pre-recovery-2026-07-10:ISSUES.md
```

The tag also preserves the former regression goldens, scenes, sources, and June verification notes. Those artifacts are historical evidence only. They do not satisfy recovery automation or human-acceptance gates.

The recovery began because the former dashboard claimed universal verification while current visual evidence showed incohesive water, the application and solver were structurally concentrated, the default suite took several minutes, and summary/detail issue state could disagree without validator failure.
