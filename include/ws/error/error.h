// ws-error — the universal error contract.
//
// ONE type, spoken by every layer: base libs, systems and the app contract all
// return `Error<Code>`, which is why this library is authored below all of them
// and may be included from anywhere — including from a `*_types.h` shared-type
// header, which no other base lib may be.
//
// Design note (why a struct and not an enum): a bare error enum cannot say WHICH
// call site failed. `message` and `source` exist so that one code, raised from
// several places, is always traceable to the exact origin.
//
// Dependency policy: the C++ standard library and nothing else — no project
// headers, no third-party headers. This library is meant to drop into any C++23
// codebase, so it must not bring a neighbourhood with it.

#pragma once

#include <optional>
#include <string>

namespace ws::core::error {

/// The universal error value. Returned as `std::expected<T, Error<Code>>`.
///
/// `Code` is always a strongly-typed `enum class` owned by the layer that
/// raises the error (`AppErrorCode`, `SystemErrorCode`, `BusErrorCode`,
/// `ModelErrorCode`, ...). Aliasing one of these per layer — `using ModelError =
/// ws::Error<ModelErrorCode>;` — is the intended usage; the struct itself is
/// never duplicated.
template <typename Code>
struct Error {
    /// What failed. Strongly typed so a caller cannot compare an auth error
    /// against a cache error by accident.
    Code code{};

    /// Why / where, in human terms. Optional: some codes carry their meaning
    /// entirely in the enum and need no elaboration.
    std::optional<std::string> message{};

    /// The origin — `file:line`, or `System::method`. Optional, but fill it in
    /// whenever the code is raised from more than one call site.
    std::optional<std::string> source{};
};

}  // namespace ws::core::error

namespace ws {

/// Convenience alias so every layer writes `Error<Code>` unqualified inside
/// `namespace ws` and its nested namespaces, exactly as the architecture's
/// examples do. `ws::Error<X>` and `ws::core::error::Error<X>` are the same
/// type — this is a using-declaration, not a wrapper.
using ws::core::error::Error;

}  // namespace ws
