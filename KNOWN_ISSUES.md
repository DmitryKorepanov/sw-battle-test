# Known Issues and Limitations

## Architecture
- **Implicit Targeting Logic:** Target selection currently relies on the presence of `HealthComponent`. This means any unit with health is automatically a valid target. Future extensions (Towers, Mines, etc.) might require explicit `AttackableComponent` or tags to distinguish between "destructible" and "valid target for AI".
- **Event Emission Placement:** Event emission is split between `Behaviors` (attack, move, march-ended) and the orchestration layer (`main.cpp`) for unit death (after cleanup). `GameWorld` no longer stores an event sink, but a future refinement could unify this further by introducing a dedicated "tick" layer that owns both state transitions and event emission in one place.
- **Double Position Storage:** Unit position is stored in both `Unit::_position` and `GameWorld::_grid` (implicitly by index). While `GameWorld` manages consistency via `moveUnit` and `friend` access, this duplication requires careful maintenance.

## Implementation Details
- **Damage Encapsulation:** `utils::dealDamage` modifies `HealthComponent` directly. While it centralizes logic, moving this into a `Unit::takeDamage` method or a `DamageSystem` would improve encapsulation.
- **TypeRegistry Edge Cases:** `TypeRegistry::remove<T>` must be called with the **concrete type** used for registration. Removing by an interface type is not supported (it will only erase that interface key), which can leave **stale alias entries** that still resolve via `get<Interface>()`.
- **Randomness:** The `Random` class uses a global generator. For deterministic replayability or better testing, this should be replaced with an injected RNG service.
