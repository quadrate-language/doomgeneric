# doomgeneric

The DOOM engine for Quadrate, via [doomgeneric](https://github.com/ozkl/doomgeneric).

## Installation

```bash
quadpm get https://github.com/quadrate-language/doomgeneric
```

## Requirements

`git`, and a quadpm new enough to run prebuild scripts and pass `native.cflags`
(0.5.0-49 or later). The engine is fetched while the module installs; the
package itself links only against `libm`.

## Usage

There is nothing to implement. doomgeneric's six hooks are already filled in, so
there are no callbacks to register — you start the engine, then drive it a frame
at a time.

```qd
use doomgeneric

fn main() {
	"-iwad" doomgeneric::AddArg
	"doom1.wad" doomgeneric::AddArg
	doomgeneric::Create

	loop {
		1 doomgeneric::KeyFire doomgeneric::PushKey
		0 doomgeneric::KeyFire doomgeneric::PushKey

		doomgeneric::Tick

		doomgeneric::TakeFrame if {
			doomgeneric::ScreenBuffer -> pixels
			// Draw it. See the 1-bit calls below for monochrome panels.
		}
	}
}
```

That is the whole contract:

- **`AddArg` then `Create`** to start. `Create` loads the IWAD and runs DOOM's
  start-up, and returns once the first frame is drawn.
- **`PushKey`** for every key press and release, before the tick that should see
  it. Takes a DOOM key code from `keys.qd`, not a keyboard one.
- **`Tick`** once a frame.
- **`ScreenBuffer`** after each tick: `Width` × `Height` pixels of XRGB8888,
  `Pitch` bytes to a row. Put it on a screen however you like.

`TakeFrame` is only there to tell you whether that tick actually produced a new
frame, so you can skip a redundant upload; blitting every tick works too. DOOM
paces itself to 35 tics a second from inside `Tick`, so you need no frame
limiter of your own.

[doom-quadrate](https://github.com/quadrate-language/doom-quadrate) is a
complete example: an SDL window, key translation and presentation, all in
Quadrate.

## API

### Starting and running

- `AddArg(arg:str -- )` - Append one DOOM command line argument. Must come before `Create`
- `Create( -- )` - Load the IWAD, run DOOM's start-up, render the first frame
- `Tick( -- )` - Advance the game by one frame

### The frame

- `ScreenBuffer( -- pixels:ptr)` - The framebuffer `Tick` renders into, XRGB8888
- `Width( -- pixels:i64)` - Framebuffer width
- `Height( -- pixels:i64)` - Framebuffer height
- `Pitch( -- bytes:i64)` - Byte length of one row
- `TakeFrame( -- ready:i64)` - Whether the last `Tick` produced a new frame. Reading clears it
- `TakeTitle( -- title:str changed:i64)` - The window title DOOM last asked for. `changed` is 1 only the first time a given title is read

### Input

- `PushKey(pressed:i64 key:i64 -- )` - Queue one key transition for the next tick. Takes a DOOM key code, not a keyboard one

`keys.qd` carries those codes: `KeyUpArrow`, `KeyFire`, `KeyUse`, `KeyEscape`,
`KeyF1`–`KeyF12` and the rest. Most are plain ASCII, so an ordinary lowercase
letter can be pushed as-is.

### One bit per pixel

For monochrome panels. The frame is dithered against a fixed 4×4 Bayer matrix,
so the threshold depends on where a pixel sits on screen and not on its
neighbours or on the previous frame — which keeps the pattern still while the
player moves.

- `MonoGray8(dest:ptr -- )` - One byte per pixel, each `0x00` or `0xff`. Needs `MonoGray8Size` bytes
- `MonoPacked(dest:ptr invert:i64 -- )` - Eight pixels to a byte, MSB leftmost. A set bit is paper; pass `invert` non-zero for panels that drive a set bit as ink. Needs `MonoPackedSize` bytes
- `MonoPreview(dest:ptr -- )` - The same reduction as XRGB8888 in a reflective panel's ink and paper, to look at on a colour screen. Needs `Pitch * Height` bytes
- `MonoStride( -- bytes:i64)` - Byte length of one row of `MonoPacked` output
- `MonoGray8Size( -- bytes:i64)` - Buffer size for `MonoGray8`
- `MonoPackedSize( -- bytes:i64)` - Buffer size for `MonoPacked`
- `SetBrightness(percent:i64 -- )` - Gain applied before dithering. 100 leaves the image alone; the default of 150 lifts the dark scenes that dominate DOOM

`MonoGray8` suits a grayscale framebuffer: the bytes already sit at the
extremes, so a panel's own threshold costs nothing on top. `MonoPacked` is what
SSD1306, Sharp and e-ink controllers expect.

## Writing another host

A host that is not Quadrate — a calculator, a framebuffer, a microcontroller —
implements the six `DG_*` hooks itself and links `src/doomgeneric/` directly
rather than through the Quadrate surface. Two pieces here are written for that
case and belong to no host in particular:

- `src/mono.c` — the 1-bit reduction, over a caller's buffer
- `src/keyqueue.c` — a key transition buffer, for hosts that learn about keys
  outside the tick and must hand them to `DG_GetKey` during it. Push with
  `keyqueue_push`, and let `DG_GetKey` forward to `keyqueue_pop`

doom-quadrate's qdos port is one such host, and uses both.

## Framebuffer size

DOOM renders 320×200 and doomgeneric scales that by whole numbers only. The
result is set in `qd.json`, which quadpm passes to the compiler:

```json
"native": {
  "cflags": ["-DDOOMGENERIC_RESX=400", "-DDOOMGENERIC_RESY=240"]
}
```

At 400×240 the game lands centred at one engine pixel per framebuffer pixel,
bordered and unresampled — which matters once each pixel is a single bit.
640×400 gives the original 2× frame with no border, and is what upstream's own
`#ifndef` defaults to if nothing sets it.

A host compiling the sources directly passes its own `-D` instead and never
reads `qd.json`.

## Sound

There is none. doomgeneric's sound backend is SDL2 plus SDL_mixer, so the engine
is compiled without `FEATURE_SOUND` and DOOM runs silent.

## Why there is nothing to implement

A doomgeneric host normally writes the six `DG_*` hooks itself, and DOOM calls
them during a tick to blit a frame or read a key. That cannot work here: the
hooks are C in a static library linked into a Quadrate program, and nothing in
that library can call back into it — Quadrate `fn`s are emitted with internal
linkage under names derived from their source file, so there is no symbol to
link against.

So this package fills the hooks in itself and leaves them hollow. They buffer
what DOOM asks for, and the work happens in your loop instead, which is why the
surface is all calls and no callbacks.

## The vendored engine

`src/doomgeneric/` is not checked in. quadpm runs `make engine` as this module's
prebuild script, which fetches upstream at the commit pinned in the Makefile and
copies it in, less the files in `ENGINE_EXCLUDE`: the per-platform backends,
each of which defines `main()` and the six `DG_*` hooks this package provides
itself, and the sound backends wanting SDL2 or allegro.

The import refuses to finish if anything it copied still defines `main()`, so a
backend added upstream is caught when the pin moves rather than as a duplicate
symbol in someone else's link.

It runs once. A rebuild finds the engine already there and never touches the
network; `make distclean` drops it, and `make import DOOM_COMMIT=<sha>` forces a
re-import at another revision.

The engine is taken unmodified: every file under `src/doomgeneric/` is
byte-identical to upstream, and the framebuffer size is set with `cflags` rather
than by patching the header that declares it.

## Tests

```bash
make test
```

`tests/mono_test.c` and `tests/keyqueue_test.c` cover the reduction and the key
buffer directly, since neither needs DOOM. The Quadrate surface is covered from
doom-quadrate, where there is a WAD to start the engine with.

## License

GPL-3.0-or-later.

DOOM is licensed "version 2 of the License, or (at your option) any later
version", so a derivative may be distributed under v3. One file upstream,
`sha1.c`, is v3-or-later already, which makes v3 the earliest version the whole
thing can be distributed under in any case.

## Contributing

Contributions welcome! Please open an issue or pull request on [GitHub](https://github.com/quadrate-language/doomgeneric).
