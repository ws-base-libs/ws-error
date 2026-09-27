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
#include <type_traits>
#include <vector>

namespace ws::core::error {

/// The code must be a **scoped enum** (`enum class`), not an int and not a plain
/// enum. That is enforced here rather than hoped for: a named code is what makes
/// `r.error().code == ModelErrorCode::FileNotFound` readable, and a numeric one
/// would quietly put us back where bare int codes started.
///
/// ANY layer defines its own typed errors by owning its own enum:
///
///     namespace system {
///     enum class error_type { NotFound, InvalidState, Timeout };   // named, yours
///     using system_error = ws::Error<error_type>;
///     }
///
/// The struct is never duplicated — only the enum is. That is the whole
/// extension model.
template <typename Code>
    requires std::is_scoped_enum_v<Code>
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

    /// Anything else worth passing back, as one string per observation — what a
    /// layer knew at the point of failure, a request id and sender, a path that
    /// was tried, the value that was rejected.
    ///
    /// Zero or more, which is what "optional" means for a list: empty is the
    /// common case and costs nothing. It is a vector rather than an
    /// `optional<vector>` deliberately — one layer of emptiness is enough — and
    /// rather than a single `optional<string>` because the useful case is a
    /// CHAIN: each layer appends what it knew as the error travels outward, and
    /// the caller gets the whole story instead of the innermost sentence.
    std::vector<std::string> context{};
};

}  // namespace ws::core::error

namespace ws {

/// Convenience alias so every layer writes `Error<Code>` unqualified inside
/// `namespace ws` and its nested namespaces, exactly as the architecture's
/// examples do. `ws::Error<X>` and `ws::core::error::Error<X>` are the same
/// type — this is a using-declaration, not a wrapper.
using ws::core::error::Error;

}  // namespace ws
