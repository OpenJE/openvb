# f3::GameContainer

[Return to `f3`](/docs/f3.md)

## C++

- [`GameContainer.hpp`](/c++/include/GameContainer.hpp)
- [`GameContainer.cpp`](/c++/source/GameContainer.cpp)

## References

- [`f3::DynamicObject`](/docs/f3/DynamicObject.md)
- [`f3::Entity`](/docs/f3/Entity.md)
- [`f3::GameEntity`](/docs/f3/GameEntity.md)
- [`f3::GameUsableObject`](/docs/f3/GameUsableObject.md)

## Inheritance

```mermaid
graph TD;
    f3::DynamicObject --> f3::Entity
    f3::Entity --> f3::GameEntity
    f3::GameEntity --> f3::GameUsableObject
    f3::GameUsableObject --> f3::GameContainer
```

[Return to `f3`](/docs/f3.md)