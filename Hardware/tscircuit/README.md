# HS-736USB Nano carrier (tscircuit)

Arduino Nano plugs into `JP_L` / `JP_R`. This board is the I/O carrier
(CAT, PTT MOSFET, RCA sense, dialect jumper, LCD, band, PWM). The MCU
is not redrawn.

CAT DIN pin 6 (+13.8 V) is not connected. Power is Nano USB 5 V.

## CLI

IndianaDell installs tscircuit under `~/.local` (`bin/tsci` runs `node`
on `cli.mjs`). The npm shebang is bun, and node cannot load `.tsx`.

From this directory, use the project CLI with bun:

```bash
BUN=/tmp/bunpkg/node_modules/bun/bin/bun.exe
# npm install --prefix /tmp/bunpkg bun  (Linux binary is named bun.exe)

"$BUN" node_modules/tscircuit/cli.mjs check netlist
"$BUN" node_modules/tscircuit/cli.mjs check schematic-placement
"$BUN" node_modules/tscircuit/cli.mjs check placement
"$BUN" node_modules/tscircuit/cli.mjs build --pcb-svgs --schematic-svgs --kicad-project
"$BUN" node_modules/tscircuit/cli.mjs check shorts
inkscape dist/index/pcb.svg --export-type=png --export-filename=dist/index/pcb.png --export-dpi=1200
inkscape dist/index/schematic.svg --export-type=png --export-filename=dist/index/schematic.png --export-dpi=1200
```

Do not `tsci push` or order boards unless asked.

`<schematicsheet>` is omitted on purpose: with it, `kicad_sch` export
is a hierarchical stub (`main.kicad_sch` never written). Sections
still group the schematic.

Known warnings: builtin symbols missing `{REFDES}` text; SOD-123 has
no 3D model on the tscircuit CDN.

Outputs: `dist/index/pcb.png`, `dist/index/schematic.png`,
`dist/index/kicad/`.
