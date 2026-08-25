# Workshop Summary

Server-authoritative text chat control for Game Masters and server admins. Enable or disable individual chat channels with simple commands while leaving voice chat unchanged.

# Workshop Description

Disable Global Chat gives Game Masters and server admins full control over which text chat channels regular players may use.

By default, only group chat is enabled. Authorized Game Masters and server admins always retain full access to every available text chat channel.

## Features

- Server-authoritative channel restrictions
- Immediate synchronization for connected players
- Individual controls for global, faction, group, local, vehicle, and direct chat
- Visible chat confirmations for every change and status request
- Localized feedback in all 13 languages supported by Arma Reforger
- Automatic protection against unauthorized sending attempts
- No hard-coded player names, IDs, or admin slots
- Voice chat remains completely unchanged

## Commands

```text
#chat global
#chat faction
#chat group
#chat local
#chat vehicle
#chat direct
#chat status
#chat all
#chat none
```

Run `#chat <channel>` again to disable that channel.

Only currently authorized Game Masters and server admins can use these commands. Regular players receive a clear message when attempting to use a restricted channel.

Tested successfully on a dedicated server.
