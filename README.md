# byte-plane-decomposition
Schema-free detection of optimal byte-plane decomposition width for lossless compression, with universality gap measurements across structured binary data.

This repository contains the algorithm, analysis tools, and experiment scripts for the paper "Entropy-Minimized Byte-Plane Decomposition: Auto-Detection and the Universality Gap in Lossless Compression."
Byte-plane decomposition — reordering serialized multi-byte data by byte position before compression — is widely deployed (HDF5, Blosc, Parquet) but every existing implementation requires the user to specify the field width. This work presents an algorithm that detects the optimal width automatically by minimizing average per-plane Shannon entropy, and characterizes when and why it helps across five datasets with universality gaps ranging from 1.00× to 40.8×.
