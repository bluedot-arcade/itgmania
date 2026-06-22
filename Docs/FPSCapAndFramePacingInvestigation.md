# FPS Cap and Frame Pacing Investigation Handoff

Date: 2026-06-23

This document summarizes the current state of the MaxFPS implementation, graphics option persistence fixes, and the ongoing VSync/frame pacing investigation.

## User-Facing Goal

- Replace the old hidden `FrameLimitPercent` behavior with a user-selectable exact `MaxFPS` preference.
- `MaxFPS` must work independently of VSync, but VSync wins when enabled.
- Theme graphics options should expose:
  - `Max FPS`: Yes/No
  - `Max FPS Value`: integer value from `30` to `1200`, step `1`
- The cap should apply everywhere.
- Refresh rate and display resolution should persist correctly from the graphics options menu.

## Where Settings Are Saved

Runtime preferences are saved in:

```text
Save/Preferences.ini
```

Relevant code:

- `src/SpecialFiles.cpp`
  - `SpecialFiles::PREFERENCES_INI_PATH = "Save/Preferences.ini"`
- `src/PrefsManager.cpp`
  - `PrefsManager::SavePrefsToDisk()`
  - `PrefsManager::SavePrefsToIni()`

The new preference key is:

```ini
MaxFPS=240
```

`MaxFPS=0` means disabled/off.

## Implemented Functional Changes

### MaxFPS Preference

File:

- `src/RageDisplay.cpp`

Old preference removed:

```cpp
Preference<float> g_fFrameLimitPercent("FrameLimitPercent", 0.0f);
```

New preference:

```cpp
Preference<int> g_iMaxFPS("MaxFPS", 0, ValidateMaxFPS);
```

Validation:

- `< 30` becomes `0` / off
- `30..1200` accepted
- `> 1200` clamped to `1200`

### Manual Limiter Behavior

The limiter now runs from `RageDisplay::FrameLimitAfterVsync()`.

Current behavior:

- If VSync is on: manual MaxFPS limiter does not run.
- If VSync is off and `MaxFPS > 0`: limiter targets the end-to-end frame cadence.
- For caps below `200 FPS`: uses `usleep()` until close to target, then spins.
- For caps `>= 200 FPS`: busy-waits instead of sleeping, because Windows `usleep()` maps to `Sleep(usec / 1000)` and caused worse visible pacing at `240 FPS`.

Important code:

- `src/RageDisplay.cpp`
  - `FrameLimitBeforeVsync()` is currently empty.
  - `FrameLimitAfterVsync()` applies the cap.
- `src/RageDisplay_OGL.cpp`
- `src/RageDisplay_D3D.cpp`
- `src/RageDisplay_GLES2.cpp`

Backend calls were moved so `FrameLimitAfterVsync()` happens after backend end-frame/base stats work, not immediately after present/swap.

### Graphics Option Persistence Fixes

File:

- `src/StepMania.cpp`

`StoreActualGraphicOptions()` was changed so actual display size/refresh are not written back over the preferred fullscreen settings when running windowed or fullscreen-borderless.

Current logic:

- Save actual `DisplayWidth` / `DisplayHeight` only when:
  - not windowed
  - not fullscreen borderless
- Save actual `RefreshRate` only when:
  - refresh is not default
  - not windowed
  - not fullscreen borderless
- VSync is still saved from actual params.

### Theme Option Rows

Themes touched:

- `Themes/KING`
- `Themes/Simply Love`

Files touched in each theme:

- `metrics.ini`
- `Scripts/SL-OperatorMenuOptions.lua`
- `Languages/en.ini`
- `Languages/it.ini`

Rows added near VSync:

```text
MaxFPS
MaxFPSValue
```

Behavior:

- `MaxFPS` Yes/No row:
  - off writes `MaxFPS=0`
  - on writes the last selected cap, defaulting to `60`
- `MaxFPSValue` row:
  - values `30..1200`
  - step `1`
  - writes exact integer to `MaxFPS`

### Refresh Rate Theme Row

Both `Themes/KING/metrics.ini` and `Themes/Simply Love/metrics.ini` were changed so refresh rate uses:

```ini
LineRefreshRate="lua,ConfRefreshRate()"
```

This matches the dynamic fallback behavior and fixed the refresh-rate row not saving correctly.

### OpenGL Default State Performance Fix

File:

- `src/RageDisplay.cpp`

Removed the per-frame default-state call:

```cpp
SetTextureFiltering(TextureUnit_1, true);
```

Reason:

- On OpenGL, `SetTextureFiltering()` queried texture mipmap state with `glGetTexLevelParameteriv()`.
- Timing diagnostics showed this call blocking for `~18-100ms` when VSync was enabled.
- Texture filtering is already set when textures are created in `RageDisplay_Legacy::CreateTexture()`, so the per-frame reset was redundant.

## Build Status

Last successful build command:

```powershell
cmake --build Build --target ITGmania --config RelWithDebInfo
```

Last successful output target:

```text
Program\ITGmania-release-symbols.exe
```

Common build issue encountered:

- Link can fail with `LNK1168` if `Program\ITGmania-release-symbols.exe` is running.
- Stop the running game process, then rebuild.

## Frame Pacing Investigation

### Initial MaxFPS Findings

With cap `240` and VSync off:

- Before limiter boundary fix, FPS could sit around `130-145` or `225-234`.
- After moving limiter accounting to the true end of frame, cap behaved correctly:

```text
235-240 FPS at MaxFPS 240
```

At `120Hz`, `VSync off + MaxFPS 120` looked visually worse and showed artifacts. This is expected because a software cap at exactly monitor refresh does not synchronize with scanout. VSync is still needed for scanout-synchronized presentation.

### VSync-On Problem

User tested title screen with:

```text
120Hz VSync on, MaxFPS 120
```

Observed:

```text
~115-119 FPS
late frames present
```

This means the issue is not gameplay load. Even the title screen misses deadlines.

### Stats Overlay Diagnostics Added

Temporary diagnostics were added to `RageDisplay::GetStats()` and related timing setters. These are investigation-only and should probably be cleaned up or hidden behind a debug preference before finalizing.

Current overlay includes:

```text
<FPS> FPS
<avg> av FPS
<VPF> VPF
OpenGL
120Hz VSync on
MaxFPS 120
8.48/22.43ms late 8
upd 0.28 draw 22.40
beg 21.68 vp 0.00 clr 0.00 base 21.68 rt 0.00
st 0.00 0.00 0.00 0.00 0.00 0.00 21.68 0.00 0.00
bga 1.31 scr 0.54 ov 0.72 end 0.46
sw 0.02 fn 0.00 up 0.01
```

Meanings:

- `avg/maxms late N`: average and max full frame time for the last stats window; `late` counts frames slower than 1.25x expected interval.
- `upd`: main loop update phase.
- `draw`: whole `SCREENMAN->Draw()` call.
- `beg`: `DISPLAY->BeginFrame()`.
- `vp`: `glViewport`.
- `clr`: `glClear`.
- `base`: base `RageDisplay::BeginFrame()` / default render-state setup.
- `rt`: offscreen render target begin.
- `st`: split of `SetDefaultRenderStates()` in this order:
  1. lighting
  2. cull
  3. zwrite
  4. ztest
  5. alphatest
  6. blend
  7. texture filter
  8. zbias
  9. perspective
- `bga`: shared BGA draw.
- `scr`: screen stack draw.
- `ov`: overlay screens draw.
- `end`: `DISPLAY->EndFrame()` from `ScreenManager`.
- `sw`: backend `SwapBuffers`.
- `fn`: post-swap `glFinish`.
- `up`: window update/message pump.

### Key Timing Evidence So Far

User samples showed:

```text
upd ~0.2ms
draw 18-30ms
sw/fn/up ~0ms
```

Then deeper split showed:

```text
draw 20ms
bga/scr/ov/end only a few ms total
beg 14-26ms
```

Then `BeginFrame()` and render-state split showed:

```text
beg 21.43 vp 0.00 clr 0.00 base 21.43 rt 0.00
st 0.00 0.00 0.00 0.00 0.00 0.00 21.43 0.00 0.00
```

Conclusion at that point:

- The recurrent stall is inside `RageDisplay_Legacy::BeginFrame()`.
- More specifically, it was inside `SetDefaultRenderStates()`.
- More specifically again, it was value 7 in `st`, which is `SetTextureFiltering(TextureUnit_1, true)`.
- It is not in:
  - game update
  - title-screen actor draw
  - stats/debug overlay draw
  - `SwapBuffers`
  - post-swap `glFinish`
  - Windows message pump

### Texture Filtering Stall

`SetDefaultRenderStates()` used to call:

```cpp
SetTextureFiltering(TextureUnit_1, true);
```

On OpenGL, this calls `RageDisplay_Legacy::SetTextureFiltering()`, which uses:

```cpp
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth1);
glGetTexLevelParameteriv(GL_TEXTURE_2D, 1, GL_TEXTURE_WIDTH, &iWidth2);
```

Those `glGet*` calls can force driver synchronization. Calling this every frame in default-state setup caused stalls like:

```text
st 0.00 0.00 0.00 0.00 0.00 0.00 99.74 0.00 0.00
```

Fix applied:

- Removed the per-frame `SetTextureFiltering(TextureUnit_1, true)` call from `RageDisplay::SetDefaultRenderStates()`.
- Texture filtering is still applied when textures are created in `RageDisplay_Legacy::CreateTexture()`:

```cpp
SetTextureFiltering(TextureUnit_1, true);
SetTextureWrapping(TextureUnit_1, false);
```

Result after removal:

```text
beg 0.01 vp 0.00 clr 0.00 base 0.00 rt 0.00
st 0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00
```

So this was a real performance bug and should remain fixed.

### Remaining VSync-On Pacing Issue

After removing the redundant texture filtering call, the long wait moved to the first real textured background draw:

```text
draw 23.22
beg 0.01 ... st all 0.00
bga 23.00
sw 0.02 fn 0.00 up 0.00
```

This suggested the OpenGL driver was deferring synchronization until the first substantial draw/texture command of the next frame.

A test was run by re-enabling `glFinish()` after `SwapBuffers()` even when VSync is on. Result during gameplay:

```text
bga 0.00
end 24.34
sw 16.00
fn 16.11
```

The wait moved from `bga` into `end/fn`, but FPS and late frames did not improve. Therefore:

- VSync-on `glFinish()` is not a fix.
- It only relocates the driver wait to an explicit sync point.
- This test was reverted.
- Current code should keep `glFinish()` only for VSync-off behavior, as before.

Current conclusion:

- The remaining `115-119 FPS` / late-frame behavior with `120Hz VSync on` appears to be driver/GPU frame-boundary synchronization rather than a specific theme actor or the MaxFPS limiter.
- The per-frame texture filtering stall is fixed, but the remaining VSync pacing behavior still needs deeper strategy if visual stutter remains unacceptable.

## Important Temporary/Diagnostic Code

The following are diagnostic and may not be appropriate for final merge as-is:

- Large timing stat block in `src/RageDisplay.cpp`
- Public timing setters in `src/RageDisplay.h`
- Instrumentation in:
  - `src/GameLoop.cpp`
  - `src/ScreenManager.cpp`
  - `src/RageDisplay_OGL.cpp`
- Extra stats overlay text in `RageDisplay::GetStats()`
- WGL swap interval log line in `RageDisplay_Legacy::TryVideoMode()`

WGL diagnostic line:

```text
WGL swap interval requested 1, set result 1, actual 1
```

## Current Open Questions

1. Is the remaining VSync-on stutter visually improved after removing the per-frame texture filtering call?
   - The timing still shows late frames, but one major blocking state call was removed.
2. Is the remaining late-frame pattern specific to OpenGL?
   - Compare with D3D, if available and stable in this build.
3. Is the remaining behavior dependent on fullscreen mode?
   - Test exclusive fullscreen vs fullscreen borderless vs windowed.
4. Is the remaining behavior dependent on driver VSync settings?
   - Test driver forced VSync / application controlled / low-latency options.
5. Should the diagnostic timing overlay be retained behind a debug preference, or removed before final merge?

## Suggested Next Steps

1. Keep the functional fixes:

- MaxFPS preference and menu rows.
- Resolution/refresh persistence fixes.
- Limiter end-of-frame placement.
- Removal of redundant per-frame `SetTextureFiltering(TextureUnit_1, true)` from `SetDefaultRenderStates()`.

2. Do not keep the VSync-on `glFinish()` test.

It moved the wait from `bga` to `end/fn` and did not improve pacing.

3. Next useful comparisons:

- OpenGL vs D3D at `120Hz VSync on`.
- Exclusive fullscreen vs borderless/windowed.
- VSync on with `MaxFPS=0` and with `MaxFPS=120` to confirm MaxFPS remains inactive while VSync is on.
- Different visual styles/backgrounds, to see whether heavy first textured draw makes the deferred wait more visible.

4. Clean up diagnostics before finalizing:

- Keep useful debug stats behind a debug preference or remove them.
- Keep `MaxFPS`, persistence fixes, theme rows, and limiter boundary changes.
- Consider keeping a minimal `MaxFPS/mode/VSync` stats line only while debugging.

## Files Modified in Main Repo

Core functional changes:

- `src/RageDisplay.cpp`
- `src/RageDisplay.h`
- `src/RageDisplay_OGL.cpp`
- `src/RageDisplay_D3D.cpp`
- `src/RageDisplay_GLES2.cpp`
- `src/StepMania.cpp`

Diagnostic changes:

- `src/GameLoop.cpp`
- `src/ScreenManager.cpp`
- additional timing code in `src/RageDisplay.cpp`, `src/RageDisplay.h`, `src/RageDisplay_OGL.cpp`

Theme changes:

- `Themes/KING/metrics.ini`
- `Themes/KING/Scripts/SL-OperatorMenuOptions.lua`
- `Themes/KING/Languages/en.ini`
- `Themes/KING/Languages/it.ini`
- `Themes/Simply Love/metrics.ini`
- `Themes/Simply Love/Scripts/SL-OperatorMenuOptions.lua`
- `Themes/Simply Love/Languages/en.ini`
- `Themes/Simply Love/Languages/it.ini`

## Worktree Notes

Observed from top-level `git status`:

```text
 M "Themes/Simply Love"
 M src/GameLoop.cpp
 M src/RageDisplay.cpp
 M src/RageDisplay.h
 M src/RageDisplay_D3D.cpp
 M src/RageDisplay_GLES2.cpp
 M src/RageDisplay_OGL.cpp
 M src/ScreenManager.cpp
 M src/StepMania.cpp
?? Themes/KING.zip
?? Themes/KING/
?? extern/libjpeg-turbo/
?? extern/libpng/
```

`Themes/Simply Love` appears to be a nested repository/submodule-like directory. Use:

```powershell
git -C "Themes\Simply Love" diff
```

`Themes/KING` is untracked from the top-level worktree, but contains the parallel theme changes.

There was a warning for `Themes/Simply Love/Scripts/SL-OperatorMenuOptions.lua`:

```text
CRLF will be replaced by LF the next time Git touches it
```

## Known Good Test Points

MaxFPS off/VSync off:

- Game can run uncapped around very high FPS, reported by user as ~`2k FPS`.

MaxFPS `240`, VSync off:

- After limiter placement fix, observed around:

```text
235-240 FPS
```

MaxFPS `120`, VSync off on a 120Hz display:

- FPS is around `118-120`, but visual artifacts/stutter can be worse because software cap is not scanout sync.

VSync on, 120Hz:

- Overlay reports `120Hz VSync on`.
- Actual FPS often around `115-119`.
- Before the texture filtering fix, timing diagnostics showed stalls in `BeginFrame()` / `SetTextureFiltering()`.
- After removing that redundant state call, `beg/base/st` are near zero, but late frames can still appear as driver synchronization at the first substantial draw or explicit `glFinish()`.
