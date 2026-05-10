#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace {

volatile std::sig_atomic_t keepRunning = 1;

void handleSignal(int) {
  keepRunning = 0;
}

std::string page() {
  return R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>SCP Foundation | Anomaly Console</title>
  <style>
    :root {
      --bg: #050607;
      --panel: rgba(12, 15, 18, 0.82);
      --panel-strong: rgba(22, 26, 31, 0.92);
      --line: rgba(255, 255, 255, 0.12);
      --red: #ff334e;
      --amber: #ffba3a;
      --green: #48ff9b;
      --cyan: #67e8f9;
      --text: #f4f7fb;
      --muted: #9aa8b7;
      --shadow: 0 30px 90px rgba(0, 0, 0, 0.55);
    }

    * { box-sizing: border-box; }

    html { scroll-behavior: smooth; }

    body {
      margin: 0;
      min-height: 100vh;
      color: var(--text);
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background:
        radial-gradient(circle at 12% 18%, rgba(255, 51, 78, 0.18), transparent 28rem),
        radial-gradient(circle at 88% 8%, rgba(103, 232, 249, 0.13), transparent 30rem),
        linear-gradient(135deg, #030405 0%, #0b1116 46%, #090708 100%);
      overflow-x: hidden;
    }

    body::before {
      content: "";
      position: fixed;
      inset: 0;
      pointer-events: none;
      background-image:
        linear-gradient(rgba(255,255,255,0.035) 1px, transparent 1px),
        linear-gradient(90deg, rgba(255,255,255,0.035) 1px, transparent 1px);
      background-size: 42px 42px;
      mask-image: radial-gradient(circle at center, black, transparent 78%);
      z-index: -2;
    }

    body::after {
      content: "";
      position: fixed;
      inset: 0;
      pointer-events: none;
      background: repeating-linear-gradient(0deg, rgba(255,255,255,0.025) 0 1px, transparent 1px 5px);
      mix-blend-mode: screen;
      opacity: 0.35;
      z-index: 20;
    }

    a { color: inherit; text-decoration: none; }

    .shell {
      width: min(1180px, calc(100% - 32px));
      margin: 0 auto;
    }

    .nav {
      position: sticky;
      top: 0;
      z-index: 10;
      backdrop-filter: blur(18px);
      background: rgba(5, 6, 7, 0.7);
      border-bottom: 1px solid var(--line);
    }

    .nav .shell {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 16px 0;
      gap: 18px;
    }

    .brand {
      display: flex;
      align-items: center;
      gap: 12px;
      font-weight: 900;
      letter-spacing: 0.16em;
      text-transform: uppercase;
    }

    .mark {
      width: 42px;
      aspect-ratio: 1;
      display: grid;
      place-items: center;
      border-radius: 50%;
      background: conic-gradient(from 40deg, var(--red), #15191f, var(--cyan), #15191f, var(--red));
      box-shadow: 0 0 32px rgba(255, 51, 78, 0.34);
      position: relative;
    }

    .mark::before {
      content: "";
      width: 24px;
      aspect-ratio: 1;
      border: 3px solid #050607;
      border-radius: 50%;
      background: rgba(244, 247, 251, 0.92);
    }

    .navlinks { display: flex; gap: 14px; flex-wrap: wrap; }

    .navlinks a {
      color: var(--muted);
      font-size: 0.78rem;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      border: 1px solid transparent;
      padding: 9px 12px;
      border-radius: 999px;
    }

    .navlinks a:hover { border-color: var(--line); color: var(--text); background: rgba(255,255,255,0.05); }

    .hero {
      min-height: 88vh;
      display: grid;
      grid-template-columns: 1.05fr 0.95fr;
      gap: 34px;
      align-items: center;
      padding: 72px 0 42px;
    }

    .eyebrow {
      color: var(--green);
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      letter-spacing: 0.18em;
      text-transform: uppercase;
      font-size: 0.78rem;
    }

    h1 {
      margin: 14px 0 18px;
      max-width: 760px;
      font-size: clamp(3rem, 8vw, 7.3rem);
      line-height: 0.84;
      letter-spacing: -0.08em;
      text-transform: uppercase;
    }

    .glitch {
      position: relative;
      text-shadow: -2px 0 rgba(255, 51, 78, 0.85), 2px 0 rgba(103, 232, 249, 0.72);
    }

    .lead {
      max-width: 690px;
      color: #c9d3df;
      font-size: clamp(1rem, 2vw, 1.22rem);
      line-height: 1.7;
    }

    .actions { display: flex; flex-wrap: wrap; gap: 14px; margin-top: 28px; }

    .button {
      padding: 13px 18px;
      border-radius: 14px;
      border: 1px solid var(--line);
      background: rgba(255,255,255,0.07);
      font-weight: 800;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      font-size: 0.82rem;
      transition: transform 0.2s ease, border-color 0.2s ease, background 0.2s ease;
    }

    .button:hover { transform: translateY(-2px); border-color: rgba(255,255,255,0.32); background: rgba(255,255,255,0.11); }
    .button.primary { background: linear-gradient(135deg, var(--red), #8c1324); border-color: rgba(255, 51, 78, 0.65); }

    .containment-vault {
      min-height: 540px;
      border: 1px solid var(--line);
      border-radius: 34px;
      background: linear-gradient(180deg, rgba(255,255,255,0.08), rgba(255,255,255,0.025));
      box-shadow: var(--shadow), inset 0 0 70px rgba(103, 232, 249, 0.05);
      position: relative;
      overflow: hidden;
      isolation: isolate;
    }

    .containment-vault::before {
      content: "";
      position: absolute;
      inset: -40%;
      background: conic-gradient(from 0deg, transparent, rgba(255, 51, 78, 0.16), transparent, rgba(103, 232, 249, 0.13), transparent);
      animation: spin 18s linear infinite;
      z-index: -2;
    }

    .containment-vault::after {
      content: "";
      position: absolute;
      inset: 1px;
      border-radius: 33px;
      background:
        linear-gradient(180deg, rgba(5,6,7,0.72), rgba(5,6,7,0.93)),
        repeating-linear-gradient(90deg, transparent 0 19px, rgba(255,255,255,0.04) 19px 20px);
      z-index: -1;
    }

    @keyframes spin { to { transform: rotate(360deg); } }

    .vault-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 22px;
      border-bottom: 1px solid var(--line);
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      color: var(--muted);
    }

    .status { color: var(--green); }

    .glyph-stage {
      min-height: 360px;
      display: grid;
      place-items: center;
      padding: 28px;
      position: relative;
    }

    .glyph {
      width: min(310px, 70vw);
      aspect-ratio: 1;
      border-radius: 50%;
      border: 2px dashed rgba(255,255,255,0.25);
      display: grid;
      place-items: center;
      position: relative;
      filter: drop-shadow(0 0 30px rgba(255, 51, 78, 0.25));
      animation: pulse 4s ease-in-out infinite;
    }

    .glyph::before, .glyph::after {
      content: "";
      position: absolute;
      inset: 18%;
      border: 10px solid rgba(244,247,251,0.92);
      border-left-color: transparent;
      border-right-color: transparent;
      transform: rotate(45deg);
      border-radius: 18px;
    }

    .glyph::after {
      inset: 32%;
      border-width: 5px;
      transform: rotate(-45deg);
      opacity: 0.8;
    }

    @keyframes pulse { 50% { transform: scale(1.025); opacity: 0.78; } }

    .blink-warning {
      position: absolute;
      left: 22px;
      right: 22px;
      bottom: 22px;
      border: 1px solid rgba(255, 186, 58, 0.45);
      border-radius: 18px;
      background: rgba(255, 186, 58, 0.08);
      padding: 16px;
      color: #ffe1a1;
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      line-height: 1.55;
    }

    section { padding: 68px 0; }

    .section-title {
      display: flex;
      justify-content: space-between;
      align-items: end;
      gap: 20px;
      margin-bottom: 24px;
    }

    h2 {
      margin: 0;
      font-size: clamp(1.8rem, 4vw, 3.4rem);
      letter-spacing: -0.06em;
      text-transform: uppercase;
    }

    .section-title p { max-width: 520px; color: var(--muted); line-height: 1.6; margin: 0; }

    .grid {
      display: grid;
      grid-template-columns: repeat(5, minmax(0, 1fr));
      gap: 16px;
    }

    .card {
      min-height: 310px;
      border: 1px solid var(--line);
      border-radius: 26px;
      background: var(--panel);
      padding: 20px;
      position: relative;
      overflow: hidden;
      box-shadow: 0 18px 50px rgba(0,0,0,0.28);
      transition: transform 0.25s ease, border-color 0.25s ease;
    }

    .card:hover { transform: translateY(-8px); border-color: rgba(255,255,255,0.28); }

    .card::before {
      content: attr(data-id);
      position: absolute;
      right: -10px;
      bottom: -18px;
      color: rgba(255,255,255,0.045);
      font-size: 5rem;
      font-weight: 950;
      letter-spacing: -0.08em;
    }

    .object-class {
      display: inline-flex;
      gap: 8px;
      align-items: center;
      color: #050607;
      background: var(--amber);
      border-radius: 999px;
      padding: 6px 10px;
      font-size: 0.72rem;
      font-weight: 900;
      letter-spacing: 0.1em;
      text-transform: uppercase;
    }

    .safe { background: var(--green); }
    .keter { background: var(--red); color: white; }
    .thaumiel { background: var(--cyan); }

    .card h3 { margin: 24px 0 10px; font-size: 1.4rem; }
    .card p { color: #c5cfdb; line-height: 1.6; margin: 0; }

    .protocol {
      display: grid;
      grid-template-columns: 0.8fr 1.2fr;
      gap: 20px;
      align-items: stretch;
    }

    .terminal, .dossier {
      border: 1px solid var(--line);
      border-radius: 28px;
      background: var(--panel-strong);
      box-shadow: var(--shadow);
      overflow: hidden;
    }

    .terminal-bar {
      display: flex;
      gap: 8px;
      padding: 14px 16px;
      border-bottom: 1px solid var(--line);
      background: rgba(255,255,255,0.04);
    }

    .dot { width: 11px; aspect-ratio: 1; border-radius: 50%; background: var(--red); }
    .dot:nth-child(2) { background: var(--amber); }
    .dot:nth-child(3) { background: var(--green); }

    .terminal pre {
      margin: 0;
      padding: 22px;
      white-space: pre-wrap;
      color: #b8ffcf;
      line-height: 1.75;
      font-size: 0.92rem;
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    }

    .dossier { padding: 26px; }
    .dossier ul { margin: 18px 0 0; padding: 0; list-style: none; display: grid; gap: 12px; }
    .dossier li { border: 1px solid var(--line); border-radius: 16px; padding: 14px; color: #d6dee8; background: rgba(255,255,255,0.035); }

    .footer {
      padding: 40px 0 56px;
      color: var(--muted);
      border-top: 1px solid var(--line);
      font-size: 0.92rem;
      line-height: 1.6;
    }

    @media (max-width: 1050px) {
      .hero, .protocol { grid-template-columns: 1fr; }
      .grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
    }

    @media (max-width: 680px) {
      .nav .shell, .section-title { align-items: flex-start; flex-direction: column; }
      .grid { grid-template-columns: 1fr; }
      .containment-vault { min-height: 470px; }
    }
  </style>
</head>
<body>
  <nav class="nav">
    <div class="shell">
      <a class="brand" href="#top" aria-label="SCP Foundation anomaly console home">
        <span class="mark" aria-hidden="true"></span>
        <span>SCP Foundation</span>
      </a>
      <div class="navlinks" aria-label="Primary navigation">
        <a href="#archive">Archive</a>
        <a href="#protocol">Protocol</a>
        <a href="#briefing">Briefing</a>
      </div>
    </div>
  </nav>

  <main id="top" class="shell">
    <header class="hero">
      <div>
        <div class="eyebrow">C++ live dossier server • clearance level 3</div>
        <h1 class="glitch">Secure. Contain. Protect.</h1>
        <p class="lead">
          A fan-made anomaly interface rendered by a tiny C++ web server: redacted case files,
          animated containment telemetry, and a cinematic control-room UI for exploring a few iconic SCP subjects.
        </p>
        <div class="actions">
          <a class="button primary" href="#archive">Open anomaly archive</a>
          <a class="button" href="#protocol">Review containment loop</a>
        </div>
      </div>

      <aside class="containment-vault" aria-label="Animated containment vault illustration">
        <div class="vault-head">
          <span>VAULT-██ / OBSERVATION FEED</span>
          <span class="status">ONLINE</span>
        </div>
        <div class="glyph-stage">
          <div class="glyph" aria-hidden="true"></div>
          <div class="blink-warning">
            ALERT: Visual continuity required. Do not blink without verbal confirmation from the observation team.
          </div>
        </div>
      </aside>
    </header>

    <section id="archive" aria-labelledby="archive-title">
      <div class="section-title">
        <h2 id="archive-title">Anomaly archive</h2>
        <p>Short, original summaries for several SCP-inspired entries. Choose a file, memorize the risk, and keep the chamber sealed.</p>
      </div>

      <div class="grid">
        <article class="card" data-id="173">
          <span class="object-class">Euclid</span>
          <h3>SCP-173</h3>
          <p>A statue-like entity that is inert under direct observation and dangerously mobile whenever sight is broken.</p>
        </article>
        <article class="card" data-id="049">
          <span class="object-class">Euclid</span>
          <h3>SCP-049</h3>
          <p>A plague-doctor figure obsessed with curing an undefined illness through fatal and anomalous procedures.</p>
        </article>
        <article class="card" data-id="096">
          <span class="object-class keter">Keter</span>
          <h3>SCP-096</h3>
          <p>A shy humanoid that becomes unstoppable after its face is viewed, even through recordings or photographs.</p>
        </article>
        <article class="card" data-id="999">
          <span class="object-class safe">Safe</span>
          <h3>SCP-999</h3>
          <p>A friendly gelatinous creature with calming, playful behavior and a talent for improving morale.</p>
        </article>
        <article class="card" data-id="682">
          <span class="object-class keter">Keter</span>
          <h3>SCP-682</h3>
          <p>A hostile reptilian organism with extreme adaptability, resilience, and a long record of failed neutralization attempts.</p>
        </article>
      </div>
    </section>

    <section id="protocol" class="protocol" aria-labelledby="protocol-title">
      <div class="terminal">
        <div class="terminal-bar"><span class="dot"></span><span class="dot"></span><span class="dot"></span></div>
        <pre>> boot containment_console.cpp
> handshake: accepted
> memetic_filter: armed
> chamber_lock: sealed
> personnel_count: 03
> blink_sequence: call / confirm / blink
> status: STABLE_FOR_NOW</pre>
      </div>
      <div class="dossier">
        <div class="eyebrow">Live containment loop</div>
        <h2 id="protocol-title">Procedure stack</h2>
        <ul>
          <li><strong>Observe:</strong> keep camera feeds, sight lines, and partner confirmations active.</li>
          <li><strong>Isolate:</strong> lock affected areas and route non-essential personnel away from breach zones.</li>
          <li><strong>Document:</strong> log anomalies with timestamps, object class, and exposure conditions.</li>
          <li><strong>Recover:</strong> re-secure the chamber before analysis, cleanup, or interviews begin.</li>
        </ul>
      </div>
    </section>

    <section id="briefing" aria-labelledby="briefing-title">
      <div class="section-title">
        <h2 id="briefing-title">Design briefing</h2>
        <p>This page uses a bespoke control-panel layout, animated CSS containment glyphs, responsive dossier cards, and a C++ socket server instead of a JavaScript framework.</p>
      </div>
    </section>
  </main>

  <footer class="footer">
    <div class="shell">
      Fan project inspired by the SCP Foundation collaborative fiction universe. This site contains original summaries and does not reproduce official article text.
    </div>
  </footer>
</body>
</html>)HTML";
}

std::string response(const std::string &body) {
  std::ostringstream out;
  out << "HTTP/1.1 200 OK\r\n"
      << "Content-Type: text/html; charset=UTF-8\r\n"
      << "Cache-Control: no-store\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n"
      << body;
  return out.str();
}

} // namespace

int main(int argc, char **argv) {
  std::signal(SIGINT, handleSignal);
  std::signal(SIGTERM, handleSignal);

  int port = 8080;
  if (argc > 1) {
    port = std::stoi(argv[1]);
  }

  int serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    std::cerr << "Failed to create socket: " << std::strerror(errno) << '\n';
    return 1;
  }

  int option = 1;
  setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(static_cast<uint16_t>(port));

  if (bind(serverFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
    std::cerr << "Failed to bind port " << port << ": " << std::strerror(errno) << '\n';
    close(serverFd);
    return 1;
  }

  if (listen(serverFd, 12) < 0) {
    std::cerr << "Failed to listen: " << std::strerror(errno) << '\n';
    close(serverFd);
    return 1;
  }

  std::cout << "SCP Foundation console running at http://localhost:" << port << "\n";
  const std::string html = page();
  const std::string httpResponse = response(html);

  while (keepRunning) {
    sockaddr_in clientAddress{};
    socklen_t clientLength = sizeof(clientAddress);
    int clientFd = accept(serverFd, reinterpret_cast<sockaddr *>(&clientAddress), &clientLength);
    if (clientFd < 0) {
      if (keepRunning) {
        std::cerr << "Accept failed: " << std::strerror(errno) << '\n';
      }
      continue;
    }

    char buffer[2048];
    recv(clientFd, buffer, sizeof(buffer), 0);
    send(clientFd, httpResponse.c_str(), httpResponse.size(), 0);
    close(clientFd);
  }

  close(serverFd);
  return 0;
}
