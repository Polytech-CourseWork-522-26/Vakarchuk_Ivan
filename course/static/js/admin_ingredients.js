async function loadIngredients() {
    try {
        const res = await fetch('/api/ingredients');
        const data = await res.json();
        const tbody = document.getElementById('ing-tbody');

        if (!data.ingredients || data.ingredients.length === 0) {
            tbody.innerHTML = '<tr><td colspan="6" style="text-align:center; color:#888; padding:30px;">Інгредієнтів поки немає</td></tr>';
            return;
        }

        tbody.innerHTML = data.ingredients.map(ing => `
            <tr id="row-${ing.id}">
                <td>${ing.id}</td>
                <td><strong>${ing.name}</strong></td>
                <td>${ing.manufacturer || '—'}</td>
                <td>${ing.category}</td>
                <td>${ing.unit}</td>
                <td style="text-align: center;">
                    <button class="btn btn-danger btn-small" onclick="deleteIngredient(${ing.id})" title="Видалити" style="padding: 6px 12px; height: auto;">🗑️ Видалити</button>
                </td>
            </tr>
        `).join('');
    } catch (e) {
        console.error("Помилка завантаження інгредієнтів:", e);
    }
}

async function addIngredient() {
    const name         = document.getElementById('ing-name').value.trim();
    const manufacturer = document.getElementById('ing-manufacturer').value.trim();
    const category     = document.getElementById('ing-category').value.trim();
    const unit         = document.getElementById('ing-unit').value;
    const msg          = document.getElementById('add-message');

    if (!name || !category) {
        msg.style.color = 'var(--danger)';
        msg.textContent = 'Назва та категорія обовʼязкові!';
        return;
    }

    try {
        const res = await fetch('/api/ingredients', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ name, manufacturer, category, unit })
        });

        const data = await res.json();
        if (data.status === 'success') {
            msg.style.color = 'var(--success)';
            msg.textContent = '✅ Інгредієнт додано!';
            document.getElementById('ing-name').value = '';
            document.getElementById('ing-manufacturer').value = '';
            document.getElementById('ing-category').value = '';
            loadIngredients();
            setTimeout(() => msg.textContent = '', 3000);
        } else {
            msg.style.color = 'var(--danger)';
            msg.textContent = '❌ ' + (data.message || 'Помилка');
        }
    } catch (e) {
        console.error("Помилка додавання:", e);
    }
}

async function deleteIngredient(id) {
    if (!confirm('Видалити інгредієнт?')) return;

    try {
        const res = await fetch(`/api/ingredients/${id}`, { method: 'DELETE' });
        const data = await res.json();

        if (data.status === 'success') {
            document.getElementById(`row-${id}`)?.remove();
        } else {
            alert('Помилка видалення');
        }
    } catch (e) {
        console.error("Помилка видалення:", e);
    }
}

loadIngredients();