// SPDX-License-Identifier: LGPL-3.0-or-later
// search.js — top-bar search. On first focus the Pagefind index named by
// input[data-pagefind] is imported; with it, results render below the box
// (Enter opens the first hit for the typed query, or submits to search.html
// when it has none; arrows move, Escape clears the query, a second Escape
// closes the mobile search row). Without it (local and preview builds) the
// form is left alone and Enter submits ?q= to Sphinx's search.html.
(() => {
  const input = document.getElementById("book-search-input");
  const list = document.getElementById("book-search-results");
  if (!input || !list || !input.dataset.pagefind) return;
  const form = input.form;
  const MAX = 10;
  let loading = null;
  let pagefind = null;
  let timer = 0;
  let seq = 0;
  let failed = false;
  let shown = ""; // query whose results are currently rendered

  input.setAttribute("aria-controls", list.id);
  input.setAttribute("aria-expanded", "false");

  const load = () => {
    if (!loading) {
      loading = import(input.dataset.pagefind)
        .then(async (pf) => {
          // Result URLs are relative to the index's site root: derive it
          // from the pagefind.js URL ("/" on jupedsim.org, the preview
          // folder on PR previews).
          const pfUrl = new URL(input.dataset.pagefind, window.location.href);
          await pf.options({ baseUrl: new URL("..", pfUrl).pathname });
          await pf.init();
          pagefind = pf;
          return pf;
        })
        .catch((err) => {
          console.debug("search: Pagefind unavailable, using Sphinx search", err);
          failed = true;
          input.placeholder = "Search (Enter)";
          return null;
        });
    }
    return loading;
  };

  // Pagefind excerpts are plain text plus <mark>; rebuild them from text
  // nodes so nothing but <mark> can ever reach the DOM.
  const excerptNode = (html) => {
    const out = document.createElement("span");
    out.className = "book-search-excerpt";
    const doc = new DOMParser().parseFromString(html || "", "text/html");
    const walk = (node, parent) => {
      for (const child of node.childNodes) {
        if (child.nodeType === Node.TEXT_NODE) {
          parent.appendChild(document.createTextNode(child.textContent));
        } else if (child.nodeType === Node.ELEMENT_NODE) {
          if (child.localName === "mark") {
            const mark = document.createElement("mark");
            mark.textContent = child.textContent;
            parent.appendChild(mark);
          } else if (!["script", "style", "template"].includes(child.localName)) {
            walk(child, parent);
          }
        }
      }
    };
    walk(doc.body, out);
    return out;
  };

  const links = () => Array.from(list.querySelectorAll("a"));

  const show = () => {
    list.hidden = false;
    input.setAttribute("aria-expanded", "true");
  };

  const hide = () => {
    list.hidden = true;
    input.setAttribute("aria-expanded", "false");
  };

  const clear = () => {
    seq++;
    clearTimeout(timer);
    shown = "";
    list.replaceChildren();
    hide();
  };

  const render = (items, q) => {
    shown = q;
    list.replaceChildren();
    for (const d of items) {
      const li = document.createElement("li");
      const a = document.createElement("a");
      a.href = d.url;
      const section = (d.filters && d.filters.section && d.filters.section[0]) || "";
      if (section) {
        const small = document.createElement("small");
        small.textContent = section;
        a.appendChild(small);
      }
      const title = document.createElement("span");
      title.className = "book-search-title";
      title.textContent = (d.meta && d.meta.title) || d.url;
      a.appendChild(title);
      a.appendChild(excerptNode(d.excerpt));
      li.appendChild(a);
      list.appendChild(li);
    }
    if (!items.length) {
      const empty = document.createElement("li");
      empty.className = "book-search-empty";
      empty.textContent = "No results";
      list.appendChild(empty);
    }
    show();
  };

  // Resolves to the rendered items, null when Pagefind is unusable, or
  // undefined when a newer query superseded this one.
  const run = async () => {
    const q = input.value.trim();
    const mine = ++seq;
    if (!q) {
      clear();
      return [];
    }
    const pf = await load();
    if (mine !== seq) return undefined;
    if (!pf) return null;
    try {
      const res = await pf.search(q);
      if (mine !== seq) return undefined;
      const items = res
        ? await Promise.all(res.results.slice(0, MAX).map((r) => r.data()))
        : [];
      if (mine !== seq) return undefined;
      render(items, q);
      return items;
    } catch (err) {
      console.debug("search: query failed", err); // e.g. aborted by navigation
      return null;
    }
  };

  // Enter: open the first hit for what is typed now (searching first if the
  // rendered list belongs to an older query); no hit -> Sphinx search.html.
  const enter = async () => {
    clearTimeout(timer);
    const q = input.value.trim();
    if (!q) return;
    let url = null;
    if (q === shown) {
      const first = links()[0];
      url = first ? first.href : null;
    } else {
      const items = await run();
      if (items === undefined) return;
      url = items && items[0] ? items[0].url : null;
    }
    if (url) window.location.href = url;
    else if (form) form.submit();
  };

  input.addEventListener("focus", load, { once: true });
  input.addEventListener("input", () => {
    clearTimeout(timer);
    timer = setTimeout(run, 150);
  });
  input.addEventListener("keydown", (e) => {
    if (failed) return; // no index: Enter submits to Sphinx search.html
    if (e.key === "Enter") {
      e.preventDefault();
      enter();
    } else if (!pagefind) {
      return;
    } else if (e.key === "ArrowDown") {
      const first = links()[0];
      if (first && !list.hidden) {
        e.preventDefault();
        first.focus();
      }
    } else if (e.key === "Escape" && input.value) {
      // First Escape clears the query; on an empty field it bubbles to the
      // document handler below, which closes the mobile search row.
      e.preventDefault();
      input.value = "";
      clear();
    }
  });
  list.addEventListener("keydown", (e) => {
    const all = links();
    const i = all.indexOf(document.activeElement);
    if (i < 0) return;
    if (e.key === "ArrowDown") {
      e.preventDefault();
      if (i + 1 < all.length) all[i + 1].focus();
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      if (i > 0) all[i - 1].focus();
      else input.focus();
    } else if (e.key === "Escape") {
      e.preventDefault();
      input.value = "";
      clear();
      input.focus();
    }
  });
  document.addEventListener("click", (e) => {
    if (!form || !form.contains(e.target)) hide();
  });
  // Keyboard focus leaving the search (e.g. Tab past the last hit) must not
  // leave the overlay covering the focused nav link.
  if (form) {
    form.addEventListener("focusout", (e) => {
      if (!form.contains(e.relatedTarget)) hide();
    });
  }
  input.addEventListener("focus", () => {
    if (input.value.trim() && list.childElementCount) show();
  });
})();

// Sphinx fallback search (searchtools.js) reads result excerpts from
// '[role="main"]', which this theme does not have; point it at .report-body.
(() => {
  const patch = () => {
    if (typeof Search === "undefined" || Search.htmlToText.jpsPatched) return;
    const htmlToText = (htmlString, anchor) => {
      const doc = new DOMParser().parseFromString(htmlString, "text/html");
      for (const query of [".headerlink", "script", "style"]) {
        doc.querySelectorAll(query).forEach((el) => el.remove());
      }
      const body = doc.querySelector(".report-body");
      if (!body) return "";
      const target = anchor ? body.querySelector(anchor) : null;
      return (target || body).textContent;
    };
    htmlToText.jpsPatched = true;
    Search.htmlToText = htmlToText;
  };
  patch();
  document.addEventListener("DOMContentLoaded", patch);
})();

// "/" focuses the bar search, unless focus is already in a text field, a
// textarea, a select or editable content (their own "/" keeps working).
// Below 56rem the search row is hidden: reveal it via #search-control first.
document.addEventListener("keydown", (e) => {
  if (e.key !== "/" || e.ctrlKey || e.metaKey || e.altKey || e.defaultPrevented) return;
  const t = e.target;
  const tag = t && t.tagName;
  // Checkboxes (the drawer and search toggles) and buttons are not text
  // fields: "/" still jumps to search from there.
  const textInput =
    tag === "INPUT" && !["checkbox", "radio", "button", "submit", "reset"].includes(t.type);
  if (t && (t.isContentEditable || textInput || tag === "TEXTAREA" || tag === "SELECT")) return;
  const input = document.getElementById("book-search-input");
  if (!input) return;
  e.preventDefault();
  if (input.offsetParent === null) {
    const sc = document.getElementById("search-control");
    if (sc) sc.checked = true;
  }
  input.focus();
});

// Opening the mobile search row (its toggle in the bar) puts the cursor in
// the field; closing it returns focus to the toggle.
(() => {
  const sc = document.getElementById("search-control");
  const input = document.getElementById("book-search-input");
  if (!sc || !input) return;
  sc.addEventListener("change", () => {
    if (sc.checked) setTimeout(() => input.focus(), 0);
  });
})();

// Escape closes what is open, innermost first: the mobile search row, then
// the menu and TOC drawers. Focus returns to the toggle that opened it, so
// keyboard users stay where they were.
document.addEventListener("keydown", (e) => {
  if (e.key !== "Escape" || e.defaultPrevented) return;
  for (const id of ["search-control", "menu-control", "toc-control"]) {
    const toggle = document.getElementById(id);
    if (toggle && toggle.checked) {
      e.preventDefault();
      toggle.checked = false;
      toggle.focus({ preventScroll: true });
      return;
    }
  }
});
