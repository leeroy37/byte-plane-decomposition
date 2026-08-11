#!/usr/bin/env bash
# Builds paper/paper.pdf from paper/paper.md via pandoc + xelatex.
#
# Requirements (Ubuntu/Debian):
#   apt-get install pandoc texlive-latex-base texlive-latex-recommended \
#     texlive-latex-extra texlive-fonts-recommended texlive-luatex \
#     texlive-xetex lmodern fonts-dejavu
#
# Usage: ./paper/build_pdf.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_MD="$(mktemp -t bpd_paper_XXXX.md)"
trap 'rm -f "$BUILD_MD"' EXIT

python3 - "$SCRIPT_DIR/paper.md" "$BUILD_MD" "$REPO_ROOT/figures/universality_gap.pdf" << 'PYEOF'
import re
import sys

src_path, dst_path, figure_path = sys.argv[1:4]
text = open(src_path, encoding="utf-8").read()

# Replace the H1 title + divider with YAML front matter so pandoc renders
# a proper title block instead of a plain heading.
text = text.replace(
    "# Entropy-Minimized Byte-Plane Decomposition: Auto-Detection and the "
    "Universality Gap in Lossless Compression\n\n---\n\n## Abstract",
    '---\n'
    'title: "Entropy-Minimized Byte-Plane Decomposition: Auto-Detection and '
    'the Universality Gap in Lossless Compression"\n'
    'author: "Steffen Goehring"\n'
    'date: "August 2026"\n'
    '---\n\n## Abstract',
    1,
)

# Attach the reproducibility footnote to its natural anchor point instead of
# leaving it as an unreferenced footnote definition.
text = text.replace(
    "We make two contributions:\n",
    "We make two contributions:[^1]\n",
    1,
)

# Embed the universality-gap figure inline instead of a "see figures/..."
# pointer, folding the existing caption text into the image's alt text.
text = text.replace(
    "**Figure 1:** Universality gap $G$ versus absolute MSB entropy for all "
    "five datasets: sensor_log (0.00, 40.8), Intel Lab (0.17, 1.19), MRI "
    "(0.47, 1.02), SAO (3.19, 1.01), NASDAQ (6.0, 1.00). The gap decreases "
    "toward 1.0 as MSB entropy increases, confirming that near-zero MSB "
    "entropy is the primary driver of large gaps. See "
    "`figures/universality_gap.pdf`.",
    "![Universality gap $G$ versus absolute MSB entropy for all five "
    "datasets: sensor\\_log (0.00, 40.8), Intel Lab (0.17, 1.19), MRI "
    "(0.47, 1.02), SAO (3.19, 1.01), NASDAQ (6.0, 1.00). The gap decreases "
    "toward 1.0 as MSB entropy increases, confirming that near-zero MSB "
    f"entropy is the primary driver of large gaps.]({figure_path})"
    "{width=75%}",
    1,
)

# Turn bare reference URLs into autolinks so xurl can break them across
# lines instead of overflowing the page margin.
def wrap(m):
    url = m.group(0)
    trail = ""
    while url and url[-1] in ".,":
        trail = url[-1] + trail
        url = url[:-1]
    return f"<{url}>{trail}"

text = re.sub(r"(?<!<)https?://\S+", wrap, text)

open(dst_path, "w", encoding="utf-8").write(text)
PYEOF

pandoc "$BUILD_MD" \
  -o "$SCRIPT_DIR/paper.pdf" \
  --pdf-engine=xelatex \
  -H "$SCRIPT_DIR/header.tex" \
  -V fontsize=11pt \
  -V papersize=letter \
  -V colorlinks=true \
  -V linkcolor=NavyBlue \
  -V urlcolor=NavyBlue \
  -V citecolor=NavyBlue

echo "Wrote $SCRIPT_DIR/paper.pdf"
