$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'verify-recovery-basin-mode.ps1') `
    -Name 'density' `
    -Baseline 'regression\recovery_basin_density_golden.bmp' `
    -VisualMode 'density' `
    -Tick 2400 `
    -AutoExitMilliseconds 60000
