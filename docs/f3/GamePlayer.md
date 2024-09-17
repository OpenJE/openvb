# F3::GamePlayer

[Return to `F3`](/docs/f3.md)

## C++

- [`GamePlayer.hpp`](/src/f3/GamePlayer.hpp)
- [`GamePlayer.cpp`](/src/f3/GamePlayer.cpp)

## References

- [`JE::DynamicObject`](https://github.com/OpenJE/openje/docs/je/DynamicObject.md)
- [`JE::Entity`](https://github.com/OpenJE/openje/docs/je/Entity.md)
- [`F3::GameEntity`](/docs/f3/GameEntity.md)
- [`F3::GameCreature`](/docs/f3/GameCreature.md)

## Inheritance

```mermaid
graph TD;
    JE::DynamicObject --> JE::Entity
    JE::Entity --> F3::GameEntity
    F3::GameEntity --> F3::GameCreature
    F3::GameCreature --> F3::GamePlayer
```

[Return to `F3`](/docs/f3.md)