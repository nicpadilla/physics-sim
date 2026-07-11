$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'verify-recovery-basin-mode.ps1') `
    -Name 'surface' `
    -Baseline 'regression\recovery_basin_surface_golden.bmp' `
    -VisualMode 'surface' `
    -Tick 2400 `
    -AutoExitMilliseconds 60000
