# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
where version numbers allow.

Product **3.5.2** is the SAS/C 68k rebuild. **3.4.9** and **3.3.126** summarise the
limited post-3.2 CVS line from `Source/V.ReadMe`. **3.2.0** through **3.0.0** is
the VaporWare history from the Voyager 3.2 product readme (2000-04-16).
Changes prior to 3.0 were already dropped in that file.

## [Unreleased]

### Changed

- Layout polish after first-paint and table work.
- Default HTTP User-Agent (spoof off) is Firefox 4.0.1 on Windows 7. Extra tokens in the Mozilla comment (AmigaVoyager) were 403’d by StackCDN.

### Fixed

- `NStream_GotInfo` deferral while `in_seturl` is set: deadlock workaround; drop it if the lockup stays gone.

## [3.5.2] - 2026-09-05

### Added

- HTTPS via AmiTLS: `Source/VSSL/amitls` builds `voyager_ssl.vlib` on `amitls.library` (TLS 1.2, SNI). `VSSL_ConnectHost` is VSSL ABI v9. OpenSSL 0.9 under `Source/VSSL/openssl` is not linked. Needs `LIBS:amitls.library` and `Certificates/cacert.pem`.
- Workbench documentation in `Internet/Voyager/Docs/` (including the VaporWare HTML manual under `Docs/voyager3docs/`) and an updated `V.ReadMe` for 3.5.2 (3.4 / 3.3 / 3.2 version history lives in this file).
- Factory fastlink **Docs** opens `file:///PROGDIR:Docs/voyager3docs/main.html` (replaces the historical www.vapor.com favourite).

### Changed

- Public Workbench release of the 3.5 restoration (classic 68k). Behaviour is Voyager 3 as shipped, plus compile/run fixes and AmiTLS for SSL/TLS support.
- Product version `3.5.2` (`rev.h`), date 5.9.2026.
- Tear-off panels remain off (`USE_TEAROFF 0`); TearOffPanel.mcc is not Enforcer-clean.

## [3.5.1] - 2026-09-04

### Added

- SAS/C 68k `smakefile` / `SCOPTIONS` for the main program, imgdecode, MimePrefs (CatComp), VFlash plugin, and libmath64.
- Image decoder linked into the executable (not `vimgdecode.library`). Decoder process is `CreateNewProcTags` so `dos.library` has a real Process. `mathffp.library` / `mathieeedoubbas.library` opened explicitly.
- CyberGraphX pixel-write paths (`USE_CGX 1`) and `imgdec_setdestscreen()` from HTML view Setup.
- Register-decorated `imgdec_*` prototypes (`ASM` / `__reg`).
- Catalog helper `voyager_catalog_str()` with CatComp builtin fallback so `GS` / `GSI` work without VAT.
- SpeedBar toolbar (`USE_SPEEDBAR 1`).
- Vapor toolkit behaviour in the binary: async helpers, `snprintf`, libmath64. Regenerated `css_parser.c` / `js_parser.c` from Bison.
- `funcdef.i` for textfit; `voyager.cd` / `voyager_cat.h` via CatComp.
- `mySystemTags()` builds a real `TagItem` array from varargs.
- Console traces behind `VLOG` as `VoyLog` / `VoyFlush` (not dos.library `VPrintf`). HTTP file log behind `VLOG_NET` (`V:voyager_net.log`).
- Default HTTP User-Agent (spoof off): Firefox 4 grammar with AmigaVoyager and OS/CPU inside the Mozilla comment.
- Factory homepage `about:` (prefs default, Home, ARexx fallback).
- Workbench release tree under `Internet/` (V, Libs, MUI, Plugins, Docs, Certificates, Buttons, installer) with icon palettes for current icon.library.

### Changed

- First release that identifies as Voyager 3.5.1. It loads real sites, shows images, runs the toolbar and menustrip, exits cleanly, and ships a Workbench drawer with updated icons.
- Product version `3.5.1` (`rev.h` / `.revinfo`), `$VER` / about / splash date 4.9.2026.
- `VLOG` and `VLOG_NET` default off in a normal build.

### Removed

- Opening `vimgdecode.library` / CPU-specific decoder variants at runtime. `VIDBase` is only a non-NULL sentinel.
- `vimgdecode` libcall pragmas and inlines; decoder `CreateTask` path.
- Runtime `vapor.library` and keyfile checks; MiamiSSL; Executive in imgdecode.
- MorphOS-only SpeedBar include; `va_list` cast to `TagItem *`.

### Fixed

- First image crashed: libcall pragmas `jsr` through dummy `VIDBase`. Calls are direct; pooled `imgclient` / `imgnode` zeroed.
- Toolbar did nothing with `USE_VAT` 0; Search no longer `memset`s a NULL ReadArgs array.
- Table hang/corruption: `colmin == 0` retry, `MAX_TABLE_COLS`, NULL cell `li`, pointers after `DeletePool`, `squashtable` bounds.
- Parser over-read of incomplete HTML; wrap in `lo_group` always advances; no modal `MUI_Request` during layout.
- Restored `MUIM_Group_InitChange` / `ExitChange`, htmlview `AskMinMax`, scroll `AutoBars`, `layoutwinfunc` child layout.
- Empty menustrip: `GSI("")` had blanked every `NewMenu` title.
- `copylines` compiles with `USE_CGX 1` (C89).
- dos.library logs use `%ld` / `%lx` for 32-bit values.
- DNS child publishes its MsgPort first; `DNSTASKS` 1; skip cache until the pool exists.
- HTTP `Host` NULL-safe; about-text does not dereference dummy `VIDBase`.
- HTTP 400 from servers: `Accept-Language` dropped if it contains control bytes; `If-Modified-Since` built from `gmtime` and omitted if invalid (uninitialised `tdate` produced garbage dates).
- First HTML paint grey band at the top: offscreen cache sized to `_left`+width / `_top`+height and blit from `(left, top)`, matching `MUI_Redraw` window coordinates. Idle event-loop `NewInput` traces removed (they flooded DOS).
- Source strings restored to ISO-8859-1 (no UTF-8 / U+FFFD).
- `get_cachebitmap` rejects non-positive or >8192 sizes.
- `ShowNStream` with no document is explicit; stale `NStream_GotInfo` ignored when `doc_loading != msg->ns`.
- SAS/C compile: GST, vapor.h macros, MUI `pack` pragmas, `lo_*.c` without broken VAT macros, first successful link (2026-01-04).

## [3.4.9] - 2005-01-01

### Changed

- Internal / MorphOS-era CVS after 3.3.126, from `Source/V.ReadMe` (3.4 through 3.4.9). Not a boxed retail product.
- Faster scrolling; border scroller mode restored
- Word-wrap and frameset-loop workarounds; `<PRE>` CR handling; double-buffered resizes
- Preliminary `DD` / `DT` / `DL`; custom pointers; Google search from the URL gadget
- JS: `navigator.cookieEnabled`, `onLine`, `cpuClass`, `vendor`
- Referer no longer includes username/password; MorphOS without keyfile

### Fixed

- Mouse-wheel scrolling was disabled then fixed; MUI lists restored

## [3.3.126] - 2002-12-27

### Added

- Branched a new layout engine and reworked the window framework

### Changed

- Limited VaporWare 3.3 line after the 3.2 boxed release (alpha from August 2000). Not a full retail release: CVS alphas, then public betas (3.3.65, 3.3.75, 3.3.79, 3.3.95, 3.3.104), then 3.3.126 dated 27.12.2002 as a 68020+ / MorphOS archive that must not be installed over 3.2. Per-build history is in `Source/V.ReadMe` (3.3.0–3.3.127).
- Runs without TearOff if the classes are missing; navigation panel can attach vertically
- CManager alias in the URL gadget; `ftp.` host names get an `ftp://` prefix
- Incremental visual refresh; `BASE TARGET`; `<NOEMBED>`; plugin alias IDs for `navigator.plugins`
- JS: `do`/`while`, `window.focus`/`blur`, Date UTC/parse constructors, form `target`/`action`, `this` in event handlers, `navigator.mimeTypes`
- 3.3.126: 68k optimizations reduced; 3.3.127 MorphOS date, cookies, SSL cert window, FTP names with spaces, fullscreen saved, layout text speed, larger URLs
- HTTP 4xx/5xx replies forced to `text/html`
- Known limitations remaining: list / `BLOCKQUOTE` indent; no printing; weak frame navigation; JS cross-frame call arguments; VFlash resize; download window closes with the last browser window; frames always show scrollbars

### Removed

- AS225 stack support

## [3.2.0] - 2000-04-16

### Added

- Support for HTTP file upload
- Support for <FONT FACE>, with customizable mappings. Extremely useful in conjunction with ttf.library
- Support for individual table and table cell backgrounds
- Direct support for FBlit, totally eliminating the need for intermediate chip mem buffering of images and thus heavily improving rendering speed on AGA systems
- Javascript 1.3, implemented as a just-in-time bytecode compiler
- General: added updated SpeedBarButton.mcc class (Olli)

### Changed

- VaporWare full release (16.4.2000). Copied from the 3.2 product readme. Pre-3.0 entries were already dropped in that file.
- Remaining 3.2 limitations: SCRIPT SRC works in the GPL tree; JS entities, ARexx exposure of the JS interpreter/DOM, asynchronous layout, and CSS were never finished.
- Heavily improved and faster SSL module
- Heavily improved PNG support
- Improved plugin API again. A V plugin version of the Apdf PDF file viewer is now available
- Heavily improved VFlash plugin
- Numerous bugfixes and minor enhancements in all parts of the code, from HTML over networking to Javascript
- Heavily improved GUI, custom placement of control panels using Tearoff classes; fastlinks by dropping an URL; reworked menu; wheel mice (NewMouse)
- Fully configurable toolbar with standard and custom ARexx/JS commands using SpeedBar
- Much faster layout, with up to 500% speed gain on nested tables compared to V3pre1 and older
- Heavily improved plugin API allowing embedded objects for any MIME type
- Shockwave Flash player plugin (embedded movies and URL handling)
- Vpdf, an embedded PDF document viewer
- SSLv3/TLSv1 via OpenSSL, 128-bit encryption
- Modularized image decoders; updated libjpeg and libpng; smart incremental JPEG decoding
- Bookmark system using CManager.mcc, shared with other Vapor applications
- Improved cache honouring HTTP/1.1 Expires
- Numerous other changes (per-version list below)
- JS: some properties and methods incomplete; JS entities `<IMG SRC=&{js-code};>` still absent
- Asynchronous / suspendable layout
- ARexx port, including JS interpreter access
- HTML: plenty of remaining layout work
- Image decoders: a PPC variant (the 3.2 modular decoder plan)

### Fixed

- Many bugfixes, including the longstanding random crashes or misbehaviors induced by the History list
- Fixed printing with TurboPrint

## [3.1.6]

### Fixed

- Layouter: fixed <H1> sizing and boldness (Olli)
- GUI: fixed prefs window minimum size calculation (Olli)

## [3.1.5]

### Added

- JS: implemented History class and window.history reference (Olli)
- JS: implemented document.replace() (Olli)
- JS: implemented document.reload() (Olli)

### Fixed

- Net: will now convert all the spaces in URLs to %20. Should fix some HTTP 400 errors (Zapek)

## [3.1.4]

### Added

- JS: implemented document.forms[] (Olli)

### Fixed

- JS: fixed a problem in Array.ToString() causing the last char of the last element to be dropped (Olli)
- JS: fixed a problem in Array.ToString() with objects not returning any sensible name (Olli)
- GUI: fixed random historylist crashes (Olli)

## [3.1.3]

### Added

- Layouter: added support for <form enctype=multipart/form-data> (Olli)
- Layouter: added suppoort for <input type=file> (Olli)

## [3.1.2]

### Added

- GUI: added a switch to disable <FONT FACE=...> matching (Zapek)
- General: added workaround for TearOff not closing muigfx.library properly (Zapek)

### Changed

- General: optimized event handlers handling for MUI4 (Zapek)

### Fixed

- GUI: removed possible crash when displaying the font prefs and there was not enough memory (Zapek)
- Prefs: now checks whether an installed Pophotkey.MCC is acceptable to use and skips over it when it's too old. Fixes prefs win crashes (Olli)

## [3.1.1]

### Fixed

- Net: fixed enforcer hits when specifying an illegal URL with proxy enabled (Olli)
- Layouter: fixed font face fallback in case of <font face=f1, f2> with f2 being present, but f1 not -- V would use the template in that case (Olli)
- General: Fixed public screens not closing on quit (Olli)

## [3.1.0]

### Added

- Added new SpeedBar, Textinput and CManager classes to distribution archive
- Added updated Installer script by RobR

### Changed

- VaporWare release

## [3.0.71]

### Fixed

- JS: fixed a lifetime issue with window.open() generated windows (Olli)
- JS: fixed default decoration settings for window.open() (Olli)
- JS: fixed window.open() size specifications (Olli)

## [3.0.70]

### Added

- Prefs: added default TTF settings (Olli)

### Fixed

- Net: fixed settofile() not working at all anymore (grumpf) (Olli)
- JS: fixed a sync problem with <SCRIPT> appearing when a table row/cell is expected (Olli)

## [3.0.69]

### Added

- Prefs: added new spoofing system, with 3 fully user configurable spoof levels (Olli)
- GUI: added progress bar to the Splash Window (Zapek)
- General: added workaround for Turboprint under CGFX (Zapek)

### Changed

- Layouter: font face names are now properly case insensitive (Olli)
- Layouter: </font> now properly resets font face to previous value (Olli)
- Layouter: <tt> didn't set up font pointers properly (Olli)

### Fixed

- Layouter: fixed broken font size after </Hx> (Olli)
- JS: fixed possible enforcer hit when running out of bytecode memory (Zapek)
- Net: fixed a screwup in handling the case of a file being both downloaded into memory and to a file -- the "Done" notification for the file stream was sent even before the file was written to disk. This broke Vpdf. Also fixed delay in sending this notification, and now uses plain DOS I/O for writing the data (as Async I/O does not even remotely make sense, except wasting resources and slowing things down) (Olli)
- Plugin: fixed NStream_GotData and _Done methods sometimes not arriving at the embedded objects properly (Olli)
- General: fixed printing on hi/true-color screens with Turboprint (Zapek)
- General: fixed navigation keys not working properly with MUI 3.8 (Zapek)

## [3.0.68]

### Changed

- GUI: completely reworked prefs layout again, following ibeta suggestions (Olli)
- General: printing with printer.device from a CLUT screen works (Zapek)

## [3.0.67]

### Added

- GUI: when resizing the window during a table layout, V will abort the current layout and start the new one (Zapek)
- Layouter: added support for <FONT FACE> (Olli)
- GUI: added a Splash Window (tm) (Zapek)

### Changed

- GUI: the transfer animation will always rotate when V is doing layout processing (Zapek)
- GUI: clears the URL status display when the mouse pointer leaves the HTML area from a border (Zapek)
- Net: could send methods to hell when updating the Cookie Browser realtime (Zapek)
- GUI: to select text in the HTMLview area, the mouse has to move at least 2 pixels horizontally or vertically (Zapek)
- GUI: it's now possible to abort a selection when pressing the right mouse button (Zapek)
- General: some more cleanup in the printing. Will use Turboprint for hi/true-color screens and printer.device for CLUT screens (Zapek)
- General: changed MorphOS detection routines (Zapek)
- GUI: reworked font prefs page (Olli)
- Layouter: some general speed up in font switching (Olli)

### Removed

- GUI: removed the "Always finish looping" option for the transfer animation. It will always stop when the page is fully displayed (Zapek)
- GUI: no longer triggers a text marking state drawing when activating the window within the HTMLview area (Zapek)

### Fixed

- Net: the filecomment of the files is marked as failed if something went wrong during the download process (Zapek)

## [3.0.66]

### Added

- GUI: added SSL version display into the DocInfoWin (Zapek)

### Changed

- GUI: now moves the HTML view area horizontally when displaying the result of a search (Zapek)
- General: replaced HandleInput by EventHandlers (Zapek)
- Cache: authentication cache saves the updated entry to disk instead of using the old one. Happened when eg. the pass changed on the server's side (Zapek)
- Net: could mess up some dates by one day thus avoiding proper cache verification (Zapek)

### Fixed

- GUI: no longer hangs when doing intensive table layouting (Zapek)
- Plugin: removed enforcer hits when embeding an object without SRC tag (Zapek)
- Net: fixed a serious bug in reading of small HTTP/1.1 chunked replies -- due to state screwup, V would only try to read the next chunk segment on the next timer event, thus slowing down the transfer to a crawl. Esspecially hurt with lame ASP pages which virtually sent a HTTP chunk for every line of data (Olli)
- General: fixed the TurboPrint code on high/true color screens (Zapek)

## [3.0.65]

### Added

- Plugin: added vplug_colorspec2rgb() (Zapek)

### Changed

- Cache: auto cache prunning never worked, grr (Zapek)
- Cache: would illegaly store POST result pages in the disk cache (Olli)

### Fixed

- GUI: fixed crash in menu shortcut emulation (Olli)
- Imagedecoder: GIF parser now handles unspecified blocks like extension blocks. Fixes GIF anims generated by Animation Shop. Bumped revision to 11.1. (Olli)
- Layouter: fixed passing of arguments to <EMBED> objects (Olli)

## [3.0.64]

### Added

- Network: added some detection for the hosting OS, like MorphOS, to update the user-agent header accordingly (Zapek)
- GUI: added workaround for nonworking menu shortcuts in MUI 3.8 (Olli)

### Changed

- Plugin: extended the plugin able to it can handle binary return data from VPLUG_HandleURLMethod() by providing a length callback (Olli)

## [3.0.63]

### Added

- General: updated for new CM cleanup scheme (Olli)

### Fixed

- General: fixed a embarassing bug in the image class which would cause invalid pointer reference and possible nuke in a race condition (Olli)
- Imagedecoder: fixed a longstanding lousy bug in the image class which would attempt to scale a NULL bitmap when a gif anim was scaled which had a error in a later frame. Probably cause of many strange and hardly reproducably crashes (found on www.detonate.net/matrix) (Olli)

## [3.0.62]

### Added

- Network: context menu "Load image..." requests a new image and not the one from the cache (Zapek)
- General: added workaround for MUI 3.8 requiring one signal bit per open app window, thus limiting total window count (Olli)

### Changed

- Imagedecoder: updated to use libpng 1.0.5 (Olli)
- Imagedecoder: raised version to 11.0 (Olli)
- GUI: now changes CM data on every modification, to avoid synchronization problems between apps (Olli)

### Removed

- GUI: main window toolbar buttons totally ignored the configured shortcuts, but instead used some old-style hardcoded ones. Also, the matching algorithm ignored qualifiers, thus causing conflicts with ramiga-<key> menu shortcuts. Also, the matching algorithm ignored the disabled state, causing havoc when e.g. back/forward was called although the history list was empty. Uhm. (Olli)

### Fixed

- Imagedecoder: fixed numerous problems in handling grayscale images with alpha channels and/or transparency (Olli)

## [3.0.61]

### Added

- JS: added navigator.plugins[] (Olli)

### Changed

- Layouter: <OPTION> text parsing now skips illegal tags in the text (notably text formatting codes) (Olli)
- JS: array.sort() and array.reverse() returned a bogus object pointer, thus screwing up on stack cleanup (Olli)
- General: image decoder module loader now checks for 68060 systems without FPU, and skips loading the 68060 module (for Morphos dev kernels) (Olli)
- General: reduced internal signal bit usage to better cope with environments where limited signal bits are available (AKA, system hacks are running) (Olli)

### Fixed

- Layouter: fixed a few weirdnesses in handling broken <BLOCKQUOTE> HTML (www.ebay.com et.al) (Olli)

## [3.0.60]

### Changed

- General: now again recognizes FBlit on AGA systems and skips the chip mem buffer completely, resulting in much faster image blittings and less memory consumption (Olli)
- General: now cleans up VViewTemp files on exit (Olli)

### Fixed

- Layouter: fixed problem with <SPAN> breaking layout (Beej)

## [3.0.59]

### Changed

- General: Minor optimizations in HTML parser (Olli)
- GUI: there was a slight chance of V sending methods to hell when closing the CachePruner window during a cache prune (Zapek)

### Fixed

- Layouter: fixed overflow in image backfilling, causing nuke on P96 and AGA systems (Olli)

## [3.0.58]

### Added

- Plugin: added VPLUG_Query_RegisterMIMEExtension so that plugins can handle files without <EMBED> tags. Bumped plugin API to version 3 (Zapek)
- JS: reworked error messages so that they fit into the new error dialog scheme (Olli)

### Fixed

- Layouter: hopefully fixed memtrashing bug in TD background rendering (Olli)
- Net: Fixed the MIME type being ignored for the index.html files automatically accessed when a directory was specified (KingGuppy)
- Layouter: fixed a long-standing bug in the HTML parser which could cause incomplete tags (during incremental parsing) to be returned, sometimes causing funny effects when URLs were cut in <xx SRC>-alike tags (Olli)
- GUI: fixed marking text in plain text rendered pages (Olli)

## [3.0.57]

### Added

- General: MD-2 mailto: interface changed so that no new MD-2 instance is started when one is already running; instead, the new mail window is opened using Rexx (Olli)
- JS: added window.open(). Support for window features is incomplete still (Olli)

## [3.0.56]

### Changed

- Layouter: speed up general background filling (Olli)
- GUI: forward/backward context have their entries in the correct order now (Zapek)
- Net: the pluginfinder never worked (Zapek)

### Fixed

- Layouter: fixed table background filling clash with bgcolor (Olli)
- Layouter: fixed several background filling issues regarding updates (Olli)
- GUI: sourceview window's 'Apply changes' doesn't screw up memory anymore (finally) (Zapek)

## [3.0.55]

### Added

- Layouter: reenabled support for table/cell backgrounds (Olli)

### Fixed

- General: oops, I screwed up the version string (Zapek)

## [3.0.54]

### Added

- JS: added date.getFullYear() (Olli)
- Cache: added semaphore protection for the cache process to avoid funny effects which can happen on systems running Executive and Dynamicache (Zapek)
- GUI: added window menu item to set window to some popular default sizes (Olli)

### Changed

- GUI: error window was created from the network task thus making NList whine about the stack size (Zapek)
- Layouter: add some missing color codes from the original NS color table reference, e.g. "indigo" (Olli)
- Cache: the estimated cache size is now more accurate, even if V doesn't exit properly (Zapek)
- GUI: the Cache Prunning window automatically updates itself when using V. Handy to know how the cache's being used (Zapek)
- Layouter: now doesn't reset layout variables after encountering duplicate <BODY> tags (Olli)
- Layouter: now ignores unknown tags while expecting <TD> (Olli)
- Layouter: setting <FONT COLOR> inside an anchor now also overwrites the "visited link" color (Olli)

### Fixed

- Prefs: some prefs fields related with cache handling were assumed being saved everytime exiting V thus causing weird things like wrong current cache size and funny cache stats. Now the relevant datas are stored into the Cache directory (Zapek)
- Cache: fixed cache pruning being done at 50% of the maximum cache size whe the slider was set to 0 (Zapek)
- GUI: fixed some wrong Cache Prunning reports when closing/reopening the window. Also reports its attempt to start the process (Zapek)
- Internal: fixed pushsyncmethod() being called from the main process (Olli)

## [3.0.53]

### Added

- GUI: clicking on an text or image link with either of the ALT keys depressed now opens a new window, ignoring any possible link target (Olli)
- GUI: added an Error window (tm) (Zapek)
- GUI: added a Cache Prune window (tm) :) (Zapek)

### Changed

- GUI: clicking on a text link now immediately changes the link color to "visited" (Olli)
- Network: it's now possible to browse password protected sites properly without the need to reload cached pages. File downloads from those site are now possible too (Zapek)
- Cache: now deletes possible bogus files on cache pruning (Zapek)

### Removed

- Prefs: removed AWD from default fastlinks (sniff) (Olli)

### Fixed

- Mail: fixed memory leak when sending mail (Olli)
- Mail: fixed quoted-printable encoding to not eat up the complete message contents until after the first >127 char (Olli)
- Mail: fixed escaping of "." in SMTP session handling (Olli)
- Net: fixed problem when parsing FTP directory listings where the date was in the future and no year was specified (gnu "ls" format) -- V would incorrectly fill in the current year, instead of the last year (Olli)
- JS: fixed assigning of function pointers (notably causing errors when manually assigning event handlers to e.g. the window object) (www.citibank.de et.al) (Olli)
- GUI: fixed Historylist events which could arrive during an unexpected state (Zapek)
- JS: fixed a really lame bug in JS where the "check for scalar" function would always fail for objects (and thus numeric variables!), thus causing variable comparisions to be executed as string comparisions always (Olli)

## [3.0.52]

### Changed

- (recompiled for release)

## [3.0.51]

### Added

- JS: added a kludge against web"designers" using something like <xxx OnEvent="javascript:actual code"> (www.bahn.de) (Olli)

### Changed

- Net: when going to a local directory, index.html in that directory will be shown if it exists (KingGuppy)
- GUI: back/forward buttons aren't cleared anymore when pressing the 'OK' button of the prefs window (Zapek)

### Removed

- GUI: password fields no longer remember their contents on back/forward operations for security reasons (Olli)

### Fixed

- Net: changed "Spoof as Mozilla" to spoof as Mozilla/4.5 instead of 4.0. "Fixes" Wells Fargo's online banking (Olli)
- Net: SMTP mailing now uses hardcoded port 25 instead of the "smtp" symbolic name to protect against people with broken protocol databases (Olli)
- JS: fixed a serious array overflow when pushing references. (www.hisoft.co.uk, but crash is only a side-effect of the overflow, which shouldn't happen in the first place) (Olli)

## [3.0.50]

### Added

- General: added updated classes, installer script and libraries (Olli)

### Changed

- VaporWare release
- GUI: back button was cleared and forward button was set when using frames (Zapek)

### Fixed

- GUI: fixed problems with <SELECT> options not being remembered correctly (Zapek)
- GUI: fixed enforcer hits which could happen when adding entries to the historylist (Zapek)
- GUI: fixed race condition after aborting an entry then adding another in the... guess what... Historylist (Zapek)

## [3.0.49]

### Added

- GUI: meta-refreshes aren't added to the Historylist anymore (Zapek)

### Changed

- GUI: Historylist wasn't working with things not needing a DNS lookup like file:/// (Zapek)
- GUI: back/next context menus don't show up when they're not needed (Zapek)

### Fixed

- GUI: possible enforcer hits when using the undo stack of the Historylist (Zapek)

## [3.0.48]

### Changed

- GUI: garbage could appear on the sendmail window when using a signature (Zapek)
- JS: now ignores rest of line on <!-- style "comments" (Olli)

### Fixed

- GUI: oops, I broke key up (Zapek)
- JS: fixed skipping of function definitions (could lead to premature end of interpreting scripts) (Olli)
- JS: hex and octal constants may now exceed 31 bit without turning negative due to int overflow (Olli)
- JS: fixed mod operation to not be int-only (Olli)
- GUI: some more Historylist fixes (tm) (Zapek)

## [3.0.47]

### Added

- General: added a way to add a signature from a file when sending mails (Zapek)

### Changed

- GUI: Historylist between windows wasn't consistent (Zapek)
- GUI: Back/Forward buttons weren't set properly when using the Historylist popdown menu to go to an URL (Zapek)
- JS: image object didn't restore background when src was changed, thus possibly causing artifacts when transpareny was used (Olli)
- General: changed backspace to scroll one page up (Zapek)
- GUI: redirects weren't handled properly in the Historylist (Zapek)

### Fixed

- GUI: loading an URL which was already in the Historylist caused a crash (Zapek)
- GUI: fixed "Retry" button really this time :) (Zapek)

## [3.0.46]

### Added

- GUI: added &COPY; entity since some people will never learn (Zapek)

### Changed

- GUI: rewrote HistoryList almost from scratch :) (Zapek)
- Net: target=_new links now get the proper referer (Olli)
- JS: error requesters now show the offending document URL (Olli)

### Removed

- GUI: "Retry" button in the download window was wrongly in a disabled state sometimes (Zapek)

### Fixed

- GUI: fixed auto-form submission when pressing enter in the only string gadget of a form (Olli)

## [3.0.45]

### Added

- Imagedecoder: added call to get a short image info string. Raised version to 10 due to new function call (Olli)

### Changed

- GUI: image popup menu now shows a short image descriptive string (Olli)
- GUI: viewing individual images now uses the image info call to generate the <TITLE> (Olli)

### Fixed

- GUI: image maps without surrounding <A>nchor showed the wrong context menu (Olli)
- JS: fixed a problem in the flex scanner code which would show up if the often used HTML comment was on the same line with JS code (Olli)

## [3.0.44]

### Added

- JS: added document.location (Olli)
- JS: implemented window.frames[] (Olli)
- JS: implemented frame.frames[] (Olli)
- JS: implemented window.scroll(), window.scrollBy() and window.scrollTo() (Olli)
- JS: implemented frame.scroll(), frame.scrollBy() and frame.scrollTo() (Olli)
- Prefs: added an option to turn on/off the autoloading of the homepage (Zapek)

### Changed

- JS: "frame" class now understands custom properties (Olli)
- Network: falls back to offline mode when there's no TCP/IP stack available (Zapek)

### Fixed

- JS: fixed setting of *.location.* from within javascript: links -- due to an internal screwup, the value set from such a snippet was immediately overwritten with the previous URL (Olli)
- Network: fixed offline mode screwup when changing from manual mode to TCP/IP stack detection (Zapek)
- GUI: fixed some more history bugs (Zapek)

## [3.0.43]

### Added

- Network: added a button to the download window which allows regetting of the selected file, handy when a download fails (Zapek)

### Changed

- Net: URL parser now skips leading spaces (Olli)
- GUI: image popup menu now has a dummy line which holds the image size (later to be expanded to hold more info) (Olli)

### Fixed

- Net: HTTP header generation was messed up again, and would sent a broken request when verifying cache objects (Olli)
- Net: fixed POST form submit in frames (Olli)

## [3.0.42]

### Added

- GUI: added a preference option to chose between requesters or HTML display when there's an error (Zapek)
- JS: implemented window.alert() and window.confirm() (Olli)

### Changed

- General: now cookie expiration takes timezones into account (Zapek)

## [3.0.41]

### Fixed

- Layouter: fixed problems with text flow when a <LI>st is started in an IMG ALIGN=LEFT section, but ends after that (Olli)
- GUI: dragging the location gadget to the fastlink area would cause crash (Olli)

## [3.0.40]

### Added

- Net: added support for http://user:pass@<uri> style URLs (Olli)

### Changed

- General: again some Executive related ajustements. V won't stop decoding pictures if you have a CPU hungry application running in the background (Zapek)
- General: files being downloaded are opened in shared mode so that you can open them with another program meanwhile (handy for mp3s :) (Zapek)
- Net: now again sends a "Pragma: no-cache" when reloading an URL on user's request (Olli)
- Net: under certain circumstances the HTTP request header could be trashed, causing an "invalid request" reply (Olli)

### Fixed

- General: fixed 2KB of chipmem memory leak everytime V was run (Zapek)
- Layouter: fixed form content remembering (Olli)

## [3.0.39]

### Added

- Network: added configurable Accept-Language option which permits some servers to send you documents in your prefered language if it has different versions of it (Zapek)

### Changed

- Layouter: pressing return on <INPUT type="text"> tags won't go to the next object like when pressing TAB (Zapek)
- GUI: link and image context menus now always use the standard font, regardless of whatever font is active for the object (Olli)
- Cache: of course some servers have to not support timezones properly for If-Modified-Since: headers so now V converts it as a GMT time (Zapek)

### Fixed

- Layouter: fixed enforcer hits on exit when there were FORMs displayed on the current page (Zapek)
- Network: fixed some bugs in the authentication handling (Zapek)

## [3.0.38]

### Changed

- GUI: modified docinfowin to not cause a layout failure with long URLs (www.boxman.dk) (Olli)
- Cache: resuming a file which was in the disk cache wasn't performed properly and ended up appending the whole file instead of the relevant part (Zapek)
- Cache: now handles timezones gracefully using locale.library when verifying the cache (Zapek)
- Layouter: now remembers form contents upon resize or back (Olli)
- Layouter: FORM RESET for popup list objects now works (Olli)

### Fixed

- Net: fixed nets_setdocmem() to actually make sure that text files are zero-terminated. This is presumbly the reason for the source editing -> update crashes (Olli)

## [3.0.37]

### Added

- JS: implemented string.lastIndexOf() (Olli)

### Changed

- Fifth prerelease, emergency update

### Fixed

- JS: fixed a serious bug when collecting function names from bytecode which caused all kinds of "heisenbugs", ranging from JS not working at all up to full blown crashes on startup (Olli)
- JS: fixed handling of "unnamed" Images; all those were coasceled into document.images[0] (Olli)

## [3.0.36]

### Changed

- Fifth prerelease
- GUI: canceling a rename ASL requester for a resume didn't work (Zapek)
- GUI: reloading a page cleared the full history (Zapek)

### Fixed

- Network: Oops, I broke HTTP error codes > 400 detection (Zapek)
- JS: function definitions are now parsed before the actual bytecode execution starts. Fixes problem with function references before definition (www.thehun.net et.al.) (Olli)

## [3.0.35]

### Changed

- Network: servers not supporting resume weren't handled properly (Zapek)
- GUI: Back button could be left active eroneously (Zapek)
- Network: downloads which aren't possible (ie. the file doesn't exist on the server) aren't saved localy as a file anymore (Zapek)

### Removed

- GUI: going to an URL and pressing "Stop" before it loads no longer adds it to the URL history list (Zapek)
- GUI: no longer let the status bar into the main window when a download goes into the download window (Zapek)

### Fixed

- Cache: fixed creation of PROGDIR:Cache yet again. Bugs are coming back sometimes... sigh (Zapek)

## [3.0.34]

### Added

- Prefs: Added appropriate backgrounds in the Certificates page. (KingGuppy)
- GUI: Added a list background to the "Downloads" window's list. (KingGuppy)

### Fixed

- Layouter: fixed highliting of AREA SHAPE=CIRCLE image map areas (Olli)

### Security

- Network: The "Don't send Referer: HTTP header" security option now works. (KingGuppy)

## [3.0.33]

### Changed

- Plugin: prefs objects aren't created when launching V anymore. This will result in an improved startup time. Plugin programmers now can expect to get a VPLUGPREFS_Dispose everytime the plugin prefs window is closed (Zapek)
- Plugin: plugins do get a VPLUGPREFS_Setup()/Load() when V starts and not when opening the plugin prefs window. This also solves the problem of VPLUGPREFS_Load() being received after VPLUGPREFS_Create() (Zapek)
- Plugin: V sends a VPLUGPREFS_Dispose when the plugin prefs object couldn't be created as documented in the API (Zapek)

### Fixed

- Prefs: Fixed toolbar editing. Really, this time. (Olli)

## [3.0.32]

### Changed

- Prefs: put some reasonable default values (Zapek)

### Fixed

- Plugin: fixed potential nuke when disposing prefs objects (Zapek)
- Prefs: fixed nuking of toolbar when deleting buttons. Sorry, partial reconfiguration needed... (Olli)
- GUI: fixed some History bugs (Zapek)

## [3.0.31]

### Changed

- Net: Refresh: URL was cut to 256 chars, nuking some click-tracking thingies (notably www.hotmail.com) (Olli)
- Layouter: improved individual viewing of images. Now shows image size in TITLE, and properly refreshes incrementally (Olli)
- GUI: rewrote history handling from scratch. Most of the longstanding bugs and weirdnesses should be history now (Zapek)

### Fixed

- Imagedecoder: fixed race condition when switching inform objects. Could potentially cause pushmethod() to NULL. Bumped version to 9.1 (Olli)
- Network: fixed bug in client inform code which would cause only the first client of a network stream to be informed of new data (Olli)
- Layouter: fixed skipping handler to actually skip tags, too. Fixes <NOSCRIPT>, <COMMENT> et.all (Olli)

## [3.0.30]

### Fixed

- Net: fixed HTTP resume again (Zapek)
- General: fixed another buffer overflow in the resume request (Zapek)
- General: fixed freeing of icon on quit (Olli)
- GUI: using mousewheel scrolling in framesets would Nuke(tm) (Olli)
- Layouter: implementation of Refresh: could trigger a refresh while a modal error dialog (most notably JS) was showing, thus effectivly dragging the context curtain under V's feet. Heavy Nuke(tm) (Olli)

## [3.0.29]

### Added

- Layouter: added kludge against bizarre large <TD WIDTH> values (Olli)

### Changed

- GUI: "Save as..." context menu wasn't asking for the path (Zapek)
- GUI: the hyperlinks "Leave failures" option didn't work at all (Zapek)
- GUI: Netinfo window labels weren't aligned when there were more than 9 simultaneous network connections (Zapek)
- Prefs: TearOff panels layout are saved to PROGDIR:Voyager.tearoff everytime the preferences are saved. Falls back loading ENV:MUI/Voyager.tearoff if the file is missing (Zapek)
- Layouter: entities are now converted everywhere, even in URLs (Olli)
- General: now sets file name as window title if no HTML document or no proper <TITLE> tag (Olli)
- Net: when chosing "Overwrite" or "Rename" in the resume requester, files could be stripped by one byte (Zapek)

### Fixed

- GUI: failed download attempts present in the download list would prevent the same file being downloaded again in future. Hopefully fixed (KingGuppy)
- Net: Fixed a bug in file downloads which would, under some circumstances, mark a file as "failed" when it had fully downloaded.
- Net: hopefully fixed HTTP resume EOT detection (Olli)

## [3.0.28]

### Fixed

- General: seems that NOSCRIPT handling had again been reversed at some point; hopefully fixed. (KingGuppy)
- General: Fixed some Enforcer hits and other fun related to form submission. (KingGuppy)
- General: fixed form submission which was f'cked up in 3.0.27 (Olli)
- GUI: the transfer animation was playing half the frames. Strange Morden didn't notice that bug :) (Zapek)

## [3.0.27]

### Fixed

- General: fixed a rather major bug in the font handling which would cause fonts to be opened only once, but successivly closed on every MUIM_Cleanup of a html view object. Besides screwing the graphics.lib font management, it could also cause the side effect of having fonts in use which had an opencount of 0 and thus could go away on a ramlib flush, causing V (and other apps) to render with a font no longer in memory. All kinds of potential side effects up to crash. Probably also the cause of the "large page bug". (Olli)
- General: fixed possible memory trashing when the resume/overwrite requester popped up (Zapek)
- General: fixed potential buffer overflow crash in form submission (Olli)

## [3.0.26]

### Added

- General: added proper demo mode, with auto-update facility (Olli)

### Changed

- Fourth prerelease

### Removed

- GUI: removed the "Always ask for path" prefs item which is no longer needed (Zapek)

## [3.0.25]

### Fixed

- General: fixed handling of Subject= and Body= in mailto: links (Olli)
- Imagedecoder: fixed a serious bug in the GIF decoder which could, under certain circumstances (no End Marker) write one line over the bitmap boundaries, thus subtly trashing free memory Bumped version to 9 (helpdesk.pandora.be, and probably many other places) (Olli)

## [3.0.24]

### Changed

- Cache: the "Tidy cache" menu item reduce the cache by 50% instead of clearing all of it (Zapek)
- Network: authentications weren't working for some servers which sent an auth request chunked but not the real object (Zapek)
- GUI: Password Manager wasn't loading all entries (Zapek)

### Fixed

- GUI: pressing "Cancel" on the resume window would throw enforcer hits and download the file anyway (Zapek)
- Net: Fixed the "Parent Directory" entry in directory views being out of alignment (KingGuppy)
- General: fixed a low-mem issue in the bitmap scaling handling (Olli)
- General: fixed a problem when the low mem handlers were triggered from the their owning process (semaphore alloc would function although it shouldn't). Same in image decoders, bumped rev to 8.2 (Olli)
- GUI: fixed possible enforcer hits when adding entries to the history list (Zapek)

## [3.0.23]

### Added

- General: added new Button set from Loonsta (Olli)

### Changed

- Layouter: now assumes </A> at table cell ends (Olli)

### Fixed

- General: fixed to open CManager.lib V10 (Olli)
- Layouter: fixed <IMG WIDTH=%> screwing table cell width calculations (Olli)
- Net: fixed trash in email address on the "mailto:"-Form warn dialog (Olli)
- Imagedecoder: Fixed a shutdown race condition on cgfx/p96 systems which could end up with a CGFX allocated BitMap() being freed by custom AGA functions, thus causing Nuke(tm). Bumped image decoder revisions to 8.1 (Olli)

## [3.0.22]

### Added

- Layouter: added <SPAN> (Olli)

### Changed

- GUI: now traps several instances of bogus visual refreshing, esspecially with many images on a page (Olli)

### Fixed

- General: Fixed serious bug when shutting down while V was running on a custom screen (calling imgdec vlib after closing it) (Olli)
- Layouter: <comment> was broken (Olli)
- Layouter: fixed <isindex> button size (Olli)
- General: fixed duplicate freeing of currendir lock when running from WB (Olli)
- Network: FTP path were screwed when using an URL deeper than the root of the server (Zapek)

## [3.0.21]

### Added

- General: added optimized images from Christian "AutoPiloT" Grande (Olli)

### Fixed

- General: fixed speedbar image creation on AGA systems (Olli)

## [3.0.20]

### Added

- Imgdecoder: URLs could be erroneously added in the history when fetching an image (Zapek)

### Changed

- Third prerelease
- GUI: password saving state can be toggled by double clicking on their entries within the Password Manager window (Zapek)
- GUI: there could be duplicates entries in the Password Manager (Zapek)
- General: cleaned up speedbar image generation (Olli)

### Removed

- GUI: no longer adds URLs to the history list when the connection to the site fails (Zapek)

### Fixed

- GUI: fixed really ultra-lame bug in the Password Manager which nuked V on exit after removing an entry (Zapek)
- JS: fixed a nifty crash capability when an object was assigned to itself (Olli)

## [3.0.19]

### Added

- Network: added EPLF support for FTP. EPLF (Easily Parsed LIST Format) is an attempt to standarise LIST outputs (Zapek)
- GUI: new style "about:cache" and other stats pages (Beej)
- Network: added resume support for FTP (Zapek)

### Changed

- General: made all strtok() calls thread-safe (Olli)
- GUI: resume isn't selectable if the local file's size is bigger or equal than the remote file (Zapek)
- Network: FTP works with servers which don't require a password like anonftpd (koobera.math.uic.edu) (Zapek)
- Network: PASV mode works for anonftpd too (Zapek)
- GUI: "Info..." context menu item works for mailto: links (Zapek)

## [3.0.18]

### Added

- GUI: added a cycle gadget to have more choices for the default startup homepage (Beej)
- GUI: added "Save as..." to every context menus (Zapek)
- Network: added resume support for HTTP (Zapek)

### Changed

- JS: changed comparision with "undefined" to always return FALSE except for undefined == undefined (Olli)
- GUI: download window didn't close automatically in all cases (Locutus)
- Layouter: will now show links in the color of a possible surrounding <FONT COLOR> tag (Olli)
- GUI: stop buttons in the network status window weren't working in all cases (Zapek)
- Imgdecoder: now it's possible to tell it to render the image as soon as possible instead of waiting for the lack of incoming data. Gives a better visual feedback but is a bit slower (Zapek)
- Cache: doesn't run the cache prunning process before confirming the requester :) (Zapek)
- General: Cookie Browser failed to open twice (Zapek)
- GUI: the URL string is automatically activated when starting V if there's no URL given as a startup argument (Zapek)
- GUI: About: now shows serial text (for NC3 versions) (Olli)

## [3.0.17]

### Added

- GUI: added a Password Manager (R) (Zapek)

### Changed

- GUI: you can abort connections from the network status window (Zapek)
- Network: the realm used for authentications wasn't handled at all (Zapek)
- Network: remembering of passwords for authentications wasn't working (Zapek)
- Network: saved authentications are cached in memory (Zapek)

### Fixed

- JS: fixed a problem with context screwup during cleanup (Olli)

## [3.0.16]

### Added

- Prefs: added options to enable SSLv2, SSLv3, TLSv1 and bug workarounds individually. Note that enabling TLSv1 or disabling the bug workarounds will most likely fail with most "secure" servers which are not apache+mod_ssl (Olli)

### Changed

- GUI: Double clicking on certificate toggles its disable/enable state (Carlos)
- GUI: "Tidy disk cache" menu item works again (Carlos)
- GUI: V now asks for confirmation before it attempt to flush disk or memory cache (Carlos)
- GUI: network status window is now a virtual group (Olli)
- Network: updated to require Voyager SSL V8+ (Olli)

### Removed

- GUI: prefs releated menu items (save/load) get disabled while settings window is opened (Carlos)

### Fixed

- SSL: no longer crashes when handshake fails and no cert is returned (Olli)
- SSL: updated to OpenSSL 0.9.4, also fixed renogiation bugs (Olli)

## [3.0.15]

### Added

- GUI: added arrows for the columns in the cookie browser (Zapek)

### Changed

- GUI: hotkeyed prefs window (Carlos)
- GUI: prefs panel now uses Popplaceholder class for telnet/mail/news configuration purposes (Carlos)
- GUI: plugin window is now more user friendly and pop ups an requester if there's no-plugin-installed (Carlos)
- GUI: plugin window now uses Floattext object to display plugins' copyright notice, which may help with longer strings (and small screens ;-) (Carlos)
- GUI: cleared layout of prefs window (Carlos)
- Network: updated to require Voyager SSL V7+ (Olli)
- GUI: cookiebrowser's sorting can be saved (Zapek)

## [3.0.14]

### Added

- GUI: added sortable columns to the cookie browser (Zapek)

### Removed

- General: no longer tries to open ppc.library (hunting red herings...) (Olli)

### Fixed

- Cache: fixed the cache cutter which could start even when the cache was disabled, oops :) (Zapek)
- Layouter: fixed 2k limit on <TEXTAREA> fields. Content is now dynamically calculated and size only limited by free memory (Olli)

## [3.0.13]

### Added

- General: added a Cookie Browser (R) (Zapek)
- JS: added document.parent (Olli)
- JS: added (topmost)window.parent (Olli)
- GUI: added "Remember this password?" option to authentication requester (Olli)

### Changed

- Layouter: now understands <COMMENT> (Olli)
- GUI: pressing "ENTER" in the pw gadget of the authentication requester is now equal to clicking "Ok" (Olli)

### Removed

- Cache: no longer uses the cache when its size is set to 0 (Zapek)

### Fixed

- JS: fixed default context object for javascript: links in framesets (www.imag.net) (Olli)

## [3.0.12]

### Changed

- General: V changes its current directory to PROGDIR: (Zapek)
- Bookmark: resurected the "Goto Bookmarks" menu item (Zapek)
- Bookmark: opens the CManager window before doing a search (Zapek)

### Removed

- GUI: no longer activates the download window when adding entries (Zapek)
- Bookmark: removed some old code related to the no longer used Bookmarks.mcc and removed the BOOKMARKS/K option from the arguments (Zapek)

### Fixed

- Layouter: fixed a few references to undefined objects in embedded_class when a <EMBED> object was Too Small (tm) to display info (Olli)
- Bookmark: fixed the menus about loading and saving of Bookmarks which wasn't correct (Zapek)
- General: fixed possible crash when using V's IPC to display an URL (Zapek)
- JS: fixed "Math" object methods; calling methods of the Math object caused total expression stack screwup, and really nifty crashes (www.rauer.de et.al.) (Olli)

## [3.0.11]

### Changed

- Bookmarks: save changes when closing the bookmark window instead of everytime an entry is modified (Zapek)
- Net: cookies weren't sent back to the server when there was no expires header in it (Zapek)

### Fixed

- Imgdecoder: fixed a long standing bug in the GIF decoder which could wreck havoc if a input suspension happened while reading clear codes. Should have happened very seldom, but would cause total puke (common with MRTG generated images, probably due to "LZW free" GD image generation). Changed revision to 7.3 (Olli)
- GUI: fixed activation of last button in toolbar ("Stop" in default config) (Olli)
- GUI: fixed initial toolbar orientation (for new SpeedBar class) (Olli)
- JS: fixed typeof() on undefined symbols (www.teltarif.de) (Olli)

## [3.0.10]

### Added

- Imgdecoder: added a workaround against Executive's behaviour to mess up with things under certain conditions. Progressive image decoding shouldn't need to get a window resize anymore to be displayed properly (Zapek)

### Changed

- Startup: MCCs error requester is a bit more friendly and tells what to do (Zapek)

### Removed

- General: removed MathTransBase opening, not required, as IEEE is used (Olli)

## [3.0.9]

### Added

- Net: added "about:blank" (Olli)
- Layouter: added several of the missing HTML 4 entities with attempted iso-8859-1 replacement chars (Olli)

### Changed

- Net: changed "Accept:" line to explicitely list all image types, to enforce receiving of PNG images (Olli)
- Net: modified "Spoof As Mozilla" User-Agent: line to list Mozilla/4.0, to fool even more lame servers into returning proper stuff (Olli)

### Removed

- Imgdecoder: removed a few excess WaitBlit()s in the chip mem buffer handling (Olli)
- Layouter: no longer pretends to be able to deal with <APPLET> tags (Olli)

### Fixed

- Imgdecoder: fixed a remaining potential low-mem nuke (Olli)
- Imgdecoder: fixed a race condition between the chip-mem copy code and the assorted low mem handler (Olli)

## [3.0.8]

### Added

- Parser: added hack to come around sites which violate the SGML specs by exceeding the maximum clause size with excessive long JS code in event handlers (www.tu-harburg.de) (Olli)
- JS: added special handling for comparisions against null/undefined (no more internal ToString) (Olli)

### Changed

- Second prerelease
- JS: for() loops would leave a bogus value on the expression stack (the increment part expression result wasn't popped) (Olli)
- Prefs: changed default setting for "Cache Images" to true (Olli)

## [3.0.7]

### Changed

- Layouter: cleaned up gray/grey color name aliases (Olli)
- Layouter: VTAB (Ascii-11) is now treated as WSP (Olli)

### Fixed

- Net: Hopefully fixed once and for all problems with parent dirs in file scheme URLs (KingGuppy)
- JS: fixed a problem with function.arguments array lifetime (Olli)
- JS: fixed DOM object lifetime problems, which would cause crash on quit or page switching (Olli)
- General: fixed bug making (under some circumstances) the app to behave as unregistered even the valid keyfile was placed in valid key directory (Carlos)

## [3.0.6]

### Changed

- Net: the "Offline mode" can now be automatically activated if the TCP/IP stack supports online detection (Zapek)
- JS: "undefined" is now "equal" to "null" (sigh) (Olli)
- JS: "var++" and "var--" could, under certain circumstances, actually evaluate to "var+1" resp. "var-1". This caused all kinds of funny side-effects when the construct was used for array iterations (numerous sites) (Olli)
- JS: it's now possible to assign "undefined" values to variables (Olli)

### Fixed

- Prefs: fixed some menu settings being reset when entering the Preference window (Zapek)
- JS: fixed problems with custom properties for document and window class (Olli)
- JS: Array class ToString method would crash when one of the elements was undefined or "null" (Olli)
- JS: A nice one: Some of the classes didn't implement the ToString method, thus causing a nuke when references to them were stored in an array and this array was converted to a string (due to a implicit type conversion, most likely). (Olli)

## [3.0.5]

### Added

- Layouter: added some caching to the size evaluation of form elements. Major speedup with form elements in tables, the active-net order form down to ~3 seconds from well over 30 seconds before (Olli)
- Cache: added more safeguards against corrupted cache data (Olli)

### Removed

- Cache: dropped the V2.x cache format compatibility code, for several problems up to crashing on lost file comments (Olli)

### Fixed

- Layouter: fixed table size cache to not flush entries when a size mismatch occured, in the hope that the cached entry can be reused in later layout stages. Reduced freshmeat.net layout to about 6s on 060/50, approx. 500% faster (Olli)

## [3.0.4]

### Added

- Prefs: added support for "Rexx" executing toolbar buttons (Olli)
- Prefs: added support for "Javascript" executing toolbar buttons. Note that the JS code is actually executed in the context of the main window object (Olli)

### Changed

- Prefs: changing toolbar styles now has immediate visual effect (Olli)
- Prefs: completed toolbar prefs editing (Olli)

## [3.0.3]

### Added

- Bookmarks: added bookmark importation (Voyager 2.x, IBrowse and AWeb) and WWW/FTP URLs exportation using CManager.mcc (Zapek)
- Prefs: added SpeedBar style prefs (Olli)
- Bookmarks: added "Search" option (Zapek)
- Prefs: added button configuration (Olli)

### Changed

- Startup: now does an initial check whether all required MCCs are available, and outputs an error requester otherwise (Olli)
- GUI: regrouped prefs pages for more logical grouping (Olli)
- Prefs: modified certificate management code to only open VSSLBase while loading the certs, to reduce prefs memory footprint (Olli)
- Prefs: changed speedbar handling to use a brush cache, and use a custom IFF loader instead of DTs (Olli)

### Fixed

- Bookmark: fixed "Load/Save As" menu items which were refering to the old Bookmarks.html (Zapek)
- GUI: the source view window now uses Textinput.mcc's own defined fixed font (KingGuppy)

## [3.0.2]

### Added

- Net: added "about:sslcache" to show statistics about the SSL session cache (Olli)
- General: from now on, only sites which have been successfully connected to will be added to the history (Zapek)
- GUI: added RMB menus over back/forward buttons (Olli)

### Changed

- GUI: now uses SpeedBar.mcc for buttons. Prepartion for configurable button system (Olli)

### Fixed

- Bookmark: saves the database everytime a modification is done thus there's less chance in having all the recently added entries being lost because of a crash (Zapek)

## [3.0.1]

### Added

- Plugin: added the code making the API for the plugin interface effective (Zapek)
- Plugin: added Use, Load and Save (Zapek)

### Changed

- Net: Form element NAMEs are now also properly urlencoded when being submitted (Olli)
- GUI: "Flush Images" now yields a result requester (was fully functional otherwise) (Olli)
- JS: JSERROR.LOG now also holds the exact code version (Olli)

### Removed

- Layouter: <INPUT TYPE=BUTTON> no longer works as "SUBMIT" (Olli)

### Fixed

- Net: Funny cookie related bug: V wouldn't honour the cookie expiration date until the cookie table was saved/loaded; this conflicted nicely with the way how php3 forces the deletion of cookies (by Set-Cookie: name=deleted; with an Expires: set 1 year in the past). Reason why login into bugs.vapor.com with V was impossible, ahem. (Olli)
- General: fixed NetConnect key recognition (hopefully) (Olli)
- Layouter: fixed <EMBED WIDTH|HEIGHT=xx%> (Olli)

## [3.0.0]

### Added

- added new Icons by Morden, remapped by Entity (Olli)

### Changed

- First Prerelease
- big pack-up, all fingers crossed

[Unreleased]: https://github.com/amigazen/Voyager/compare/v3.5.2...HEAD
[3.5.2]: https://github.com/amigazen/Voyager/compare/v3.5.1...v3.5.2
[3.5.1]: https://github.com/amigazen/Voyager/releases/tag/v3.5.1
[3.4.9]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#349---2005-01-01
[3.3.126]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#33126---2002-12-27
[3.2.0]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#320---2000-04-16
[3.1.6]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#316
[3.1.5]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#315
[3.1.4]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#314
[3.1.3]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#313
[3.1.2]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#312
[3.1.1]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#311
[3.1.0]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#310
[3.0.71]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3071
[3.0.70]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3070
[3.0.69]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3069
[3.0.68]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3068
[3.0.67]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3067
[3.0.66]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3066
[3.0.65]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3065
[3.0.64]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3064
[3.0.63]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3063
[3.0.62]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3062
[3.0.61]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3061
[3.0.60]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3060
[3.0.59]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3059
[3.0.58]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3058
[3.0.57]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3057
[3.0.56]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3056
[3.0.55]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3055
[3.0.54]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3054
[3.0.53]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3053
[3.0.52]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3052
[3.0.51]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3051
[3.0.50]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3050
[3.0.49]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3049
[3.0.48]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3048
[3.0.47]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3047
[3.0.46]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3046
[3.0.45]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3045
[3.0.44]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3044
[3.0.43]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3043
[3.0.42]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3042
[3.0.41]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3041
[3.0.40]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3040
[3.0.39]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3039
[3.0.38]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3038
[3.0.37]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3037
[3.0.36]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3036
[3.0.35]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3035
[3.0.34]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3034
[3.0.33]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3033
[3.0.32]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3032
[3.0.31]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3031
[3.0.30]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3030
[3.0.29]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3029
[3.0.28]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3028
[3.0.27]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3027
[3.0.26]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3026
[3.0.25]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3025
[3.0.24]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3024
[3.0.23]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3023
[3.0.22]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3022
[3.0.21]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3021
[3.0.20]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3020
[3.0.19]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3019
[3.0.18]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3018
[3.0.17]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3017
[3.0.16]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3016
[3.0.15]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3015
[3.0.14]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3014
[3.0.13]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3013
[3.0.12]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3012
[3.0.11]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3011
[3.0.10]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#3010
[3.0.9]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#309
[3.0.8]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#308
[3.0.7]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#307
[3.0.6]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#306
[3.0.5]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#305
[3.0.4]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#304
[3.0.3]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#303
[3.0.2]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#302
[3.0.1]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#301
[3.0.0]: https://github.com/amigazen/Voyager/blob/main/CHANGELOG.md#300
