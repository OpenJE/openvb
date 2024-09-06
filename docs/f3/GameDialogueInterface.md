# f3::GameDialogueInterface

[Return to `f3`](/docs/f3.md)

## C++

- [`GameDialogueInterface.hpp`](/src/f3/GameDialogueInterface.hpp)
- [`GameDialogueInterface.cpp`](/src/f3/GameDialogueInterface.cpp)

## References

- [`f3::HeapTracker`](/docs/f3/HeapTracker.md)
- [`f3::GUI::Interface`](/docs/f3/GUI/Interface.md)
- [`f3::CEngineInterface`](/docs/f3/CEngineInterface.md)
- [`f3::DialogueInterface`](/docs/f3/DialogueInterface.md)
- [`f3::CGameInterface`](/docs/f3/CGameInterface.md)

## Inheritance

```mermaid
graph TD;
    f3::HeapTracker --> f3::GUI::Interface
    f3::GUI::Interface --> f3::CEngineInterface
    f3::CEngineInterface --> f3::DialogueInterface
    f3::CEngineInterface --> f3::CGameInterface
    f3::DialogueInterface --> f3::GameCSSkillMenuInterface
    f3::CGameInterface --> f3::GameCSSkillMenuInterface
```

[Return to `f3`](/docs/f3.md)