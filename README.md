# LVGL for Wayland

LVGL configured to work with Wayland wl_shell on Linux.

When cloning this repository, also make sure to download submodules (`git submodule update --init --recursive`) otherwise you will be missing key components.

## Build the project

```
mkdir build && cd build
cmake ..
make
```

## Run the demo application

```
./lvgl-demo-wl
```
