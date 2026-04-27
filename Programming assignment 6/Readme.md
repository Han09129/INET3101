# Module 6: Modifying `ls` and `whoami` (System Programming in C)

## 1. Problem Statement

This assignment focuses on modifying source code from the GNU Coreutils package to better understand system programming in C and how common Linux commands are implemented.

Two modifications were required:

* **Part 1 (`ls.c`)**: Change the default behavior of the `ls` command so that it displays output in **long format** by default (same as `ls -l`).
* **Part 2 (`whoami.c`)**: Modify the `whoami` command so that it prints `"You are: "` before displaying the current user ID.

---

## 2. Description of the Solution

### Modification to `ls.c`

The `ls` command determines its output format inside the `decode_switches` function. By default, when no command-line flags are provided, it uses a multi-column format.

To change this behavior, the logic controlling the default format was modified. Specifically, the portion handling the `LS_LS` mode (normal `ls` execution) was updated to use `long_format` instead of `many_per_line`.

#### Key Change:

```c
: ls_mode == LS_LS ? long_format
```

This ensures that running `ls` without any flags produces the same output as `ls -l`, while still preserving functionality for other flags such as `-C` and `-1`.

---

### Modification to `whoami.c`

The original implementation prints only the username using:

```c
puts(pw->pw_name);
```

This was modified to include a descriptive prefix:

```c
printf("You are: %s\n", pw->pw_name);
```

This change required no structural modifications—only an update to the output formatting.

---

## 3. How to Build and Run

### Build

```bash
chmod +x build_on_wsl.sh
./build_on_wsl.sh
```

### Run

```bash
./build/coreutils-9.7/src/ls
./build/coreutils-9.7/src/whoami
```

---

## 4. Expected Output

### `ls`

Displays files in long format by default:

```
-rw-r--r-- 1 user user 654 Aug 25 17:17 README.md
```

### `whoami`

```
You are: username
```

---

## 5. Files Included

* `ls.c` – Modified to change default output format
* `whoami.c` – Modified to change output message
* `README.md` – This file

## 6. Screenshot
