# ws-error

**Universal C++23 error contract for `std::expected` — strongly-typed `Error<Code>`
carrying `code` + `message` + `source`, so every failure is traceable to the call
site that raised it.**

Header-only. Zero dependencies beyond the C++ standard library. One type, spoken
by every layer of a codebase.

---

## What this is

Most error enums answer *what* failed and nothing else. When the same code can be
raised from five places — `FileNotFound` from the config reader, the model loader,
the snapshot store — the caller cannot tell which one fired.

`Error<Code>` pairs a strongly-typed enum with two optional context fields:

| Field | Type | Purpose |
|---|---|---|
| `code` | `Code` (a strongly-typed `enum class`) | What failed. Strongly typed so an auth error cannot be compared against a cache error by accident. |
| `message` | `std::optional<std::string>` | Why, in human terms. |
| `source` | `std::optional<std::string>` | Where — `file:line` or `System::method`. Fill it in whenever the code has more than one call site. |

It is designed to be the `E` in `std::expected<T, E>`.

## Usage

```cpp
#include <ws/error/error.h>
#include <expected>

// Each layer owns its own code enum and aliases the struct once.
enum class ModelErrorCode { FileNotFound, ContextOverflow };
using ModelError = ws::Error<ModelErrorCode>;

auto load_model(std::string const& path) -> std::expected<Model, ModelError> {
    if (!exists(path)) {
        return std::unexpected(ModelError{
            .code    = ModelErrorCode::FileNotFound,
            .message = std::format("{} does not exist", path),
            .source  = "ModelSystem::load",     // <- makes the code traceable
        });
    }
    // ...
}

// Caller:
if (auto r = load_model(p); !r) {
    log(r.error().code, r.error().message.value_or("(no detail)"),
                     r.error().source.value_or("(unknown origin)"));
}
```

Errors are **values**: no exceptions cross a boundary, and the caller decides what
a failure means.

## API reference

```cpp
namespace ws::core::error {

template <typename Code>
struct Error {
    Code                              code{};
    std::optional<std::string>        message{};
    std::optional<std::string>        source{};
};

}  // namespace ws::core::error

namespace ws {
using ws::core::error::Error;    // convenience alias — same type, not a wrapper
}
```

- Aggregate-initialise it (`Error<Code>{ .code = ..., .message = ..., .source = ... }`).
- Value-initialise it (`Error<Code>{}`) when only the code matters.
- `ws::Error<X>` and `ws::core::error::Error<X>` are the same type.

That is the entire surface. There is deliberately no `make_error()` helper and no
virtual base: this is a value type, and every layer's ergonomics come from its own
`using` alias.

## Build & test

Header-only — consuming it needs only the include path:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The test suite is dependency-free on purpose (no Google Test, no framework): the
library builds and tests on any C++23 compiler with no network and no package
manager. Tests are built only when this is the top-level project, so consuming it
through FetchContent never drags our test binary into your build.

Consuming via CMake:

```cmake
FetchContent_Declare(ws-error GIT_REPOSITORY <this repo> GIT_TAG v0.1.0)
FetchContent_MakeAvailable(ws-error)
target_link_libraries(your_target PRIVATE ws-error)
```

```cpp
#include <ws/error/error.h>
```

## Layout

```
include/ws/error/error.h   the public header — THIS PATH IS PERMANENT
src/                       intentionally empty (header-only)
src/details/               intentionally empty (header-only)
tests/                     standalone, dependency-free
```

`include/ws/error/error.h` and the CMake target `ws-error` will not change: the
include statement and the `target_link_libraries` line above are stable across
every version bump.

## Extension points

The intended extension is **your code enum, not this struct**:

1. Define `enum class YourErrorCode { ... }` where the errors are raised.
2. `using YourError = ws::Error<YourErrorCode>;` in that layer's types header.
3. Add codes to the enum as your domain grows.

If you find yourself wanting to add fields to `Error`, resist: a field that only
one layer needs belongs in that layer's own error type, and a field every layer
needs is worth proposing upstream as a versioned change. Keeping the struct to
three fields is what makes one type acceptable everywhere.

## Licence

To be added by the owning organisation before publication.
