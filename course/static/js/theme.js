// Immediate execution to prevent flash of unstyled theme
(function () {
    const savedTheme = localStorage.getItem('smartcook-theme') || 'light';
    document.documentElement.setAttribute('data-theme', savedTheme);
})();

document.addEventListener('DOMContentLoaded', () => {
    const savedTheme = localStorage.getItem('smartcook-theme') || 'light';
    
    // Create the switcher HTML
    const switcherContainer = document.createElement('div');
    switcherContainer.className = 'theme-switcher-wrap';
    
    switcherContainer.innerHTML = `
        <button class="theme-btn" id="theme-menu-btn" aria-label="Змінити тему">
            <span class="theme-icon-current">☀️</span>
            <span class="theme-text-current">Тема</span>
            <span class="theme-arrow">▼</span>
        </button>
        <div class="theme-dropdown" id="theme-dropdown-menu">
            <div class="theme-option" data-val="light">
                <span class="theme-opt-icon">☀️</span>
                <span class="theme-opt-text">Шавлія & Золото</span>
            </div>
            <div class="theme-option" data-val="dark">
                <span class="theme-opt-icon">🌙</span>
                <span class="theme-opt-text">Обсидіан & Аметист</span>
            </div>
            <div class="theme-option" data-val="chrome">
                <span class="theme-opt-icon">✨</span>
                <span class="theme-opt-text">Рідкий Хром</span>
            </div>
        </div>
    `;
    
    // Find where to append it
    const navbar = document.querySelector('.navbar');
    if (navbar) {
        const logo = navbar.querySelector('.logo');
        if (logo) {
            logo.insertAdjacentElement('afterend', switcherContainer);
        } else {
            navbar.appendChild(switcherContainer);
        }
    } else {
        // If no navbar (e.g. login/register), float it in the top right
        switcherContainer.classList.add('floating-theme-switcher');
        document.body.appendChild(switcherContainer);
    }
    
    const btn = switcherContainer.querySelector('#theme-menu-btn');
    const dropdown = switcherContainer.querySelector('#theme-dropdown-menu');
    const options = switcherContainer.querySelectorAll('.theme-option');
    const currentIcon = switcherContainer.querySelector('.theme-icon-current');
    const currentText = switcherContainer.querySelector('.theme-text-current');
    
    const icons = {
        light: '☀️',
        dark: '🌙',
        chrome: '✨'
    };
    
    const names = {
        light: 'Шавлія',
        dark: 'Обсидіан',
        chrome: 'Хром'
    };
    
    // Update active visual state
    function updateSwitcherUI(theme) {
        currentIcon.textContent = icons[theme] || '☀️';
        currentText.textContent = names[theme] || 'Тема';
        
        options.forEach(opt => {
            if (opt.getAttribute('data-val') === theme) {
                opt.classList.add('active');
            } else {
                opt.classList.remove('active');
            }
        });
    }
    
    updateSwitcherUI(savedTheme);
    
    // Toggle dropdown
    btn.addEventListener('click', (e) => {
        e.stopPropagation();
        dropdown.classList.toggle('show');
    });
    
    // Select option
    options.forEach(opt => {
        opt.addEventListener('click', (e) => {
            const themeVal = opt.getAttribute('data-val');
            
            // Add transitioning class to animate colors smoothly
            document.documentElement.classList.add('theme-transitioning');
            
            localStorage.setItem('smartcook-theme', themeVal);
            document.documentElement.setAttribute('data-theme', themeVal);
            updateSwitcherUI(themeVal);
            dropdown.classList.remove('show');
            
            // Dispatch custom event for dynamic components to adjust if needed
            window.dispatchEvent(new CustomEvent('smartcook-theme-changed', { detail: themeVal }));
            
            setTimeout(() => {
                document.documentElement.classList.remove('theme-transitioning');
            }, 400);
        });
    });
    
    // Close dropdown on click outside
    document.addEventListener('click', () => {
        dropdown.classList.remove('show');
    });
});
