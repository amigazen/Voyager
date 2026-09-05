# Voyager V3 3.5

This is Voyager V3 3.5, an HTML 4 web browser for Amiga, rebuilt from the GPL source drop of VaporWare's Voyager available at original Voyager developer [Zapek's GitHub](https://github.com/zapek/Voyager)

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** is using modern software development tools and methods to update and rerelease classic Amiga open source software. Projects include a new AWeb, a new Amiga Python 2, and the ToolKit project - a universal SDK for Amiga.

Key to the amigazen project approach is ensuring every project can be built with the same common set of development tools and configurations, so the ToolKit project was created to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The original authors of the *Voyager* software are not affiliated with the amigazen project. This software is redistributed on terms described in the documentation, particularly the file LICENSE.md

The amigazen project philosophy philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all projects are gratefully received at [GitHub](https://github.com/amigazen/). While the focus now is on classic 68k software, it is intended that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About Voyager V3 3.5

Voyager is one of the classic Amiga web browsers of the late 1990s, alongside IBrowse and AWeb. Oliver Wagner started it in 1995; David Gerber co-developed it with him. It is a MUI application with an HTML 3.2 layout engine (woth work in progress HTML 4 features), a JavaScript 1.3 / ECMA-Script bytecode compiler and interpreter, a plugin API, and SSL through a loadable `voyager_ssl.vlib` module.

There were also a CaOS embedded build and a MorphOS bundle. Historically it ran on 68k, ColdFire, and PPC. This 3.5.2 restoration targets the classic 68k SAS/C line for AmigaOS only.

The last widely shipped commercial release was Voyager 3.2 (16 April 2000). An alpha release 3.3 followed in August 2000. The source in this tree is derived from the the later "3.4" GPL drop (copyright through 2003, with CVS timestamps into 2004–2005). 

This project's aim is to take that GPL tree and produce a Voyager that:

- Builds with SAS/C against the ToolKit standard for Amiga SDK configuration
- Runs again on classic 68k Amiga as a Workbench application
- Talks to current TCP/IP stacks via `bsdsocket.library`
- Loads HTTPS using a VSSL plugin backed by modern SSL libraries rather than the historical OpenSSL 0.9 tree

Fixes required to compile, link, start, layout, draw, network, and exit cleanly are in scope.

## HTML Standards Support

Gerber documented Voyager as **HTML 3.2**, with later tags marked `+`. The layout sources still say “V HTML4 Layout engine”. CSS was work in progress: `<STYLE>` is hidden (historically scanned for JavaScript); `parse_css()` in this tree is a stub.

### HTML Element Support

| Element | Support | Notes |
|---------|---------|-------|
| **Document Structure** | | |
| `HEAD` | ✅ | Author list; tag itself is `ht_dummy` (content still parsed) |
| `BODY` | ✅ | `BGCOLOR`, `TEXT`, `LINK`, `ALINK`, `VLINK`, `BACKGROUND` |
| `TITLE`, `BASE`, `ISINDEX` | ✅ | |
| `META` | ✅ | `http-equiv`: refresh, pragma no-cache, content-type |
| `LINK` | ❌ | Author: ignored, not important; not tokenized |
| `STYLE` | ❌ | Author: hidden, searched for JS. This tree skips the inner text (`TOFIX`); CSS is not applied |
| `SCRIPT` / `NOSCRIPT` | ✅ | JavaScript; `NOSCRIPT`+ |
| **Text Formatting** | | |
| `H1`–`H6` | ✅ | |
| `P` | ✅ | `left`, `right`, `center`, `middle`+ |
| `BR` | ✅ | `clear=left/right/all` |
| `HR` | ✅ | `width`, `size`, `noshade`, `color` |
| `PRE`, `XMP` | ✅ | |
| `LISTING` | ❌ | Author: not handled, obsolete; tokenized, no layout |
| `PLAINTEXT` | ❌ | Author: not handled, obsolete |
| `DIV`, `CENTER` | ✅ | |
| `SPAN` | ❌ | Tokenized no-op in this tree; not on the author’s HTML 3.2 list |
| `NOBR` | ✅ | Netscape+ |
| `WBR` | ❌ | |
| `ADDRESS` | ❌ | Author: no special action |
| `BLOCKQUOTE` | ❌ | Author: ignored, useless |
| `Q` | ❌ | HTML 4 |
| **Text Style** | | |
| `TT`, `I`, `B`, `U`, `STRIKE`, `S`+ | ✅ | |
| `INS`+ | ✅ | Underline |
| `DEL` | ✅ | Strikethrough (this tree; not on the author’s short list) |
| `BIG`, `SMALL` | ✅ | |
| `SUB`, `SUP` | ❌ | Author: ignored; not tokenized |
| `EM`, `STRONG`, `DFN`, `CODE`, `SAMP`, `KBD`, `VAR`, `CITE` | ✅ | |
| `FONT` | ✅ | `color`, `size`, `face`+ |
| `BASEFONT` | ✅ | |
| `ABBR`, `ACRONYM`, `BDO` | ❌ | HTML 4 |
| `BLINK` | ❌ | |
| **Lists** | | |
| `UL`, `OL`, `LI` | ✅ | |
| `DL`, `DD` | ✅ | |
| `DT` | ❌ | Author: not handled (this tree still shares the `LI` case) |
| `DIR`, `MENU` | ❌ | Author: ignored, useless |
| **Links and Images** | | |
| `A` | ✅ | `href`, `title`, `target`+, `accesskey`+; `rel`/`rev` unused |
| `IMG` | ✅ | `src`, `alt`, `align`, `width`, `height`, `hspace`, `vspace`, `border`, `ismap`, `usemap`, `lowsrc`+ |
| `MAP`, `AREA` | ✅ | `shape`, `href`, `target`, `coords`, `accesskey`+ |
| **Tables** | | |
| `TABLE` | ✅ | `bgcolor`+, `background`+, `cellpadding`, `cellspacing`, `border`, `width`, `height`+, border colors+, `align` |
| `TR` | ✅ | `bgcolor`+, `background`+, `valign`, `align` |
| `TD` / `TH` | ✅ | `bgcolor`+, `background`+, `width`, `height`, `rowspan`, `colspan`, `valign`, `align`, `nowrap` |
| `CAPTION` | ❌ | Tokenized, no layout; not on the author’s list |
| `THEAD`, `TBODY` | ❌ | Token lines commented out |
| `TFOOT`, `COL`, `COLGROUP` | ❌ | HTML 4 table model |
| **Forms** | | |
| `FORM` | ✅ | `action`, `target`, `method`, `enctype` |
| `INPUT` | ✅ | `text`, `password`, `checkbox`, `button`+, `radio`, `submit`, `image`, `reset`, `file`, `hidden`; `name`, `value`, `checked`, `size`, `maxlength`, `src`, `align` |
| `SELECT` | ✅ | `size`, `multiple`, `disabled` |
| `OPTION` | ✅ | |
| `OPTGROUP` | ❌ | HTML 4 |
| `TEXTAREA` | ✅ | `disabled`, `rows`, `cols`, `readonly`+ |
| `BUTTON` | ✅ | Element present in the GPL tree (author listed `INPUT type=button`+) |
| `LABEL`, `FIELDSET`, `LEGEND` | ❌ | HTML 4 |
| **Frames** | | |
| `FRAMESET` / `FRAME` | ✅ | + |
| `NOFRAMES` | ✅ | + |
| `IFRAME` | ❌ | |
| `LAYER`, `ILAYER` | ❌ | Netscape 4 |
| **Embedded Content** | | |
| `APPLET` | ❌ | Author: not supported, no JVM |
| `PARAM` | ❌ | Tokenized, no layout |
| `EMBED` | ✅ | + `type`, `src`, `width`, `height`, `hspace`, `vspace`, `border`, `align` |
| `NOEMBED` | ✅ | + |
| `OBJECT` | ❌ | |
| `BGSOUND`, `MARQUEE` | ❌ | Internet Explorer |
| `<!-- -->` | ✅ | |

### HTML Features

| Feature | Support | Notes |
|---------|---------|-------|
| Character / numeric entities | ✅ | Named, `&#nnn;`, hex |
| Nested tables | ✅ | Major 3.x layout work |
| File upload | ✅ | `INPUT type=file` / HTTP POST |
| Framesets | ✅ | |
| Inline frames | ❌ | No `IFRAME` |
| Client-side image maps | ✅ | |
| CSS 1 / CSS 2 | ❌ | Never a Voyager product feature |
| Inline `style=` / `class=` styling | ❌ | `id` used for JS lookup |
| XHTML 1.0 | ❌ | |
| `data:` URLs | ❌ | |

## HTTP, FTP, mail, SSL

Author list, still present in this tree unless noted.

| Feature | Support | Notes |
|---------|---------|-------|
| **HTTP/1.1** | ✅ | Falls back to 1.0 when the peer fails 1.1 |
| Content negotiation | ✅ | `Accept` / `Accept-Language` |
| Persistent connections (keep-alive) | ✅ | Not used on SSL in this tree (`trying11` is off for SSL) |
| Resume | ✅ | `Range: bytes=` |
| Uploads / POST | ✅ | |
| Proxies | ✅ | Including `Proxy-Authorization` |
| Cache control | ✅ | Expirations, `If-Modified-Since`, pragma no-cache |
| Authentication | ✅ | Basic |
| Language tags | ✅ | |
| Cookies | ✅ | |
| User-Agent spoofing | ✅ | For broken sites; 3.5.2 also has a Firefox-grammar default so CDNs do not 403 |
| **FTP** anonymous and normal login | ✅ | |
| FTP persistent connections | ✅ | Prefs `keepftp` |
| FTP resume | ✅ | `REST` |
| FTP proxies | ✅ | |
| Advanced `LIST` / `ls -l` parsing | ✅ | |
| EPLF | ✅ | |
| **SMTP** | ✅ | Simple mail from `mailto:` / form mail |
| **SSL** SSLv2 / SSLv3 / TLS 1.0 | ❌ | Historical OpenSSL 0.9 VSSL (up to 168-bit keys, certificates). **3.5.2** ships AmiTLS (TLS 1.2, SNI) instead |

## Cache, images, plugins, editor

| Feature | Support | Notes |
|---------|---------|-------|
| Variable memory cache | ✅ | Images kept while memory lasts |
| Variable disk cache | ✅ | Automatic cleanup; no index, always consistent (author) |
| Inline image decode | ✅ | GIF (animated / progressive), JPEG, PNG (alpha), XBM |
| **VFlash** | ✅ | Flash 3 and some Flash 4; plugin in this tree |
| **VPDF** | ❌ | PDF viewer by Emmanuel Lesueur; not bundled in 3.5.2 |
| HTML source editor | ✅ | Internal view/edit with HTML tag highlighting (`sourceview` / Textinput) |

## JavaScript Support

Gerber documented **JavaScript 1.3 / ECMA-Script**. The engine compiles source to `JSOP_*` bytecode (`js_compile` / Bison `js.y`) and interprets it (`js_interpret`). That is Netscape 1.2/1.3 (strict equality, `switch`, `RegExp`, `for`/`in`, `with`). It is not JavaScript 1.5 / ECMAScript 3.

### JavaScript Language Features

| Feature | Support | Notes |
|---------|---------|-------|
| **Data Types** | | |
| Numbers | ✅ Full | Integer and floating-point |
| Strings | ✅ Full | String literals and operations |
| Booleans | ✅ Full | `true` and `false` |
| `null`, `undefined` | ✅ Full | Null and undefined values |
| Objects | ✅ Full | Object properties and constructors |
| Arrays | ✅ Full | Array objects and methods |
| Functions | ✅ Full | Function objects |
| **Operators** | | |
| Arithmetic | ✅ Full | `+`, `-`, `*`, `/`, `%`, `++`, `--` |
| Comparison | ✅ Full | `==`, `!=`, `===`, `!==`, `<`, `>`, `<=`, `>=` |
| Logical | ✅ Full | `&&`, `\|\|`, `!` |
| Bitwise | ✅ Full | `&`, `\|`, `^`, `~`, `<<`, `>>`, `>>>` (`JSOP_OP_GGG`) |
| Assignment | ✅ Full | `=`, `+=`, `-=`, etc. |
| Ternary | ✅ Full | `? :` conditional operator |
| **Control Flow** | | |
| `if`/`else` | ✅ Full | Conditional statements |
| `while`, `do/while` | ✅ Full | Looping constructs |
| `for`, `for/in` | ✅ Full | Iteration |
| `switch` | ✅ Present | In the GPL parser |
| `break`, `continue` | ✅ Full | Loop control |
| `return` | ✅ Full | Function return values |
| **Functions** | | |
| Function declarations | ✅ Full | Named functions, function expressions, Function objects |
| `this` keyword | ✅ Full | Context object reference |
| `new` operator | ✅ Full | Object instantiation |
| `typeof`, `delete`, `void` | ✅ Full | In `js.y` |
| `with` | ✅ Full | Pushes an object context |
| `var` declarations | ✅ Full | Variable declarations |
| `try` / `catch` / `finally` | ❌ | Not in the lexer or parser |
| `instanceof` | ❌ | |
| `in` (as `prop in obj`) | ❌ | `TOKEN_IN` is only used for `for`/`in` |
| Labeled `break` / `continue` | ❌ | |
| `const` | ❌ | Netscape 1.5 extension |

### Browser Object Model (BOM)

| Object | Support | Notes |
|--------|---------|-------|
| **window** | ✅ Full | Top-level window (`htmlwin`) |
| `window.open()` / `close()` | ✅ Full | Window control as implemented in 3.x |
| `window.alert()`, `confirm()` | ✅ Full | Dialogs |
| `window.prompt()` | ❌ | Not on the window property table |
| `window.setTimeout()` / `setInterval()` | ✅ Full | Also `clearTimeout` / `clearInterval` |
| `window.status` | ✅ Full | Status bar text |
| `window.defaultStatus` | ❌ | |
| `window.location` | ✅ Full | Location object |
| `window.history` | ✅ Full | History list |
| `window.navigator` | ✅ Full | Navigator object |
| `window.document` | ✅ Full | Document / HTML view |
| `window.frames` | ✅ Full | Frame access |
| `window.opener` | ❌ | Internal `f_opener` only; not a JS property |
| `window.parent` / `self` / `top` | ✅ Full | |
| `screen` | ✅ Full | Global `screen` object (`js_screen.c`), not a `window.screen` property |
| **document** | ✅ Partial | `write`/`writeln`, `open`/`close`, cookie, title, URL, referrer, `lastModified`, `domain`, `all` (alias for the document) |
| `document.bgColor`, `fgColor`, `linkColor`, `alinkColor`, `vlinkColor` | ❌ | |
| `document.body` | ❌ | |
| `document.applets[]` | ❌ | |
| `document.getElementById()` | ✅ Partial | Looks up `id` via `MM_JS_FindByID`; not a DOM Level 1 tree API |
| `document.getElementsByTagName()` / `getElementsByName()` | ❌ | |
| `document.createElement()` / `createTextNode()` / `appendChild()` | ❌ | No DOM tree |
| `innerHTML` / `outerHTML` | ❌ | |
| JavaScript entities (`&{...};`) | ❌ | Listed as incomplete in the 3.2 readme; still absent |
| `document.forms[]` | ✅ Full | Form collection |
| `document.images[]` | ✅ Full | Image collection |
| `document.links[]` / `anchors[]` | ✅ Full | Same collection of anchors |
| `document.embeds[]` / `plugins[]` | ✅ Full | Embed collection |
| **form** | ✅ Full | Form object, `elements[]`, `submit()` / `reset()` |
| **form elements** | ✅ Full | Text, password, textarea, select, checkbox, radio, button, hidden |
| **location** | ✅ Full | `href`, URL parts, reload/replace as implemented |
| **history** | ✅ Full | `back()`, `forward()`, `go()` |
| **navigator** | ✅ Full | `appName`, `appCodeName`, `appVersion`, `userAgent`, `platform`, `plugins`, `mimeTypes`, `javaEnabled`, and others on `js_navigator.c` |
| **plugin / mimeType** | ✅ Full | Plugin and MIME type objects |
| `Image()` constructor | ❌ | |
| `Option()` constructor | ❌ | |
| `XMLHttpRequest` | ❌ | Not an HTML 4 DOM feature; absent here |

### JavaScript Built-in Objects

| Object | Support | Notes |
|--------|---------|-------|
| **String** | ✅ Full | Including `match`, `replace`, `search`, `split`, `concat` |
| **Number** | ✅ Full | Number object |
| **Boolean** | ✅ Full | Boolean object |
| **Array** | ✅ Full | `join`, `reverse`, `sort`, `concat`, `slice`, `splice`, `push`, `pop`, `shift`, `unshift` |
| **Math** | ✅ Full | Mathematical functions and constants |
| **Date** | ✅ Full | Local and UTC getters/setters |
| **Object** | ✅ Full | Base object type |
| **Function** | ✅ Partial | `arguments`, `arity`, `caller` |
| `Function.prototype.apply` / `call` | ❌ | |
| **RegExp** | ✅ Present | `js_regexp.c`; String `match` / `replace` / `search` / `split` use it |
| **Error** / `throw` | ❌ | |

### JavaScript Event Handlers

| Event Handler | Support | Notes |
|---------------|---------|-------|
| `onClick`, `onDblClick` | ✅ Full | Wired in `layout_parse.c` |
| `onLoad`, `onUnload` | ✅ Full | |
| `onSubmit`, `onReset` | ✅ Full | |
| `onChange`, `onSelect` | ✅ Full | |
| `onFocus`, `onBlur` | ✅ Full | |
| `onMouseOver`, `onMouseOut` | ✅ Full | |
| `onMouseDown`, `onMouseUp`, `onMouseMove` | ✅ Full | |
| `onKeyDown`, `onKeyUp` | ✅ Full | |
| `onKeyPress` | ❌ | Not in the event-name table |
| `onError`, `onAbort` | ✅ Full | |
| `onResize`, `onMove` | ✅ Full | Names are registered; behaviour is period Netscape |
| `addEventListener` / `attachEvent` | ❌ | No DOM Events |

### JavaScript Limitations

The 3.2 readme is out of date on `SCRIPT SRC` (that path works in this tree). Remaining gaps for an HTML 4 generation browser are the ❌ rows above: no CSS, no DOM tree, no `try`/`catch`, no `XMLHttpRequest`.

Voyager can append JavaScript errors to `JSERROR.LOG` in the program drawer when that preference is enabled.

## Roadmap

The last public VaporWare **boxed** product release was Voyager 3.2 in 2000. A limited 3.3 line followed in CVS (public betas, then 3.3.126 / MorphOS), then 3.4.x. Source was later published under the GNU GPL. 

Version **3.5.2** is the first public amigazen project release. It is derived from that GPL tree. The goals are:

- Ensure Voyager can be built with supported SDKs (SAS/C, NDK, MUI, CyberGraphX, ToolKit)
- Make the application start, fetch, layout, draw, and quit on classic Amiga
- Replace runtime dependencies that cannot be satisfied in 2026 (keyfile / `vapor_toolkit.library` checks, MiamiSSL, the OpenSSL 0.9 VSSL build as the shipping TLS plugin)
- Package a Workbench drawer with binaries, MUI classes, plugins, certificates, and documentation
- Keep behaviour aligned with Voyager 3 as users knew it

Notable changes in 3.5.2 compared to a stock 3.2 binary install are restoration and porting, not new browsing features:

- SAS/C 68k `smakefile` / `SCOPTIONS` for the main program, imgdecode, MimePrefs, VFlash, and libmath64
- Image decoder linked into the executable (not a separate `vimgdecode.library` selected at runtime)
- CyberGraphX pixel-write paths
- SpeedBar toolbar enabled; TearOff panels disabled due to Enforcer hits in the MUI TearOff.mcc
- Runtime `vapor_toolkit.library` and keyfile checks removed; registration nag removed
- HTTPS via AmiTLS (`amitls.library`, BearSSL, TLS 1.2, SNI) as a new `voyager_ssl.vlib`; OpenSSL 0.9 under `Source/VSSL/openssl` is historical and not linked
- Default User-Agent uses Firefox 4 grammar with AmigaVoyager in the Mozilla comment so current CDNs do not reject the client; the Spoof menu can still identify as Voyager
- Compile, layout, image, HTTP header, and first-paint fixes required to run at all (see [CHANGELOG.md](CHANGELOG.md))


See [CHANGELOG.md](CHANGELOG.md) for the detailed 3.5.2 log and the copied VaporWare Voyager 3.x history.

## Frequently Asked Questions

### What is the aim of this Voyager project?

The GPL release of that code was a generous act. However, the tree as originally published did not build or run out of the box with standard Amiga tools. The job of this Voyager 3.5 release is to restore Voyager V3 so it can be compiled, installed, and used again, with some maintenance also possible.

### Does this Voyager work on Workbench 3.1, 3.5 or 3.9?

The 3.5.2 68k build is aimed at classic Amiga with MUI, `bsdsocket.library`, and optional cybergraphics.libraru for RTG. It is built in the AmigaOS 3.2 NDK / ToolKit environment used by other amigazen project releases. A TCP/IP stack must be running. AmiTLS (`amitls.library`) and an included CA bundle (`Certificates/cacert.pem`) are required for HTTPS support

Tear-off control panels from Voyager 3 are not enabled in 3.5.2 (`USE_TEAROFF` is 0) because `TearOffPanel.mcc` is not clean of Enforcer hits and the source code is not available for maintenance.

### Will Voyager support modern websites?

No more than Voyager 3 ever did. HTML5, CSS3, WebAssembly, WebGL, and the Chromium/WebKit/Gecko engines are a different class of software. Classic Amiga Voyager remains a 1990s / early-2000s browser. Sites that still speak that dialect — including many Amiga sites and archive/proxy services — are the intended target.

### What happened to VaporWare Voyager after 3.2?

Voyager 3.2 was the last full public product release (16.4.2000). Development continued in CVS; the GPL sources in this repository are that later tree. Historical notes remain on [zapek.com](https://zapek.com/software/voyager/). VaporWare automatic software updates and keyfile infrastructure dependencies have been removed in order to make this 3.5 release standalone and available to all.

### Will this version be available for OS4, AROS or MorphOS?

The GPL tree already contains MorphOS conditionals. The first priority is a stable classic 68k SAS/C build. Native ports can be revisited once that line is solid. A 68k binary may run on OS4 in emulation the same way other 68k MUI software does; that is not the primary test target for 3.5.

### Can I contribute to the new Voyager?

Yes please! Whether code, testing or translations and documentation, all contributions are welcome and will remain free and open source in the spirit and letter of the (LICENSE.md)). See also (CONTRIBUTING.md)

## Contact

- At GitHub https://github.com/amigazen/Voyager
- on the web at http://www.amigazen.com/voyager/ (Amiga browser compatible)
- or email aweb@amigazen.com

## Acknowledgements

*Amiga* is a trademark of **Amiga Inc**.

Original Voyager by Oliver Wagner, with David Gerber, Jon Bright, Matt Sealey, Simone Tellini, Jerome Fisher, and the rest of the VaporWare Voyager team, released under the GNU GPL.

MUI by Stefan Stuntz. SpeedBar by Simone Tellini. Historical TearOff classes by Szymon Ulatowski. [AmiTLS from amigazen project](https://github.com/amigazen/AmiTLS) as the current VSSL backend.

Voyager V3 is dedicated to the memory of Oliver Wagner, 1969 - 2016.