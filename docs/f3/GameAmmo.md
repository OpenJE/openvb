# F3::GameAmmo

[Return to `F3`](/docs/f3.md)

## C++

- [`GameAmmo.hpp`](/src/f3/GameAmmo.hpp)
- [`GameAmmo.cpp`](/src/f3/GameAmmo.cpp)

## References

- [`JE::DynamicObject`](https://github.com/OpenJE/openje/docs/je/DynamicObject.md)
- [`JE::Entity`](https://github.com/OpenJE/openje/docs/je/Entity.md)
- [`F3::GameEntity`](/docs/f3/GameEntity.md)
- [`F3::GameItem`](/docs/f3/GameItem.md)

## Inheritance

```mermaid
graph TD;
    JE::DynamicObject --> JE::Entity
    JE::Entity --> F3::GameEntity
    F3::GameEntity --> F3::GameItem
    F3::GameItem --> F3::GameAmmo
```

[Return to `F3`](/docs/f3.md)