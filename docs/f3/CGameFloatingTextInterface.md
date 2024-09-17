# F3::CGameFloatingTextInterface

[Return to `F3`](/docs/f3.md)

## C++

- [`CGameFloatingTextInterface.hpp`](/src/f3/CGameFloatingTextInterface.hpp)
- [`CGameFloatingTextInterface.cpp`](/src/f3/CGameFloatingTextInterface.cpp)

## References

- [`JE::HeapTracker`](https://github.com/OpenJE/openje/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](https://github.com/OpenJE/openje/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](https://github.com/OpenJE/openje/docs/je/CEngineInterface.md)
- [`JE::CFloatingTextInterface`](https://github.com/OpenJE/openje/docs/je/CFloatingTextInterface.md)
- [`F3::CGameInterface`](/docs/f3/CGameInterface.md)

## Inheritance

```mermaid
graph TD;

    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CFloatingTextInterface
    JE::CEngineInterface --> F3::CGameInterface
    JE::CFloatingTextInterface --> F3::CGameFloatingTextInterface
    F3::CGameInterface --> F3::CGameFloatingTextInterface
```

[Return to `F3`](/docs/f3.md)