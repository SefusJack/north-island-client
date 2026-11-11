# Electron Control scripts

While the **Electron Control** (item 34128) runs, the client can ask a Lua script what the player and the pet do on each battle
turn, ahead of the Remote Control Plus settings. The plain Remote Control (34058) never runs a script.

## The file

Choose the script in game: open the Electron Control's Level-up Setting (the Plus window), go to its third tab, type or paste
the file's path in the **Script** box and press OK. A relative path is taken from the user folder. Each character keeps its own
choice (in `sd<id xor 7654321>.script` in the user folder). With the box empty, the script is `sd<id xor 7654321>.lua` in the
user folder, beside the Remote Control's `sd<…>.asd` file.

The script is read when the Electron Control starts and when you press OK in the Plus window while it runs, so after editing
the file, press OK again to reload it. "Electron Control script loaded" shows when it loaded; a load error, or "Script not
found", shows on screen instead, and the Plus settings are used.

## Debug mode

Tick **Debug** beside the Script box and press OK. Then errors, the script's `print()` lines and each turn's choice
("player: Hagendis Attack (15044) on slot 12", or "... cannot be used now: the Plus settings decide") also go to the chat's
system lines and to the log (`north-island-client.log` in the user folder). With Debug off, `print()` shows nothing and errors
only show briefly on screen.

## decide(b)

The script defines `decide(b)`, which is called once for each of your fighters when it is their turn to command. It returns:

```lua
return { skill = 10001, target = foe }   -- a skill on a fighter (a fighter table or a slot number)
return { item = 34298, target = b.self } -- a bag item on a fighter
return nil                               -- no choice: the Plus settings decide, then the plain auto attack
```

The client checks the choice the same way it checks the Plus settings (sealed, SP, whom the skill may reach). When the choice can't
be used, or `decide` raises an error (the error shows on screen), the Plus settings decide instead.

### b

| Field | Meaning |
| --- | --- |
| `b.actor` | `"player"` or `"pet"`: who is commanding |
| `b.self` | the commanding fighter |
| `b.player`, `b.pet` | your player and your fighting pet (nil when not in the battle) |
| `b.fighters`, `b.allies`, `b.enemies` | lists of fighters (allies are on the commanding fighter's side) |
| `b.battle`, `b.round` | battle counter and command round (from 1) |
| `b.memory` | a table kept for the whole battle (also the global `memory`), emptied when a new battle starts |
| `b.can_use(skill, target)` | true when the commanding fighter may use the skill on the target now: learnt (a role stunt only the role's own), SP enough, not sealed, a target the skill may reach (side, alive or fallen) |
| `b.has_item(item)` | true when the bag holds the item |
| `b.skill_level(skill)` | the level the commanding player learnt the skill to, whatever the SP; 0 when not learnt, and always 0 for a pet |

### A fighter

`slot`, `side`, `row`, `col`, `kind` (2 a player, 4 a pet), `id`, `owner`, `spawn` (differs for each fighter that enters the
battle, so a monster that takes a fallen one's slot has a new one), `name`, `hp`, `max_hp`, `sp`, `max_sp`, `hp_pct`,
`sp_pct` (0..100), `level`, `element`, `alive`, `statuses` (skill ids shown as status icons), `status_kinds` (main status kinds 1..8).

### Helpers

`filter(list, fn)`, `alive(list)`, `fallen(list)`, `lowest(list, key)`, `highest(list, key)`, `has_status(fighter, skill)`;
`print(...)` writes a line to the chat and the log in debug mode.

## Limits

Scripts can't read or write files or load other code. Only Lua's base, `table`, `string` and `math` libraries are there. A script may hold 16 MB, and loading it or one call of `decide` may run 2,000,000 Lua instructions; past that
it fails with an error.

## Example

See [examples/electron-control.lua](examples/electron-control.lua).
