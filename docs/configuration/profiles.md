---
layout: default
title: "Profiles"
category: "CONFIGURATION"
status: "implemented"
---

<h2 id="profile-management">Profile Configurations</h2>
<p>Define custom shell startup profiles in <code>~/.config/meridian/profiles.json</code> for Bash, Zsh, Fish, or direct SSH sessions.</p>

<h2 id="profiles-json-spec">profiles.json Example</h2>
<pre><code class="language-json">{
  "default_profile": "zsh",
  "profiles": [
    {
      "name": "zsh",
      "command": "/bin/zsh",
      "args": ["-l"],
      "env": { "SHELL": "/bin/zsh" }
    },
    {
      "name": "docker-dev",
      "command": "/usr/bin/docker",
      "args": ["exec", "-it", "dev-container", "/bin/bash"],
      "theme": "cyberpunk_edge"
    }
  ]
}</code></pre>