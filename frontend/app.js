(() => {
  const root = document.getElementById("fingertips-root");
  const query = new URLSearchParams(window.location.search);
  const apiFromQuery = query.get("api");
  const apiFromStorage = window.localStorage.getItem("fingertips.api");
  const localHosts = new Set(["localhost", "127.0.0.1", "::1"]);
  const pageHostApi = window.location.protocol.startsWith("http") && localHosts.has(window.location.hostname)
    ? `${window.location.protocol}//${window.location.hostname}:8000`
    : "http://localhost:8000";
  const apiBase = (apiFromQuery || apiFromStorage || pageHostApi).replace(/\/$/, "");
  const demoImagePath = "assets/demo-fig.png";

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
    tactile: ['<circle cx="6" cy="6" r="1.4"/>', '<circle cx="6" cy="12" r="1.4"/>', '<circle cx="6" cy="18" r="1.4"/>', '<circle cx="14" cy="6" r="1.4"/>', '<circle cx="14" cy="12" r="1.4"/>', '<circle cx="14" cy="18" r="1.4"/>', '<path d="M19 4v16"/>'],
    text: ['<path d="M4 7h16"/>', '<path d="M4 12h13"/>', '<path d="M4 17h9"/>'],
    copy: ['<rect x="9" y="9" width="13" height="13" rx="2"/>', '<path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>'],
    alert: ['<circle cx="12" cy="12" r="10"/>', '<line x1="12" y1="8" x2="12" y2="12"/>', '<line x1="12" y1="16" x2="12.01" y2="16"/>'],
  };

  function icon(name, size = 16, fill = "none") {
    return `<svg width="${size}" height="${size}" viewBox="0 0 24 24" fill="${fill}" stroke="currentColor" stroke-width="${fill === "none" ? "2" : "0"}" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">${iconPaths[name].join("")}</svg>`;
  }

  function brandMark(size = "small") {
    return `<img class="brand-mark ${size}" src="assets/fingertips-logo.png" alt="" aria-hidden="true" />`;
  }

  function renderShell() {
    root.innerHTML = `
      <div class="app-shell">
        <aside class="sidebar">
          <div class="brand-lockup">${brandMark()}<span>Fingertips</span></div>
          <nav class="nav-list" aria-label="Input mode">
            ${modeButton("home", "Home", "home", true)}
            ${modeButton("image", "Image", "image")}
            ${modeButton("audio", "Audio", "audio")}
          </nav>
          <div class="status-card">
            <div class="api-state" data-api-pill>
              <span class="api-light"></span>
              <span data-api-label>Checking...</span>
            </div>
            <p data-api-copy>Looking for the local API.</p>
          </div>
        </aside>

        <main class="workspace">
          ${homePanel()}
          ${imagePanel()}
          ${audioPanel()}
          ${outputPanel()}
        </main>
      </div>
    `;
  }

  function modeButton(mode, label, symbol, selected = false) {
    return `<button class="nav-button${selected ? " active" : ""}" type="button" data-mode="${mode}">${icon(symbol, 18)}<span>${label}</span></button>`;
  }

  function homePanel() {
    return `
      <section class="view home-view active" data-panel="home">
        <header class="hero">
          <div class="hero-copy">
            <div class="hero-brand">${brandMark("large")}<span>Fingertips</span></div>
            <h1>Turn lessons into tactile-ready notes.</h1>
            <p>Upload a classroom image or recording and get a short, plain-language summary with four-character chunks ready for the tactile device flow.</p>
            <div class="hero-actions">
              <button class="primary-action" type="button" data-jump="image" data-demo-image>Start with image ${icon("arrow", 15)}</button>
              <button class="secondary-action" type="button" data-jump="audio">Start with audio</button>
            </div>
          </div>
          <div class="process-panel" aria-label="Processing flow">
            <div class="process-step"><span>01</span><strong>Capture</strong><p>Upload a page, scene, or classroom recording.</p></div>
            <div class="process-step"><span>02</span><strong>Condense</strong><p>Keep the main learning point in simple wording.</p></div>
            <div class="process-step"><span>03</span><strong>Output</strong><p>Preview normalized chunks for the hardware display.</p></div>
          </div>
        </header>

        <section class="home-grid" aria-label="Core workflows">
          ${workflowCard("image", "Image input", "Worksheets, slides, boards, diagrams, and scenes.", "Try images", "image")}
          ${workflowCard("audio", "Audio input", "Recorded explanations become compact learner notes.", "Try audio", "audio")}
          ${featureCard("tactile", "Device-ready chunks", "Preview normalized four-character output for serial transport.")}
        </section>
      </section>
    `;
  }

  function workflowCard(symbol, heading, copy, action, jump) {
    const demoAttribute = jump === "image" ? " data-demo-image" : "";
    return `
      <article class="workflow-card">
        <div class="card-icon">${icon(symbol, 18)}</div>
        <h2>${heading}</h2>
        <p>${copy}</p>
        <button type="button" data-jump="${jump}"${demoAttribute}>${action} ${icon("arrow", 14)}</button>
      </article>
    `;
  }

  function featureCard(symbol, heading, copy) {
    return `<article class="workflow-card quiet"><div class="card-icon">${icon(symbol, 18)}</div><h2>${heading}</h2><p>${copy}</p><span class="card-note">text4 serial preview</span></article>`;
  }

  function imagePanel() {
    return `
      <section class="view tool-view" data-panel="image">
        <header class="tool-header">
          <div>
            <span class="eyebrow">Image workflow</span>
            <h1>Capture the page. Keep the point.</h1>
            <p>Use a camera or upload a file, then generate a short learner-facing note.</p>
          </div>
        </header>
        <div class="tool-layout">
          <div class="media-stage">
            <video data-camera autoplay playsinline muted></video>
            <canvas data-canvas hidden></canvas>
            <img data-image-preview alt="Captured image" />
            <div class="empty-state" data-image-empty>${icon("image", 38)}<span>No image selected</span></div>
          </div>
          <div class="tool-actions">
            <button class="secondary-action" type="button" data-start-camera>${icon("video")}<span data-camera-label>Start camera</span></button>
            <button class="secondary-action" type="button" data-capture disabled>${icon("capture")}Capture</button>
            <label class="secondary-action">${icon("upload")}Upload<input type="file" data-image-file accept="image/*" /></label>
            <button class="secondary-action" type="button" data-clear-image disabled>Clear</button>
            <button class="primary-action stretch" type="button" data-send-image disabled>Summarize ${icon("arrow", 15)}</button>
          </div>
        </div>
      </section>
    `;
  }

  function audioPanel() {
    return `
      <section class="view tool-view" data-panel="audio">
        <header class="tool-header">
          <div>
            <span class="eyebrow">Audio workflow</span>
            <h1>Record the explanation. Extract the takeaway.</h1>
            <p>Record live audio or upload a file, then reduce it to the essential point.</p>
          </div>
        </header>
        <div class="tool-layout">
          <div class="audio-stage">
            <div class="waveform" data-wave aria-hidden="true">${"<i></i>".repeat(12)}</div>
            <p data-audio-label>Press record or upload an audio file</p>
          </div>
          <div class="tool-actions">
            <button class="record-action" type="button" data-record><svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><circle cx="12" cy="12" r="6"/></svg>Record</button>
            <button class="secondary-action" type="button" data-stop disabled><svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><rect x="6" y="6" width="12" height="12" rx="1"/></svg>Stop</button>
            <label class="secondary-action">${icon("upload")}Upload<input type="file" data-audio-file accept="audio/*" /></label>
            <button class="secondary-action" type="button" data-clear-audio disabled>Clear</button>
            <button class="primary-action stretch" type="button" data-send-audio disabled>Summarize ${icon("arrow", 15)}</button>
          </div>
        </div>
      </section>
    `;
  }

  function outputPanel() {
    return `
      <div class="result-zone" aria-live="polite">
        <div class="loading-card hidden" data-loading><span class="spinner"></span><p>Processing...</p></div>
        <div class="result-card hidden" data-result>
          <div class="result-header">
            <div><span>Essential summary</span><p data-result-meta></p></div>
            <button class="copy-button" type="button" data-copy>${icon("copy", 14)}<span>Copy</span></button>
          </div>
          <div class="summary-text" data-summary></div>
          <ol class="sentence-stack hidden" data-sentences></ol>
        </div>
        <div class="error-card hidden" data-error>${icon("alert", 16)}<span data-error-copy></span></div>
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
    record: root.querySelector("[data-record]"),
    stop: root.querySelector("[data-stop]"),
    audioFile: root.querySelector("[data-audio-file]"),
    clearAudio: root.querySelector("[data-clear-audio]"),
    sendAudio: root.querySelector("[data-send-audio]"),
  };

  function setMode(mode) {
    root.querySelectorAll("[data-mode]").forEach(button => {
      button.classList.toggle("active", button.dataset.mode === mode);
    });
    root.querySelectorAll("[data-panel]").forEach(panel => {
      panel.classList.toggle("active", panel.dataset.panel === mode);
    });
    if (mode !== "image") stopCamera();
    hideOutput();
  }

  function setApiBadge(kind, label, message) {
    dom.apiPill.className = `api-state ${kind}`;
    dom.apiLabel.textContent = label;
    dom.apiCopy.textContent = message;
  }

  async function refreshHealth() {
    const controller = new AbortController();
    const timeout = window.setTimeout(() => controller.abort(), 2500);
    try {
      const response = await fetch(`${apiBase}/health`, {
        headers: { Accept: "application/json" },
        signal: controller.signal,
      });
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      state.apiHealth = await response.json();
      if (state.apiHealth.demo_mode) {
        setApiBadge("demo", "Demo API", "Demo fallbacks active.");
      } else {
        setApiBadge("live", "Live API", "Backend connected.");
      }
    } catch {
      state.apiHealth = null;
      setApiBadge("offline", "Backend offline", `Could not reach ${apiBase}.`);
    } finally {
      window.clearTimeout(timeout);
    }
  }

  async function requestJson(path, options) {
    const response = await fetch(`${apiBase}${path}`, options);
    const raw = await response.text();
    let data;
    try {
      data = raw ? JSON.parse(raw) : {};
    } catch {
      data = { detail: raw || response.statusText };
    }
    if (!response.ok) throw new Error(data.detail || response.statusText);
    return data;
  }

  function showLoading() {
    dom.loading.classList.remove("hidden");
    dom.result.classList.add("hidden");
    dom.error.classList.add("hidden");
  }

  function hideOutput() {
    dom.loading.classList.add("hidden");
    dom.result.classList.add("hidden");
    dom.error.classList.add("hidden");
    dom.sentences.classList.add("hidden");
    dom.summary.classList.remove("hidden");
  }

  function showProblem(message) {
    dom.loading.classList.add("hidden");
    dom.errorCopy.textContent = message;
    dom.error.classList.remove("hidden");
  }

  function showAnswer(data) {
    const text = data.simple_text || data.summary || "";
    const sentences = Array.isArray(data.simple_sentences)
      ? data.simple_sentences.filter(Boolean)
      : text.split(/(?<=[.!?])\s+/).map(part => part.trim()).filter(Boolean);

    dom.loading.classList.add("hidden");
    dom.summary.textContent = text;
    dom.resultMeta.textContent = `${data.mode === "image" ? "From image" : "From audio"} · essential only${state.apiHealth?.demo_mode ? " · demo mode" : ""}`;
    dom.sentences.replaceChildren();

    if (sentences.length > 1) {
      sentences.slice(0, 2).forEach((sentence, index) => {
        const item = document.createElement("li");
        item.className = "sentence-card";
        item.innerHTML = `<span>${index + 1}</span><p></p>`;
        item.querySelector("p").textContent = sentence;
        dom.sentences.appendChild(item);
      });
      dom.summary.classList.add("hidden");
      dom.sentences.classList.remove("hidden");
    } else {
      dom.summary.classList.remove("hidden");
      dom.sentences.classList.add("hidden");
    }

    dom.result.classList.remove("hidden");
  }

  function revokeImageUrl() {
    if (state.imageObjectUrl) {
      URL.revokeObjectURL(state.imageObjectUrl);
      state.imageObjectUrl = null;
    }
  }

  function stopCamera() {
    if (state.cameraStream) {
      state.cameraStream.getTracks().forEach(track => track.stop());
      state.cameraStream = null;
    }
    dom.capture.disabled = true;
  }

  function showImageSource(which) {
    dom.camera.classList.toggle("active", which === "camera");
    dom.imagePreview.classList.toggle("active", which === "preview");
    dom.imageEmpty.classList.toggle("hidden", which !== "empty");
  }

  function resetImage() {
    stopCamera();
    revokeImageUrl();
    state.imageBlob = null;
    dom.imagePreview.removeAttribute("src");
    dom.imageFile.value = "";
    dom.sendImage.disabled = true;
    dom.clearImage.disabled = true;
    dom.cameraLabel.textContent = "Start camera";
    showImageSource("empty");
    hideOutput();
  }

  function acceptImage(blob) {
    state.imageBlob = blob;
    revokeImageUrl();
    state.imageObjectUrl = URL.createObjectURL(blob);
    dom.imagePreview.src = state.imageObjectUrl;
    dom.sendImage.disabled = false;
    dom.clearImage.disabled = false;
    dom.cameraLabel.textContent = "Restart camera";
    showImageSource("preview");
  }

  async function loadDemoImage() {
    try {
      hideOutput();
      stopCamera();
      const response = await fetch(demoImagePath, { cache: "force-cache" });
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      acceptImage(await response.blob());
    } catch (error) {
      showProblem(`Could not load the demo image: ${error.message}`);
    }
  }

  async function beginCamera() {
    try {
      hideOutput();
      stopCamera();
      revokeImageUrl();
      state.imageBlob = null;
      dom.imagePreview.removeAttribute("src");
      dom.sendImage.disabled = true;
      state.cameraStream = await navigator.mediaDevices.getUserMedia({ video: { facingMode: "environment" } });
      dom.camera.srcObject = state.cameraStream;
      dom.capture.disabled = false;
      dom.clearImage.disabled = false;
      dom.cameraLabel.textContent = "Camera ready";
      showImageSource("camera");
    } catch (error) {
      showProblem(`Camera access denied: ${error.message}`);
    }
  }

  function captureImage() {
    dom.canvas.width = dom.camera.videoWidth;
    dom.canvas.height = dom.camera.videoHeight;
    dom.canvas.getContext("2d").drawImage(dom.camera, 0, 0);
    dom.canvas.toBlob(blob => {
      if (!blob) {
        showProblem("Could not capture an image.");
        return;
      }
      stopCamera();
      acceptImage(blob);
    }, "image/jpeg", 0.85);
  }

  async function sendImage() {
    if (!state.imageBlob) return;
    const form = new FormData();
    form.append("file", state.imageBlob, "image.jpg");
    showLoading();
    dom.sendImage.disabled = true;
    try {
      showAnswer(await requestJson("/image", { method: "POST", body: form }));
    } catch (error) {
      showProblem(`Error: ${error.message}`);
    } finally {
      dom.sendImage.disabled = false;
    }
  }

  function resetAudio() {
    if (state.recorder?.state === "recording") {
      try { state.recorder.stop(); } catch {}
    }
    state.recorder = null;
    state.audioParts = [];
    state.audioBlob = null;
    state.audioName = "recording.webm";
    dom.audioFile.value = "";
    dom.wave.classList.remove("active", "ready");
    dom.audioLabel.textContent = "Press record or upload an audio file";
    dom.record.disabled = false;
    dom.stop.disabled = true;
    dom.clearAudio.disabled = true;
    dom.sendAudio.disabled = true;
    hideOutput();
  }

  function loadAudio(blob, name) {
    state.audioBlob = blob;
    state.audioName = name;
    dom.wave.classList.remove("active");
    dom.wave.classList.add("ready");
    dom.audioLabel.textContent = `Audio loaded · ${name}`;
    dom.clearAudio.disabled = false;
    dom.sendAudio.disabled = false;
  }

  async function startRecording() {
    try {
      if (!window.MediaRecorder) throw new Error("This browser does not support in-browser audio recording.");
      hideOutput();
      state.audioBlob = null;
      state.audioParts = [];
      dom.audioFile.value = "";

      const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
      state.recorder = new MediaRecorder(stream);
      state.recorder.ondataavailable = event => {
        if (event.data.size > 0) state.audioParts.push(event.data);
      };
      state.recorder.onstop = () => {
        stream.getTracks().forEach(track => track.stop());
        loadAudio(new Blob(state.audioParts, { type: "audio/webm" }), "recording.webm");
        dom.record.disabled = false;
        dom.stop.disabled = true;
      };

      state.recorder.start(250);
      dom.wave.classList.remove("ready");
      dom.wave.classList.add("active");
      dom.audioLabel.textContent = "Recording...";
      dom.record.disabled = true;
      dom.stop.disabled = false;
      dom.clearAudio.disabled = false;
      dom.sendAudio.disabled = true;
    } catch (error) {
      showProblem(`Audio setup failed: ${error.message}`);
    }
  }

  function stopRecording() {
    if (state.recorder?.state === "recording") state.recorder.stop();
    dom.stop.disabled = true;
  }

  async function sendAudio() {
    if (!state.audioBlob) return;
    const form = new FormData();
    const previousLabel = dom.audioLabel.textContent;
    form.append("file", state.audioBlob, state.audioName);
    showLoading();
    dom.sendAudio.disabled = true;
    dom.audioLabel.textContent = "Processing audio...";
    try {
      showAnswer(await requestJson("/lecture", { method: "POST", body: form }));
    } catch (error) {
      showProblem(`Error: ${error.message}`);
    } finally {
      dom.audioLabel.textContent = previousLabel;
      dom.sendAudio.disabled = false;
    }
  }

  dom.copyButton.addEventListener("click", async () => {
    const sentenceText = Array.from(dom.sentences.querySelectorAll("p")).map(item => item.textContent.trim());
    const text = sentenceText.length ? sentenceText.join(" ") : dom.summary.textContent;
    if (!text) return;
    try {
      await navigator.clipboard.writeText(text);
      dom.copyButton.classList.add("copied");
      dom.copyButton.querySelector("span").textContent = "Copied";
      setTimeout(() => {
        dom.copyButton.classList.remove("copied");
        dom.copyButton.querySelector("span").textContent = "Copy";
      }, 1500);
    } catch {}
  });

  root.addEventListener("click", event => {
    const mode = event.target.closest("[data-mode]");
    const jump = event.target.closest("[data-jump]");
    if (mode) setMode(mode.dataset.mode);
    if (jump) {
      setMode(jump.dataset.jump);
      if (jump.hasAttribute("data-demo-image")) void loadDemoImage();
    }
  });

  root.querySelector("[data-start-camera]").addEventListener("click", beginCamera);
  dom.capture.addEventListener("click", captureImage);
  dom.clearImage.addEventListener("click", resetImage);
  dom.sendImage.addEventListener("click", sendImage);
  dom.imageFile.addEventListener("change", () => {
    const file = dom.imageFile.files[0];
    if (!file) return;
    hideOutput();
    stopCamera();
    acceptImage(file);
  });

  dom.record.addEventListener("click", startRecording);
  dom.stop.addEventListener("click", stopRecording);
  dom.clearAudio.addEventListener("click", resetAudio);
  dom.sendAudio.addEventListener("click", sendAudio);
  dom.audioFile.addEventListener("change", () => {
    const file = dom.audioFile.files[0];
    if (!file) return;
    hideOutput();
    if (state.recorder?.state === "recording") state.recorder.stop();
    loadAudio(file, file.name);
  });

  window.addEventListener("beforeunload", () => {
    stopCamera();
    revokeImageUrl();
  });

  refreshHealth();
})();
