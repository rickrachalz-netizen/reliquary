# RELIQUARY — Gameplay Framework

This document describes the C++ gameplay framework built from the RELIQUARY design
doc, and how to wire it up in the Unreal Editor. The code is intentionally
content-agnostic (no hard asset references) so designers own the tuning and the
look, while the systems and rules live in code.

Everything lives under `Source/RELIQUARY/Game/` (subsystems, data, progression)
and `Source/RELIQUARY/Game/World/` (placeable actors).

---

## The core loop, in code

```
Base Camp (L_Lobby)  ──Embark──►  Run maps (L_Run, procedural)
      ▲                                   │
      │                          gather from ARLResourceNode
      │                          spend ExcessMana at ARLPowerAltar (boons)
      │                          bank every 3 maps (ARLBankingCrate)
      │                                   │
      └──Extract / Die────  ARLChallengeAltar (charge + boss) ──Onward──┘
```

- **Extract** → keep the run bag, gain XP/essence, all run power stripped, autosave.
- **Die** → forfeit the run bag, run power stripped, respawn at base camp.

The run's RNG seed is **rolled and locked at embark** (`URLRunSubsystem::Embark`)
so a map can't be re-fished by restarting.

---

## Subsystems (auto-created on the GameInstance)

| Subsystem | Responsibility |
|---|---|
| `URLItemRegistry` | Loads the `URLGameData` content library and resolves ids → definitions. |
| `URLRunSubsystem` | The whole run lifecycle: embark, 10 levels, run bag, boons, excess mana, banking, extract/death. |
| `URLCraftingSubsystem` | Turns gathered materials into rolled gear. |
| `URLProfileSubsystem` | Save/load of the persistent hero (autosaves in base camp). |

Because they're `UGameInstanceSubsystem`s, they exist for free — no Blueprint
wiring needed. Access them from BP with `Get Game Instance Subsystem`.

---

## Progression

`ARLPlayerState` is the character sheet: class, spec, level (1–30), an
**exponential XP curve** (`GetExperienceForLevel`), talent points, and a
Heart-of-the-God essence track. At max level `GetDisplayClassName()` returns the
**evolved, spec-derived title** ("Ashen Pyromancer", etc.).

`ApplyStatsToOwner()` is the stat aggregator: it recomputes final attributes from
**class/level base + equipped gear + active run boons** and pushes them onto the
pawn's `URLAttributeSet`. Pass `bResetVitals=false` for mid-run changes so a boon
doesn't heal you.

`URLAttributeSet` implements the wide stat set (Strength/Agility/Intellect,
Crit/Haste/Adaptability, AttackPower/SpellPower, Mana, MoveSpeed, and the
`ExcessMana` run currency), plus a meta `IncomingDamage`/`IncomingHealing`
channel for GameplayEffects.

---

## Data (author as assets)

Create these as data assets and register them all in one `URLGameData` asset,
then point **Project Settings → Game → RELIQUARY → GameData** at it.

- `URLClassDefinition` — base stats + per-level growth for Warrior/Rogue/Mage.
- `URLTalentTree` — a spec's talent nodes (grant tags + flat stats).
- `URLMaterialDefinition` — one per distinct material (oak, ironwood, feywood…),
  with its `SourceZones` so routes are plannable.
- `URLGearDefinition` — gear templates (slot, base modifiers, cantrip/set tags).
- `URLRecipe` — ingredients → output gear; supports specific-material *or*
  whole-family ingredients.
- `URLBoonDefinition` — run-power upgrades bought at power altars.

---

## Placeable world actors

| Actor | Notes |
|---|---|
| `ARLResourceNode` | Harvestable/destructible. Implements `ICombatDamageable`, so the existing melee/ability traces already shatter it. Set `MaterialId` + yield. Drops into the run bag and grants excess mana. |
| `ARLChallengeAltar` | Interact to `ActivateChallenge` (spawns `BossClass`); charge by standing in radius while the boss lives. When charged + boss dead, call `ChooseExtract` / `ChooseOnward` from UI. |
| `ARLPowerAltar` | Interact opens a deterministic boon offer (`GetOffer`); `PurchaseFromOffer` spends excess mana. One purchase per altar. |
| `ARLBankingCrate` | Interact ships the current run bag home mid-run. |

All four implement `IRLInteractable`. The player character exposes
`DoInteract()` (a short sphere-sweep in front of the character) — bind it to an
input action.

---

## Editor wiring checklist

1. **Build** the C++ (the `RELIQUARY` module now depends on `DeveloperSettings`).
2. Set the level GameMode to a `ARLGameMode` child (installs `ARLPlayerState`).
   Mark run maps `bIsRunMap = true`.
3. Author the data assets above; collect them into one `URLGameData`; assign it
   in Project Settings (or `DefaultGame.ini`, section already stubbed).
4. Place `ARLResourceNode`s (with meshes + material ids) around run maps.
5. Place one `ARLChallengeAltar` per run map with a `BossClass` and `NextZone`;
   have the boss call `NotifyBossDefeated()` on death.
6. Place `ARLPowerAltar`s (unique `AltarId`s) and, where offered, an
   `ARLBankingCrate`.
7. In base camp, wire a character-creation UI to
   `URLProfileSubsystem::CreateNewHero`, and an embark interactable to
   `URLRunSubsystem::Embark`.
8. Build the minimalist HUD off the subsystem delegates (`OnRunPhaseChanged`,
   `OnRunBagChanged`, `OnBoonsChanged`, `OnExperienceChanged`, …).

---

## Design-doc coverage

- Exponential power growth — gear aggregation + stacking boons + essence track.
- Fun, legible combat — GAS attribute set with readable stats (incl. Adaptability).
- Persistent heroes — `ARLPlayerState` + `URLSaveGame` + autosave.
- Runs & extraction — `URLRunSubsystem` (10 levels, seed lock, banking, forfeit).
- Temporary run power — boons + excess mana, wiped on return to base camp.
- Crafting & distinct materials — recipes, per-zone materials, quality-driven rolls.
- Inventory — `URLInventoryComponent` (stash, gear, equipment, recipes).

Combat feel, procedural map generation, the talent-tree *UI*, and final-boss
tuning are left as content/Blueprint work on top of these systems.
