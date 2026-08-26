# Mod Plan: Three Steps Walking Speed

## Goal
Reduce Arma Reforger's mouse-wheel walking-speed control to exactly three fixed steps so squad members can coordinate pace more reliably during tactical missions.

## Research Summary
- `SCR_CharacterControllerComponent.OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)` is the main player-input hook for character movement preparation.
- `CharacterControllerComponent.GetDynamicSpeed()` exposes the current mouse-wheel or walk-button speed value as a normalized `<0,1>` float.
- `CharacterControllerComponent.SetDynamicSpeed(float value)` can override that value directly.
- `CharacterControllerComponent.GetDynamicSpeed()` is documented as the value "set by mousewheel or by pressing walk button", which makes it the correct feature to clamp instead of trying to rebuild locomotion input from scratch.

## Implementation Plan
1. Create a minimal addon scaffold based on the same structure used by `ServerRulesQuiz`.
2. Add a `modded class SCR_CharacterControllerComponent` script.
3. Let vanilla input run first via `super.OnPrepareControls(...)`.
4. Read the dynamic speed chosen by mouse wheel.
5. Snap it to the nearest of three allowed tiers.
6. Push the snapped value back with `SetDynamicSpeed(...)`.
7. Show a small local help/chat message whenever the tier changes so the player immediately knows which pace is active.

## Assumptions
- The dynamic-speed value alone controls the wheel-based walking-speed granularity for keyboard/mouse users.
- Sprint remains controlled by the separate sprint input and should continue to work normally.
- The exact three normalized values may need minor tuning after in-game testing, but the snapping architecture will stay the same.

## Initial Tier Values
- Slow: `0.20`
- Standard: `0.45`
- Fast: `0.70`

These are intentionally kept below the top end of the dynamic-speed range so the mod stays focused on three walking paces rather than bleeding into sprint-like behavior.
