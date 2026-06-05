@(
    @{
        Name = 'demo-add-directional'
        Baseline = 'regression\demo_scene_replay_add_directional_golden.bmp'
        ReplayFile = 'regression\replays\demo-add-directional.replay'
        Arguments = @(
            '--dump-frame-after-ticks', '240',
            '--auto-exit-ms', '30000'
        )
    },
    @{
        Name = 'demo-add-omni'
        Baseline = 'regression\demo_scene_replay_add_omni_golden.bmp'
        ReplayFile = 'regression\replays\demo-add-omni.replay'
        Arguments = @(
            '--dump-frame-after-ticks', '240',
            '--auto-exit-ms', '30000'
        )
    }
)
