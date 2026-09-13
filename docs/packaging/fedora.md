---
layout: default
title: "Fedora / RPM"
category: "PACKAGING"
status: "implemented"
---

<h2 id="fedora-rpm">Fedora / RHEL RPM Package</h2>
<pre><code class="language-bash"># Install locally built RPM
sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<h2 id="spec-file">RPM Spec File & Compilation</h2>
<p>The spec file is located in <code>packaging/rpm/meridian-terminal.spec</code>. To build locally:</p>
<pre><code class="language-bash">rpmbuild -ba packaging/rpm/meridian-terminal.spec</code></pre>

<h2 id="copr-repo">Fedora COPR Repository Publishing</h2>
<p>Meridian packages are automatically built for Fedora 39, 40, 41, and Rawhide via automated COPR webhooks on release tags.</p>