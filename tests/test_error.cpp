// ws-error — standalone test suite.
//
// Deliberately dependency-free: no Google Test, no framework, nothing but the
// standard library and the header under test. tech_spec §14.3 requires base libs
// to be "built and tested standalone, zero project dependencies"; taking that to
// its strongest form means this suite compiles and runs on any C++23 compiler
// with no network and no package manager. It is also why the brief's
// "grep-verify zero third-party includes anywhere in the lib" passes literally.
//
// The project's own test suite (tests/unit/**) uses Google Test; that is the
// project's choice for the project's tests and is not imposed on this library.
//
// Exit code 0 = all checks passed. Any failure prints and exits 1.

#include <ws/error/error.h>

#include <cstdio>
#include <optional>
#include <string>
#include <type_traits>

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool ok, char const* what) {
    ++g_checks;
    if (ok) {
        std::printf("  ok    %s\n", what);
    } else {
        ++g_failures;
        std::printf("  FAIL  %s\n", what);
    }
}

void section(char const* name) { std::printf("%s\n", name); }

enum class Code {
    Unspecified,
    FileNotFound,
    ContextOverflow,
};

using TestError = ws::Error<Code>;

}  // namespace

int main() {
    section("construct: value-initialised");
    {
        TestError const e{};
        check(e.code == Code::Unspecified, "code value-initialises");
        check(!e.message.has_value(), "message defaults to empty");
        check(!e.source.has_value(), "source defaults to empty");
    }

    section("aggregate initialisation: every field round-trips");
    {
        TestError const e{.code = Code::FileNotFound,
                          .message = std::string{"model file missing at the configured path"},
                          .source = std::string{"ModelSystem::initialize"}};
        check(e.code == Code::FileNotFound, "code round-trips");
        check(e.message.has_value(), "message is present");
        check(*e.message == "model file missing at the configured path", "message round-trips");
        check(e.source.has_value(), "source is present");
        check(*e.source == "ModelSystem::initialize", "source round-trips");
    }

    section("context fields are genuinely optional");
    {
        TestError const e{.code = Code::ContextOverflow};
        check(e.code == Code::ContextOverflow, "code set without context");
        check(!e.message.has_value(), "message may be omitted");
        check(!e.source.has_value(), "source may be omitted");
    }

    section("traceability: one code, several call sites");
    {
        // The whole point of `source`: the same enum value from two places must
        // remain distinguishable at the call site that inspects it.
        TestError const from_init{.code = Code::FileNotFound, .source = "ModelSystem::initialize"};
        TestError const from_load{.code = Code::FileNotFound, .source = "ModelSystem::load"};
        check(from_init.source != from_load.source,
              "same code from two call sites stays distinguishable");
    }

    section("aliases: ws::Error and ws::core::error::Error are one type");
    {
        static_assert(std::is_same_v<ws::Error<Code>, ws::core::error::Error<Code>>,
                      "ws::Error must be a using-declaration, not a wrapper");
        check(true, "ws::Error<X> is ws::core::error::Error<X>");
    }

    section("usable as std::expected's error type");
    {
        // Not a runtime assertion so much as the compile-time contract every
        // consumer depends on: this must be a complete, destructible value type.
        static_assert(std::is_nothrow_destructible_v<TestError>);
        static_assert(std::is_copy_constructible_v<TestError>);
        static_assert(std::is_move_constructible_v<TestError>);
        check(true, "complete, copyable, movable, destructible value type");
    }

    std::printf("\nws-error tests: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
