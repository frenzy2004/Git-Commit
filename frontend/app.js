(() => {
  const root = document.getElementById("fingertips-root");
  const query = new URLSearchParams(window.location.search);
  const apiFromQuery = query.get("api");
  const apiFromStorage = window.localStorage.getItem("fingertips.api");
  const pageHostApi = window.location.protocol.startsWith("http") && window.location.hostname
    ? `${window.location.protocol}//${window.location.hostname}:8000`
    : "http://localhost:8000";
  const apiBase = (apiFromQuery || apiFromStorage || pageHostApi).replace(/\/$/, "");

  if (apiFromQuery) {
    window.localStorage.setItem("fingertips.api", apiBase);
  }

  const state = {
    apiHealth: null,
    cameraStream: null,
    imageObjectUrl: null,
    imageBlob: null,
    recorder: null,
    audioBlob: null,
    audioName: "recording.webm",
    audioParts: [],
  };

  const iconPaths = {
    home: ['<path d="M3 10.5 12 3l9 7.5"/>', '<path d="M5 9.5V21h14V9.5"/>'],
    image: ['<rect x="3" y="3" width="18" height="18" rx="2"/>', '<circle cx="9" cy="9" r="1.5"/>', '<path d="M21 15l-5-5L5 21"/>'],
    audio: ['<rect x="9" y="2" width="6" height="12" rx="3"/>', '<path d="M19 10v2a7 7 0 0 1-14 0v-2"/>', '<line x1="12" y1="19" x2="12" y2="22"/>'],
    arrow: ['<line x1="5" y1="12" x2="19" y2="12"/>', '<polyline points="12 5 19 12 12 19"/>'],
    video: ['<polygon points="23 7 16 12 23 17 23 7"/>', '<rect x="1" y="5" width="15" height="14" rx="2"/>'],
    capture: ['<circle cx="12" cy="12" r="10"/>', '<circle cx="12" cy="12" r="3"/>'],
    upload: ['<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>', '<polyline points="17 8 12 3 7 8"/>', '<line x1="12" y1="3" x2="12" y2="15"/>'],
    camera: ['<path d="M23 19a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h4l2-3h6l2 3h4a2 2 0 0 1 2 2z"/>', '<circle cx="12" cy="13" r="4"/>'],
    braille: ['<circle cx="6" cy="6" r="1.6"/>', '<circle cx="6" cy="12" r="1.6"/>', '<circle cx="6" cy="18" r="1.6"/>', '<circle cx="14" cy="6" r="1.6"/>', '<circle cx="14" cy="12" r="1.6"/>', '<circle cx="14" cy="18" r="1.6"/>', '<path d="M19 4v16"/>'],
    copy: ['<rect x="9" y="9" width="13" height="13" rx="2"/>', '<path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>'],
    alert: ['<circle cx="12" cy="12" r="10"/>', '<line x1="12" y1="8" x2="12" y2="12"/>', '<line x1="12" y1="16" x2="12.01" y2="16"/>'],
  };

  function icon(name, size = 14, fill = "none") {
    return `<svg width="${size}" height="${size}" viewBox="0 0 24 24" fill="${fill}" stroke="currentColor" stroke-width="${fill === "none" ? "2" : "0"}" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">${iconPaths[name].join("")}</svg>`;
  }

  function brailleMark(extraClass = "") {
    return `<span class="brand-dots ${extraClass}" aria-hidden="true"><i></i><i></i><i></i><i></i><i></i><i></i></span>`;
  }

  function renderShell() {
    root.innerHTML = `
      <div class="layout">
        <aside class="rail">
          <div class="logo-row">${brailleMark()}<span class="logo-word">Fingertips</span></div>
          <nav class="mode-list" aria-label="Input mode">
            ${modeButton("home", "Home", "home", true)}
            ${modeButton("image", "Image", "image")}
            ${modeButton("audio", "Audio", "audio")}
          </nav>
          <div class="rail-status">
            <div class="api-pill" data-api-pill>
              <span class="api-dot"></span>
              <span data-api-label>Checking…</span>
            </div>
            <p data-api-copy>Auto-detecting backend.</p>
          </div>
        </aside>

        <main class="stage">
          ${homePanel()}
          ${imagePanel()}
