---
layout: default
title: "Security"
category: "DEVELOPER"
status: "implemented"
---

<h2 id="risk-tiers">Risk Classification Tiers (Low to Critical)</h2>
<p><code>ai::RiskClassifier</code> intercepts commands before execution, flagging destructive patterns (e.g. <code>rm -rf /</code>, <code>mkfs</code>, <code>dd if=/dev/zero</code>, fork bombs).</p>

<h2 id="redaction-engine">Secret & API Key Redactor</h2>
<p><code>ai::SecretRedactor</code> automatically strips Bearer tokens, AWS keys (<code>AKIA...</code>), and RSA/SSH private keys from logs and terminal scrollbacks.</p>

<h2 id="audit-logging">Security Audit Log & Whitelisting</h2>
<p>Flagged commands are recorded in <code>~/.local/share/meridian/security_audit.log</code> with user confirmation timestamps.</p>