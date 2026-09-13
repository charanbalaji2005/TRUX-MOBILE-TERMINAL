module.exports = function(addArt) {
  addArt('pkg-fedora', 'Fedora / RPM', 'PACKAGING', 'implemented',
    'Building and installing RPM packages with DNF and spec files.',
    [
      { id: 'fedora-rpm', text: 'Fedora / RHEL RPM Package', level: 2 },
      { id: 'spec-file', text: 'RPM Spec File & Compilation', level: 2 },
      { id: 'copr-repo', text: 'Fedora COPR Repository Publishing', level: 2 }
    ],
    `
<h2 id="fedora-rpm">Fedora / RHEL RPM Package</h2>
<pre><code class="language-bash"># Install locally built RPM
sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<h2 id="spec-file">RPM Spec File & Compilation</h2>
<p>The spec file is located in <code>packaging/rpm/meridian-terminal.spec</code>. To build locally:</p>
<pre><code class="language-bash">rpmbuild -ba packaging/rpm/meridian-terminal.spec</code></pre>

<h2 id="copr-repo">Fedora COPR Repository Publishing</h2>
<p>Meridian packages are automatically built for Fedora 39, 40, 41, and Rawhide via automated COPR webhooks on release tags.</p>
`
  );

  addArt('pkg-debian', 'Ubuntu / Debian / DEB', 'PACKAGING', 'implemented',
    'Installing and building .deb packages for Ubuntu, Debian, and Linux Mint.',
    [
      { id: 'debian-deb', text: 'Ubuntu & Debian DEB Package', level: 2 },
      { id: 'deb-packaging', text: 'Building .deb Packages', level: 2 }
    ],
    `
<h2 id="debian-deb">Ubuntu & Debian DEB Package</h2>
<pre><code class="language-bash">sudo apt update
sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>

<h2 id="deb-packaging">Building .deb Packages</h2>
<pre><code class="language-bash">dpkg-deb --build packaging/debian meridian-terminal_2.0.0_amd64.deb</code></pre>
`
  );

  addArt('pkg-arch', 'Arch / AUR', 'PACKAGING', 'implemented',
    'PKGBUILD packaging and AUR installation for Arch Linux and Manjaro.',
    [
      { id: 'arch-aur', text: 'Arch Linux PKGBUILD', level: 2 },
      { id: 'aur-install', text: 'AUR Helper Installation', level: 2 }
    ],
    `
<h2 id="arch-aur">Arch Linux PKGBUILD</h2>
<pre><code class="language-bash">cd packaging/arch
makepkg -si</code></pre>

<h2 id="aur-install">AUR Helper Installation</h2>
<pre><code class="language-bash">yay -S meridian-terminal-bin</code></pre>
`
  );

  addArt('pkg-opensuse', 'openSUSE', 'PACKAGING', 'implemented',
    'Installing RPM packages with Zypper on openSUSE Tumbleweed and Leap.',
    [
      { id: 'opensuse-zypper', text: 'openSUSE Zypper Installation', level: 2 },
      { id: 'obs-build', text: 'Open Build Service (OBS) Integration', level: 2 }
    ],
    `
<h2 id="opensuse-zypper">openSUSE Zypper Installation</h2>
<pre><code class="language-bash">sudo zypper install ./meridian-terminal.rpm</code></pre>

<h2 id="obs-build">Open Build Service (OBS) Integration</h2>
<p>Meridian maintains spec configurations compatible with openSUSE Tumbleweed and Leap repositories.</p>
`
  );

  addArt('pkg-appimage', 'AppImage', 'PACKAGING', 'implemented',
    'Standalone self-contained AppImage package for any Linux distribution.',
    [
      { id: 'appimage-guide', text: 'Standalone AppImage Execution', level: 2 },
      { id: 'building-appimage', text: 'Building AppImages with appimagetool', level: 2 }
    ],
    `
<h2 id="appimage-guide">Standalone AppImage Execution</h2>
<pre><code class="language-bash">chmod +x meridian-terminal.AppImage
./meridian-terminal.AppImage</code></pre>

<h2 id="building-appimage">Building AppImages with appimagetool</h2>
<pre><code class="language-bash">./scripts/build_appimage.sh</code></pre>
`
  );

  addArt('pkg-flatpak', 'Flatpak', 'PACKAGING', 'planned',
    'Flatpak container package roadmap and sandbox permissions.',
    [
      { id: 'flatpak-roadmap-spec', text: 'Flatpak Container Roadmap', level: 2 },
      { id: 'flatpak-permissions', text: 'Host PTY & Windowing Permissions', level: 2 }
    ],
    `
<h2 id="flatpak-roadmap-spec">Flatpak Container Roadmap</h2>
<p>Flatpak packaging with <code>org.freedesktop.Flatpak</code> Host PTY permissions is currently planned.</p>

<h2 id="flatpak-permissions">Host PTY & Windowing Permissions</h2>
<p>Terminal emulators require <code>--talk-name=org.freedesktop.Flatpak</code> to spawn host processes via <code>flatpak-spawn --host</code>.</p>
`
  );

  addArt('pkg-snap', 'Snap', 'PACKAGING', 'planned',
    'Snap packaging roadmap with classic confinement.',
    [
      { id: 'snap-roadmap-spec', text: 'Snap Package Roadmap', level: 2 },
      { id: 'classic-confinement', text: 'Classic Confinement Approval', level: 2 }
    ],
    `
<h2 id="snap-roadmap-spec">Snap Package Roadmap</h2>
<p>Snap packaging with classic confinement is planned for the Canonical Snap Store.</p>

<h2 id="classic-confinement">Classic Confinement Approval</h2>
<p>Terminal emulators require classic confinement to execute user shells and arbitrary binaries outside the snap sandbox.</p>
`
  );

  addArt('pkg-macos', 'macOS', 'PACKAGING', 'development',
    'macOS Darwin BSD PTY support, Homebrew formula, and DMG installer.',
    [
      { id: 'macos-status-spec', text: 'macOS Darwin Status', level: 2 },
      { id: 'homebrew-install', text: 'Homebrew Tap & Installation', level: 2 }
    ],
    `
<h2 id="macos-status-spec">macOS Darwin Status</h2>
<p>Darwin BSD PTY abstraction and Homebrew formula (<code>brew install meridian-terminal</code>) are in active development for Apple Silicon (M1-M4) and Intel x86_64.</p>

<h2 id="homebrew-install">Homebrew Tap & Installation</h2>
<pre><code class="language-bash">brew tap charanbalaji2005/meridian
brew install --cask meridian-terminal</code></pre>
`
  );

  addArt('pkg-windows', 'Windows', 'PACKAGING', 'development',
    'Windows 10/11 ConPTY pseudoterminal adapter and MSI installer.',
    [
      { id: 'windows-conpty-spec', text: 'Windows ConPTY Core', level: 2 },
      { id: 'winget-install', text: 'Windows Package Manager (winget)', level: 2 }
    ],
    `
<h2 id="windows-conpty-spec">Windows ConPTY Core</h2>
<p>Windows Console PTY (ConPTY) adapter and MSI / winget installer are in development.</p>

<h2 id="winget-install">Windows Package Manager (winget)</h2>
<pre><code class="language-powershell">winget install Meridian.Terminal</code></pre>
`
  );
};
