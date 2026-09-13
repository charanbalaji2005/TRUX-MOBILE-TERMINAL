# resources/fonts/

Empty. Bundled fallback fonts are meaningless without a renderer to
load and shape them — see `docs/renderer.md`. `TerminalConfig` already
persists `font_family`/`font_size` (`docs/configuration.md`) for
whenever a renderer exists to use them.
