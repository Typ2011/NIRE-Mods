# Mod Plan: Server Rules Quiz

## Concept
A mod that requires players to read server rules and pass a quiz before they can spawn on a PvE server. Player quiz completion state persists across sessions.

## Phases

### Phase 1: Foundation — COMPLETE
Core quiz system with persistence and basic UI.

**Features Built:**
- JSON configuration file for rules and quiz questions
- Dynamic question support (any number of questions)
- Player state persistence (tracks who has passed the quiz)
- Game mode component that hooks into player connect/spawn events
- Basic quiz dialog UI showing rules then questions
- Pass/fail logic with retry capability

**Key Files Created:**
- `addon.gproj` - Project file
- `Config/ServerRulesConfig.json` - Rules and questions config
- `Scripts/Game/SRQ_QuizConfig.c` - Config data structures and JSON loading
- `Scripts/Game/SRQ_PlayerStateManager.c` - Persistent player state tracking
- `Scripts/Game/SRQ_QuizManagerComponent.c` - Game mode component
- `Scripts/Game/SRQ_QuizDialog.c` - Quiz UI dialog handler
- `Scripts/Game/SRQ_ModdedPlayerController.c` - Modded player controller for quiz integration
- `UI/layouts/QuizDialog_UI.layout` - Active dialog layout (created manually in Workbench)

### Phase 2: Full UI Polish — COMPLETE
Enhanced UI with visual answer feedback, progress indicator, and timed advance.

**Features Built:**
- Visual feedback per answer: correct button turns green, wrong button turns red,
  and the correct answer is highlighted green when the player chose wrong
- Colored feedback banner between the question and the answer buttons
  (green "Correct!" / red "Wrong - correct answer was: …")
- All answer buttons disabled during the 1.4-second feedback window (no accidental double-clicks)
- Progress indicator ("Question X / Y") in the header
- Score display updates live as questions are answered
- Larger dialog (700 × 580 px vs original 320 × 200 px)
- `ArmaReforgerScripted.GetCallqueue().CallLaterByName()` used for the timed advance

**Key Files Modified/Created:**
- `UI/layouts/QuizDialog_UI.layout` — active layout, created manually in Workbench
- `Scripts/Game/SRQ_QuizDialog.c` — fully rewritten with enhanced logic

**Architecture notes:**
- `ScrollLayoutWidget` was dropped — not supported by the MCP layout tool; rules are displayed
  in a plain `RichTextWidget "RulesText"` instead
- `UI/Textures/QuizIcons.imageset` was dropped — `SetColorInt()` on widgets achieves the same goal
- Layout files must be created directly in Workbench (not via `project_write`) to ensure slot class
  GUIDs are registered in `resourceDatabase.rdb`; `wb_resources rebuild` must never be run on layouts

### Phase 3: Admin Features — COMPLETE
Server administration tools for managing the quiz system.

**Features Built:**
- Admin command backend for resetting a connected player's quiz state and viewing quiz statistics
- Configurable retry limit with automatic disconnect after too many failed attempts
- Centralized quiz attempt logging and persistent aggregate statistics
- Config option to require the quiz on every connect or only until the player first passes
- Server-side RPC submission for quiz results and admin actions

**Key Files Modified/Created:**
- `Scripts/Game/SRQ_QuizLogger.c` — persistent stats tracker and server log helper
- `Scripts/Game/SRQ_QuizManagerComponent.c` — retry-limit handling, runtime config creation, automatic global service bootstrap, and admin command backend
- `Scripts/Game/SRQ_ModdedPlayerController.c` — server RPC bridge for quiz status, quiz results, config delivery, and admin command responses
- `Scripts/Game/SRQ_ModdedBaseGameMode.c` — automatic server startup hook on the default base game mode
- `Scripts/Game/SRQ_QuizDialog.c` — converted to a configurable-dialog-based UI handler
- `Scripts/Game/SRQ_PlayerStateManager.c` — adds passed-player counting and identity reset helpers
- `Scripts/Game/SRQ_QuizConfig.c` — adds retry-limit and repeat-on-connect config fields plus JSON import/export helpers
- `Config/ServerRulesConfig.json` — packaged default config copied into the server profile on first start
- `Config/SRQ_QuizDialogPresets.conf` — dialog preset definition used to open the quiz as a real dialog/menu

## Architecture Notes

### Class Prefix
All classes use the `SRQ_` prefix (Server Rules Quiz).

### Key Systems

**Config Loading:**
- `SRQ_QuizManagerComponent` is now a plain singleton service; it no longer needs to be attached to a gamemode entity
- `modded class SCR_BaseGameMode` calls the singleton during `OnGameStart()` so server startup always initializes the service
- The service creates `$profile:ServerRulesQuiz/ServerRulesConfig.json` automatically on first start
- The packaged `Config/ServerRulesConfig.json` is copied into the profile when available; otherwise built-in defaults are written directly
- Runtime config is always loaded from the server profile so admins can edit one stable file between restarts
- Questions are dynamic - add as many as needed
- `passingScore` determines minimum correct answers required
- `maxAttemptsBeforeKick` disconnects a player after that many failed attempts (`0` disables the limit)
- `showQuizOnEveryConnect` forces the quiz every join instead of only for first-time players

**Automatic Bootstrap:**
- The mod no longer depends on manually adding a gamemode component
- `SCR_BaseGameMode.OnGameStart()` bootstraps the service and emits startup logs
- `SCR_BaseGameMode.OnPlayerConnected()` logs whether each player still needs the quiz
- `SCR_PlayerController.OnControlledEntityChanged()` triggers quiz check on entity spawn (client-side secondary trigger)
- The primary trigger is the polling loop started by `SRQ_RpcAuditReady` — see Quiz Flow
- The server singleton decides whether the player needs the quiz and returns the current config as JSON via RPC
- The owner client deserializes config and opens the quiz dialog locally

**"Is local controller" check:**
- `GetGame().GetPlayerController()` is unreliable across dedicated-server / peer-client boundaries
  and must NOT be used to identify the local player's controller
- All checks use `SCR_PlayerController.s_pLocalPlayerController == this` via the helper `SRQ_IsLocalController()`
- `s_pLocalPlayerController` is the authoritative static field Bohemia sets for the owning client's controller;
  it is null on a dedicated server (no local human player) and on proxy instances, so the check correctly
  filters out all non-local-player code paths

**Player State:**
- Stored at `$profile:ServerRulesQuiz/PlayerState.json` on server
- Uses player identity ID (persistent across sessions)
- `SRQ_PlayerStateManager` is a singleton managing all state

**Quiz Flow:**
1. Game mode startup triggers `SRQ_QuizManagerComponent.Bootstrap()`
2. Server creates/loads runtime config and writes startup logs
3. `OnPlayerAuditSuccess` → `MarkPlayerAudited` → `SRQ_RpcAuditReady` sent to owner client
4. Client receives `SRQ_RpcAuditReady` → sets `m_bSRQAuditReady = true` → calls `SRQ_CheckQuizRequired()`
5. If the player has a controlled entity → `SRQ_RpcRequestQuizStatus()` sent to server immediately
6. If no entity yet → `SRQ_QueueStatusRetry()` starts a 500 ms poll loop until the entity is ready
7. `OnControlledEntityChanged` also calls `SRQ_CheckQuizRequired()` as a secondary trigger (fires on client when entity changes)
8. Server replies with `needsQuiz` + config JSON via `SRQ_RpcReceiveQuizStatus`
9. Owner client opens `SRQ_QuizDialog` via `SCR_ConfigurableDialogUi.CreateFromPreset()` with preset tag `srq_quiz`
10. On completion → `SCR_PlayerController.SRQ_RpcSubmitQuizResult()`
11. On pass → `SRQ_PlayerStateManager.SetPlayerPassed()` → persists to file
12. On repeated failure → retry limit handler disconnects the player owner through RPC

**Admin Commands:**
- `/srq` or `/srqhelp` shows available quiz admin commands
- `/srqstats` returns persistent totals plus current connected/pending state
- `/srqreset <playerId|exact name>` clears a connected player's pass state and re-requires the quiz
- Command execution is backed by `SRQ_QuizManagerComponent.HandleAdminCommand()`

**Logging and Stats:**
- `SRQ_QuizLogger` stores aggregate counters at `$profile:ServerRulesQuiz/QuizStats.json`
- Every attempt is printed to the server log with player, score, and attempt count
- Admin resets and retry-limit disconnects are also logged and counted

**UI:**
- `SRQ_QuizDialog` now inherits from `SCR_ConfigurableDialogUi` instead of using a raw HUD overlay
- `Config/SRQ_QuizDialogPresets.conf` defines the dialog preset tag `srq_quiz`
- The preset is opened with `SCR_ConfigurableDialogUi.CreateFromPreset()` so the quiz uses real dialog/menu input capture
- Quiz widgets still live in `UI/layouts/QuizDialog_UI.layout`
- Rules phase: `RichTextWidget "RulesText"` (visible) + all quiz widgets hidden
- Quiz phase: `RichTextWidget "ContentText"` (question) + 4 `ButtonWidget "Answer0–3"`
- Feedback: `ImageWidget "FeedbackBanner"` + `TextWidget "FeedbackText"` (hidden until answer)
- Advance after answer: `ArmaReforgerScripted.GetCallqueue().CallLaterByName(this, "AdvanceQuestion", 1400, false)`
- Color constants (ARGB): `COLOR_BTN_CORRECT = 0xFF1B5E20`, `COLOR_BTN_WRONG = 0xFFB71C1C`

### Config File Format
```json
{
  "rules": ["Rule 1", "Rule 2", ...],
  "passingScore": 2,
  "maxAttemptsBeforeKick": 3,
  "showQuizOnEveryConnect": false,
  "questions": [
    {
      "question": "Question text?",
      "answers": ["A", "B", "C", "D"],
      "correctIndex": 0
    }
  ]
}
```

## Changes Outside Plan
- Removed the manual gamemode-component requirement by converting the quiz backend into an automatically initialized singleton service.
- Added a `SCR_BaseGameMode` startup hook so server boot always creates the runtime config and emits SRQ logs.
- Replaced the raw widget-overlay quiz opening path with a configurable dialog preset so the quiz can use real menu/dialog input handling.
- **Bugfix (2026-03-16):** Fixed missing braces in `OnControlledEntityChanged` (bare `return` always executed). Replaced `GetGame().GetPlayerController() != this` with `SCR_PlayerController.s_pLocalPlayerController != this` (via helper `SRQ_IsLocalController()`) everywhere in the file — the former is unreliable on dedicated server + peer client. Added polling fallback in `SRQ_CheckQuizRequired`: when audit is ready but `GetControlledEntity()` is null (player not yet spawned), `SRQ_QueueStatusRetry()` is called to retry every 500 ms until the entity is available.
