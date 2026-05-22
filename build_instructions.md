# WiiStation — Build & Wii U Port Instructions

This project requires a specific, older version of devkitPPC to compile properly. Modern devkitPPC versions (like those currently on Docker Hub or pacman) have linker changes that cause `R_PPC_REG` and startup symbol errors (`__app_start`, `_SDA_BASE_`, etc.).

**Required Toolchain:**
- **devkitPPC r41-2**
- **libOGC2 (up to git 7456c4ab) + SDL + GNU Lightning + Lightrec**

---

## Where to Place the Files (Native/Manual Setup)

If you are setting this up on your host OS (Windows/Linux) or building a custom Docker container, follow these steps to place the files in the correct directories:

### 1. Install devkitPPC r41-2
1. Download **devkitPPC r41-2** for your OS from the archive:
   [https://wii.leseratte10.de/devkitPro/](https://wii.leseratte10.de/devkitPro/)
2. Extract the archive.
3. Place the `devkitPPC` folder in your devkitPro directory (e.g., `C:\devkitPro\devkitPPC` on Windows, or `/opt/devkitpro/devkitPPC` on Linux).
4. Ensure your environment variables are set:
   - `DEVKITPRO` points to your devkitPro base folder (e.g., `/opt/devkitpro`)
   - `DEVKITPPC` points to the devkitPPC folder (e.g., `/opt/devkitpro/devkitPPC`)

### 2. Install the modified base rules and libogc2
1. Download the `lightrec+Libogc2.zip` archive containing the modified rules and compiled libraries:
   [https://github.com/xjsxjs197/WiiSXRX_2022/raw/main/lightrec+Libogc2.zip](https://github.com/xjsxjs197/WiiSXRX_2022/raw/main/lightrec+Libogc2.zip)
2. Extract the ZIP file. You will see two folders: `devkitPPC` and `libogc2`.
3. **Copy the contents of the extracted `devkitPPC` folder** and overwrite the files in your installation's `devkitPPC` folder (specifically `devkitPPC/base_rules`).
4. **Copy the extracted `libogc2` folder** into your devkitPro base directory (e.g., `/opt/devkitpro/libogc2`).

### 3. Install the compiled SDL library
1. Download `libSDL.a` from:
   [https://github.com/xjsxjs197/WiiSXRX_2022/raw/main/libSDL.a](https://github.com/xjsxjs197/WiiSXRX_2022/raw/main/libSDL.a)
2. Place this `libSDL.a` file into your libogc2 Wii library folder, overwriting the existing one if necessary:
   `/opt/devkitpro/libogc2/lib/wii/libSDL.a` (or `C:\devkitPro\libogc2\lib\wii\libSDL.a`)

---

## How to Build

Once your environment is set up with the specific old versions and files placed correctly, you can build the emulator:

### 1. Build the Source Dependencies (First Time Only)

Open a terminal (or your MSYS2/Docker shell), navigate to the root of the repository (`WiiStation_BindTilt`), and run the dependency build script:

```bash
cd WiiStation_BindTilt
bash build_deps.sh
```

This will compile `zlib`, `libchdr`, and `opengx`.

### 2. Build the WiiStation Executable

Navigate to the `Gamecube` folder and run `make`:

```bash
cd Gamecube
make -f Makefile_Wii
```

This will produce `WiiSXRX_debug.elf` and `WiiSXRX_debug.dol`.

---

## Container Alternative (Recommended)

If you don't want to pollute your host system with old devkitPPC versions, you can use Podman or Docker to build an image that does all the manual file placement for you:

### 1. Place Required Files in the `vendor/devkitPPC/` directory
Ensure the following files are downloaded and placed into `vendor/devkitPPC/` inside your project:
- `devkitPPC-r41-2-linux_x86_64.pkg.tar.xz`
- `libSDL.a`

*(The `lightrec+Libogc2` ZIP will be automatically downloaded by the Dockerfile during the build).*

### 2. Create the `Dockerfile.build`
Create a file named `Dockerfile.build` in the root of the repo with these contents:

```dockerfile
FROM devkitpro/devkitppc:latest

USER root
RUN apt-get update && apt-get install -y wget unzip xz-utils

# 1. Overwrite modern devkitPPC with r41-2 from local vendor file
COPY WiiStation_BindTilt/vendor/devkitPPC/devkitPPC-r41-2-linux_x86_64.pkg.tar.xz /tmp/
RUN rm -rf /opt/devkitpro/devkitPPC/bin /opt/devkitpro/devkitPPC/lib /opt/devkitpro/devkitPPC/powerpc-eabi && \
    cd / && \
    tar -xf /tmp/devkitPPC-r41-2-linux_x86_64.pkg.tar.xz && \
    rm /tmp/devkitPPC-r41-2-linux_x86_64.pkg.tar.xz

# 2. Download and apply lightrec+Libogc2 (modified base_rules + libogc2 runtime)
RUN cd /tmp && \
    wget -q https://github.com/xjsxjs197/WiiSXRX_2022/raw/main/lightrec+Libogc2.zip && \
    unzip -qo lightrec+Libogc2.zip && \
    cp -r lightrec+Libogc2/devkitPPC/* /opt/devkitpro/devkitPPC/ && \
    cp -r lightrec+Libogc2/libogc2 /opt/devkitpro/ && \
    rm -rf lightrec+Libogc2 lightrec+Libogc2.zip

# 3. Install compiled libSDL.a from local vendor file
COPY WiiStation_BindTilt/vendor/devkitPPC/libSDL.a /opt/devkitpro/libogc2/lib/wii/libSDL.a

# 4. Create wii_rules symlink so dep Makefiles resolve correctly
RUN ln -sf /opt/devkitpro/libogc2/wii_rules /opt/devkitpro/devkitPPC/wii_rules

ENV PATH="/opt/devkitpro/devkitPPC/bin:${PATH}"
WORKDIR /src
```

### 3. Build and Run the Container

From the **parent directory** of the `WiiStation_BindTilt` repo (so that the `COPY` commands work correctly), run:

```bash
# Build the image (one time)
podman build --no-cache -t wiistation-r41 -f WiiStation_BindTilt/Dockerfile.build .

# Open a shell inside the container
podman run --rm -it -v "${PWD}:/src" -w /src/WiiStation_BindTilt localhost/wiistation-r41 bash
```

Inside the container, run `bash build_deps.sh` and `make -f Makefile_Wii` as described in the "How to Build" section.
