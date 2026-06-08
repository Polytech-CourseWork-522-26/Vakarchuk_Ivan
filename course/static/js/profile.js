let isEditMode = false;

document.addEventListener('DOMContentLoaded', () => {
    loadProfile();
});

async function loadProfile() {
    try {
        const res = await fetch('/api/profile');
        if (!res.ok) throw new Error();
        
        const user = await res.json();

        // Заповнюємо режим перегляду
        document.getElementById('view-email').textContent = user.email || '—';
        document.getElementById('view-diet').textContent = user.diet || 'Без обмежень';
        document.getElementById('view-allergies').textContent = user.allergies || 'Немає';

        // Заповнюємо форму редагування
        document.getElementById('email').value = user.email || '';
        document.getElementById('diet').value = user.diet || '';
        document.getElementById('allergies').value = user.allergies || '';
    } catch (e) {
        console.error("Не вдалося завантажити профіль", e);
    }
}

function switchToEdit() {
    document.getElementById('view-mode').style.display = 'none';
    document.getElementById('edit-mode').style.display = 'block';
    isEditMode = true;
}

function switchToView() {
    document.getElementById('view-mode').style.display = 'block';
    document.getElementById('edit-mode').style.display = 'none';
    isEditMode = false;
    loadProfile(); // оновлюємо дані
}

// Обробка збереження
document.getElementById('profile-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const messageSpan = document.getElementById('message');
    messageSpan.textContent = '';

    const email = document.getElementById('email').value.trim();
    const diet = document.getElementById('diet').value.trim();
    const allergies = document.getElementById('allergies').value.trim();

    if (!email || !email.includes('@')) {
        showMessage("Введіть коректний email!", "red");
        return;
    }

    try {
        const response = await fetch('/api/profile', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ email, diet, allergies })
        });

        const data = await response.json();

        if (data.status === 'success') {
            showMessage("✅ Профіль успішно оновлено!", "green");
            setTimeout(() => {
                switchToView();
            }, 1500);
        } else {
            showMessage("❌ Помилка при збереженні", "red");
        }
    } catch (err) {
        showMessage("❌ Помилка з'єднання з сервером", "red");
        console.error(err);
    }
});

function showMessage(text, color) {
    const msg = document.getElementById('message');
    msg.style.color = color;
    msg.textContent = text;
    setTimeout(() => { msg.textContent = ''; }, 5000);
}