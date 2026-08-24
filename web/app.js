const expression = document.querySelector('#expression');
const result = document.querySelector('#result');
const steps = document.querySelector('#steps');
const status = document.querySelector('#status');
const modeLabel = document.querySelector('#mode-label');
const history = document.querySelector('#history');
const historyKey = 'calcx-history';
let mode = 'calculate';

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
function showResult() {
  const input = expression.value.trim();
  if (!input) { result.textContent = 'Enter an expression first.'; return; }
  status.textContent = 'API READY';
  result.textContent = `C++ engine request queued: ${mode}(${input})`;
  steps.innerHTML = '<div class="step"><strong>Engine boundary</strong>The browser shell is ready for the CalcX C++ adapter. Use the command-line engine while the HTTP bridge is under construction.</div>';
  saveHistory(input, result.textContent);
}

document.querySelectorAll('[data-insert]').forEach(button => button.addEventListener('click', () => insert(button.dataset.insert)));
document.querySelector('#backspace').addEventListener('click', () => { const start = expression.selectionStart; const end = expression.selectionEnd; if (start !== end) insert(''); else if (start) { expression.value = expression.value.slice(0, start - 1) + expression.value.slice(end); expression.setSelectionRange(start - 1, start - 1); } expression.focus(); });
document.querySelector('#solve').addEventListener('click', showResult);
document.querySelector('#clear-history').addEventListener('click', () => { localStorage.removeItem(historyKey); renderHistory(); });
document.querySelector('#theme-toggle').addEventListener('click', () => document.documentElement.classList.toggle('light'));
document.querySelectorAll('.mode').forEach(button => button.addEventListener('click', () => { document.querySelector('.mode.active').classList.remove('active'); button.classList.add('active'); mode = button.dataset.mode; modeLabel.textContent = mode.toUpperCase(); }));
expression.addEventListener('keydown', event => { if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') showResult(); });
renderHistory();
