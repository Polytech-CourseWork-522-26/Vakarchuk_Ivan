document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('edit-form');

    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const id = document.getElementById('recipe-id').value;
        const data = {
            name: document.getElementById('name').value,
            description: document.getElementById('description').value,
            instructions: document.getElementById('instructions').value,
            calories: parseInt(document.getElementById('calories').value),
            video_url: document.getElementById('video_url').value
        };

        try {
            const res = await fetch(`/api/recipe/${id}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            });

            if (res.ok) {
                alert("✅ Рецепт успішно оновлено!");
                window.location.href = `/recipe/${id}`;
            } else {
                alert("Помилка при збереженні");
            }
        } catch (err) {
            console.error(err);
            alert("Помилка з'єднання");
        }
    });
});