document.addEventListener('DOMContentLoaded', () => {
    const loginForm = document.getElementById('login-form');
    const loginError = document.getElementById('login-error');

    // Вхід через логін + пароль (Користувач / Адмін)
    loginForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        loginError.textContent = '';

        const username = document.getElementById('user-name').value.trim();
        const password = document.getElementById('user-password').value.trim();

        if (!username || !password) {
            loginError.textContent = 'Будь ласка, заповніть усі поля!';
            return;
        }

        try {
            const response = await fetch('/api/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });

            const data = await response.json();

            if (data.status === 'success') {
                window.location.href = '/';
            } else {
                loginError.textContent = data.message || 'Невірний логін або пароль';
            }
        } catch (err) {
            console.error('Помилка:', err);
            loginError.textContent = 'Сталася помилка з\'єднання з сервером.';
        }
    });

    // Вхід як гість
    const guestBtn = document.getElementById('guest-btn');
    if (guestBtn) {
        guestBtn.addEventListener('click', async () => {
            loginError.textContent = '';

            try {
                const response = await fetch('/guest-login', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({})
                });

                const data = await response.json();

                if (data.status === 'success') {
                    window.location.href = '/';
                } else {
                    loginError.textContent = 'Не вдалося увійти як гість';
                }
            } catch (err) {
                console.error('Помилка входу гостя:', err);
                loginError.textContent = 'Помилка з\'єднання з сервером';
            }
        });
    }
});