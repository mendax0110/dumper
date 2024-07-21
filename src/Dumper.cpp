#include <iostream>
#include <string>
#include "../include/LibDumper.h"

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: " << argv[0] << " <library_path> [<symbol_name> | --auto-dump]" << std::endl;
        return 1;
    }

    std::string library_path = argv[1];
    std::string symbol_name;
    bool auto_dump = (argc == 3 && std::string(argv[2]) == "--auto-dump");

    if (argc == 3 && !auto_dump)
    {
        symbol_name = argv[2];
    }

    LibDumper<void> dumper;
    if (!dumper.LoadLibraryFromPath(library_path))
    {
        std::cerr << "Failed to load library" << std::endl;
        return 1;
    }

    if (auto_dump)
    {
        dumper.AutoDumpSymbols();
    }
    else if (!symbol_name.empty())
    {
        void* handle = dumper.getHandle();
        if (!handle)
        {
            std::cerr << "Library handle is null" << std::endl;
            return 1;
        }

        const uint8_t* library_base = dumper.GetBaseAddress(symbol_name);
        if (!library_base)
        {
            std::cerr << "Failed to get library base address" << std::endl;
            return 1;
        }

        std::cout << "Library base address: " << static_cast<const void*>(library_base) << std::endl;

        if (!dumper.ValidateHeaders(library_base))
        {
            std::cerr << "Invalid library headers" << std::endl;
            return 1;
        }

        dumper.PrintLibInfo(library_base);
    }
    else
    {
        std::cerr << "Invalid usage" << std::endl;
        return 1;
    }

    return 0;
}
