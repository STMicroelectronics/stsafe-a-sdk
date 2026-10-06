# STSAFE-A Software Development Kit

The **STSAFE-A Software Development Kit** offers developers a collection of application examples demonstrating the use of the STSAFE-A secure element from STMicroelectronics.
This SDK serves as software reference implementations for integrating [STSELib](https://github.com/STMicroelectronics/STSELib) and STSAFE-A on host microcontroller platforms.

## Table of Contents

- [Hardware and Software Prerequisites](#hardware-and-software-prerequisites)
- [Getting Started](#getting-started)
  - [1. Hardware Setup](#1-hardware-setup)
  - [2. Clone the Repository](#2-clone-the-repository)
  - [3. Install STM32 Cryptographic Library (CMOX)](#3-install-stm32-cryptographic-library-cmox)
  - [4. Open, Build, and Run a Project with STM32CubeIDE](#4-open-build-and-run-a-project-with-stm32cubeide)
  - [5. Observe the Output](#5-observe-the-output)
  - [6. Select an Example](#6-select-an-example)
- [Repository Structure](#repository-structure)
- [Documentation](#documentation)

```
+------------------------+---------------------------------------------------------------------+
|                        |                                                                     |
|                        |   +-------------------------------------------------------------+   |
|       APPLICATION      |   |                 Example Projects / Tools                    |   |
|                        |   +-------------------------------------------------------------+   |
|                        |                                                                     |
|========================|=====================================================================|
|                        |   +-------------------------------------------------------------+   |
|                        |   |                             |                               |   |
|                        |   |                             |                               |   |
|       MIDDLEWARE       |   |            STSELib          |       STM32Cryptolib          |   |
|                        |   |                             |           (CMOX)              |   |
|                        |   |                             |                               |   |
|                        |   +-------------------------------------------------------------+   |
|                        |                                                                     |
|========================|=====================================================================|
|                        |                                                                     |
|                        |   +-------------------------------------------------------------+   |
|        PLATFORM        |   |                       NUCLEO-L452RE                         |   |
|                        |   +-------------------------------------------------------------+   |
|                        |                                                                     |
+------------------------+---------------------------------------------------------------------+

```

## Hardware and Software Prerequisites

The following hardware (HW) and software (SW) prerequisites are required to work with this package:

- **Hardware prerequisites**
  - [NUCLEO-L452RE](https://www.st.com/en/evaluation-tools/nucleo-l452re.html) STM32 development board  
  - [X-NUCLEO-ESE01A1](https://www.st.com/en/evaluation-tools/x-nucleo-ese01a1.html) Nucleo expansion board

- **Software prerequisites**
  - [Git](https://git-scm.com/downloads) to clone the repository and its submodules
  - One of the following compatible toolchains/IDEs:
    - [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)  
    - [Keil uVision 5.37](https://www.st.com/en/partner-products-and-services/arm-keil-mdk.html)  
    - [IAR EWARM 9.40.1](https://www.st.com/en/partner-products-and-services/iar-embedded-workbench-for-arm.html)

  - [X-CUBE-CRYPTOLIB 4.5.0](https://www.st.com/en/embedded-software/x-cube-cryptolib.html)
  - A serial terminal, such as [Tera Term](https://teratermproject.github.io/) or [PuTTY](https://www.putty.org/), to view example output

## Getting Started

The steps below use STM32CubeIDE and the `01_Echo_loop` example to check communication with the STSAFE-A120 before running other examples.

### 1. Hardware Setup

1. With power disconnected, plug the X-NUCLEO-ESE01A1 expansion board onto the NUCLEO-L452RE Arduino connectors. Check the connector alignment and ensure the boards are fully seated.
2. Connect the NUCLEO-L452RE to your PC through its ST-LINK USB connector using a data-capable USB cable. This connection powers the board and provides programming, debugging, and a virtual COM port.
3. Check that your PC detects the ST-LINK interface and the virtual COM port. On Windows, the COM port appears under **Ports (COM & LPT)** in Device Manager. If the interfaces are not detected, check the cable and install the [ST-LINK USB driver](https://www.st.com/en/development-tools/stsw-link009.html).

### 2. Clone the Repository

The SDK uses Git submodules for `Middleware/STSELib`, `stse-host-platform`, and `stse-crypto-platform`. Clone the repository with its dependencies:

```sh
git clone --recurse-submodules https://github.com/STMicroelectronics/stsafe-a-sdk.git
cd stsafe-a-sdk
```

If you already cloned the repository without submodules, run the following from the repository root:

```sh
git submodule update --init --recursive
```

Use Git rather than GitHub's **Download ZIP** option, which does not include the submodule contents.

### 3. Install STM32 Cryptographic Library (CMOX)

CMOX is distributed separately and is not included in this repository. Install it before building a project:

1. Download **X-CUBE-CRYPTOLIB v4.5.0** from [STMicroelectronics](https://www.st.com/en/embedded-software/x-cube-cryptolib.html) and extract the archive.
2. Locate `STM32CubeExpansion_Crypto_V4.5.0/Middlewares/ST/STM32_Cryptographic/` in the extracted package.
3. Copy the **contents** of that directory into `Middleware/STM32_Cryptographic/` in this repository. Do not create an extra nested `STM32_Cryptographic` directory.

The destination should contain:

```text
Middleware/STM32_Cryptographic/
|-- ReadMe.md                         # SDK installation instructions (already present)
|-- CMOX.chm
|-- LICENSE.txt
|-- ReadMe.txt
|-- Release_Notes.html
|-- _htmresc/
|-- include/
|-- interface/
|   `-- cmox_low_level_template.c
|-- legacy_v3/
`-- lib/
```

4. Open `Middleware/STM32_Cryptographic/interface/cmox_low_level_template.c` and comment out these two lines in `cmox_ll_init()`:

```c
//  __HAL_RCC_CRC_RELEASE_RESET();
//  __HAL_RCC_CRC_CLK_ENABLE();
```

CRC clock management is already handled by the project's `system.c`. See the local [CMOX installation instructions](Middleware/STM32_Cryptographic/ReadMe.md) for the complete function example.

### 4. Open, Build, and Run a Project with STM32CubeIDE

1. Launch STM32CubeIDE and select or create a workspace.
2. Select **File > Import > General > Existing Projects into Workspace**.
3. Set the root directory to `Applications/Projects/01_Echo_loop/STM32CubeIDE/` inside your clone and select the detected project. Leave **Copy projects into workspace** unchecked so the project's relative paths to shared SDK files remain valid, then select **Finish**.
4. Right-click the imported project in Project Explorer and select **Build Project**. Check the Console view for a successful build.
5. With the board connected, select **Run > Debug** to program it and start a debug session. If prompted, select the project's STM32 debug configuration.
6. Open the serial terminal as described below before resuming execution. If the debugger stops at `main()`, select **Resume** (`F8`) to run the example.

If the build reports missing STSELib or platform files, repeat the submodule initialization in step 2. For missing CMOX headers or libraries, check the destination directory in step 3.

The [project template](Applications/Projects/00_Project_template) also includes `EWARM` and `MDK-ARM` project folders for IAR and Keil users; the walkthrough above uses STM32CubeIDE.

### 5. Observe the Output

Open your serial terminal on the ST-LINK virtual COM port with these settings:

| Setting | Value |
| --- | --- |
| Baud rate | 115200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |

The echo example starts immediately and repeatedly prints a generated message and the message returned by the STSAFE-A120. It does not require a key press. Other examples may display a prompt; press a key only when requested.

If no output appears, check the COM port and serial settings, ensure execution has resumed in the debugger, and reset the board to see the startup output. If initialization or echo errors appear, check the expansion board connection before proceeding to provisioning examples.

### 6. Select an Example

Start with group `01`, which does not require running a provisioning example first:

| Example | Purpose |
| --- | --- |
| [01_Echo_loop](Applications/Projects/01_Echo_loop) | Check communication by sending and verifying echoed messages |
| [01_Device_authentication](Applications/Projects/01_Device_authentication) | Authenticate the device |
| [01_Device_authentication_multi_steps](Applications/Projects/01_Device_authentication_multi_steps) | Follow the individual device authentication steps |
| [01_Random_number](Applications/Projects/01_Random_number) | Generate random data with the secure element |
| [01_Hash](Applications/Projects/01_Hash) | Compute a hash |

The numeric prefix indicates dependencies between example groups:

| Group | Prerequisite groups |
| --- | --- |
| `00` | None (project template) |
| `01` | None |
| `02` | None (configuration examples) |
| `03` | `02` |
| `04` | None (configuration examples) |
| `05` | `02` and `04` |

Configuration and provisioning examples change device settings or keys. Read the selected example's source and any accompanying README before running it, and follow the [project dependency guide](Applications/Projects/ReadMe.md) rather than running all examples indiscriminately.

## Repository Structure

```text
stsafe-a-sdk/
|-- Applications/
|   |-- Apps_utils/             # Shared application utilities
|   `-- Projects/               # Example projects and IDE configurations
|-- Middleware/
|   |-- STM32_Cryptographic/    # CMOX library (install separately)
|   `-- STSELib/                # Secure element library (Git submodule)
|-- stse-crypto-platform/       # Cryptographic adaptation layer (Git submodule)
`-- stse-host-platform/         # Host hardware adaptation layer (Git submodule)
```

## Documentation

A dedicated [wiki page](https://github.com/STMicroelectronics/stsafe-a-sdk/wiki) is available on GitHub.  
This wiki can be used as an entry point for embedded system developers. It covers the application hardware and software description, and provides developers with product information and standard STSAFE-A use cases.
