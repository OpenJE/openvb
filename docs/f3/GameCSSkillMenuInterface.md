# f3::GameCSSkillMenuInterface

[Return to `f3`](/docs/f3.md)

## C++

- [`GameCSSkillMenuInterface.hpp`](/c++/include/GameCSSkillMenuInterface.hpp)
- [`GameCSSkillMenuInterface.cpp`](/c++/source/GameCSSkillMenuInterface.cpp)

## References

- [`f3::HeapTracker`](/docs/f3/HeapTracker.md)
- [`f3::GUI::Interface`](/docs/f3/GUI/Interface.md)
- [`f3::CEngineInterface`](/docs/f3/CEngineInterface.md)
- [`f3::CSMenuInterface`](/docs/f3/CSMenuInterface.md)
- [`f3::CGameInterface`](/docs/f3/CGameInterface.md)

## Inheritance

```mermaid
graph TD;
    f3::HeapTracker --> f3::GUI::Interface
    f3::GUI::Interface --> f3::CEngineInterface
    f3::CEngineInterface --> f3::CSMenuInterface
    f3::CEngineInterface --> f3::CGameInterface
    f3::CSMenuInterface --> f3::GameCSSkillMenuInterface
    f3::CGameInterface --> f3::GameCSSkillMenuInterface
```

[Return to `f3`](/docs/f3.md)