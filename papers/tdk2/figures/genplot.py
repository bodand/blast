import sys
import numpy as np

def parse_runlog(filepath):
    data = {}
    proj, method = None, None

    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line == "peak,time,cpu":
                continue
            if line.startswith("----") and line.endswith("----"):
                proj = line.strip("- ")
                data.setdefault(proj, {})
            elif line in ["xargs parallel", "xargs", "blast"]:
                method = line
                data[proj].setdefault(method, {"mem_mb": [], "time_s": []})
            else:
                parts = [p.strip() for p in line.split(",")]
                peak_kb = float(parts[0].replace("KB", "").strip())
                time_s = float(parts[1].replace("s", "").strip())
                data[proj][method]["mem_mb"].append(peak_kb / 1000.0 / 1000.0) # To GB
                data[proj][method]["time_s"].append(time_s)
    return data

def get_stats(data, proj, method, metric_key):
    vals = data[proj][method][metric_key]
    q25, med, q75 = np.percentile(vals, [25, 50, 75])
    return med, med - q25, q75 - med, q25, q75

def generate_broken_time_tikz(data):
    methods = [
        ("blast", "blue", "*"),
        ("xargs", "red", "square*"),
        ("xargs parallel", "teal", "triangle*")
    ]

    common_opts = (
        "    scale only axis,\n"
        "    width=3.2cm,\n"
        "    symbolic x coords={blast, xargs, xargs parallel},\n"
        "    xtick={blast, xargs, xargs parallel},\n"
        "    xmin=blast, xmax=xargs parallel,\n"
        "    ymajorgrids=true,\n"
        "    grid style=dashed,\n"
        "    enlarge x limits=0.25,\n"
        "    error bars/y dir=both, error bars/y explicit,\n"
        "    error bars/error bar style={line width=0.9pt, solid},\n"
        "    error bars/error mark options={rotate=90, mark size=4pt, line width=0.9pt, solid}"
    )

    lines = [r"\begin{tikzpicture}"]

    # 1. SYSTEMD
    lines.append(
        f"\\begin{{axis}}[\n"
        f"    name=ax_sys,\n"
        f"{common_opts},\n"
        f"    height=5.4cm,\n"
        f"    title={{systemd}},\n"
        f"    ylabel={{Idő (s)}},\n"
        f"    ymin=0, ymax=6.2,\n"
        f"    xticklabel style={{rotate=30, anchor=east, font=\\small}},\n"
        f"    scaled y ticks=false\n"
        f"]"
    )
    for method, color, mark in methods:
        med, err_low, err_high, q25, q75 = get_stats(data, "systemd", method, "time_s")
        lines.append(f"\\addplot[only marks, color={color}, mark={mark}, mark size=2.2pt] coordinates {{({method},{med:.2f}) +- ({err_low:.2f},{err_high:.2f})}};")
    lines.append(r"\end{axis}")

    # 2. blAST (Top)
    _, _, _, q25_x, q75_x = get_stats(data, "blAST", "xargs", "time_s")
    span_top = max(q75_x - q25_x, q75_x * 0.04)
    ymin_top = max(0, q25_x - span_top * 2.8)
    ymax_top = q75_x + span_top * 1.8

    lines.append(
        f"\n\\begin{{axis}}[\n"
        f"    name=ax_blast_top,\n"
        f"    at={{(ax_sys.north east)}},\n"
        f"    anchor=north west,\n"
        f"    xshift=1.8cm,\n"
        f"{common_opts},\n"
        f"    height=2.2cm,\n"
        f"    title={{blAST}},\n"
        f"    ymin={ymin_top:.2f}, ymax={ymax_top:.2f},\n"
        f"    axis x line*=top,\n"
        f"    xtick=\\empty,\n"
        f"    scaled y ticks=false\n"
        f"]"
    )
    for method, color, mark in methods:
        if method == "xargs":
            med, err_low, err_high, q25, q75 = get_stats(data, "blAST", method, "time_s")
            lines.append(f"\\addplot[only marks, color={color}, mark={mark}, mark size=2.2pt] coordinates {{({method},{med:.2f}) +- ({err_low:.2f},{err_high:.2f})}};")
    lines.append(r"\end{axis}")

    # 3. blAST (Bottom)
    _, _, _, q25_b, q75_b = get_stats(data, "blAST", "blast", "time_s")
    _, _, _, q25_xp, q75_xp = get_stats(data, "blAST", "xargs parallel", "time_s")
    low_min = min(q25_b, q25_xp)
    low_max = max(q75_b, q75_xp)
    span_bot = max(low_max - low_min, low_max * 0.08)
    ymin_bot = max(0, low_min - span_bot * 0.8)
    ymax_bot = low_max + span_bot * 1.5

    lines.append(
        f"\n\\begin{{axis}}[\n"
        f"    name=ax_blast_bot,\n"
        f"    at={{(ax_blast_top.south west)}},\n"
        f"    anchor=north west,\n"
        f"    yshift=0pt,\n"
        f"{common_opts},\n"
        f"    height=3.2cm,\n"
        f"    ymin={ymin_bot:.2f}, ymax={ymax_bot:.2f},\n"
        f"    axis x line*=bottom,\n"
        f"    xticklabel style={{rotate=30, anchor=east, font=\\small}},\n"
        f"    scaled y ticks=false\n"
        f"]"
    )
    for method, color, mark in methods:
        if method != "xargs":
            med, err_low, err_high, q25, q75 = get_stats(data, "blAST", method, "time_s")
            lines.append(f"\\addplot[only marks, color={color}, mark={mark}, mark size=2.2pt] coordinates {{({method},{med:.2f}) +- ({err_low:.2f},{err_high:.2f})}};")
    lines.append(r"\end{axis}")

    # 4. LLVM (Top)
    _, _, _, q25_x, q75_x = get_stats(data, "LLVM", "xargs", "time_s")
    span_top = max(q75_x - q25_x, q75_x * 0.04)
    ymin_top = (max(0, q25_x - span_top * 2.8)) / 1000.0
    ymax_top = (q75_x + span_top * 1.8) / 1000.0

    lines.append(
        f"\n\\begin{{axis}}[\n"
        f"    name=ax_llvm_top,\n"
        f"    at={{(ax_blast_top.north east)}},\n"
        f"    anchor=north west,\n"
        f"    xshift=1.8cm,\n"
        f"{common_opts},\n"
        f"    height=2.2cm,\n"
        f"    title={{LLVM}},\n"
        f"    ymin={ymin_top:.2f}, ymax={ymax_top:.2f},\n"
        f"    axis x line*=top,\n"
        f"    xtick=\\empty,\n"
        f"    scaled y ticks=false,\n"
        f"    yticklabel={{\\pgfmathprintnumber{{\\tick}}e}}\n" 
        f"]"
    )
    for method, color, mark in methods:
        if method == "xargs":
            med, err_low, err_high, q25, q75 = get_stats(data, "LLVM", method, "time_s")
            lines.append(f"\\addplot[only marks, color={color}, mark={mark}, mark size=2.2pt] coordinates {{({method},{med/1000.0:.2f}) +- ({err_low/1000.0:.2f},{err_high/1000.0:.2f})}};")
    lines.append(r"\end{axis}")

    # 5. LLVM (Bottom)
    _, _, _, q25_b, q75_b = get_stats(data, "LLVM", "blast", "time_s")
    _, _, _, q25_xp, q75_xp = get_stats(data, "LLVM", "xargs parallel", "time_s")
    low_min = min(q25_b, q25_xp)
    low_max = max(q75_b, q75_xp)
    span_bot = max(low_max - low_min, low_max * 0.08)
    ymin_bot = (max(0, low_min - span_bot * 0.8)) / 1000.0
    ymax_bot = (low_max + span_bot * 1.5) / 1000.0

    lines.append(
        f"\n\\begin{{axis}}[\n"
        f"    name=ax_llvm_bot,\n"
        f"    at={{(ax_llvm_top.south west)}},\n"
        f"    anchor=north west,\n"
        f"    yshift=0pt,\n"
        f"{common_opts},\n"
        f"    height=3.2cm,\n"
        f"    ymin={ymin_bot:.2f}, ymax={ymax_bot:.2f},\n"
        f"    axis x line*=bottom,\n"
        f"    xticklabel style={{rotate=30, anchor=east, font=\\small}},\n"
        f"    scaled y ticks=false,\n"
        f"    yticklabel={{\\pgfmathprintnumber{{\\tick}}e}}\n"
        f"]"
    )
    for method, color, mark in methods:
        if method != "xargs":
            med, err_low, err_high, q25, q75 = get_stats(data, "LLVM", method, "time_s")
            lines.append(f"\\addplot[only marks, color={color}, mark={mark}, mark size=2.2pt] coordinates {{({method},{med/1000.0:.2f}) +- ({err_low/1000.0:.2f},{err_high/1000.0:.2f})}};")
    lines.append(r"\end{axis}")

    # =========================================================================
    # HULLÁMOS SZEPARÁTOROK RAJZOLÁSA A GRAFIKONOK ÖSSZETÉTELÉRE
    # =========================================================================
    lines.append(r"")
    lines.append(r"    % blAST grafikon elszakított papír hatás")
    lines.append(r"    \draw[decorate, decoration={snake, amplitude=1.2pt, segment length=6pt}, white, line width=5pt] (ax_blast_top.south west) -- (ax_blast_top.south east);")
    lines.append(r"    \draw[decorate, decoration={snake, amplitude=1.2pt, segment length=6pt}, black, line width=0.4pt] ([yshift=1.5pt]ax_blast_top.south west) -- ([yshift=1.5pt]ax_blast_top.south east);")
    lines.append(r"    \draw[decorate, decoration={snake, amplitude=1.2pt, segment length=6pt}, black, line width=0.4pt] ([yshift=-1.5pt]ax_blast_top.south west) -- ([yshift=-1.5pt]ax_blast_top.south east);")

    lines.append(r"")
    lines.append(r"    % LLVM grafikon elszakított papír hatás")
    lines.append(r"    \draw[decorate, decoration={snake, amplitude=1.2pt, segment length=6pt}, white, line width=5pt] (ax_llvm_top.south west) -- (ax_llvm_top.south east);")
    lines.append(r"    \draw[decorate, decoration={snake, amplitude=1.2pt, segment length=6pt}, black, line width=0.4pt] ([yshift=1.5pt]ax_llvm_top.south west) -- ([yshift=1.5pt]ax_llvm_top.south east);")
    lines.append(r"    \draw[decorate, decoration={snake, amplitude=1.2pt, segment length=6pt}, black, line width=0.4pt] ([yshift=-1.5pt]ax_llvm_top.south west) -- ([yshift=-1.5pt]ax_llvm_top.south east);")
    
    lines.append(r"\end{tikzpicture}")
    return "\n".join(lines) + "\n"

def generate_memory_tikz(data):
    projects = ["systemd", "blAST", "LLVM"]
    methods = [
        ("blast", "blue", "*"),
        ("xargs", "red", "square*"),
        ("xargs parallel", "teal", "triangle*")
    ]

    lines = [
        r"\begin{tikzpicture}",
        r"\begin{groupplot}[",
        r"    group style={",
        r"        group size=3 by 1,",
        r"        horizontal sep=1.8cm,",
        r"    },",
        r"    scale only axis,",
        r"    width=3.2cm, height=5.4cm, % Szinkronizálva az idő grafikon magasságával",
        r"    symbolic x coords={blast, xargs, xargs parallel},",
        r"    xtick={blast, xargs, xargs parallel},",
        r"    xmin=blast, xmax=xargs parallel,",
        r"    xticklabel style={rotate=30, anchor=east, font=\small},",
        r"    ymin=0,",
        r"    ymajorgrids=true,",
        r"    grid style=dashed,",
        r"    enlarge x limits=0.25,",
        r"    error bars/y dir=both, error bars/y explicit,",
        r"    error bars/error bar style={line width=0.9pt, solid},",
        r"    error bars/error mark options={rotate=90, mark size=4pt, line width=0.9pt, solid}",
        r"]"
    ]

    for i, p in enumerate(projects):
        ylabel_opt = ", ylabel={Memória (GB)}" if i == 0 else ""
        lines.append(f"\n\\nextgroupplot[title={{{p}}}{ylabel_opt}]")
        for method, color, mark in methods:
            med, err_low, err_high, q25, q75 = get_stats(data, p, method, "mem_mb")
            lines.append(f"\\addplot[only marks, color={color}, mark={mark}, mark size=2.2pt] coordinates {{({method},{med:.2f}) +- ({err_low:.2f},{err_high:.2f})}};")

    lines.append(r"\end{groupplot}")
    lines.append(r"\end{tikzpicture}")
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 generate_plots.py <runlog> [time_out.tex] [mem_out.tex]")
        sys.exit(1)

    runlog_path = sys.argv[1]
    time_out = sys.argv[2] if len(sys.argv) > 2 else "figures/time.tex"
    mem_out = sys.argv[3] if len(sys.argv) > 3 else "figures/mem.tex"

    parsed_data = parse_runlog(runlog_path)

    with open(time_out, "w") as f:
        f.write(generate_broken_time_tikz(parsed_data))

    with open(mem_out, "w") as f:
        f.write(generate_memory_tikz(parsed_data))
