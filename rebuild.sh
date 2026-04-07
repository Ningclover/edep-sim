cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/nfs/data/1/xning/edep-sim-working/edep-sim/install
cmake --build build -j$(nproc)
cmake --install build 
