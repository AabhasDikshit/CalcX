const expression = document.querySelector('#expression');
const result = document.querySelector('#result');
const steps = document.querySelector('#steps');
const status = document.querySelector('#status');
const modeLabel = document.querySelector('#mode-label');
const history = document.querySelector('#history');
const variableInput = document.querySelector('#variable');
const apiBaseInput = document.querySelector('#api-base');
const apiSaveButton = document.querySelector('#save-api-base');
const historyKey = 'calcx-history';
const apiBaseKey = 'calcx-api-base';
let mode = 'calculate';

const endpointMap = {
  calculate: 'calculate',
  differentiate: 'differentiate',
  integrate: 'integrate',
  solve: 'solve'
};

function insert(value) {
  const start = expression.selectionStart;
  const end = expression.selectionEnd;
  expression.value = expression.value.slice(0, start) + value + expression.value.slice(end);
  expression.focus();
  expression.setSelectionRange(start + value.length, start + value.length);
}

function renderHistory() {
  const items = JSON.parse(localStorage.getItem(historyKey) || '[]');
  history.innerHTML = items.length ? items.map(item => `<article class="history-item"><small>${item.mode}</small><p>${escapeHtml(item.input)} &rarr; ${escapeHtml(item.output)}</p></article>`).join('') : '<p class="empty">No calculations yet.</p>';
}
function escapeHtml(value) { return value.replace(/[&<>'"]/g, character => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', "'": '&#39;', '"': '&quot;' })[character]); }
function saveHistory(input, output) {
  const items = JSON.parse(localStorage.getItem(historyKey) || '[]');
  items.unshift({ input, output, mode: mode.toUpperCase() });
  localStorage.setItem(historyKey, JSON.stringify(items.slice(0, 12)));
  renderHistory();
}

function apiBase() {
  const value = (apiBaseInput.value || '').trim();
  return value.replace(/\/+$/, '');
}

function saveApiBase() {
  const value = apiBase();
  if (!value) {
    status.textContent = 'API URL REQUIRED';
    return;
  }
  localStorage.setItem(apiBaseKey, value);
  status.textContent = 'API URL SAVED';
}

function renderSteps(stepItems) {
  if (!stepItems?.length) {
    steps.innerHTML = '';
    return;
  }
  steps.innerHTML = stepItems
    .map((item, index) => `<div class="step"><strong>Step ${index + 1}: ${escapeHtml(item.rule || 'detail')}</strong>${escapeHtml(item.explanation || item.result || '')}</div>`)
    .join('');
}

function buildPayload(input) {
  const variable = (variableInput.value || 'x').trim() || 'x';
  if (mode === 'calculate') return { expression: input };
  if (mode === 'differentiate') return { expression: input, variable };
  if (mode === 'integrate') return { expression: input, variable };
  return { equation: input, variable };
}

function renderModeResult(data) {
  if (mode === 'calculate') {
    result.textContent = `${data.result_text}  (parsed: ${data.parsed})`;
    renderSteps([]);
    return;
  }
  if (mode === 'differentiate') {
    result.textContent = data.derivative;
    renderSteps(data.steps || []);
    return;
  }
  if (mode === 'integrate') {
    result.textContent = data.supported ? data.antiderivative : data.message;
    renderSteps([]);
    return;
  }
  if (!data.supported) {
    result.textContent = data.message || 'Equation solver does not support this form yet.';
    renderSteps([]);
    return;
  }
  const solutions = (data.solutions || []).map(solution => solution.text).join(', ');
  result.textContent = solutions ? `x = ${solutions}` : 'No explicit solutions returned.';
  renderSteps([]);
}

async function showResult() {
  const input = expression.value.trim();
  if (!input) { result.textContent = 'Enter an expression first.'; return; }
  const base = apiBase();
  if (!base) {
    status.textContent = 'SET API URL';
    result.textContent = 'Set the API Base URL first.';
    return;
  }

  const endpoint = endpointMap[mode];
  status.textContent = 'REQUESTING';
  try {
    const response = await fetch(`${base}/api/v1/${endpoint}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(buildPayload(input))
    });
    const data = await response.json();
    if (!response.ok || !data.ok) {
      throw new Error(data.error || `Request failed with status ${response.status}`);
    }
    renderModeResult(data);
    status.textContent = 'OK';
    saveHistory(input, result.textContent);
  } catch (error) {
    status.textContent = 'API ERROR';
    result.textContent = `Backend error: ${error.message}`;
    steps.innerHTML = '<div class="step"><strong>Check backend</strong>Ensure the CalcX C++ API server is running and reachable from this device.</div>';
  }
}

document.querySelectorAll('[data-insert]').forEach(button => button.addEventListener('click', () => insert(button.dataset.insert)));
document.querySelector('#backspace').addEventListener('click', () => { const start = expression.selectionStart; const end = expression.selectionEnd; if (start !== end) insert(''); else if (start) { expression.value = expression.value.slice(0, start - 1) + expression.value.slice(end); expression.setSelectionRange(start - 1, start - 1); } expression.focus(); });
document.querySelector('#solve').addEventListener('click', showResult);
document.querySelector('#clear-history').addEventListener('click', () => { localStorage.removeItem(historyKey); renderHistory(); });
document.querySelector('#theme-toggle').addEventListener('click', () => document.documentElement.classList.toggle('light'));
apiSaveButton.addEventListener('click', saveApiBase);
document.querySelectorAll('.mode').forEach(button => button.addEventListener('click', () => { document.querySelector('.mode.active').classList.remove('active'); button.classList.add('active'); mode = button.dataset.mode; modeLabel.textContent = mode.toUpperCase(); }));
expression.addEventListener('keydown', event => { if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') showResult(); });
apiBaseInput.value = localStorage.getItem(apiBaseKey) || 'http://localhost:8080';
const queryApi = new URLSearchParams(window.location.search).get('api');
if (queryApi) {
  apiBaseInput.value = queryApi;
  saveApiBase();
}
renderHistory();
