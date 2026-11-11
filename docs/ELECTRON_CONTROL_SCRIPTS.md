# Electron Control scripts

While the **Electron Control** (item 34128) runs, the client can ask a Lua script what the player and the pet do on each battle
turn, ahead of the Remote Control Plus settings. The plain Remote Control (34058) never runs a script.

## The file

The script is `sd<id xor 7654321>.lua` in the per-player user folder, beside the Remote Control's `sd<…>.asd` file. It is read
each time the Electron Control is started, so to reload an edited script, stop the remote and start it again. "Electron Control
script loaded" shows when it loaded; a load error shows on screen instead, and the Plus settings are used.

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
| `b.can_use(skill, target)` | true when the commanding fighter may use the skill on the target now |
| `b.has_item(item)` | true when the bag holds the item |

### A fighter

`slot`, `side`, `row`, `col`, `kind` (2 a player, 4 a pet), `id`, `owner`, `name`, `hp`, `max_hp`, `sp`, `max_sp`, `hp_pct`,
`sp_pct` (0..100), `level`, `element`, `alive`, `statuses` (skill ids shown as status icons), `status_kinds` (main status kinds 1..8).

### Helpers

`filter(list, fn)`, `alive(list)`, `fallen(list)`, `lowest(list, key)`, `highest(list, key)`, `has_status(fighter, skill)`;
`print(...)` shows a line on screen.

## Limits

Scripts can't read or write files or load other code. Only Lua's base, `table`, `string` and `math` libraries are there. A script may hold 16 MB, and loading it or one call of `decide` may run 2,000,000 Lua instructions; past that
it fails with an error.

## Example

See [examples/electron-control.lua](examples/electron-control.lua).
