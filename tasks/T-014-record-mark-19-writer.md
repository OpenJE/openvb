---
status: done
assignee: FastArrow (research)
---

# T-014 — Who writes the sub-attack record's `+0x19` mark?

**Closed** (B5, `a252575`): it is not an independent mark. `CalculateAndQueueDamage_0x404060`
snapshots the action's `+0x71` unarmed flag into each queued hit record at insert (@0x4041A1, via
`v22`@esp+25h onto node+0x19, damage anchored at node+0x08). So the finisher's `local_11` is
">= 2 **unarmed** records queued", and `eff+0xAC = 1` selects the flurry pair {38,39}; weapon
volleys never take it.

Consumed by T-012. Corroborated three ways - FastArrow's write site, the coordinator's raw check,
and NiceWolf's independent map of the consumer fork @0x4040a8/0x4040d4. Anti-chasing notes: the
`+0x19` stores at 0x404B4F/0x404D03 are InitiateDialogue clone-and-requeue offset aliasing, not
volley records; the 0x40386B string-select byte remains unverified.
