document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('create-recipe-form');
    const submitBtn = document.getElementById('submit-btn');
    const errorSpan = document.getElementById('submit-error');
    const photoInput = document.getElementById('recipe-photo');

    let selectedIngredient = null;
    let addedIngredients = [];

    const searchInput = document.getElementById('ingredient-search-input');
    const searchDropdown = document.getElementById('search-dropdown');
    const qtyInput = document.getElementById('ingredient-qty');
    const addBtn = document.getElementById('add-ing-row-btn');
    const ingredientsContainer = document.getElementById('ingredients-list-container');

    // ===== PREVIEW ФОТО =====
    // Вставляємо блок preview одразу після input[type=file]
    const previewWrap = document.createElement('div');
    previewWrap.id = 'photo-preview-wrap';
    previewWrap.style.cssText = 'margin-top:10px; display:none;';
    previewWrap.innerHTML = `
        <img id="photo-preview-img"
             style="max-width:100%; max-height:220px; border-radius:12px; object-fit:cover; border:1px solid #eee;">
        <button type="button" id="remove-photo-btn"
                style="display:block; margin-top:6px; background:none; border:none; color:#ff3b30; cursor:pointer; font-size:13px;">
            ✕ Видалити фото
        </button>`;
    photoInput.parentElement.appendChild(previewWrap);

    photoInput.addEventListener('change', () => {
        const file = photoInput.files[0];
        if (!file) {
            previewWrap.style.display = 'none';
            photoInput.parentElement.classList.remove('has-file');
            return;
        }
        // Перевірка типу та розміру (макс 5 МБ)
        if (!file.type.startsWith('image/')) {
            errorSpan.textContent = 'Оберіть файл зображення (jpg, png, webp...)';
            photoInput.value = '';
            previewWrap.style.display = 'none';
            photoInput.parentElement.classList.remove('has-file');
            return;
        }
        if (file.size > 5 * 1024 * 1024) {
            errorSpan.textContent = 'Фото не може бути більше 5 МБ';
            photoInput.value = '';
            previewWrap.style.display = 'none';
            photoInput.parentElement.classList.remove('has-file');
            return;
        }
        errorSpan.textContent = '';
        const reader = new FileReader();
        reader.onload = (ev) => {
            document.getElementById('photo-preview-img').src = ev.target.result;
            previewWrap.style.display = 'block';
            photoInput.parentElement.classList.add('has-file');
        };
        reader.readAsDataURL(file);
    });

    document.getElementById('remove-photo-btn').addEventListener('click', (e) => {
        e.stopPropagation(); // Запобігаємо відкриттю діалогу вибору файлу при кліку на видалення
        photoInput.value = '';
        previewWrap.style.display = 'none';
        document.getElementById('photo-preview-img').src = '';
        photoInput.parentElement.classList.remove('has-file');
    });

    // ===== ПОШУК ІНГРЕДІЄНТІВ =====
    searchInput.addEventListener('input', () => {
        const query = searchInput.value.trim().toLowerCase();
        searchDropdown.innerHTML = '';

        if (query.length < 1) {
            searchDropdown.style.display = 'none';
            return;
        }

        const filtered = ALL_SYSTEM_INGREDIENTS.filter(i =>
            i.name.toLowerCase().includes(query)
        );

        filtered.forEach(item => {
            const div = document.createElement('div');
            div.className = 'dropdown-item';
            div.textContent = `${item.name} (${item.unit})`;
            div.onclick = () => {
                searchInput.value = item.name;
                selectedIngredient = item;
                qtyInput.focus();
                searchDropdown.style.display = 'none';
            };
            searchDropdown.appendChild(div);
        });

        searchDropdown.style.display = 'block';
    });

    // ===== ДОДАВАННЯ ІНГРЕДІЄНТА =====
    addBtn.addEventListener('click', () => {
        if (!selectedIngredient) {
            alert("Оберіть інгредієнт зі списку!");
            return;
        }
        const qty = parseFloat(qtyInput.value);
        if (!qty || qty <= 0) {
            alert("Вкажіть правильну кількість!");
            return;
        }
        if (addedIngredients.some(i => i.id === selectedIngredient.id)) {
            alert("Цей інгредієнт вже додано!");
            return;
        }
        addedIngredients.push({
            id: selectedIngredient.id,
            name: selectedIngredient.name,
            unit: selectedIngredient.unit,
            qty: qty
        });
        renderIngredients();
        searchInput.value = '';
        qtyInput.value = '';
        selectedIngredient = null;
    });

    function renderIngredients() {
        ingredientsContainer.innerHTML = addedIngredients.map((item, index) => `
            <div style="display:flex; justify-content:space-between; align-items:center; background:white; padding:10px; margin:5px 0; border-radius:8px; border:1px solid #eee;">
                <span><b>${item.name}</b> — ${item.qty} ${item.unit}</span>
                <button data-index="${index}" style="color:#ff3b30; font-size:18px; background:none; border:none; cursor:pointer;">✕</button>
            </div>
        `).join('');
    }

    ingredientsContainer.addEventListener('click', e => {
        if (e.target.tagName === 'BUTTON') {
            const index = parseInt(e.target.dataset.index);
            addedIngredients.splice(index, 1);
            renderIngredients();
        }
    });

    // ===== САБМІТ =====
    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        errorSpan.textContent = '';
        submitBtn.disabled = true;
        submitBtn.textContent = 'Збереження...';

        if (addedIngredients.length === 0) {
            alert("Додайте хоча б один інгредієнт!");
            submitBtn.disabled = false;
            submitBtn.textContent = 'Зберегти рецепт';
            return;
        }

        const fd = new FormData(form);
        fd.append('ingredients_json', JSON.stringify(addedIngredients));

        try {
            const res = await fetch('/create', { method: 'POST', body: fd });
            if (res.redirected || res.ok) {
                triggerConfetti();
                setTimeout(() => {
                    window.location.href = '/';
                }, 1000);
            } else {
                const data = await res.text();
                errorSpan.textContent = 'Помилка збереження';
                console.error(data);
            }
        } catch (err) {
            errorSpan.textContent = "Помилка з'єднання з сервером";
            console.error(err);
        } finally {
            // keep button disabled on success redirect to prevent double submission
        }
    });

    function triggerConfetti() {
        const colors = ['#ff6b35', '#2ec4b6', '#ffb703', '#ef4444', '#10b981'];
        for (let i = 0; i < 70; i++) {
            const dot = document.createElement('div');
            dot.style.position = 'fixed';
            dot.style.zIndex = '99999';
            dot.style.width = Math.random() * 8 + 6 + 'px';
            dot.style.height = Math.random() * 8 + 6 + 'px';
            dot.style.borderRadius = '50%';
            dot.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
            dot.style.left = '50%';
            dot.style.top = '40%';
            
            const angle = Math.random() * Math.PI * 2;
            const velocity = Math.random() * 250 + 150;
            const dx = Math.cos(angle) * velocity;
            const dy = Math.sin(angle) * velocity;
            
            dot.animate([
                { transform: 'translate(0, 0) scale(1)', opacity: 1 },
                { transform: `translate(${dx}px, ${dy - 50}px) scale(1.2)`, opacity: 0.9, offset: 0.3 },
                { transform: `translate(${dx * 1.4}px, ${dy + 400}px) scale(0.4)`, opacity: 0 }
            ], {
                duration: 1500,
                easing: 'cubic-bezier(0.1, 0.8, 0.2, 1)',
                fill: 'forwards'
            });
            
            document.body.appendChild(dot);
            setTimeout(() => dot.remove(), 1500);
        }
    }
});