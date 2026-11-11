-- Electron Control example: heal the most hurt ally, else hit the weakest enemy.
-- Skill and item ids are examples; put in the ones your character and pet have.

local HAND_ATTACK = 10001
local HEAL = 11077        -- Cure 2 Players
local HP_BOTTLE = 34298

function decide(b)
  -- heal an ally under 40% HP
  local hurt = lowest(alive(b.allies), "hp_pct")
  if hurt and hurt.hp_pct < 40 then
    if b.can_use(HEAL, hurt) then
      return { skill = HEAL, target = hurt }
    end
    if b.actor == "player" and b.has_item(HP_BOTTLE) then
      return { item = HP_BOTTLE, target = hurt }
    end
  end

  -- count each fighter's turns this battle, to show b.memory
  b.memory[b.actor] = (b.memory[b.actor] or 0) + 1

  -- attack the enemy with the least HP
  local foe = lowest(alive(b.enemies), "hp")
  if foe then
    return { skill = HAND_ATTACK, target = foe }
  end
  return nil
end
