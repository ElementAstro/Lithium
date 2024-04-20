

#ifndef CARBON_WINDOWS_HPP
#define CARBON_WINDOWS_HPP

#include <string>

#ifdef CARBON_WINDOWS
#define VC_EXTRA_LEAN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Carbon {
namespace detail {
struct Loadable_Module {
    template <typename T>
    static std::wstring to_wstring(const T &t_str) {
        return std::wstring(t_str.begin(), t_str.end());
    }

    template <typename T>
    static std::string to_string(const T &t_str) {
        return std::string(t_str.begin(), t_str.end());
    }

#if defined(_UNICODE) || defined(UNICODE)
    template <typename T>
    static std::wstring to_proper_string(const T &t_str) {
        return to_wstring(t_str);
    }
#else
    template <typename T>
    static std::string to_proper_string(const T &t_str) {
        return to_string(t_str);
    }
#endif

    static std::string get_error_message(DWORD t_err) {
        using StringType = LPTSTR;

#if defined(_UNICODE) || defined(UNICODE)
        std::wstring retval = L"Unknown Error";
#else
        std::string retval = "Unknown Error";
#endif
        StringType lpMsgBuf = nullptr;

        if (FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, t_err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<StringType>(&lpMsgBuf), 0, nullptr) != 0 &&
            lpMsgBuf) {
            retval = lpMsgBuf;
            LocalFree(lpMsgBuf);
        }

        return to_string(retval);
    }

    struct DLModule {
        explicit DLModule(const std::string &t_filename)
            : m_data(LoadLibrary(to_proper_string(t_filename).c_str())) {
            if (!m_data) {
                throw Carbon::exception::load_module_error(
                    get_error_message(GetLastError()));
            }
        }

        DLModule(DLModule &&) = default;
        DLModule &operator=(DLModule &&) = default;
        DLModule(const DLModule &) = delete;
        DLModule &operator=(const DLModule &) = delete;

        ~DLModule() { FreeLibrary(m_data); }

        HMODULE m_data;
    };

    /*
    TODO: Fix -Wcast-function-type here
    template <typename T>
    struct DLSym {
        DLSym(DLModule &t_mod, const std::string &t_symbol)
            : m_symbol(reinterpret_cast<T>(
                  GetProcAddress(t_mod.m_data, t_symbol.c_str()))) {
            if (!m_symbol) {
                throw Carbon::exception::load_module_error(
                    get_error_message(GetLastError()));
            }
        }

        T m_symbol;
    };
    */
    template <typename T>
    struct DLSym {
        using FunctionPtr = T;

        DLSym(DLModule &t_mod, const std::string &t_symbol)
            : m_symbol(get_function_pointer(t_mod, t_symbol)) {
            if (!m_symbol) {
                throw Carbon::exception::load_module_error(
                    get_error_message(GetLastError()));
            }
        }

        FunctionPtr m_symbol;

    private:
        // Helper function to safely retrieve the function pointer
        static FunctionPtr get_function_pointer(DLModule &t_mod,
                                                const std::string &t_symbol) {
            FARPROC proc = GetProcAddress(t_mod.m_data, t_symbol.c_str());
            if (!proc) {
                throw std::runtime_error(
                    "Failed to retrieve function pointer for symbol: " +
                    t_symbol);
            }
            return reinterpret_cast<FunctionPtr>(proc);
        }
    };

    Loadable_Module(const std::string &t_module_name,
                    const std::string &t_filename)
        : m_dlmodule(t_filename),
          m_func(m_dlmodule, "create_module_" + t_module_name),
          m_moduleptr(m_func.m_symbol()) {}

    DLModule m_dlmodule;
    DLSym<Create_Module_Func> m_func;
    ModulePtr m_moduleptr;
};
}  // namespace detail
}  // namespace Carbon
#endif
