document.addEventListener('DOMContentLoaded', () => {
    const registerForm = document.getElementById('main-form');
    const inputName = document.getElementById('user-name');
    const inputEmail = document.getElementById('user-email');
    const inputPassword = document.getElementById('user-password');
    const inputDiet = document.getElementById('user-diet');
    const inputAllergies = document.getElementById('user-allergies');

    function showError(errorId, message) {
        document.getElementById(errorId).textContent = message;
    }

    function clearErrors() {
        document.getElementById('name-error').textContent = '';
        document.getElementById('email-error').textContent = '';
        document.getElementById('password-error').textContent = '';
    }

    registerForm.addEventListener('submit', function(event) {
        event.preventDefault(); // Повністю блокуємо стандартне перезавантаження
        clearErrors();

        const nameValue = inputName.value.trim();
        const emailValue = inputEmail.value.trim();
        const passwordValue = inputPassword.value.trim();
        const dietValue = inputDiet.value.trim();
        const allergiesValue = inputAllergies.value.trim();

        let isFormValid = true;

        // Валідація логіну
        if (nameValue.length === 0) {
            showError('name-error', 'Поле логіну не може бути порожнім');
            isFormValid = false;
        } else if (nameValue.length < 3) {
            showError('name-error', 'Логін має містити не менше 3 символів');
            isFormValid = false;
        }

        // Валідація Email
        if (emailValue.length === 0) {
            showError('email-error', 'Поле пошти не може бути порожнім');
            isFormValid = false;
        } else if (!emailValue.includes('@')) {
            showError('email-error', 'Введіть коректний email (має містити @)');
            isFormValid = false;
        }

        // Валідація паролю
        if (passwordValue.length === 0) {
            showError('password-error', 'Пароль не може бути порожнім');
            isFormValid = false;
        } else if (passwordValue.length < 6) {
            showError('password-error', 'Пароль має містити не менше 6 символів');
            isFormValid = false;
        }

        if (!isFormValid) return; // Якщо є локальні помилки, на сервер не йдемо

        // Формуємо дані для відправки в Crow (стандартний URL-encoded формат)
        const formData = new URLSearchParams();
        formData.append('username', nameValue);
        formData.append('password', passwordValue);
        formData.append('email', emailValue);
        formData.append('diet', dietValue);
        formData.append('allergies', allergiesValue);

     // 3 POST
        fetch('/register', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: formData.toString()
        })
        .then(response => response.json().then(data => ({ status: response.status, body: data })))
        .then(resObj => {
            if (resObj.status === 200 && resObj.body.status === 'success') {
                console.log('Успішна реєстрація!');
                window.location.href = resObj.body.redirect; // Перенаправляємо на /login
            } else {
                // Якщо сервер повернув помилку (наприклад, логін зайнятий), виводимо її над полем користувача
                showError('name-error', resObj.body.message || 'Помилка реєстрації');
            }
        })
        .catch(err => {
            console.error('Помилка мережі:', err);
            showError('name-error', 'Не вдалося зв\'язатися з сервером');
        });
    });
});