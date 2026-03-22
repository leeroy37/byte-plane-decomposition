#!/usr/bin/env python3
"""
Figure 1: Universality gap G versus absolute MSB entropy.

Generates the scatter plot for the paper showing all five datasets
positioned on the gap curve.
"""

import matplotlib.pyplot as plt
import numpy as np

# Data points: (MSB entropy in bits, universality gap G, label)
datasets = [
    (0.00, 40.8,  "Sensor telemetry\n(synthetic)"),
    (0.17,  1.19, "Intel Lab\n(real IoT)"),
    (0.47,  1.01, "MRI\n(Silesia)"),
    (3.19,  1.02, "SAO\n(Silesia)"),
    (6.0,   1.00, "NASDAQ ITCH\n(real financial)"),
]

fig, ax = plt.subplots(figsize=(8, 5))

msb_entropy = [d[0] for d in datasets]
gap = [d[1] for d in datasets]
labels = [d[2] for d in datasets]

# Scatter plot with log Y axis
ax.scatter(msb_entropy, gap, s=100, c='#2563eb', zorder=5, edgecolors='black', linewidth=0.5)

# Labels
offsets = [(0.2, 1.3), (0.3, 1.15), (0.3, -0.15), (0.3, 0.0), (-1.5, 0.05)]
for i, (x, y, label) in enumerate(datasets):
    dx, dy = offsets[i]
    ax.annotate(label, (x, y), textcoords="offset points",
                xytext=(dx * 30, dy * 30), fontsize=8,
                arrowprops=dict(arrowstyle='->', color='gray', lw=0.8) if i == 0 else None,
                ha='left' if i < 4 else 'right')

ax.set_yscale('log')
ax.set_xlabel('Absolute MSB Entropy (bits/byte)', fontsize=11)
ax.set_ylabel('Universality Gap G', fontsize=11)
ax.set_title('Universality Gap vs. MSB Entropy', fontsize=13)
ax.set_xlim(-0.5, 7.5)
ax.set_ylim(0.8, 60)
ax.axhline(y=1.0, color='gray', linestyle='--', linewidth=0.5, alpha=0.7)
ax.text(7.0, 1.05, 'G = 1 (no benefit)', fontsize=7, color='gray', ha='right')
ax.grid(True, alpha=0.3)
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)

plt.tight_layout()
plt.savefig('figures/universality_gap.pdf', dpi=300, bbox_inches='tight')
plt.savefig('figures/universality_gap.png', dpi=150, bbox_inches='tight')
print("Saved figures/universality_gap.pdf and .png")
