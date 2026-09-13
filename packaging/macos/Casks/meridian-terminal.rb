cask "meridian-terminal" do
  version "2.5.0"
  sha256 :no_check

  url "https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v#{version}/Meridian-Terminal-#{version}-macOS-arm64.dmg",
      verified: "github.com/charanbalaji2005/Meridian-Shell/"
  name "Meridian Terminal"
  desc "GPU-accelerated terminal emulator, unified developer environment, and AI platform"
  homepage "https://github.com/charanbalaji2005/Meridian-Shell"

  livecheck do
    url :url
    strategy :github_latest
  end

  auto_updates true
  depends_on macos: ">= :big_sur"

  app "Meridian.app"
  binary "#{appdir}/Meridian.app/Contents/MacOS/meridian", target: "meridian"
  binary "#{appdir}/Meridian.app/Contents/MacOS/meridian-shell", target: "meridian-shell"

  zap trash: [
    "~/.config/meridian",
    "~/Library/Application Support/Meridian",
    "~/Library/Preferences/org.meridian-terminal.MeridianTerminal.plist",
    "~/Library/Saved Application State/org.meridian-terminal.MeridianTerminal.savedState",
  ]
end

