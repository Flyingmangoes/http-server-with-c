# HTTP SERVER IN C

---

## Requirements

### Windows

You can build this using one of these:
MinGW-w64 (gcc)
MSYS2

---

## Compiling

### Windows

```bash
gcc -Isrc/include ^
    src\main.c ^
    src\program\server.c ^
    src\program\http.c ^
    src\program\bstring.c ^
    -o httpserver.exe
```

---

## Run

### Windows
```bash
./httpserver.exe
```

