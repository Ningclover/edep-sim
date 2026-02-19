# Environment Setup on WC Server

Setting up ROOT, Geant4, PyROOT, and edep-sim (`TG4Event`) on a WC server (wcgpu1)
using the WCWC software ecosystem.

---

## Prerequisites

- Account on a WC server (e.g. `wcgpu1`)
- WCWC installed at `/wcwc/`
- `direnv` available (`which direnv`)
- A working directory on NFS (e.g. `/nfs/data/1/<username>/optic-gpu/`)

---

## Step 1 — Create a Spack environment from WCWC packages

WCWC provides pre-built ROOT and Geant4 packages.
Use `wcwc list <name>` to browse available builds and find the hash you want.

```bash
wcwc list root        # find ROOT builds → note the hash (e.g. kl5ziao)
wcwc list geant4      # find Geant4 builds → note the hash (e.g. wsqk5qa)
wcwc list py-numpy    # find NumPy builds → note the hash (e.g. x6roszw)
```

Then create the environment in one command using `wcwc env`:

```bash
wcwc env -e /nfs/data/1/xning/optic-gpu/env \
    root/kl5ziao geant4/wsqk5qa py-numpy/x6roszw
```

- `-e` sets the environment directory
- The trailing `spec/hash` arguments are added to the environment
- All packages come from WCWC's pre-built tree — no compilation needed

`wcwc env` is idempotent: you can run it again later to add more packages.

Spack creates a symlink tree at `env/.spack-env/view/` pointing to all packages.

---

## Step 2 — Build edep-sim against Geant4 11

Clone the fork (already patched for Geant4 11 compatibility):

```bash
git clone https://github.com/Ningclover/edep-sim \
    /nfs/data/1/xning/optic-gpu/edep-sim
```

Build inside the activated env so CMake finds ROOT and Geant4:

```bash
wcwc shell -e /nfs/data/1/xning/optic-gpu/env -c "
    cmake -B /nfs/data/1/xning/optic-gpu/edep-sim/build \
          -S /nfs/data/1/xning/optic-gpu/edep-sim \
          -DCMAKE_INSTALL_PREFIX=/nfs/data/1/xning/optic-gpu/install
    cmake --build /nfs/data/1/xning/optic-gpu/edep-sim/build -j\$(nproc)
"
```

Key build outputs:

| File | Purpose |
|------|---------|
| `edep-sim/build/app/edep-sim` | Simulation binary |
| `edep-sim/build/io/libedepsim_io.so` | ROOT I/O library — contains `TG4Event`, `TG4HitSegment`, etc. |

> **Why a fork?** The upstream edep-sim was written for Geant4 10.x.
> Geant4 11 removed several per-particle process classes and changed some APIs.
> The fork applies the necessary compatibility patches.

---

## Step 3 — Install matplotlib locally

matplotlib is not available in WCWC.  Install it to a project-local directory
using `pip --target` (avoids polluting `~/.local` or the system Python):

```bash
pip install --target /nfs/data/1/xning/optic-gpu/pypackages matplotlib
```

This installs matplotlib and all its dependencies (Pillow, contourpy, cycler, etc.)
into `pypackages/`, self-contained within the project.

---

## Step 4 — Configure `.envrc` for auto-activation with direnv

Create `/nfs/data/1/xning/optic-gpu/.envrc`:

```bash
source /wcwc/spack/share/spack/setup-env.sh
spack env activate /nfs/data/1/xning/optic-gpu/env

# ROOT .so files live under lib/root/ — Spack does not set LD_LIBRARY_PATH,
# so the dynamic linker can't find libHist.so etc. unless we add it here.
# This MUST be set before Python starts; setting os.environ at runtime is too late.
ENV_VIEW=/nfs/data/1/xning/optic-gpu/env/.spack-env/view
export LD_LIBRARY_PATH="${ENV_VIEW}/lib/root:${ENV_VIEW}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

# edep-sim I/O library (TG4Event, TG4HitSegment, etc.)
export LD_LIBRARY_PATH="/nfs/data/1/xning/optic-gpu/edep-sim/build/io:${LD_LIBRARY_PATH}"

# Cling (ROOT's C++ JIT) header search path — needed to parse TG4Event.h
export ROOT_INCLUDE_PATH="/nfs/data/1/xning/optic-gpu/edep-sim/io${ROOT_INCLUDE_PATH:+:${ROOT_INCLUDE_PATH}}"

# matplotlib and other local pip packages
export PYTHONPATH="/nfs/data/1/xning/optic-gpu/pypackages${PYTHONPATH:+:${PYTHONPATH}}"
```

Allow it once (direnv requires explicit user approval):

```bash
direnv allow /nfs/data/1/xning/optic-gpu
```

From this point, `cd /nfs/data/1/xning/optic-gpu` automatically activates the full environment.

---

## Step 5 — Verify

```bash
cd /nfs/data/1/xning/optic-gpu    # direnv loads the env

python3 - << 'EOF'
import ROOT
ROOT.gSystem.Load("/nfs/data/1/xning/optic-gpu/edep-sim/build/io/libedepsim_io.so")
ROOT.gROOT.ProcessLine('#include "TG4Event.h"')
from ROOT import TG4Event, TFile, TChain, TF1, TH2D, TH1D
print("OK — TG4Event:", TG4Event)
print("OK — TH2D:    ", TH2D)
EOF
```

Expected output:
```
OK — TG4Event: <class cppyy.gbl.TG4Event at 0x...>
OK — TH2D:     <class cppyy.gbl.TH2D at 0x...>
```

---

## Environment variable reference

| Variable | Value added | Why it is needed |
|----------|-------------|-----------------|
| `LD_LIBRARY_PATH` | `env/view/lib/root` | ROOT shared libs (`libHist.so`, `libCore.so`, etc.) |
| `LD_LIBRARY_PATH` | `env/view/lib` | Other Spack package libs |
| `LD_LIBRARY_PATH` | `edep-sim/build/io` | `libedepsim_io.so` — `TG4Event` and friends |
| `ROOT_INCLUDE_PATH` | `edep-sim/io` | Cling JIT finds `TG4Event.h` without manual `AddIncludePath` |
| `PYTHONPATH` | `optic-gpu/pypackages` | matplotlib, Pillow, etc. |
| `PYTHONPATH` | `env/view/lib/root` | ROOT Python bindings (`import ROOT`) — set by `spack env activate` |

> **Key gotcha:** `LD_LIBRARY_PATH` must be set **before Python starts**.
> The dynamic linker reads it at process startup; modifying `os.environ` inside
> a running Python process has no effect on already-loaded shared libraries.
> `.envrc` sets it in the shell, which is inherited by every child process.

---

## Directory layout

```
optic-gpu/
├── .envrc                  ← direnv config (auto-activates everything)
├── env/                    ← Spack environment
│   ├── spack.yaml          ← package specs (ROOT, Geant4, NumPy)
│   ├── .spack-env/view/    ← symlink tree to all installed packages
│   └── opt/                ← local install tree (if any packages rebuilt)
├── edep-sim/               ← edep-sim source (Geant4 11 patched fork)
│   ├── build/
│   │   ├── app/edep-sim        ← simulation binary
│   │   └── io/libedepsim_io.so ← ROOT I/O library
│   └── io/TG4Event.h           ← header needed by Cling
├── pypackages/             ← pip --target install (matplotlib etc.)
├── install/                ← edep-sim install prefix
└── scripts/                ← helper scripts
    ├── plot_trajectories.py
    ├── extract_tracks.C
    └── run-test.sh
```
