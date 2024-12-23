# vcpkg install
This is a guide of the variaus ways to install vcpkg for maneging cmake and ninja version.
## Windows:
Winget
```
winget install vcpkg
```
PowerShell
```
iex (iwr -useb https://aka.ms/vcpkg-init.ps1)
```
## Mac
Brew
```
brew install vcpkg
```
bash/zsh/fish
```
. <(curl https://aka.ms/vcpkg-init.sh -L)
```
## linux
TODO: Find the package from yum and apt
bash/zsh/fish
```
. <(curl https://aka.ms/vcpkg-init.sh -L)
```
## Manual
The script should work and due to additional requirements this sould just be used only as a backup method

### Windows

#### Prerequisites

-   A terminal
-   MSVC
-   [CMake](https://cmake.org/download/)
-   [Git](https://git-scm.com/downloads)
1. Clone the repo
```
git clone https://github.com/microsoft/vcpkg.git
```
2. Run bootstrap script
```
cd vcpkg; .\bootstrap-vcpkg.bat
```

### Mac/Linux
1. Clone the repo
```
git clone https://github.com/microsoft/vcpkg.git
```
2. Run bootstrap script
```
cd vcpkg && bootstrap-vcpkg.bat
```
3. Configure the `VCPKG_ROOT` environment variable
```
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```
__NOTE: To make it permenent, add it to your profile (e.g., `~/.bashrc` or `~/.zshrc`)__