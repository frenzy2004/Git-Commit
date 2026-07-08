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
          ${audioPanel()}
          ${outputPanel()}
        </main>
      </div>
    `;
  }

  function modeButton(mode, label, symbol, selected = false) {
    return `<button class="mode-choice${selected ? " selected" : ""}" type="button" data-mode="${mode}">${icon(symbol, 18)}<span>${label}</span></button>`;
  }

  function homePanel() {
    return `
      <section class="screen home-screen showing" data-panel="home">
        <header class="home-hero">
          <div class="home-title">${brailleMark("large")}<h1>Fingertips</h1></div>
          <p>Fingertips turns classroom images and audio into short, essential explanations for deaf-blind learners, then prepares the text for tactile playback on a future braille device.</p>
        </header>

        <div class="feature-grid">
          <article class="feature-card feature-lead">
            <div class="feature-copy">
              <h2>Accessible learning, reduced to the main idea</h2>
              <p>Upload a worksheet, slide, scene, or voice recording. Fingertips keeps only the information a student needs most, in simple words that are easier to read tactually.</p>
            </div>
            <div class="quick-actions">
              <button class="control accent" type="button" data-jump="image">Try images ${icon("arrow")}</button>
              <button class="control" type="button" data-jump="audio">Try audio</button>
            </div>
          </article>
          ${featureCard("image", "Image understanding", "Reads pages, diagrams, and scenes, then returns only the core meaning in one or two short lines.")}
          ${featureCard("audio", "Audio simplification", "Transcribes recordings and reduces long speech into a compact takeaway instead of a long paragraph.")}
          ${featureCard("braille", "Device-ready output", "Splits the final text into 4-character chunks so it can be sent directly to the tactile hardware flow.")}
        </div>
      </section>
    `;
  }

  function featureCard(symbol, heading, copy) {
    return `<article class="feature-card"><div class="feature-symbol">${icon(symbol, 18)}</div><h3>${heading}</h3><p>${copy}</p></article>`;
  }

  function imagePanel() {
    return `
      <section class="screen" data-panel="image">
        <div class="screen-head">
          <h2>Image</h2>
          <p>Capture or upload a textbook page, slide, worksheet, or diagram. Fingertips keeps only the main idea in very simple words.</p>
        </div>

        <div class="tool-card">
          <div class="camera-box">
            <video data-camera autoplay playsinline muted></video>
            <canvas data-canvas hidden></canvas>
            <img data-image-preview alt="Captured image" />
            <div class="empty-media" data-image-empty>${icon("camera", 40)}<span>No image selected</span></div>
          </div>
          <div class="button-row">
            <button class="control" type="button" data-start-camera>${icon("video")}<span data-camera-label>Start camera</span></button>
            <button class="control" type="button" data-capture disabled>${icon("capture")}Capture</button>
            <label class="control">${icon("upload")}Upload<input type="file" data-image-file accept="image/*" /></label>
            <button class="control" type="button" data-clear-image disabled>Clear</button>
            <button class="control accent push-end" type="button" data-send-image disabled>Summarize ${icon("arrow")}</button>
          </div>
        </div>
      </section>
    `;
  }

  function audioPanel() {
    return `
      <section class="screen" data-panel="audio">
        <div class="screen-head">
          <h2>Audio</h2>
          <p>Record live or upload a recording. Fingertips transcribes the audio and keeps only the key takeaway.</p>
        </div>

        <div class="tool-card">
          <div class="sound-box">
            <div class="bars" data-wave aria-hidden="true">${"<i></i>".repeat(10)}</div>
            <p data-audio-label>Press record or upload an audio file</p>
          </div>
          <div class="button-row">
            <button class="control record" type="button" data-record><svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><circle cx="12" cy="12" r="6"/></svg>Record</button>
            <button class="control" type="button" data-stop disabled><svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><rect x="6" y="6" width="12" height="12" rx="1"/></svg>Stop</button>
            <label class="control">${icon("upload")}Upload<input type="file" data-audio-file accept="audio/*" /></label>
            <button class="control" type="button" data-clear-audio disabled>Clear</button>
            <button class="control accent push-end" type="button" data-send-audio disabled>Summarize ${icon("arrow")}</button>
          </div>
        </div>
      </section>
    `;
  }

  function outputPanel() {
    return `
      <div class="output-zone" aria-live="polite">
        <div class="wait-card is-hidden" data-loading><span class="spinner"></span><p>Processing…</p></div>
        <div class="answer-card is-hidden" data-result>
          <div class="answer-head">
            <div><h3>Essential Summary</h3><p data-result-meta></p></div>
            <button class="copy-action" type="button" data-copy>${icon("copy")}<span>Copy</span></button>
          </div>
          <div class="summary-copy" data-summary></div>
          <ol class="sentence-list is-hidden" data-sentences></ol>
        </div>
        <div class="problem-card is-hidden" data-error>${icon("alert", 16)}<span data-error-copy></span></div>
      </div>
    `;
  }

  renderShell();

  const dom = {
    apiPill: root.querySelector("[data-api-pill]"),
    apiLabel: root.querySelector("[data-api-label]"),
    apiCopy: root.querySelector("[data-api-copy]"),
    loading: root.querySelector("[data-loading]"),
    result: root.querySelector("[data-result]"),
    resultMeta: root.querySelector("[data-result-meta]"),
    summary: root.querySelector("[data-summary]"),
    sentences: root.querySelector("[data-sentences]"),
    error: root.querySelector("[data-error]"),
    errorCopy: root.querySelector("[data-error-copy]"),
    copyButton: root.querySelector("[data-copy]"),
    camera: root.querySelector("[data-camera]"),
    canvas: root.querySelector("[data-canvas]"),
    imagePreview: root.querySelector("[data-image-preview]"),
    imageEmpty: root.querySelector("[data-image-empty]"),
    cameraLabel: root.querySelector("[data-camera-label]"),
    capture: root.querySelector("[data-capture]"),
    imageFile: root.querySelector("[data-image-file]"),
    clearImage: root.querySelector("[data-clear-image]"),
    sendImage: root.querySelector("[data-send-image]"),
    wave: root.querySelector("[data-wave]"),
    audioLabel: root.querySelector("[data-audio-label]"),
