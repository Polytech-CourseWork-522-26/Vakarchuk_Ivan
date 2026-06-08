// ================================================
// recipe.js — ФІНАЛЬНА ВЕРСІЯ
// ================================================

let isFavorite = false;   // Глобальна змінна

document.addEventListener("DOMContentLoaded", () => {
    console.log("📌 recipe.js loaded");

    const data = window.__RECIPE__;
    if (!data) {
        console.error("❌ window.__RECIPE__ не знайдено");
        return;
    }

    console.log("📦 Рецепт завантажено:", data.name);
    console.log("📋 Інгредієнтів:", data.ingredients ? data.ingredients.length : 0);

    renderIngredients(data.ingredients);
    checkIfFavorite(data.id);
    setupSteps();
});

// =========================
// 🔥 RENDER INGREDIENTS
// =========================
function renderIngredients(ingredients) {
    const container = document.getElementById("recipe-data");
    if (!container) {
        console.error("❌ Контейнер #recipe-data не знайдено");
        return;
    }

    container.innerHTML = "";

    if (!ingredients || ingredients.length === 0) {
        container.innerHTML = "<p style='color:#888;'>Інгредієнти не вказані</p>";
        return;
    }

    ingredients.forEach(ing => {
        const row = document.createElement("div");
        row.className = "ingredient-list-row";

        row.innerHTML = `
            <span class="ingredient-name">${ing.name}</span>
            <span class="ingredient-qty-unit">
                <span>${ing.qty} ${ing.unit}</span>
                <button onclick="addToShopping(${ing.id}, '${ing.name.replace(/'/g, "\\'")}', ${ing.qty})" 
                        class="btn btn-secondary btn-small" style="padding: 6px 12px; font-size: 12px; height: auto;">
                    🛒 В кошик
                </button>
            </span>
        `;

        row.addEventListener("click", (e) => {
            if (e.target.closest("button")) return;
            row.classList.toggle("checked");
        });

        container.appendChild(row);
    });
}

// =========================
// 🛒 ADD TO SHOPPING LIST
// =========================
window.addToShopping = function(id, name, qty) {
    fetch("/api/add-to-shopping-list", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `ingredient_id=${id}&quantity=${qty}`
    })
    .then(r => r.json())
    .then(data => {
        if (data.status === "success") {
            alert(`✅ ${name} додано до списку покупок!`);
        }
    })
    .catch(err => console.error(err));
};

// =========================
// ❤️ FAVORITES
// =========================
window.toggleFavorite = function(recipeId) {
    if (isFavorite) {
        removeFromFavorites(recipeId);
    } else {
        addToFavorites(recipeId);
    }
};

window.addToFavorites = function(recipeId) {
    fetch("/api/add-favorite", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `recipe_id=${recipeId}`
    })
    .then(r => r.json())
    .then(data => {
        if (data.status === "success") {
            isFavorite = true;
            updateFavoriteButton();
            alert('✅ Додано в улюблені!');
        }
    })
    .catch(err => console.error(err));
};

window.removeFromFavorites = function(recipeId) {
    fetch("/api/remove-favorite", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `recipe_id=${recipeId}`
    })
    .then(r => r.json())
    .then(data => {
        if (data.status === "success") {
            isFavorite = false;
            updateFavoriteButton();
        }
    })
    .catch(err => console.error(err));
};

function updateFavoriteButton() {
    const btn = document.getElementById("fav-btn");
    if (!btn) return;

    if (isFavorite) {
        btn.innerHTML = "❤️ В улюблених";
        btn.className = "btn btn-primary btn-small";
        btn.style.background = ""; 
    } else {
        btn.innerHTML = "❤️ Додати в улюблене";
        btn.className = "btn btn-secondary btn-small";
        btn.style.background = ""; 
    }
}

//3 GET
async function checkIfFavorite(recipeId) {
    try {
        const res = await fetch(`/api/check-favorite?recipe_id=${recipeId}`);
        const data = await res.json();
        isFavorite = data.isFavorite || false;
        updateFavoriteButton();
    } catch (err) {
        console.error(err);
    }
}

// =========================
// ⚠️ REPORT
// =========================
window.reportRecipe = function(recipeId) {
    const reason = prompt("Вкажіть причину скарги (українською можна):", "");

    if (!reason || reason.trim() === "") {
        alert("Причина скарги не може бути порожньою!");
        return;
    }

    fetch("/api/report-recipe", {
        method: "POST",
        headers: { "Content-Type": "application/json" },   // ← змінили
        body: JSON.stringify({ recipe_id: recipeId, reason: reason })  // ← змінили
    })
    .then(r => r.json())
    .then(data => {
        if (data.status === "success") {
            alert("✅ Скарга успішно відправлена адміністратору!");
        } else {
            alert("Помилка: " + (data.message || "Невідома помилка"));
        }
    })
    .catch(err => {
        console.error(err);
        alert("Не вдалося з'єднатися з сервером.");
    });
};
// =========================
// EDIT & DELETE
// =========================
window.editRecipe = function(recipeId) {
    window.location.href = `/recipe/${recipeId}/edit`;
};
// 3 DELETE
window.deleteRecipe = function(recipeId) {
    if (!confirm("⚠️ Ви впевнені, що хочете видалити цей рецепт?")) return;

    fetch(`/api/recipe/${recipeId}`, { method: 'DELETE' })
    .then(r => r.json())
    .then(data => {
        if (data.status === "success") {
            window.location.replace("/");
        } else {
            alert("Помилка: " + (data.message || "Немає прав"));
        }
    })
    .catch(err => {
        console.error(err);
        alert("Помилка з'єднання з сервером");
    });
};

function setupSteps() {
    const rawContainer = document.getElementById("recipe-instructions-list");
    const stepsContainer = document.getElementById("recipe-steps-container");
    if (!rawContainer || !stepsContainer) return;

    const text = rawContainer.innerText.trim();
    if (!text) return;

    const lines = text.split("\n")
        .map(l => l.trim())
        .filter(l => l.length > 0);

    stepsContainer.innerHTML = "";

    lines.forEach((line, index) => {
        // Clean leading step numbers
        const cleanedLine = line.replace(/^\d+[\s.)\-]+/, "");

        const stepRow = document.createElement("div");
        stepRow.className = "recipe-step-row";
        stepRow.innerHTML = `
            <div class="recipe-step-number">${index + 1}</div>
            <div class="recipe-step-content">${cleanedLine}</div>
        `;

        stepRow.addEventListener("click", () => {
            const isActive = stepRow.classList.contains("active");
            document.querySelectorAll(".recipe-step-row").forEach(r => r.classList.remove("active"));
            if (!isActive) {
                stepRow.classList.add("active");
            }
        });

        stepsContainer.appendChild(stepRow);
    });
}