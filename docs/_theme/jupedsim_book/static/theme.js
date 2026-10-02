/* theme.js — light/dark/auto toggle. Stores "light" or "dark" in
   localStorage under "theme"; "auto" removes the key and follows
   prefers-color-scheme. The head snippet applies the same rule before
   first paint; this file only wires the button and the media listener. */
(function () {
  var KEY = 'theme';
  var ORDER = ['auto', 'light', 'dark'];
  var root = document.documentElement;
  var mq = window.matchMedia ? window.matchMedia('(prefers-color-scheme: dark)') : null;

  function stored() {
    try { return localStorage.getItem(KEY); } catch (e) { return null; }
  }
  function mode() {
    var s = stored();
    return s === 'light' || s === 'dark' ? s : 'auto';
  }
  function apply() {
    var m = mode();
    root.dataset.theme = m === 'auto' ? (mq && mq.matches ? 'dark' : 'light') : m;
    root.dataset.themeMode = m;
    var b = document.getElementById('book-theme-toggle');
    if (b) b.textContent = 'Theme: ' + m;
  }
  function init() {
    var b = document.getElementById('book-theme-toggle');
    if (b) {
      b.addEventListener('click', function () {
        var next = ORDER[(ORDER.indexOf(mode()) + 1) % ORDER.length];
        try {
          if (next === 'auto') localStorage.removeItem(KEY); else localStorage.setItem(KEY, next);
        } catch (e) { /* storage unavailable: still switch for this page */ }
        if (next === 'auto') { root.dataset.theme = mq && mq.matches ? 'dark' : 'light'; }
        else { root.dataset.theme = next; }
        root.dataset.themeMode = next;
        b.textContent = 'Theme: ' + next;
      });
    }
    apply();
  }
  if (mq && mq.addEventListener) mq.addEventListener('change', apply);
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init); else init();
})();
