---
description: How to build and run the FacetoneSoftPhone application
---

This workflow guides you through the process of building and running the `FacetoneSoftPhone` application.

## Prerequisites

Ensure you have the following installed:
- `cmake`
- `make`
- `g++` (or another C++17 compatible compiler)
- `qt6-base-dev` and `qt6-declarative-dev` (or similar Qt6 development packages)
- `linphone-dev` (liblinphone development files)

## Build Instructions

1.  Open a terminal in the project root directory.
2.  Create a `build` directory:
    ```bash
    mkdir -p build
    ```
3.  Navigate into the `build` directory:
    ```bash
    cd build
    ```
4.  Run `cmake` to configure the project:
    ```bash
    cmake ..
    ```
5.  Compile the project:
    ```bash
    make
    ```

## Run Instructions

1.  Before running, you may want to update your SIP credentials in `src/main.cpp`:
    - `username`
    - `password`
    - `domain` (SIP server domain)
2.  If you have modified the code, rebuild using `make`.
3.  Run the executable from the `build` directory:
    ```bash
    ./facetonesoftphone
    ```

> [!NOTE]
> Ensure your SIP server (e.g., FreeSWITCH) is accessible and configured to handle registration and calls.
