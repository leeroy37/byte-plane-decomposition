#!/usr/bin/env bash
# Builds an arXiv-ready LaTeX source package from paper/paper.md.
#
# Unlike build_pdf.sh (which targets a standalone PDF via xelatex+DejaVu),
# this targets plain pdflatex with standard Latin Modern fonts, since arXiv
# explicitly prefers (La)TeX source over a pre-compiled PDF and compiles
# submissions with its own TeX Live install. See:
# https://info.arxiv.org/help/submit/index.html#formats-for-text-of-submission
#
# Output: paper/arxiv/main.tex, paper/arxiv/universality_gap.pdf, and (for
# local review only, not part of the arXiv upload) paper/arxiv/main.pdf.
# To submit: tar czf submission.tar.gz -C paper/arxiv main.tex universality_gap.pdf
#
# Requirements (Ubuntu/Debian):
#   apt-get install pandoc texlive-latex-base texlive-latex-recommended \
#     texlive-latex-extra texlive-fonts-recommended lmodern
#
# Usage: ./paper/build_tex.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUT_DIR="$SCRIPT_DIR/arxiv"
mkdir -p "$OUT_DIR"

BUILD_MD="$(mktemp -t bpd_paper_tex_XXXX.md)"
trap 'rm -f "$BUILD_MD"' EXIT

cp "$REPO_ROOT/figures/universality_gap.pdf" "$OUT_DIR/universality_gap.pdf"

python3 - "$SCRIPT_DIR/paper.md" "$BUILD_MD" << 'PYEOF'
import re
import sys

src_path, dst_path = sys.argv[1:3]
text = open(src_path, encoding="utf-8").read()

# --- Title block -----------------------------------------------------
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

# --- Footnote anchor ---------------------------------------------------
text = text.replace(
    "We make two contributions:\n",
    "We make two contributions:[^1]\n",
    1,
)

# --- Inline the figure, using a bare relative filename (arXiv flattens
# the submission directory, so the figure must sit next to main.tex). ---
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
    "entropy is the primary driver of large gaps.](universality_gap.pdf)"
    "{width=75%}",
    1,
)

# --- Reference URLs as autolinks (needs xurl in the LaTeX header, or at
# minimum keeps them as real \url{} hyperlinks instead of raw text). ----
def wrap_url(m):
    url = m.group(0)
    trail = ""
    while url and url[-1] in ".,":
        trail = url[-1] + trail
        url = url[:-1]
    return f"<{url}>{trail}"

text = re.sub(r"(?<!<)https?://\S+", wrap_url, text)

# --- Replace symbols plain pdflatex + inputenc(utf8) can't render. -----
# ×, †, ←, →, ·, –, —, á, ü are all fine as literal UTF-8 under
# pdflatex + inputenc(utf8) + fontenc(T1) + lmodern + textcomp, so those
# are left untouched. Greek letters need math mode in prose but a plain
# ASCII word inside verbatim/code blocks (math markup isn't interpreted
# there); checkmarks/crosses need the pifont dingbat macros everywhere.

GREEK_MATH = {"μ": r"$\mu$", "Δ": r"$\Delta$", "σ": r"$\sigma$",
              "τ": r"$\tau$", "Σ": r"$\Sigma$"}
GREEK_ASCII = {"μ": "mu", "Δ": "Delta", "σ": "sigma",
               "τ": "tau", "Σ": "sum"}
CHECK = {"✓": r"\ding{51}", "✗": r"\ding{55}"}
CODE_ONLY = {"∈": "in", "∪": "U", "≥": ">="}

lines = text.split("\n")
in_code = False
out_lines = []
for line in lines:
    if line.strip().startswith("```"):
        in_code = not in_code
        out_lines.append(line)
        continue
    table = GREEK_ASCII if in_code else GREEK_MATH
    for ch, repl in table.items():
        line = line.replace(ch, repl)
    for ch, repl in CHECK.items():
        line = line.replace(ch, repl)
    if in_code:
        for ch, repl in CODE_ONLY.items():
            line = line.replace(ch, repl)
    out_lines.append(line)
text = "\n".join(out_lines)

open(dst_path, "w", encoding="utf-8").write(text)
PYEOF

HEADER_TEX="$(mktemp -t bpd_header_tex_XXXX.tex)"
trap 'rm -f "$BUILD_MD" "$HEADER_TEX"' EXIT
cat > "$HEADER_TEX" << 'EOF'
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{lmodern}
\usepackage{textcomp}
\usepackage{pifont}
\usepackage{microtype}
\usepackage[margin=1in]{geometry}
\usepackage{longtable}
\usepackage{booktabs}
\usepackage{array}
\usepackage{xurl}
\setlength{\emergencystretch}{3em}
\setlength{\tabcolsep}{3pt}
% Allow line breaks after underscores (e.g. "sensor_log") so long
% identifiers wrap instead of overflowing narrow table columns.
\makeatletter
\let\bpd@origunderscore\_
\renewcommand{\_}{\bpd@origunderscore\allowbreak}
\makeatother
EOF

pandoc "$BUILD_MD" \
  -o "$OUT_DIR/main.tex" \
  -s \
  -H "$HEADER_TEX" \
  -V fontsize=11pt \
  -V papersize=letter \
  -V colorlinks=true \
  -V linkcolor=NavyBlue \
  -V urlcolor=NavyBlue \
  -V citecolor=NavyBlue

# Give "NASDAQ" a LaTeX discretionary hyphen (\-) so it can wrap inside
# the narrowest table columns (both data cells and header cells, which
# pandoc emits as multi-line minipages with no single "NASDAQ ... &"
# line to pattern-match) instead of overflowing into the next column.
# \- is invisible except where a line actually needs to break there, so
# applying it document-wide is safe -- full-width prose never needs it.
# This has to happen as a post-pass on the generated LaTeX rather than
# via the markdown source: Markdown's own backslash-escape handling
# would eat the "\" and leave a permanent, always-visible "-" instead.
sed -i 's/NASDAQ/NAS\\-DAQ/g' "$OUT_DIR/main.tex"

echo "Wrote $OUT_DIR/main.tex and $OUT_DIR/universality_gap.pdf"

if command -v pdflatex >/dev/null; then
  ( cd "$OUT_DIR" && pdflatex -interaction=nonstopmode main.tex >/dev/null && pdflatex -interaction=nonstopmode main.tex >/dev/null )
  echo "Local review build: $OUT_DIR/main.pdf (not part of the arXiv upload)"
fi
