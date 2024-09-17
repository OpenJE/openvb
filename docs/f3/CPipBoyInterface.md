# F3::CPipBoyInterface

[Return to `F3`](/docs/f3.md)

## C++

- [`CPipBoyInterface.hpp`](/src/f3/CPipBoyInterface.hpp)
- [`CPipBoyInterface.cpp`](/src/f3/CPipBoyInterface.cpp)

## References

- [`JE::HeapTracker`](https://github.com/OpenJE/openje/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](https://github.com/OpenJE/openje/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](https://github.com/OpenJE/openje/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> F3::CPipBoyInterface
```

[Return to `F3`](/docs/f3.md)