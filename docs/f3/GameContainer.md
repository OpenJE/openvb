# F3::GameContainer

[Return to `F3`](/docs/f3.md)

## C++

- [`GameContainer.hpp`](/src/f3/GameContainer.hpp)
- [`GameContainer.cpp`](/src/f3/GameContainer.cpp)

## References

- [`JE::DynamicObject`](https://github.com/OpenJE/openje/docs/je/DynamicObject.md)
- [`JE::Entity`](https://github.com/OpenJE/openje/docs/je/Entity.md)
- [`F3::GameEntity`](/docs/f3/GameEntity.md)
- [`F3::GameUsableObject`](/docs/f3/GameUsableObject.md)

## Inheritance

```mermaid
graph TD;
    JE::DynamicObject --> JE::Entity
    JE::Entity --> F3::GameEntity
    F3::GameEntity --> F3::GameUsableObject
    F3::GameUsableObject --> F3::GameContainer
```

[Return to `F3`](/docs/f3.md)