// SPDX-License-Identifier: LGPL-3.0-or-later
// switcher.js — docs-mode version switcher. Populates
// select#book-version-switcher from its data-json list of
// {name, version, url, preferred?} and navigates to the same page in the
// chosen version (falling back to that version's root). Always stays on the
// current origin: absolute URLs are reduced to their path.
(function () {
    "use strict";

    function pathOf(url) {
        let path;
        try {
            path = new URL(url, window.location.href).pathname;
        } catch (e) {
            return null;
        }
        if (path.endsWith("/index.html")) {
            path = path.slice(0, -"index.html".length);
        }
        return path.endsWith("/") ? path : path + "/";
    }

    function init() {
        const select = document.getElementById("book-version-switcher");
        if (!select || !select.dataset.json) {
            return;
        }
        const match = select.dataset.match || "";
        const page = (select.dataset.page || "").replace(/^\/+/, "");

        fetch(select.dataset.json, { cache: "no-cache" })
            .then(function (response) {
                if (!response.ok) {
                    throw new Error("HTTP " + response.status);
                }
                return response.json();
            })
            .then(function (entries) {
                if (!Array.isArray(entries)) {
                    throw new Error("versions.json is not a list");
                }
                const here = window.location.pathname;
                let selected = null;
                let selectedLength = -1;
                let byVersion = null;
                for (const entry of entries) {
                    if (!entry || typeof entry.url !== "string") {
                        continue;
                    }
                    const path = pathOf(entry.url);
                    if (path === null) {
                        continue;
                    }
                    const label = String(entry.name || entry.version || path);
                    const option = new Option(label, path);
                    select.add(option);
                    if (
                        (here.startsWith(path) || here + "/" === path) &&
                        path.length > selectedLength
                    ) {
                        selected = option;
                        selectedLength = path.length;
                    }
                    if (byVersion === null && match && entry.version === match) {
                        byVersion = option;
                    }
                }
                if (select.options.length === 0) {
                    return;
                }
                const current = selected || byVersion;
                if (current) {
                    current.selected = true;
                } else {
                    select.selectedIndex = -1;
                }
                select.addEventListener("change", function () {
                    const root = select.value;
                    const target = root + page;
                    fetch(target, { method: "HEAD", cache: "no-cache" })
                        .then(function (response) {
                            window.location.assign(response.ok ? target : root);
                        })
                        .catch(function () {
                            window.location.assign(root);
                        });
                });
                select.hidden = false;
            })
            .catch(function (error) {
                console.debug("version switcher disabled:", error.message);
            });
    }

    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", init);
    } else {
        init();
    }
})();
