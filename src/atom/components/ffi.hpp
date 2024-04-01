#ifndef ATOM_COMPONENT_FFI_HPP
#define ATOM_COMPONENT_FFI_HPP

#include <dlfcn.h>
#include <ffi.h>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

// Exception class for FFI errors
class FFIException : public std::exception {
public:
    explicit FFIException(const std::string &message) : message_(message) {}
    const char *what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

template <typename ReturnType, typename... Args>
class FFIWrapper {
public:
    FFIWrapper(ffi_type **argTypes, int numArgs, ffi_type *returnType)
        : argTypes_(argTypes), numArgs_(numArgs), returnType_(returnType) {
        cif_ = new ffi_cif;
        ffi_prep_cif(cif_, FFI_DEFAULT_ABI, numArgs_, returnType_, argTypes_);
    }

    template <typename... CallArgs>
    ReturnType call(void *funcPtr, CallArgs... args) {
        if constexpr (sizeof...(CallArgs) != sizeof...(Args)) {
            throw FFIException("Incorrect number of arguments provided.");
        }

        void *argsArray[] = {&args...};
        ReturnType result;
        ffi_call(cif_, FFI_FN(funcPtr), &result, argsArray);
        return result;
    }

    ~FFIWrapper() { delete cif_; }

private:
    ffi_cif *cif_;
    ffi_type **argTypes_;
    int numArgs_;
    ffi_type *returnType_;
};

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string &libraryPath) : handle_(nullptr) {
        handle_ = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (!handle_) {
            throw FFIException("Failed to load dynamic library.");
        }
    }

    ~DynamicLibrary() {
        if (handle_) {
            dlclose(handle_);
        }
    }

    template <typename FuncType>
    void addFunction(const std::string &functionName) {
        void *funcPtr = dlsym(handle_, functionName.c_str());
        if (!funcPtr) {
            throw FFIException("Failed to find symbol.");
        }
        functionMap_[functionName] = funcPtr;
    }

    template <typename ReturnType, typename... Args>
    ReturnType callFunction(const std::string &functionName, Args... args) {
        auto it = functionMap_.find(functionName);
        if (it == functionMap_.end()) {
            throw FFIException("Function not found in the library.");
        }

        void *funcPtr = it->second;

        ffi_type **argTypes = new ffi_type *[sizeof...(Args)];
        for (size_t i = 0; i < sizeof...(Args); ++i) {
            argTypes[i] = &ffi_type_pointer;
        }

        ffi_type *returnType = &ffi_type_pointer;

        FFIWrapper<ReturnType, Args...> ffiWrapper(argTypes, sizeof...(Args),
                                                   returnType);
        return ffiWrapper.call(funcPtr, args...);
    }

private:
    void *handle_;
    std::unordered_map<std::string, void *> functionMap_;
};

#endif
