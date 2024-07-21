## Dumper
Dumper is a simple tool to dump information from a .dylib file.

### Build Process
To build Dumper, simply run the following command:

1. Clone the repository
```bash
git clone https://github.com/mendax0110/dumper.git
```

2. Change directory to the repository
```bash
cd dumper
```

3. Create the build directory
```bash
mkdir build
```

4. Change directory to the build directory
```bash
cd build
```

5. Generate the build files
```bash
cmake ..
```

6. Build the project
```bash
cmake --build .
```

### How to use Dumper

To use Dumper, simply run the following command:

```bash
./dumper <path_to_library> <symbol_name>
```

To use auto dump mode, simply run the following command:

```bash
./dumper <path_to_library> --auto-dump
```

### Example
```bash
./dumper libHello.dylib "hello_world"
```

### Supported Platforms
- Linux
- macOS