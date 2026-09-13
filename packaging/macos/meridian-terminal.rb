class MeridianTerminal < Formula
  desc "Modern Terminal & Unified Developer Environment"
  homepage "https://github.com/charanbalaji2005/Meridian-Shell"
  url "https://github.com/charanbalaji2005/Meridian-Shell/archive/refs/tags/v2.5.0.tar.gz"
  license "GPL-3.0-or-later"

  depends_on "cmake" => :build

  def install
    system "make", "all"
    bin.install "build/meridian-shell"
    bin.install "build/meridian"
  end

  test do
    system "#{bin}/meridian-shell", "-c", "echo 'Meridian Terminal Live'"
  end
end
