$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'verify-recovery-basin-mode.ps1') `
    -Name 'early-surface' `
    -Baseline 'regression\recovery_basin_early_surface_golden.bmp' `
    -VisualMode 'surface' `
    -Tick 240 `
    -AutoExitMilliseconds 15000
