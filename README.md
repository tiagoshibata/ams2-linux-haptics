# AMS2 Linux Haptics

Linux native companion tool for Automobilista 2.

## What

Companion tool capable of reading AMS2's shared memory telemetry natively -
nothing runs in Wine, and no Windows APIs are involved!

<details>
<summary>Why</summary>

* Easy setup - no Wine/Proton setup needed
* Ease of use and development for Linux developers
* Personally, I just want more gaming software to run natively in Linux
</details>

<details>
<summary>How</summary>

I believe this is the first project to have **native** support for sim racing
telemetry. Other projects require some bridge to expose Wine's shared memory to
Linux.

This project scans the target process to locate the shared memory address in its
memory space, then reads it with `process_vm_readv`. It's technically not
"sharing" the memory, and instead copies the memory from the other process, but
the performance hit is negligible.

This "just works" in non-hardened kernels, as long as you're running AMS2 and
this companion app under the same user. If you have a hardened kernel, see
"troubleshooting" below.
</details>

## Features

Under early development. The core technical feature (reading telemetry natively)
works, but there's not much more.

* `ams2_telemetry_logger` dumps all telemetry values as a JSON
* Other under-development binaries show a TUI with real-time plots and gauges

## Troubleshooting

This app needs tracing capability in order to read the memory from AMS2. If you
have a hardened kernel, you can (sorted from "least intrusive"/"most preferred"
to "most intrusive"):

1. run this with `CAP_SYS_PTRACE` privilege
2. run this with `sudo`
3. change your system settings to allow unprivileged debugging

## Game support

Currently, only AMS2 is supported. Expanding to other titles should be trivial,
but I don't own them.

## Requirements

TODO

## Build

```bash
cmake -B build
cmake --build build
```

## Usage

TODO
