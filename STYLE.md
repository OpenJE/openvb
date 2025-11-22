---

# 0. Goals & Constraints

- **Goal:** Produce C++ code that looks and feels like Black Isle’s Jefferson / Van Buren engine circa 2002–2003.
- **Target toolchain:** MSVC, C++98 dialect, Windows + DirectX (8/8.1 era).
- **Philosophy:** Pragmatic, engine‑style C++:
  - Namespaced, object‑oriented.
  - Manual memory management (`new`/`delete`).
  - Uses STL containers (`std::vector`, `std::map`, etc.).
  - Minimal exceptions in engine paths; errors via return codes + logging.
  - Naming, error messages, and subsystems consistent with the strings in the binary.

---

# 1. Project Structure

## 1.1 Files

- Use **paired `.h`/`.cpp` files** for each major class or subsystem.
  - Example:
    - `GameEntity.h` / `GameEntity.cpp`
    - `Gfx_Texture.h` / `Gfx_Texture.cpp`
    - `VFX_ParticleSystem.h` / `VFX_ParticleSystem.cpp`
- Use `#pragma once` at the top of all headers (VC style):

  ```cpp
  #pragma once
  ```

- Use a precompiled header (e.g. `stdafx.h`) included **first in every `.cpp`**:

  ```cpp
  #include "stdafx.h"
  #include "GameEntity.h"
  #include "GameWeapon.h"
  ```

## 1.2 Namespaces & Modules

Create namespaces for the major subsystems seen in the strings:

- `namespace Game { ... }` – core gameplay:
  - `GameEntity`, `GameCreature`, `GameWeapon`, `GameWorldMap`, `GameJournal`, `GameClient`, `GameServer`, etc.
- `namespace Gfx { ... }` – graphics abstraction:
  - `Gfx_Engine`, `Gfx_Texture`, `Gfx_Texture_Manager`, `Gfx_RenderManager`, `Gfx_AnimationObject`, `Gfx_AnimationData`, `Gfx_AnimMeshInstance`, etc.
- `namespace G3D { ... }` – 3D scene and mesh:
  - `G3D_Mesh`, `G3D_Node`, `G3D_Camera`, `G3D_DirectionalLight`.
- `namespace VFX { ... }` – particle and visual effects:
  - `VFX_Effect`, `VFX_EffectInstance`, `VFX_ParticleSystem`, `VFX_VEGInstance`, `VFX_Rain`, `VFX_Snow`, `VFX_Wind`.
- `namespace GUI { ... }` – user interface framework:
  - `GUI::Window`, `GUI::Button`, `GUI::Label`, `GUI::Picture`, etc.
- `namespace net { ... }` – networking:
  - `net::server`, `net::client`, `net::connection`, `net::socket`, `net::packet`, `net::buffer`, etc.
- `namespace Common { ... }` – utilities (chunks, polygons, exceptions, etc.).
- `namespace SS { ... }` – string system:
  - `SS::StringSystem`, `SS::EngineStringSystem`.

**Guideline:**
Put classes in one of these namespaces and/or give them the appropriate prefix:

- Gameplay: `GameSomething`
- Graphics: `Gfx_Something`
- VFX: `VFX_Something`
- 3D: `G3D_Something`
- GUI: `GUI::Something` or `CWhateverInterface` (see below).

---

# 2. Formatting & Layout

## 2.1 Indentation and braces

- Indent with **tabs**, NOT spaces.
- Use **Allman** brace style:

  ```cpp
  if (bIsVisible)
  {
      Render();
  }
  else
  {
      // ...
  }
  ```

- One class per header, one namespace per file where practical.

## 2.2 Line length and spacing

- Aim for **≤ 100 characters** per line.
- Put spaces:
  - After keywords: `if (`, `while (`, `for (`.
  - Around binary operators: `a + b`, `x == y`.
  - No spaces just inside parentheses: `Func(a, b)` not `Func( a, b )`.

Examples:

```cpp
for (int nIndex = 0; nIndex < nCount; ++nIndex)
{
    float fAlpha = 0.0f;
    // ...
}
```

---

# 3. Naming Conventions

## 3.1 Types (classes, structs, enums)

- **Classes/structs:** PascalCase.
- **Subsystem prefixes** are common:
  - `GameEntity`, `GameWeapon`, `GameActionAttack`, `GameAnimationCache`.
  - `Gfx_Texture`, `Gfx_Engine`, `Gfx_RenderManager`, `Gfx_AnimationObject`.
  - `G3D_Mesh`, `G3D_Node`, `G3D_Camera`.
  - `VFX_Effect`, `VFX_ParticleSystem`, `VFX_Rain`.
  - `StringSystem`, `EngineStringSystem`.
- **GUI and “interface” classes** use `C` prefix:

  ```cpp
  class CEngineInterface;
  class CPipBoyInterface;
  class CMainmenuInterface;
  class CInventoryInterface;
  class CActionbarInterface;
  class COptionsVideoInterface;
  ```

- **Enums:**
  - Type names: `VFX_EffectType`, `VFX_WaterMotionType`, `VFX_CollisionType`, `VFX_LightType`, `GFX_RENDER_PHASE`, etc.
  - Enum values: ALL_CAPS with subsystem prefix:

    ```cpp
    enum VFX_EffectType
    {
        VFXEFFECTTYPE_NONE = 0,
        VFXEFFECTTYPE_MODEL,
        VFXEFFECTTYPE_SPRITE,
        VFXEFFECTTYPE_LIGHT,
        VFXEFFECTTYPE_TRAIL,
        VFXEFFECTTYPE_WATER,
        VFXEFFECTTYPE_SOUND,
        VFXEFFECTTYPE_RAIN,
        VFXEFFECTTYPE_SNOW,
        VFXEFFECTTYPE_WIND,
        VFXEFFECTTYPE_LIGHTNING,
        VFXEFFECTTYPE_PARTICLESYSTEM,
        VFXEFFECTTYPE_COLLISION,
        VFXEFFECTTYPE_MODELEFFECT,
        VFXEFFECTTYPE_MOTIONBLUR
    };
    ```

## 3.2 Functions and methods

- **Methods:** PascalCase, verbs first:

  ```cpp
  void OnCreate();
  void OnDestroy();
  void OnUpdate(float fDeltaTime);
  void OnServerCreate();
  void OnServerUpdate();
  void Queue_Walk();
  void Queue_Limp();
  void QueueToCombat();
  void QueryDefaultAnimation();
  ```

- The `_` separator is used occasionally to separate operation and state (`Queue_To_Crouch`, `Queue_Limp`); mimic it where you see it in the original strings.

- **Global functions / native script functions:**

  ```cpp
  void WorldMapSetAreaVisible(int nAreaId, bool bVisible);
  void SetWorldMapAreaExplorable(int nAreaId, bool bExplorable);
  void SetAreaRestStatus(int nAreaId, bool bAllowed);

  // Script-callable (note A_ prefix):
  void A_SendMessageToTeam(int nFromId, int nTeamId, int nMessageId);
  void A_Wander(int nEntityId);
  void A_PickupItem(int nEntityId, int nItemId);
  ```

- **Overloads:** Use overloads or additional parameters rather than cryptic suffixes.

## 3.3 Variables and members

### 3.3.1 Member variables

- Prefix with `m_`, then CamelCase:

  ```cpp
  class GameWeapon
  {
  private:
      int   m_nCaliber;
      int   m_nModifier;
      bool  m_bUsesPrimaryAmmo;
      bool  m_bUsesSecondaryAmmo;
      float m_fReloadTime;
  };
  ```

### 3.3.2 Statics and globals

- **Static members:** `s_` prefix.

  ```cpp
  class Gfx_Engine
  {
  private:
      static Gfx_Engine* s_pInstance;
  };
  ```

- **Globals / singletons:** `g_` prefix (if you need them):

  ```cpp
  extern Game::GameClient* g_pGameClient;
  ```

### 3.3.3 Locals and parameters (Hungarian‑lite)

Use short type hints, as seen in the engine’s messages:

- `p` – pointer: `pOwner`, `pEntity`, `pTexture`.
- `n` – integer number: `nIndex`, `nCount`, `nDamage`, `nCaliber`.
- `b` – boolean: `bVisible`, `bActive`, `bFound`.
- `f` – float: `fDeltaTime`, `fAlpha`, `fRange`.
- `dw` – 32‑bit `DWORD`: `dwFlags`, `dwUID`.
- `sz` / `psz` – C strings: `szBuffer`, `pszName`.

Examples:

```cpp
bool GameWeapon::ReloadPrimaryAmmo(GameEntity* pOwner)
{
    if (!pOwner)
        return false;

    int nAmmoNeeded = 0;
    float fReloadTime = 0.0f;
    bool bHasAmmo = false;

    // ...
}
```

### 3.3.4 GUI identifiers

For GUI elements, follow the prefixes you see in the strings:

- Buttons: `btn...`
  - `btnInventory`, `btnCombatSpeed`, `btnMusicVolume`.
- Labels: `lbl...`
  - `lblHeadBiologicalValue`, `lblWeapon01AP_up`.
- Pictures: `pic...`
  - `picWeapon01Icon_up`, `picTargetHighlight`.
- Windows: `win...`
  - `winActionbar`, `winMiniMap`, `winMessageBox`.
- Text edits: `edit...` or `TextEdit` (seen as `TextEdit`).
- Toggle/checkbox: `btn...` plus state suffixes (`Up/Down/Selected/Disabled`).
- Special: `bl...` often for “block” or “label group” (`blHotkeys`, `blGame`), `tn...` for “tiny number” / spinner.

Example:

```cpp
GUI::Window* winActionbar = NULL;
GUI::Button* btnInventory = NULL;
GUI::Label*  lblActionPointsValue = NULL;
GUI::Picture* picTargetHighlight = NULL;
```

---

# 4. Error Handling & Logging

## 4.1 General philosophy

- Don’t throw exceptions in engine/gameplay hot paths.
- Prefer **return codes** (`bool`, `HRESULT`) plus **logging**.
- Use assertions for internal invariants (you can define `ASSERT` wrapping `_ASSERTE` / `assert`).

## 4.2 Log message format

The strings show a consistent pattern:

- `"ClassName::MethodName() - Message...\n"`
  or
- `"Subsystem_Class::Method - Message...\n"`

Examples to emulate:

```cpp
LogError("GameWeapon::ReloadPrimaryAmmo() - This weapon doesn't even use ammo.\n");
LogError("Gfx_Engine::ProcessFogOfWar - trying to use an invalid texture!\n");
LogError("Gfx_AnimationObject::ApplyAnimation - pModelTrackMask == NULL\n");
LogError("GameEntity::Queue_Limp() - Unknown weapon stance encountered.\n");
LogError("GameActionAttack::OnServerUpdate() - Target entity is already dead.\n");
```

For resource or system errors:

```cpp
LogError("Gfx_Texture_Manager::LoadTextureFromBuffer() failed to load: %s\n", pszFilename);
LogError("ResourceSystem::FileStreamSeek - Tried a bad seek from the end of file %s - endpos = %d tried = %d\n",
         pszFilename, nEndPos, nTried);
```

**Guideline:**

- Always include the class and method name at the start of the message.
- End messages with `\n`.
- For functions that are not class methods (e.g. static helpers), use a subsystem prefix: `World::CreateEffect()`, `MapEffectSetActive()`.

## 4.3 HRESULT and DirectX errors

- When calling DirectX APIs, check `HRESULT` and log with hex codes and descriptive strings:

  ```cpp
  HRESULT hr = m_pDevice->CreateTexture(...);
  if (FAILED(hr))
  {
      LogError("Gfx_Texture_Manager::AddTexture - CreateTexture Failed (%x %s)\n", hr, pszName);
      return false;
  }
  ```

---

# 5. Memory Management

- Use raw pointers and manual `new`/`delete`:

  ```cpp
  Gfx_Texture* pTexture = new Gfx_Texture();
  // ...
  delete pTexture;
  pTexture = NULL;
  ```

- Use `NULL` (not `nullptr`) for null pointers.
- If implementing custom allocators (`ResourceSystem`, `ResourceCache`), keep their style:

  - Functions like `ResourceSystem::MemStreamOpen`, `ResourceSystem::MemStreamSeek`.
  - Log on allocation failures: `ResourceSystem::OpenFile - Couldn't allocate enough memory for ...`.

- Avoid reference-counted smart pointers unless you can hide them under existing interfaces. They likely used manual reference counting or simple ownership.

---

# 6. Use of STL and Standard Library

The strings clearly show STL use (`vector<T> too long`, `map/set<T> too long`, etc.), so:

- **Allowed (and encouraged)** in reconstruction:
  - `std::vector`
  - `std::map`
  - `std::set`
  - `std::list`
  - `std::deque`
  - `std::string`
  - Standard exceptions and I/O, but use them judiciously in engine code.

- Prefer fully‑qualified names in headers:

  ```cpp
  #include <vector>
  #include <string>

  class GameParty
  {
  private:
      std::vector<GameEntity*> m_PartyMembers;
      std::string              m_sName;
  };
  ```

- You can use `using std::vector;` in `.cpp` files if it improves readability, but avoid `using namespace std;` in headers.

- For authenticity, avoid template‑heavy designs; use STL in straightforward ways.

---

# 7. Windows & DirectX Conventions

- Use Win32 / DirectX types:
  - `DWORD`, `WORD`, `BYTE`, `BOOL`, `HRESULT`.
- Handle errors by mapping them to descriptive strings, as seen in the string table:
  - `D3DERR_OUTOFVIDEOMEMORY`, `D3DERR_INVALIDCALL`, `DDERR_SURFACELOST`, `VFW_E_*`, `DMUS_E_*`, etc.
- Wrap DirectX objects in C++ classes:
  - `Gfx_D3D`, `Gfx_Engine`, `Gfx_Texture`, `Gfx_RenderStyle`, etc.
- Use D3D 8/9‑style FVF / shader assembly style in `.fx` content (exact effect strings are already inside your resource data; mimic the naming, not necessarily rewrite them).

---

# 8. Scripting Interface Style

The strings show many script-facing functions:

- `A_SendMessageToSquad`, `A_AddFloatingSpeechText`, `A_Wander`, `A_PickupItem`, `A_MoveToEntity`.
- Engine‑side wrappers that say “Native...” or “Script_Small::Debugger()`.

Guidelines:

- **Script-exposed native functions** should be free functions or static functions with:

  - Simple parameter lists (`int` IDs, primitive types, vector structs).
  - C‑style names that start with `A_` (for “Action”) or `Native...` when appropriate.

  ```cpp
  void A_SendMessageToTeam(int nFromEntityId, int nTeamId, int nMessageId);
  void A_AddFloatingSpeechText(int nEntityId, int nStrRef);
  ```

- **Error messages** for script functions follow the pattern:

  ```cpp
  LogError("A_SendMessageToTeam() was unable to resolve address of parameter %d.\n", nParamIndex);
  LogError("DialogueCall() was unable to retrieve the function name.\n");
  ```

---

# 9. Comments & Documentation

- Use `//` for single‑line comments.
- Use `/* ... */` only for short blocks or temporarily commenting out code.
- Avoid Doxygen-style tags unless you really want to; the original code likely kept comments fairly plain.

Examples:

```cpp
// Apply damage, considering DR/DT, criticals, etc.
bool GameWeapon::ApplyDamage(GameEntity* pAttacker, GameEntity* pTarget)
{
    // ...
}
```

---

# 10. Example: Full Class Skeleton in BIS Style

Here is a more complete example of a Van Buren–style class using the guidelines:

```cpp
// GameWeapon.h
#pragma once

#include "GameEntity.h"

namespace Game
{
    class GameWeapon
    {
    public:
        GameWeapon();
        ~GameWeapon();

        bool ReloadPrimaryAmmo(GameEntity* pOwner);
        bool ReloadSecondaryAmmo(GameEntity* pOwner);

        bool CalculateDamage(GameEntity* pAttacker,
                             GameEntity* pTarget,
                             int         nCalledTarget,
                             int&        rnDamageOut) const;

        float CalculateRangedToHit(GameEntity* pAttacker,
                                   GameEntity* pTarget) const;

    private:
        bool ResolveTargets(GameEntity* pAttacker, GameEntity* pTarget) const;

    private:
        int   m_nCaliber;
        int   m_nModifier;
        bool  m_bUsesPrimaryAmmo;
        bool  m_bUsesSecondaryAmmo;
        float m_fReloadTime;
    };
}
```

```cpp
// GameWeapon.cpp
#include "stdafx.h"
#include "GameWeapon.h"
#include "StringSystem.h"
#include "ResourceSystem.h"
#include "Log.h"

using namespace Game;

GameWeapon::GameWeapon()
: m_nCaliber(0)
, m_nModifier(0)
, m_bUsesPrimaryAmmo(false)
, m_bUsesSecondaryAmmo(false)
, m_fReloadTime(0.0f)
{
}

GameWeapon::~GameWeapon()
{
}

bool GameWeapon::ReloadPrimaryAmmo(GameEntity* pOwner)
{
    if (!m_bUsesPrimaryAmmo)
    {
        LogError("GameWeapon::ReloadPrimaryAmmo() - This weapon doesn't even use ammo.\n");
        return false;
    }

    if (!pOwner)
    {
        LogError("GameWeapon::ReloadPrimaryAmmo() - NULL owner pointer.\n");
        return false;
    }

    // TODO: Look up ammo in pOwner's inventory and adjust m_fReloadTime.

    return true;
}

bool GameWeapon::CalculateDamage(GameEntity* pAttacker,
                                 GameEntity* pTarget,
                                 int         nCalledTarget,
                                 int&        rnDamageOut) const
{
    if (!pAttacker || !pTarget)
    {
        LogError("GameWeapon::CalculateDamage() - NULL attacker or target.\n");
        return false;
    }

    if (!ResolveTargets(pAttacker, pTarget))
    {
        LogError("GameWeapon::CalculateDamage() - ResolveTargets() failed.\n");
        return false;
    }

    switch (nCalledTarget)
    {
    case CALLEDTARGET_HEAD:
        // ...
        break;

    case CALLEDTARGET_TORSO:
        // ...
        break;

    default:
        LogError("GameWeapon::CalculateDamage() - Unknown CALLEDTARGET encountered: %d\n",
                 nCalledTarget);
        return false;
    }

    // Set rnDamageOut appropriately...
    rnDamageOut = 10; // placeholder

    return true;
}

float GameWeapon::CalculateRangedToHit(GameEntity* pAttacker,
                                       GameEntity* pTarget) const
{
    if (!pAttacker || !pTarget)
    {
        LogError("GameWeapon::CalculateRangedToHit() - NULL attacker or target.\n");
        return 0.0f;
    }

    // TODO: Implement based on skills, range, etc.
    return 75.0f;
}

bool GameWeapon::ResolveTargets(GameEntity* pAttacker, GameEntity* pTarget) const
{
    if (!pAttacker || !pTarget)
        return false;

    // TODO: visibility checks, etc.
    return true;
}
```

This example follows:

- Namespaces & prefixes.
- `m_` member naming.
- PascalCase functions, `OnXxx` style where appropriate.
- Logging style matching the real messages.

---
