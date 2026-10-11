/* Build a local symbol index from Sphinx's rendered declarations. */
document.addEventListener("DOMContentLoaded", () => {
  const form = document.querySelector(".api-search");
  if (!form) return;

  const input = form.querySelector("input");
  const results = document.getElementById("api-search-results");
  const status = document.getElementById("api-search-status");
  const declarations = Array.from(document.querySelectorAll("article dl.cpp > dt.sig[id]"), (signature) => {
    const label = signature.cloneNode(true);
    label.querySelectorAll(".headerlink").forEach((link) => link.remove());
    label.querySelectorAll("br").forEach((br) => br.replaceWith(" "));
    return { id: signature.id, label: label.textContent.trim().replace(/\s+/g, " ") };
  });

  function search() {
    const query = input.value.trim().toLowerCase();
    results.replaceChildren();
    results.hidden = !query;
    if (!query) {
      status.textContent = "Type to find matching API declarations.";
      return;
    }
    const matches = declarations.filter((symbol) => symbol.label.toLowerCase().includes(query));
    status.textContent = matches.length
      ? `${matches.length} matching declaration${matches.length === 1 ? "" : "s"}.`
      : "No matching API declarations. Try a different symbol name.";
    for (const symbol of matches) {
      const item = document.createElement("li");
      const link = document.createElement("a");
      link.href = `#${symbol.id}`;
      link.textContent = symbol.label;
      item.append(link);
      results.append(item);
    }
  }

  input.addEventListener("input", search);
  form.addEventListener("submit", (event) => {
    event.preventDefault();
    search();
  });
  // Keep declaration and contents jumps together with keyboard focus.
  document.querySelector("article").addEventListener("click", (event) => {
    const link = event.target.closest('a[href^="#"]');
    if (!link || event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
    const target = document.getElementById(decodeURIComponent(link.hash.slice(1)));
    if (!target) return;
    event.preventDefault();
    history.pushState(null, "", link.hash);
    target.setAttribute("tabindex", "-1");
    target.focus({ preventScroll: true });
    target.scrollIntoView({ behavior: "instant", block: "start" });
  });
  search();
});
