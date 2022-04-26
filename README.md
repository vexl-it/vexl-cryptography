# Vexl cryptography
Vexl cryptography (VC) is a shared cryptographic library used for Vexl mobile app. The purpose of this library is to implement high-level interface of various cryptographic algorithms using the OpenSSL framework.
This library includes implemention of following algorithms:
```markdown
 - ECIES
 - AES-256
 - ECDSA
 - HMAC
 - SHA-256
```

## Dependencies
This project assumes, you use Mac with Apple silicon for building the library. Althought It should be possible to use the intel mac variant, you may run into compatibility issues when compiling for arm targets.

### Dependencies for Apple targets
Apple targets depend on latest version of xcode. You can download latest version of Xcode from the Mac AppStore.

### Dependencies for Android targets
To be able to compile the android target for this library, please install android studio and the latest Native Developer Kit (NDK). The NDK provides cross-compilers for the android platform with various ABIs. To install NDK in android studio, go to `Tools -> SDK Manager -> Android SDK -> SDK Tools`, check `NDK` and click apply. 

### Dependencies for Linux target
Currently it is not possible to natively build any C code directly for linux operating system on Mac. To mitigate this limitation, you need to install docker. Our custom build script uses docker to spin arm and x86 containers, to build the library for the linux target.

### Dependencies for Windows target
To be able to compile the windows target, you have to install MinWG cross compiler. On apple silicon, you run the folliwng command
```shell
brew install mingw-w64
```

## Building
Vexl cryptography is built to support multiple architectures across wide spectrum of platforms.
VC currently supports following targets:

```markdown
 - darwin-x86_64        (macOS with Intel CPU, 64-bit)
 - darwin-arm64         (macOS with Apple Silicon SoC, 64-bit)
 - ios-simulator-x86_64 (iOS simulator running on macOS with Intel CPU, 64-bit)
 - ios-simulator-arm64  (iOS simulator running on macOS with Apple Silicon SoC, 64-bit)
 - ios-arm64            (iOS devices, 64-bit)
 - android-x86_64       (android with 64-bit x86 cpu)
 - android-x86          (android with 32-bit x86 cpu)
 - android-armv8        (android with 64-bit arm processors)
 - android-armv4        (android with 32-bit arm processors, compatible with armv7 processors)
 - linux-x86_64         (linux running on a 64-bit x86 cpu)
 - linux-aarch64        (linux running on a 64-bit arm cpu)
```

To build the vexl crypto library for all targets, just run

```shell
make all
```

Although It can be useful to compile every target at once,  To compile VC for specific target, follow one of the following building sections.

### Building for Apple devices.
```shell
make apple
```
The archive for all resulting architectures can be found in `./product/apple/vexl_crypto_ios_xcframework.zip`

*Please note the the library for apple targets is only one singular `.xcframework` library. This library uses apples latest methods for bundling multiple architectures.* 

### Building for Android
```shell
make android
```
The archive for all resulting architectures can be found in `./product/android/vexl_crypto_android_frameworks.zip`

### Building for Linux
```shell
make linux
```
The archive for all resulting architectures can be found in `./product/linux/vexl_crypto_linux_frameworks.zip`

### Building for Windows
```shell
make windows
```
The archive for all resulting architectures can be found in `./product/windows/vexl_crypto_windows_frameworks.zip`

### Cleaning the build
Sometimes it is useful to run the `make` command from scratch. For cleaning all the build files run
```shell
make clean
```

## Building OpenSSL library
Vexl crypto is built around the open source cryptoraphic library OpenSSL. To be able to support all the platforms and architectures, we provide a custom script for building the openssl library for different targets.
To build OpenSSL run the script with a specific target arguemnt. To get the list of supported targets, run the help function:

```shell
./build.sh --help
```

The resulting openssl library can then be found in `../openssl/lib` folder.

## Testing
The vexl cyptography repository provides you with two types of tests. Integration tests and a performance tests.

First build the library for current host target
```shell
make
```
Then to run the integration tests, just run
```shell
make run
```
Or to run the performance tests, run:
```shell
make run-tests
```

## Implementation example
For the latest library usage example please check the source code for integration tests in `./tests/Tests.c` 