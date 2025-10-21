# lssrc

`lssrc` is a simple project listing tool for x64 Linux. It sorts by mtime in descending order and shows an emoji based on project contents. Usage: `lssrc [dir]`.

Example output:

```shell
$ lssrc ~/src
🔨 lssrc
🔨 dwm-flexipatch
🔨 st-flexipatch
🦀 rustlings
```

Build with `make`.

Install with `make install`.

Usage example, dwm keybinding:

```c
static const char *lssrc[] = {
    "/usr/bin/zsh", "-c",
    "lssrc ~/src | dmenu | cut -d' ' -f2- | xargs -rI {} codium ~/src/'{}'",
    NULL};
static const Key keys[] = {
    // ...
{MODKEY | ShiftMask, XK_p, spawn, {.v = lssrc}},
    // ...
};
```

Usage example, Hyprland keybinding:

```text
bind = $mainMod SHIFT, R, exec, lssrc ~/src | wofi -d | cut -d' ' -f2- | xargs -rI{} codium ~/src/'{}'
```
