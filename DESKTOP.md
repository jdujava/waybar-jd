# Desktop changes

This is my `desktop` branch of Waybar, based on the revision with native Mango
modules. It adds the tag window indicators, icon sizing controls, calendar
scrolling and pinning, per-application tray scaling and tray/MPD fixes used by
my setup.

Configuration lives in `~/.config/waybar/`. Module options are in
`config.jsonc`; fonts, colors, spacing and icon offsets are in `style.css`.

## Installation and development

The complete Arch package recipe lives in
[AUR-jdujava](https://github.com/jdujava/AUR-jdujava). It fetches this fork's
`desktop` branch directly from GitHub; no local source checkout is required.
With that PKGBUILD repository configured, install through Paru:

```sh
paru -S waybar-jd-git
```

The package supplies `/usr/bin/waybar`. For development, clone the fork anywhere
and use its native Meson build:

```sh
git clone --branch desktop https://github.com/jdujava/waybar-jd.git
cd waybar-jd
meson setup build --prefix=/usr/local --bindir=bin --buildtype=release \
    --wrap-mode=nofallback --auto-features=disabled \
    -Dmango=true -Dniri=false -Dexperimental=false \
    -Dlibudev=enabled -Drfkill=enabled -Dlibnl=enabled \
    -Dpulseaudio=enabled -Dmpd=enabled -Ddbusmenu-gtk=enabled
meson compile -C build
sudo meson install -C build --no-rebuild --tags runtime
```

Compilation runs without root. The runtime install tag installs only the binary,
not defaults or systemd services. For an existing build, use
`meson setup --reconfigure build` with the same options, then compile again.
`meson compile -C build -j 2` limits parallel compilation when desired.

Launch `waybar` by name: `/usr/local/bin/waybar` overrides the package through
normal PATH lookup. Remove or archive that development binary to return to
`/usr/bin/waybar`, then restart the bar. Existing Zsh shells may need `rehash`.
Optional dotfiles build helpers are conveniences, not build dependencies.

Pacman tracks the packaged defaults in `/etc/xdg/waybar/config.jsonc` and
`/etc/xdg/waybar/style.css` as backup files. Personal configuration remains in
`~/.config/waybar/`.

## Wi-Fi bitrate queries

The network module has a `wifi-bitrate` option, enabled by default. Set it to
`false` if your formats do not use `{rxBitrate}` or `{txBitrate}`.

This avoids a station query that some drivers reject. Signal strength, SSID,
frequency and the ordinary traffic counters still work.

## Reviewing and carrying the changes

Review the custom series against the official upstream's current default branch
(`upstream/master`):

```sh
git fetch upstream
git log --oneline desktop..upstream/master
git log --oneline upstream/master..desktop
git diff upstream/master...desktop
```

Keep source changes on this repository's `desktop` branch and clone that branch
on another machine. The official Waybar repository is the `upstream` remote;
the customized public fork is `origin`. Dotfiles hold personal configuration.
Keep source pins, patches and source inventories out of dotfiles.

Before future implementation changes, fetch and inspect upstream, then rebase
the custom series onto its current default branch. Preserve the original tip
under a private ref or in a verified bundle and use a temporary branch in an
isolated worktree for the candidate. Keep coherent feature commits, fold fixups
into their owning features, and keep documentation and maintenance in one final
commit. When regrouping history, first verify that the complete tree at the
original base is identical to the preserved tip; only then replay it upstream.

Inspect overlapping upstream code before resolving conflicts. Preserve custom
semantics alongside new upstream behavior, removing only demonstrably redundant
hunks. Report changes to desktop behavior, configuration, protocol support or
build requirements and any conflict decisions. Build and check the candidate
before integrating it into `desktop`; do not push rewritten history unless
requested. Keep equality checkpoints, grouping and conflict reports in private
backups rather than dotfiles.

For an offline backup, use `git bundle create waybar.bundle desktop` and verify
it with `git bundle verify waybar.bundle`. Clone the verified bundle to recover
the branch. A local backup is not needed for normal package installation.

Use the remote's current symbolic default-branch ref if its name changes; no
fixed upstream revision needs to be maintained in this document.
