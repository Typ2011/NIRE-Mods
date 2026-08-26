# Server Rules Quiz Workshop Copy

## Summary
Require players to read your server rules and pass a configurable quiz before they can fully clear server access, with persistent tracking, admin reset tools, live quiz stats, and optional retry-limit enforcement.

## Description
Server Rules Quiz adds a clean onboarding gate for Arma Reforger servers that need players to acknowledge rules before settling in.

Players are shown your custom rules, then a multiple-choice quiz loaded from JSON. Passing players are tracked by identity so they do not need to repeat the process every session unless you enable repeat-on-connect behavior.

The mod includes:
- Configurable rules, questions, answers, and passing score
- Persistent pass tracking across sessions
- Polished quiz UI with progress, score, and answer feedback
- Admin chat commands for quiz stats and player reset
- Configurable retry limits with automatic disconnect after repeated failures
- Server-side logging of attempts, resets, and retry-limit removals

Admin commands:
- `/srq` or `/srqhelp` - show available quiz admin commands
- `/srqstats` - show aggregate quiz statistics and current server state
- `/srqreset <playerId|exact name>` - clear a connected player's quiz clearance and require the quiz again

Config options:
- `passingScore`
- `maxAttemptsBeforeKick`
- `showQuizOnEveryConnect`
- `rules[]`
- `questions[]`

Ideal for PvE, milsim, realism, training, and community servers that want a lightweight rules gate without building a custom scenario flow from scratch.
