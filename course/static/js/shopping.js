document.addEventListener("DOMContentLoaded", () => {
    let items = [];
    try {
        const raw = document.getElementById("shopping-data")?.textContent?.trim();
        if (raw) items = JSON.parse(raw);
    } catch (e) {
        console.error("Помилка парсингу:", e);
    }

    const favContainer    = document.getElementById("fav-items");
    const manualContainer = document.getElementById("manual-items");
    const emptyMsg        = document.getElementById("empty-message");

    if (!items || items.length === 0) {
        emptyMsg.style.display = "block";
        return;
    }

    emptyMsg.style.display    = "none";
    favContainer.innerHTML    = renderItems(items.filter(i => i.source === "favorites"));
    manualContainer.innerHTML = renderItems(items.filter(i => i.source !== "favorites"));
});

function renderItems(items) {
    if (!items || !items.length) return "<p style='color:var(--text-secondary); padding: 10px 0;'>Порожньо</p>";
    return items.map(i => `
        <div class="shopping-item-row" onclick="toggleShoppingRow(this)">
            <div class="shopping-item-left">
                <input type="checkbox" class="shopping-checkbox" onclick="event.stopPropagation(); toggleShoppingRow(this.parentElement.parentElement, this.checked)">
                <span class="shopping-item-name">${escapeHtml(i.name)}</span>
            </div>
            <span class="qty" style="font-weight: 700; color: var(--primary);">${i.quantity} ${escapeHtml(i.unit)}</span>
        </div>
    `).join("");
}

window.toggleShoppingRow = function(row, isChecked) {
    const checkbox = row.querySelector(".shopping-checkbox");
    if (checkbox) {
        if (isChecked === undefined) {
            checkbox.checked = !checkbox.checked;
        } else {
            checkbox.checked = isChecked;
        }
        row.classList.toggle("checked", checkbox.checked);
    }
};

window.clearShoppingList = async function() {
    if (!confirm("Очистити весь список?")) return;
    await fetch("/api/clear-shopping-list", { method: "POST" });
    location.reload();
};

function escapeHtml(str) {
    if (!str) return "";
    return String(str).replace(/[&<>"']/g, m => ({
        "&": "&amp;", "<": "&lt;", ">": "&gt;",
        '"': "&quot;", "'": "&#039;"
    }[m]));
}