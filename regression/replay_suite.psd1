@(
    @{
        Name = 'recovery-basin'
        Baseline = 'regression\recovery_basin_surface_golden.bmp'
        ReplayFile = 'regression\replays\recovery-basin.replay'
        Arguments = @(
            '--software-renderer',
            '--scene-path', 'scenes/starter_basin.pscene',
            '--visual-mode', 'surface',
            '--dump-frame-after-ticks', '2400',
            '--auto-exit-ms', '60000'
        )
    }
)
