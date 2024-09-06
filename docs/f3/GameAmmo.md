# f3::GameAmmo

[Return to `f3`](/docs/f3.md)

## C++

- [`GameAmmo.hpp`](/c++/include/GameAmmo.hpp)
- [`GameAmmo.cpp`](/c++/source/GameAmmo.cpp)

## References

- [`f3::DynamicObject`](/docs/f3/DynamicObject.md)
- [`f3::Entity`](/docs/f3/Entity.md)
- [`f3::GameEntity`](/docs/f3/GameEntity.md)
- [`f3::GameItem`](/docs/f3/GameItem.md)

## Inheritance

```mermaid
graph TD;
    f3::DynamicObject --> f3::Entity
    f3::Entity --> f3::GameEntity
    f3::GameEntity --> f3::GameItem
    f3::GameItem --> f3::GameAmmo
```

[Return to `f3`](/docs/f3.md)