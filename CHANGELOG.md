# Changelog

All notable changes to this AmigaOS SAS/C port of Voyager are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Dates are commit dates from this repository.

## [Unreleased]

Layout polish remaining after the 2026-09-04 runtime work.

- Diagnostic `Printf` / `Flush(Output())` is still on many paths, including htmlview Draw / ShowNStream and htmlwin SetURL / GotInfo. Wrap or drop it on hot paths once debugging is done so redraw cannot stall on DOS.
- `NStream_GotInfo` deferral while `in_seturl` was added against a deadlock; if that lockup stays gone without it, the original immediate GotInfo flow can come back.

## [0.3.0] - 2026-09-04

First build that loads real sites (amiga-news.de, Aminet, amigazen.com), shows images, runs the toolbar and menustrip, and exits cleanly.

### Added

- Image decoder linked into the executable instead of `vimgdecode.library`.
- Decoder runs as a `CreateNewProcTags` process so `dos.library` (cache files, `Delay`, error requesters) has a real Process context.
- Explicit `mathffp.library` / `mathieeedoubbas.library` open for the statically linked decoder (SAS/C auto-open does not cover it).
- CyberGraphX pixel-write paths (`USE_CGX 1`) and `imgdec_setdestscreen()` from HTML view Setup so decoded frames actually reach the screen bitmap.
- Register-decorated `imgdec_*` prototypes (`ASM` / `__reg`) matching `imgdecode.c` so callers pass arguments in the registers the callee reads.
- Catalog helper `voyager_catalog_str()` with CatComp built-in fallback so `GS` / `GSI` work without VAT.
- SpeedBar toolbar (`USE_SPEEDBAR 1`) using `<mui/SpeedBar_mcc.h>` and `<mui/SpeedButton_mcc.h>`.
- Diagnostic `Printf` tracing for init, layout, parse, tables, images, network, decoder, and commands. Decoder output can go to `PROGDIR:imgdec.log`.
- HTTP request/response logging (`net_log_http_request` / `net_log_http_response_line`).
- 256-byte User-Agent buffer and a Firefox-compatible default UA when spoof is off, so CDNs that reject `AmigaVoyager` (e.g. amigazen.com) still serve pages. The Spoof menu still identifies as Voyager.
- `mySystemTags()` builds a real `TagItem` array from varargs.

### Fixed

- First image crashed the machine: libcall pragmas still did `jsr` through dummy `VIDBase`. Pragmas removed; calls are direct.
- Pooled `imgclient` / `imgnode` allocations zeroed so stale `isspecial` cannot take a bad decoder path.
- Toolbar buttons did nothing when `USE_VAT` was 0: ARexx branch was an empty statement. Commands now fall through to `send_internal_command`.
- `send_internal_command` no longer `memset`s a NULL ReadArgs array (Search / zero-argument commands).
- Table layout hang and heap corruption: `colmin == 0` retry loop, `MAX_TABLE_COLS` guard, NULL cell `li`, pointers cleared after `DeletePool`, `squashtable` `amaxrow`/`amaxcol` bookkeeping, `canlose` memset and cleanup bounds.
- HTML parser over-read past the received chunk: mid-tag tokens rolled back when the document is incomplete; `layout_do` / `layout_do_text` never report more bytes than they were given.
- PRE / wide-token wrap in `lo_group` always makes forward progress; layout no longer opens a modal `MUI_Request`.
- Restored original `MUIM_Group_InitChange` / `ExitChange`, htmlview `AskMinMax`, scroll `AutoBars`, and `layoutwinfunc` child layout so HTML content displays.
- Empty application menustrip: `GSI("")` had blanked every `NewMenu` title.
- `copylines` compiles with `USE_CGX 1` (C89, no split-brace `else`).
- dos.library logs used `%ld` / `%lx` for 32-bit values (`%d` is 16-bit and corrupted traces).
- DNS child creates its MsgPort before other work; `DNSTASKS` is 1; cache lookup skipped until the pool exists.
- HTTP `Host` / User-Agent tolerate a NULL host; about-text no longer dereferences dummy `VIDBase`.
- HTML view cache bitmap uses `_width` × `_height` (not `left+width` / `top+height` as a size), blits from source `(0,0)` to the saved `left`/`top`, and rejects blit coordinates outside `[-32768,32767]`. Reading `_left`/`_top` after `MUI_Redraw()` was producing Guru 80000002 and orange stripes.
- `get_cachebitmap` refuses non-positive or >8192 dimensions so `AllocBitMap` is not called with garbage sizes.
- `ShowNStream` with no document is an explicit path (clear imagemode/textmode/usedamageclip, `ExitChange`) instead of falling through.
- Ignore stale `NStream_GotInfo` when `doc_loading != msg->ns`.
- `NStream_GotInfo` is deferred while `in_seturl` is set (deadlock workaround; see Unreleased).

### Removed

- Opening/closing `vimgdecode.library` and CPU-specific decoder variants (`68020` / `68040fpu` / …). `VIDBase` is only a non-NULL availability sentinel.
- `vimgdecode` libcall pragmas and inlines.
- Tear-off panels (`USE_TEAROFF 0`) until TearOffPanel.mcc is Enforcer-clean.
- MorphOS-only SpeedBar include path.
- Casting `va_list` to `TagItem *` for `SystemTagList`.
- Decoder `CreateTask` path (no Process structure, unsafe for DOS).

## [0.2.0] - 2026-01-04

SAS/C 68k build revived. The binary linked; it did not yet run as a browser.

### Added

- SAS/C `smakefile` and `SCOPTIONS` for the main program, imgdecode, VFlash plugin, and libmath64.
- Vapor toolkit behaviour folded into the binary: async helpers, extra VAT replacements, `snprintf`, libmath64.
- Regenerated `css_parser.c` and `js_parser.c` from Bison (generated files were missing from the GPL drop).
- `funcdef.i` for textfit; catalog rebuild via CatComp (`voyager.cd` / `voyager_cat.h`).
- Init-path debug `Printf` after the first successful link.

### Fixed

- VAbout module builds without a `rev` command.
- imgdecode SAS/C compile (CyberGraphX / VAT bases still unresolved at this point).
- imgdecode Makefile vs the SAS/C smakefile so the two builds do not fight.
- Header locations for MUI classes; GCC-only debug macros that SAS/C rejected.
- `vapor.h` dispatcher macros for SAS/C; MUI headers with GCC `pack` pragmas.
- All `lo_*.c` modules no longer depend on broken VAT macros.
- Remaining link errors so Voyager produces an AmigaOS executable again.

### Removed

- Runtime dependency on `vapor.library` and keyfile checks.
- MiamiSSL.
- Executive (`executive.library`) from imgdecode (unsupported).

## [0.1.0] - 2025-11-05

### Added

- Initial import of the original GPL Voyager sources, plus VAT, VSSL, libmath64, and VFlash.
- Header files that were missing from that GPL drop.

[Unreleased]: https://github.com/amigazen/Voyager/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/amigazen/Voyager/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/amigazen/Voyager/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/amigazen/Voyager/releases/tag/v0.1.0
