#pragma once

#include <iostream>
#include <vector>
#include <optional>
#include <string>
#include <type_traits>
#include <iomanip>
#include <dlfcn.h>
#include <cstring>
#include <map>

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <cstring>
#include <mach-o/dyld.h>
#endif

struct Symbol
{
    std::string name;
    std::string value;
    std::string size;
};

struct SymbolInfo
{
    std::string type;
};

template<typename T>
class LibDumper
{
public:
    LibDumper() = default;
    ~LibDumper()
    {
        if (handle_)
        {
            dlclose(handle_);
        }
    }
    
    bool LoadLibraryFromPath(const std::string& library_path);
    bool ValidateHeaders(const uint8_t* library_base);
    void PrintLibInfo(const uint8_t* library_base);
    void AutoDumpSymbols();
    [[nodiscard]] void* getHandle() const { return handle_; }
    [[nodiscard]] const uint8_t* GetBaseAddress(const std::string& symbol_name) const;

private:
    void* handle_ = nullptr;

    std::map<std::string, std::optional<SymbolInfo>> symbols = {
            {"_malloc", SymbolInfo{"function"}},
            {"_free", SymbolInfo{"function"}},
            {"_printf", SymbolInfo{"function"}},
            {"_strcmp", SymbolInfo{"function"}},
            {"_memcpy", SymbolInfo{"function"}},
            {"__stack_chk_fail", SymbolInfo{"function"}},
            {"__assert_rtn", SymbolInfo{"function"}},
            {"__dso_handle", SymbolInfo{"variable"}},
            {"_environ", SymbolInfo{"variable"}},
            {"__builtin_return_address", SymbolInfo{"function"}},
            {"__builtin_frame_address", SymbolInfo{"function"}},
            {"_dlopen", SymbolInfo{"function"}},
            {"_dlclose", SymbolInfo{"function"}},
            {"_dlsym", SymbolInfo{"function"}},
            {"_dlerror", SymbolInfo{"function"}},
            {"__cxa_atexit", SymbolInfo{"function"}},
            {"__cxa_finalize", SymbolInfo{"function"}},
            {"__cxa_pure_virtual", SymbolInfo{"function"}},
            {"__cxa_begin_catch", SymbolInfo{"function"}},
            {"__cxa_end_catch", SymbolInfo{"function"}},
            {"__cxa_rethrow", SymbolInfo{"function"}},
            {"__cxa_throw", SymbolInfo{"function"}},
            {"__cxa_guard_acquire", SymbolInfo{"function"}},
            {"__cxa_guard_release", SymbolInfo{"function"}},
            {"__cxa_guard_abort", SymbolInfo{"function"}}
    };
};

void displaySymbols(const std::vector<Symbol>& symbols)
{
    const int nameWidth = 60;
    const int valueWidth = 20;
    const int sizeWidth = 12;

    std::cout << std::left 
              << std::setw(nameWidth) << "Name" 
              << std::setw(valueWidth) << "Value" 
              << std::setw(sizeWidth) << "Size" 
              << std::endl;

    std::cout << std::left 
              << std::setw(nameWidth) << std::setfill('-') << "" 
              << std::setw(valueWidth) << std::setfill('-') << "" 
              << std::setw(sizeWidth) << std::setfill('-') << "" 
              << std::setfill(' ') << std::endl;

    for (const auto& symbol : symbols)
    {
        std::cout << std::left 
                  << std::setw(nameWidth) << symbol.name 
                  << std::setw(valueWidth) << symbol.value 
                  << std::setw(sizeWidth) << symbol.size 
                  << std::endl;
    }
}

#if defined(__APPLE__)

template<>
bool LibDumper<void>::LoadLibraryFromPath(const std::string& library_path)
{
    handle_ = dlopen(library_path.c_str(), RTLD_LAZY);
    if (!handle_)
    {
        std::cerr << "Failed to load library: " << dlerror() << std::endl;
        return false;
    }
    return true;
}

template<>
bool LibDumper<void>::ValidateHeaders(const uint8_t* library_base)
{
    if (!library_base)
    {
        std::cerr << "Library base is null" << std::endl;
        return false;
    }

    auto header = reinterpret_cast<const mach_header_64*>(library_base);

    if (header->magic != MH_MAGIC_64 && header->magic != MH_CIGAM_64)
    {
        std::cerr << "Invalid Mach-O magic number" << std::endl;
        return false;
    }
    return true;
}

template<>
void LibDumper<void>::PrintLibInfo(const uint8_t* library_base)
{
    auto header = reinterpret_cast<const mach_header_64*>(library_base);
    auto cmd = reinterpret_cast<const load_command*>(reinterpret_cast<const uint8_t*>(library_base) + sizeof(mach_header_64));

    for (uint32_t i = 0; i < header->ncmds; i++)
    {
        if (cmd->cmd == LC_SYMTAB)
        {
            auto symtab = reinterpret_cast<const symtab_command*>(cmd);
            auto symtab_data = reinterpret_cast<const nlist_64*>(library_base + symtab->symoff);
            const char* strtab = reinterpret_cast<const char*>(library_base + symtab->stroff);

            std::vector<Symbol> symbols;

            for (uint32_t j = 0; j < symtab->nsyms; j++)
            {
                if (symtab_data[j].n_un.n_strx)
                {
                    Symbol symbol;
                    symbol.name = strtab + symtab_data[j].n_un.n_strx;
                    symbol.value = std::to_string(symtab_data[j].n_value);
                    symbol.size = std::to_string(symtab_data[j].n_desc);
                    symbols.push_back(symbol);
                }
            }

            displaySymbols(symbols);
        }

        if (cmd->cmd == LC_SEGMENT_64)
        {
            auto segment = reinterpret_cast<const segment_command_64*>(cmd);
            auto sections = reinterpret_cast<const section_64*>(reinterpret_cast<const uint8_t*>(segment) + sizeof(segment_command_64));

            for (uint32_t j = 0; j < segment->nsects; j++)
            {
                if (std::strcmp(sections[j].sectname, "__text") == 0)
                {
                    std::cout << "----------------------Text Section----------------------" << std::endl;
                    std::cout << "Text section: " << std::endl;
                    std::cout << "Address: " << std::hex << sections[j].addr << std::endl;
                    std::cout << "Size: " << std::dec << sections[j].size << std::endl;
                    std::cout << "--------------------------------------------------------" << std::endl;
                }
            }
        }

        if (cmd->cmd == LC_DYLD_INFO_ONLY)
        {
            auto dyld_info = reinterpret_cast<const dyld_info_command*>(cmd);
            std::cout << "----------------------Dyld Info----------------------" << std::endl;
            std::cout << "Rebase offest: " << dyld_info->rebase_off << std::endl;
            std::cout << "Rebase size: " << dyld_info->rebase_size << std::endl;
            std::cout << "Bind offest: " << dyld_info->bind_off << std::endl;
            std::cout << "Bind size: " << dyld_info->bind_size << std::endl;
            std::cout << "Weak bind offest: " << dyld_info->weak_bind_off << std::endl;
            std::cout << "Weak bind size: " << dyld_info->weak_bind_size << std::endl;
            std::cout << "Lazy bind offest: " << dyld_info->lazy_bind_off << std::endl;
            std::cout << "Lazy bind size: " << dyld_info->lazy_bind_size << std::endl;
            std::cout << "Export offest: " << dyld_info->export_off << std::endl;
            std::cout << "Export size: " << dyld_info->export_size << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
        }

        if (cmd->cmd == LC_CODE_SIGNATURE)
        {
            auto code_signature = reinterpret_cast<const linkedit_data_command*>(cmd);
            std::cout << "----------------------Code Signature----------------------" << std::endl;
            std::cout << "Code signature offest: " << code_signature->dataoff << std::endl;
            std::cout << "Code signature size: " << code_signature->datasize << std::endl;
            std::cout << "----------------------------------------------------------" << std::endl;
        }

        if (cmd->cmd == LC_FUNCTION_STARTS)
        {
            auto function_starts = reinterpret_cast<const linkedit_data_command*>(cmd);
            std::cout << "----------------------Function Starts----------------------" << std::endl;
            std::cout << "Function starts offest: " << function_starts->dataoff << std::endl;
            std::cout << "Function starts size: " << function_starts->datasize << std::endl;
            std::cout << "----------------------------------------------------------" << std::endl;
        }

        cmd = reinterpret_cast<const load_command*>(reinterpret_cast<const uint8_t*>(cmd) + cmd->cmdsize);
    }
}

template<>
const uint8_t* LibDumper<void>::GetBaseAddress(const std::string& symbol_name) const
{
    if (!handle_)
    {
        std::cerr << "Handle is null" << std::endl;
        return nullptr;
    }

    void* symbol = dlsym(handle_, symbol_name.c_str());
    if (!symbol)
    {
        std::cerr << "Failed to find symbol: " << dlerror() << std::endl;
        return nullptr;
    }

    Dl_info info;
    if (!dladdr(symbol, &info))
    {
        std::cerr << "dladdr failed: " << dlerror() << std::endl;
        return nullptr;
    }

    if (!info.dli_fbase)
    {
        std::cerr << "dli_fbase is null" << std::endl;
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(info.dli_fbase);
}

template<typename T>
void LibDumper<T>::AutoDumpSymbols()
{
    if (!handle_)
    {
        std::cerr << "Error: Library handle is not initialized." << std::endl;
        return;
    }

    for (const auto& [symbol, info] : symbols)
    {
        void* sym = dlsym(handle_, symbol.c_str());
        if (!sym)
        {
            std::cerr << "Failed to find symbol: " << symbol << " - " << dlerror() << std::endl;
            continue;
        }

        Dl_info dl_info;
        if (!dladdr(sym, &dl_info))
        {
            std::cerr << "dladdr failed: " << dlerror() << std::endl;
            continue;
        }

        if (!dl_info.dli_fbase)
        {
            std::cerr << "dli_fbase is null" << std::endl;
            continue;
        }

        const auto* library_base = reinterpret_cast<const uint8_t*>(dl_info.dli_fbase);

        std::cout << "Library base address: " << static_cast<const void*>(library_base) << std::endl;

        if (!ValidateHeaders(library_base))
        {
            std::cerr << "Error: Invalid library headers." << std::endl;
            return;
        }

        PrintLibInfo(library_base);
    }
}

#endif