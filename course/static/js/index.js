let allIngredients = [];
let selectedIngredientIds = [];
let currentResults = [];

function showGuestModal() {
    const modal = document.getElementById('guest-modal');
    if (modal) modal.style.display = 'flex';
}

document.addEventListener('DOMContentLoaded', async () => {
    await loadAllIngredients();
    await renderInitialRecipes();

    document.getElementById('name-search').addEventListener('input', (e) => {
        searchByName(e.target.value.trim());
    });

    setupIngredientSearch();
});

async function loadAllIngredients() {
    try {
        const res = await fetch('/api/ingredients');
        const data = await res.json();
        allIngredients = data.ingredients || [];
    } catch (e) {
        console.error("Не вдалося завантажити інгредієнти", e);
    }
}

async function renderInitialRecipes() {
    const res = await fetch('/api/recipes');
    const data = await res.json();
    currentResults = data.recipes || [];
    renderRecipes(currentResults);
}

// ===== БЕЗПЕЧНЕ ЕКРАНУВАННЯ =====
function escHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}

function renderRecipes(recipes) {
    const container = document.getElementById('results');
    container.innerHTML = '';

    if (recipes.length === 0) {
        container.innerHTML = `
            <div style="grid-column:1/-1; text-align:center; padding:60px 20px; color:var(--text-secondary); animation: cardFadeIn 0.4s ease-out;">
                <div style="font-size: 64px; margin-bottom: 15px; animation: panWobble 2s infinite ease-in-out; display: inline-block;">🍳</div>
                <p style="font-size: 18px; font-weight: 700; color: var(--text-primary);">Нічого не знайдено 😔</p>
                <p style="font-size: 14px; margin-top: 6px;">Спробуйте змінити назву страви або набір інгредієнтів!</p>
            </div>`;
        return;
    }

    recipes.forEach(r => {
        const allergenWarning = r.hasAllergen
            ? `<span style="color:var(--danger); font-weight:700; display:block; margin-bottom:8px; font-size: 13px;">⚠️ Містить алерген!</span>`
            : '';

        // ===== МЕДІА НАД НАЗВОЮ =====
        let mediaBanner = '';
        if (r.photo_path) {
            mediaBanner = `
                <div class="recipe-card-img-wrap">
                    <img src="/${escHtml(r.photo_path)}"
                         alt="${escHtml(r.name)}"
                         loading="lazy"
                         class="recipe-card-img"
                         onerror="this.parentElement.style.display='none'">
                    <!-- Steam Effect -->
                    <div class="steam-container">
                        <div class="steam-line"></div>
                        <div class="steam-line"></div>
                        <div class="steam-line"></div>
                    </div>
                </div>`;
        } else if (r.video_url) {
            const ytId = extractYoutubeId(r.video_url);
            if (ytId) {
                mediaBanner = `
                    <div class="recipe-card-img-wrap" style="background:#000;">
                        <img src="https://img.youtube.com/vi/${ytId}/mqdefault.jpg"
                             alt="Відео: ${escHtml(r.name)}"
                             loading="lazy"
                             class="recipe-card-img"
                             style="opacity:0.85;">
                        <div style="position:absolute; top:50%; left:50%; transform:translate(-50%,-50%);
                                     background:rgba(255,87,34,0.9); border-radius:50%; width:48px; height:48px;
                                     display:flex; align-items:center; justify-content:center; color:white; font-size:20px;
                                     box-shadow: 0 4px 10px rgba(0,0,0,0.3); pointer-events: none;">
                            ▶
                        </div>
                    </div>`;
            }
        } else {
            mediaBanner = `
                <div class="recipe-card-img-wrap" style="display: flex; align-items: center; justify-content: center; background: var(--primary-light);">
                    <span style="font-size: 48px; animation: panWobble 3s infinite ease-in-out; display: inline-block;">🍳</span>
                </div>`;
        }

        const card = `
            <div class="recipe-card">
                ${mediaBanner}
                <div class="recipe-card-content">
                    ${allergenWarning}
                    <a href="/recipe/${r.id}" class="recipe-card-title">
                        ${escHtml(r.name)}
                    </a>
                    <p class="recipe-card-desc">
                        ${escHtml(r.description)}
                    </p>
                    <div class="recipe-card-meta">
                        <span class="recipe-card-calories">🔥 ${r.calories} ккал</span>
                        <span class="recipe-card-author">Автор: <strong>${escHtml(r.author || 'Гість')}</strong></span>
                    </div>
                </div>
            </div>`;
        container.insertAdjacentHTML('beforeend', card);
    });
}

// Витягуємо YouTube video ID з різних форматів URL
function extractYoutubeId(url) {
    if (!url) return null;
    const patterns = [
        /watch\?v=([a-zA-Z0-9_-]{11})/,
        /youtu\.be\/([a-zA-Z0-9_-]{11})/,
        /embed\/([a-zA-Z0-9_-]{11})/
    ];
    for (const p of patterns) {
        const m = url.match(p);
        if (m) return m[1];
    }
    return null;
}

// ==================== ПОШУК ЗА НАЗВОЮ ====================
async function searchByName(query) {
    if (!query) {
        renderRecipes(currentResults);
        return;
    }
    try {
        const res = await fetch(`/api/search/name?q=${encodeURIComponent(query)}`);
        const data = await res.json();
        renderRecipes(data.recipes || []);
    } catch (e) {
        console.error(e);
    }
}

// ==================== ПОШУК ЗА ІНГРЕДІЄНТАМИ ====================
function setupIngredientSearch() {
    const input = document.getElementById('ing-search-input');
    const dropdown = document.getElementById('ing-dropdown');
    const addBtn = document.getElementById('add-ing-btn');

    input.addEventListener('input', () => {
        const term = input.value.toLowerCase().trim();
        dropdown.innerHTML = '';
        if (term.length < 1) { dropdown.style.display = 'none'; return; }

        const filtered = allIngredients.filter(ing =>
            ing.name.toLowerCase().includes(term)
        );
        filtered.forEach(ing => {
            const div = document.createElement('div');
            div.className = 'dropdown-item';
            div.textContent = ing.name;
            div.onclick = () => {
                addIngredient(ing);
                input.value = '';
                dropdown.style.display = 'none';
            };
            dropdown.appendChild(div);
        });
        dropdown.style.display = filtered.length ? 'block' : 'none';
    });

    addBtn.addEventListener('click', () => {
        const term = input.value.trim();
        const found = allIngredients.find(i => i.name.toLowerCase() === term.toLowerCase());
        if (found) { addIngredient(found); input.value = ''; }
    });

    document.addEventListener('click', (e) => {
        if (!input.contains(e.target) && !dropdown.contains(e.target)) {
            dropdown.style.display = 'none';
        }
    });
}

function addIngredient(ing) {
    if (selectedIngredientIds.includes(ing.id)) return;
    selectedIngredientIds.push(ing.id);

    const container = document.getElementById('selected-ingredients');
    const tag = document.createElement('div');
    tag.className = 'ing-tag';
    tag.innerHTML = `${escHtml(ing.name)}<button onclick="removeIngredient(${ing.id}, this)">×</button>`;
    container.appendChild(tag);
    searchByIngredients();
}

window.removeIngredient = function(id, btn) {
    selectedIngredientIds = selectedIngredientIds.filter(i => i !== id);
    btn.parentElement.remove();
    searchByIngredients();
};

async function searchByIngredients() {
    if (selectedIngredientIds.length === 0) {
        renderRecipes(currentResults);
        return;
    }
    try {
        const ids = selectedIngredientIds.join(',');
        const res = await fetch(`/api/search/ingredients?ids=${ids}`);
        const data = await res.json();
        renderRecipes(data.recipes || []);
    } catch (e) {
        console.error(e);
    }
}