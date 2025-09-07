
#ifndef APP_WEB_PAGE_H
#define APP_WEB_PAGE_H
const char WebPage[] PROGMEM = R"html(<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dotmatrix Digital Clock</title>
    <style>
        :root {
            --primary-color: #6366f1;
            --primary-hover: #4f46e5;
            --success-color: #10b981;
            --warning-color: #f59e0b;
            --error-color: #ef4444;
            --bg-primary: #ffffff;
            --bg-secondary: #f8fafc;
            --bg-tertiary: #f1f5f9;
            --text-primary: #1e293b;
            --text-secondary: #64748b;
            --border-color: #e2e8f0;
            --shadow: 0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1);
            --shadow-lg: 0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1);
        }

        [data-theme="dark"] {
            --bg-primary: #0f172a;
            --bg-secondary: #1e293b;
            --bg-tertiary: #334155;
            --text-primary: #f8fafc;
            --text-secondary: #cbd5e1;
            --border-color: #475569;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg-primary);
            color: var(--text-primary);
            line-height: 1.6;
            transition: background-color 0.3s ease, color 0.3s ease;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 2rem;
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 3rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid var(--border-color);
        }

        .header h1 {
            font-size: 2rem;
            font-weight: 700;
            background: linear-gradient(135deg, var(--primary-color), var(--primary-hover));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }

        .theme-toggle {
            background: var(--bg-secondary);
            border: 1px solid var(--border-color);
            border-radius: 0.5rem;
            padding: 0.5rem;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        .theme-toggle:hover {
            background: var(--bg-tertiary);
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 2rem;
            margin-bottom: 2rem;
        }

        .card {
            background: var(--bg-secondary);
            border: 1px solid var(--border-color);
            border-radius: 1rem;
            padding: 2rem;
            box-shadow: var(--shadow);
            transition: all 0.3s ease;
        }

        .card:hover {
            box-shadow: var(--shadow-lg);
            transform: translateY(-2px);
        }

        .card h2 {
            font-size: 1.5rem;
            font-weight: 600;
            margin-bottom: 1.5rem;
            color: var(--text-primary);
        }

        .form-group {
            margin-bottom: 1.5rem;
        }

        .form-group label {
            display: block;
            font-weight: 500;
            margin-bottom: 0.5rem;
            color: var(--text-primary);
        }

        .form-control {
            width: 100%;
            padding: 0.75rem 1rem;
            border: 1px solid var(--border-color);
            border-radius: 0.5rem;
            background: var(--bg-primary);
            color: var(--text-primary);
            font-size: 1rem;
            transition: all 0.3s ease;
        }

        .form-control:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px rgb(99 102 241 / 0.1);
        }

        .checkbox-group {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            margin-bottom: 1rem;
        }

        .checkbox-group input[type="checkbox"] {
            width: 1.25rem;
            height: 1.25rem;
            accent-color: var(--primary-color);
        }

        .checkbox-group label {
            margin-bottom: 0;
            cursor: pointer;
        }

        .radio-group {
            display: flex;
            gap: 1rem;
            margin-bottom: 1rem;
        }

        .radio-group label {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            cursor: pointer;
        }

        .radio-group input[type="radio"] {
            width: 1.25rem;
            height: 1.25rem;
            accent-color: var(--primary-color);
        }

        .btn {
            background: var(--primary-color);
            color: white;
            border: none;
            padding: 0.75rem 1.5rem;
            border-radius: 0.5rem;
            font-size: 1rem;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s ease;
            text-decoration: none;
            display: inline-block;
        }

        .btn:hover {
            background: var(--primary-hover);
            transform: translateY(-1px);
        }

        .btn-secondary {
            background: var(--bg-tertiary);
            color: var(--text-primary);
            border: 1px solid var(--border-color);
        }

        .btn-secondary:hover {
            background: var(--border-color);
        }

        .btn-success {
            background: var(--success-color);
        }

        .btn-success:hover {
            background: #059669;
        }

        /* Popup overlay styles */
        .popup-overlay {
            position: fixed;
            bottom: 2rem;
            left: 50%;
            transform: translateX(-50%);
            z-index: 1000;
            display: none;
            animation: slideUp 0.3s ease-out;
        }

        .popup {
            background: var(--bg-primary);
            border: 1px solid var(--border-color);
            border-radius: 0.75rem;
            box-shadow: var(--shadow-lg);
            padding: 1rem 1.5rem;
            min-width: 300px;
            max-width: 400px;
            text-align: center;
            position: relative;
        }

        .popup.success {
            border-color: var(--success-color);
            background: rgb(16 185 129 / 0.05);
        }

        .popup.error {
            border-color: var(--error-color);
            background: rgb(239 68 68 / 0.05);
        }

        .popup.loading {
            border-color: var(--primary-color);
            background: rgb(99 102 241 / 0.05);
        }

        .popup-content {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.75rem;
        }

        .popup-icon {
            width: 1.25rem;
            height: 1.25rem;
            flex-shrink: 0;
        }

        .popup-icon.success {
            color: var(--success-color);
        }

        .popup-icon.error {
            color: var(--error-color);
        }

        .popup-icon.loading {
            color: var(--primary-color);
        }

        .popup-message {
            font-weight: 500;
            color: var(--text-primary);
        }

        .popup-spinner {
            border: 2px solid var(--border-color);
            border-top: 2px solid var(--primary-color);
            border-radius: 50%;
            width: 1.25rem;
            height: 1.25rem;
            animation: spin 1s linear infinite;
            flex-shrink: 0;
        }

        @keyframes slideUp {
            from {
                opacity: 0;
                transform: translateX(-50%) translateY(1rem);
            }
            to {
                opacity: 1;
                transform: translateX(-50%) translateY(0);
            }
        }

        @keyframes slideDown {
            from {
                opacity: 1;
                transform: translateX(-50%) translateY(0);
            }
            to {
                opacity: 0;
                transform: translateX(-50%) translateY(1rem);
            }
        }

        .popup-overlay.hiding {
            animation: slideDown 0.3s ease-in;
        }

        /* Autocomplete styles */
        .autocomplete-container {
            position: relative;
        }

        .autocomplete-dropdown {
            position: absolute;
            top: 100%;
            left: 0;
            right: 0;
            background: var(--bg-primary);
            border: 1px solid var(--border-color);
            border-top: none;
            border-radius: 0 0 0.5rem 0.5rem;
            max-height: 200px;
            overflow-y: auto;
            z-index: 1000;
            display: none;
        }

        .autocomplete-item {
            padding: 0.75rem 1rem;
            cursor: pointer;
            border-bottom: 1px solid var(--border-color);
            transition: background-color 0.2s ease;
        }

        .autocomplete-item:hover {
            background: var(--bg-secondary);
        }

        .autocomplete-item:last-child {
            border-bottom: none;
        }

        .city-info {
            margin-top: 8px;
            font-size: 0.9em;
            color: var(--text-secondary);
        }
        
        /* System Information Styles */
        .system-info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .info-card {
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 20px;
            display: flex;
            align-items: center;
            gap: 16px;
            transition: all 0.3s ease;
        }
        
        .info-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.1);
        }
        
        .info-icon {
            font-size: 2em;
            width: 50px;
            text-align: center;
        }
        
        .info-content {
            flex: 1;
        }
        
        .info-label {
            font-size: 0.9em;
            color: var(--text-secondary);
            margin-bottom: 4px;
            font-weight: 500;
        }
        
        .info-value {
            font-size: 1.1em;
            font-weight: 600;
            color: var(--text-primary);
            margin-bottom: 2px;
        }
        
        .info-sub {
            font-size: 0.8em;
            color: var(--text-tertiary);
        }
        
        .btn-secondary {
            background: var(--secondary-color);
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1em;
            transition: all 0.3s ease;
        }
        
        .btn-secondary:hover {
            background: var(--secondary-hover);
            transform: translateY(-1px);
        }
        
        /* Spinner and Loading States */
        .spinner {
            display: inline-block;
            width: 16px;
            height: 16px;
            border: 2px solid transparent;
            border-top: 2px solid currentColor;
            border-radius: 50%;
            animation: spin 1s linear infinite;
            margin-right: 8px;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        .btn:disabled {
            opacity: 0.7;
            cursor: not-allowed;
            transform: none !important;
        }

        @media (max-width: 768px) {
            .container {
                padding: 1rem;
            }
            
            .grid {
                grid-template-columns: 1fr;
            }
            
            .header {
                flex-direction: column;
                gap: 1rem;
                text-align: center;
            }
        }

        .icon {
            width: 1.25rem;
            height: 1.25rem;
            fill: currentColor;
        }
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            <h1>Dotmatrix Digital Clock</h1>
            <button class="theme-toggle" onclick="toggleTheme()" title="Toggle theme">
                <svg class="icon" id="theme-icon" viewBox="0 0 24 24">
                    <path d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"/>
                </svg>
            </button>
        </header>

        <!-- Popup Overlay -->
        <div id="popup-overlay" class="popup-overlay">
            <div id="popup" class="popup">
                <div class="popup-content">
                    <div id="popup-icon" class="popup-icon"></div>
                    <span id="popup-message" class="popup-message"></span>
                </div>
            </div>
        </div>

        <div class="grid">
            <!-- Display Options -->
            <div class="card">
                <h2>Display Options</h2>
                <form id="operation_mode_form">
                    <div class="checkbox-group">
                        <input type="checkbox" id="clock-checkbox" name="operation-mode" checked>
                        <label for="clock-checkbox">Clock</label>
                    </div>
                    <div class="checkbox-group">
                        <input type="checkbox" id="date-checkbox" name="operation-mode">
                        <label for="date-checkbox">Date</label>
                    </div>
                    <div class="checkbox-group">
                        <input type="checkbox" id="weather-checkbox" name="operation-mode">
                        <label for="weather-checkbox">Weather</label>
                    </div>
                    <div class="checkbox-group">
                        <input type="checkbox" id="snake-checkbox" name="operation-mode">
                        <label for="snake-checkbox">Snake Game</label>
                    </div>
                    <div class="checkbox-group">
                        <input type="checkbox" id="ip-checkbox" name="operation-mode">
                        <label for="ip-checkbox">IP Address</label>
                    </div>
                    <div class="checkbox-group">
                        <input type="checkbox" id="message-checkbox" name="operation-mode">
                        <label for="message-checkbox">Message</label>
                    </div>
                    <div class="form-group">
                        <input type="text" id="message-input" class="form-control" placeholder="Enter your message..." maxlength="255">
                    </div>
                </form>
            </div>
        
        <!-- General Settings -->
        <div class="card">
            <h2>General Settings</h2>
            <form id="general_settings_form">
                <input type="hidden" id="timezone" value="">
                <div class="form-group">
                    <label for="city-input">Location</label>
                    <div class="autocomplete-container">
                        <input type="text" id="city-input" class="form-control" placeholder="Enter city name..." autocomplete="off">
                        <div id="city-dropdown" class="autocomplete-dropdown"></div>
                    </div>
                    <div id="city-info" class="city-info"></div>
                </div>
                <div class="form-group">
                    <label for="brightness">Display Brightness</label>
                    <input type="range" id="brightness" class="form-control" min="1" max="15" value="15">
                </div>
                <div class="form-group">
                    <label>Temperature Units</label>
                    <div class="radio-group">
                        <label>
                            <input type="radio" id="celsius-radio" name="weather-units" value="c">
                            Celsius
                        </label>
                        <label>
                            <input type="radio" id="fahrenheit-radio" name="weather-units" value="f">
                            Fahrenheit
                        </label>
                    </div>
                </div>
                <div class="form-group">
                    <label for="mdns-domain">Network Name (MDNS Domain)</label>
                    <input type="text" id="mdns-domain" class="form-control" placeholder="digiclk" maxlength="63">
                    <small class="form-text">Used for accessing the device via http://[name].local/ (requires restart to take effect)</small>
                </div>
            </form>
        </div>
        
        <!-- System Information Section -->
        <div class="card">
            <h2>System Information</h2>
            <div class="system-info-grid">
                <div class="info-card">
                    <div class="info-icon">📶</div>
                    <div class="info-content">
                        <div class="info-label">WiFi Status</div>
                        <div class="info-value" id="wifi-status">Loading...</div>
                        <div class="info-sub" id="wifi-ssid">-</div>
                    </div>
                </div>
                
                <div class="info-card">
                    <div class="info-icon">🌐</div>
                    <div class="info-content">
                        <div class="info-label">IP Address</div>
                        <div class="info-value" id="ip-address">Loading...</div>
                        <div class="info-sub" id="wifi-strength">-</div>
                    </div>
                </div>
                
                <div class="info-card">
                    <div class="info-icon">⏱️</div>
                    <div class="info-content">
                        <div class="info-label">Uptime</div>
                        <div class="info-value" id="uptime">Loading...</div>
                        <div class="info-sub" id="uptime-seconds">-</div>
                    </div>
                </div>
                
                <div class="info-card">
                    <div class="info-icon">💾</div>
                    <div class="info-content">
                        <div class="info-label">Memory</div>
                        <div class="info-value" id="memory-usage">Loading...</div>
                        <div class="info-sub" id="memory-details">-</div>
                    </div>
                </div>
                
                <div class="info-card">
                    <div class="info-icon">💿</div>
                    <div class="info-content">
                        <div class="info-label">Storage</div>
                        <div class="info-value" id="storage-usage">Loading...</div>
                        <div class="info-sub" id="storage-details">-</div>
                    </div>
                </div>
                
                <div class="info-card">
                    <div class="info-icon">🔧</div>
                    <div class="info-content">
                        <div class="info-label">Hardware</div>
                        <div class="info-value" id="chip-model">Loading...</div>
                        <div class="info-sub" id="cpu-freq">-</div>
                    </div>
                </div>
            </div>
            
            <button type="button" class="btn btn-secondary" onclick="refreshSystemInfo()">Refresh System Info</button>
        </div>
    </div>

    <script>
        let currentTheme = localStorage.getItem('theme') || 'light';
        let selectedCity = null;
        let searchTimeout = null;
        
        function toggleTheme() {
            currentTheme = currentTheme === 'light' ? 'dark' : 'light';
            document.documentElement.setAttribute('data-theme', currentTheme);
            localStorage.setItem('theme', currentTheme);
            updateThemeIcon();
        }

        function updateThemeIcon() {
            const icon = document.getElementById('theme-icon');
            if (currentTheme === 'dark') {
                icon.innerHTML = '<path d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"/>';
            } else {
                icon.innerHTML = '<path d="M12 2.25a.75.75 0 01.75.75v2.25a.75.75 0 01-1.5 0V3a.75.75 0 01.75-.75zM7.5 12a4.5 4.5 0 119 0 4.5 4.5 0 01-9 0zM18.894 6.166a.75.75 0 00-1.06-1.06l-1.591 1.59a.75.75 0 101.06 1.061l1.591-1.59zM21.75 12a.75.75 0 01-.75.75h-2.25a.75.75 0 010-1.5H21a.75.75 0 01.75.75zM17.834 18.894a.75.75 0 001.06-1.06l-1.59-1.591a.75.75 0 10-1.061 1.06l1.59 1.591zM12 18a.75.75 0 01.75.75V21a.75.75 0 01-1.5 0v-2.25A.75.75 0 0112 18zM7.758 17.303a.75.75 0 00-1.061-1.06l-1.591 1.59a.75.75 0 001.06 1.061l1.591-1.59zM6 12a.75.75 0 01-.75.75H3a.75.75 0 010-1.5h2.25A.75.75 0 016 12zM6.697 7.757a.75.75 0 001.06-1.06l-1.59-1.591a.75.75 0 00-1.061 1.06l1.59 1.591z"/>';
            }
        }

        function showStatus(message, type = 'success') {
            const overlay = document.getElementById('popup-overlay');
            const popup = document.getElementById('popup');
            const icon = document.getElementById('popup-icon');
            const messageEl = document.getElementById('popup-message');
            
            // Remove any existing classes
            popup.className = 'popup';
            icon.className = 'popup-icon';
            
            // Set content and styling based on type
            messageEl.textContent = message;
            
            if (type === 'success') {
                popup.classList.add('success');
                icon.classList.add('success');
                icon.innerHTML = '<svg viewBox="0 0 24 24"><path fill="currentColor" d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"/></svg>';
            } else if (type === 'error') {
                popup.classList.add('error');
                icon.classList.add('error');
                icon.innerHTML = '<svg viewBox="0 0 24 24"><path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/></svg>';
            }
            
            // Show popup
            overlay.style.display = 'block';
            
            // Auto-hide after 3 seconds
            setTimeout(() => {
                hidePopup();
            }, 3000);
        }

        function showLoading(message = 'Loading...') {
            const overlay = document.getElementById('popup-overlay');
            const popup = document.getElementById('popup');
            const icon = document.getElementById('popup-icon');
            const messageEl = document.getElementById('popup-message');
            
            // Remove any existing classes
            popup.className = 'popup';
            icon.className = 'popup-icon';
            
            // Set loading state
            popup.classList.add('loading');
            icon.classList.add('loading');
            icon.innerHTML = '<div class="popup-spinner"></div>';
            messageEl.textContent = message;
            
            // Show popup
            overlay.style.display = 'block';
        }

        function hidePopup() {
            const overlay = document.getElementById('popup-overlay');
            overlay.classList.add('hiding');
            
            // Wait for animation to complete before hiding
            setTimeout(() => {
                overlay.style.display = 'none';
                overlay.classList.remove('hiding');
            }, 300);
        }

      function submitModeOfOperation() {
            const form = document.getElementById('operation_mode_form');
            const submitBtn = document.querySelector('#operation_mode_form .btn');
            let strLine = "";
            
            if (form["clock-checkbox"].checked) {
                strLine += "/&CLOCK";
            }
            if (form["date-checkbox"].checked) {
                strLine += "/&DATE";
            }
            if (form["weather-checkbox"].checked) {
                strLine += "/&WEATHER";
            }
            if (form["snake-checkbox"].checked) {
                strLine += "/&SNAKE";
            }
            if (form["ip-checkbox"].checked) {
                strLine += "/&IP";
            }
            if (form["message-checkbox"].checked) {
                const message = form["message-input"].value.trim();
                if (message === "") {
                    showStatus("Message cannot be empty when selected.", "error");
                    return;
                }
                strLine += "/&MSG=" + encodeURIComponent(message);
            }
            
            if (strLine === "") {
                showStatus("Please select at least one display mode.", "error");
                return;
            }
            
            // Show loading state
            const originalText = submitBtn.textContent;
            submitBtn.textContent = "Updating...";
            submitBtn.disabled = true;
            submitBtn.innerHTML = '<span class="spinner"></span> Updating...';
            
            // Add nocache parameter to prevent caching
            const nocache = "/&nocache=" + Math.random() * 1000000;
            
            fetch(strLine + nocache)
                .then(response => {
                    if (response.ok) {
                        showStatus("Display modes updated successfully!", "success");
                    } else {
                        showStatus("Failed to update display modes.", "error");
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    showStatus("Error updating display modes.", "error");
                })
                .finally(() => {
                    // Restore button state
                    submitBtn.textContent = originalText;
                    submitBtn.disabled = false;
                    submitBtn.innerHTML = originalText;
                });
        }

      function setCntl(settings) {
        for (const key in settings) {
                if (settings[key] === "" || settings[key] === undefined || settings[key] === null) {
                    showStatus("Please fill out all fields.", "error");
            return;
          }
        }
            const strLine = `&CNTL=${encodeURIComponent(JSON.stringify(settings))}`;
        const nocache = "/&nocache=" + Math.random() * 1000000;
            
            showLoading("Updating settings...");
            fetch(strLine + nocache)
                .then(() => {
                    showStatus("Settings updated successfully!");
                })
                .catch(() => {
                    showStatus("Failed to update settings.", "error");
                });
        }

        function updateGeneralSettings() {
            const btn = document.querySelector('#general_settings_form .btn');
            const originalText = btn.textContent;
            
            // Show loading state
            btn.textContent = "Updating...";
            btn.disabled = true;
            btn.innerHTML = '<span class="spinner"></span> Updating...';
            
            const cityInput = document.getElementById('city-input');
            const brightnessSlider = document.getElementById('brightness');
            const celsiusRadio = document.getElementById('celsius-radio');
            const mdnsDomainInput = document.getElementById('mdns-domain');
            
            // Get city coordinates from the city info data attributes
            const cityInfo = document.getElementById('city-info');
            let latitude = null;
            let longitude = null;
            
            if (cityInfo && cityInfo.hasAttribute('data-latitude') && cityInfo.hasAttribute('data-longitude')) {
                latitude = parseFloat(cityInfo.getAttribute('data-latitude'));
                longitude = parseFloat(cityInfo.getAttribute('data-longitude'));
            }
            
            const settings = {
                brightness: parseInt(brightnessSlider.value),
                weatherUnits: celsiusRadio.checked ? 'c' : 'f'
            };
            
            // Add MDNS domain if provided
            if (mdnsDomainInput.value.trim()) {
                settings.mdnsDomain = mdnsDomainInput.value.trim();
            }
            
            // Only add coordinates if we have them
            if (latitude !== null && longitude !== null) {
                settings.latitude = latitude;
                settings.longitude = longitude;
            }
            
            // Use GET request with URL-encoded JSON
            const jsonString = JSON.stringify(settings);
            const encodedJson = encodeURIComponent(jsonString);
            const url = `/&CNTL=${encodedJson}`;
            
            fetch(url)
            .then(response => {
                if (response.ok) {
                    showStatus('Settings updated successfully!', 'success');
                } else {
                    showStatus('Failed to update settings.', 'error');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                showStatus('Error updating settings.', 'error');
            })
            .finally(() => {
                // Restore button state
                btn.textContent = originalText;
                btn.disabled = false;
                btn.innerHTML = originalText;
            });
        }

        // City search functionality
        function searchCities(query) {
            if (query.length < 2) {
                hideCityDropdown();
                return;
            }

            const url = `https://geocoding-api.open-meteo.com/v1/search?name=${encodeURIComponent(query)}&count=10&language=en&format=json`;
            
            fetch(url)
                .then(response => response.json())
                .then(data => {
                    if (data.results && data.results.length > 0) {
                        showCityDropdown(data.results);
                    } else {
                        hideCityDropdown();
                    }
                })
                .catch(error => {
                    console.error('Error searching cities:', error);
                    hideCityDropdown();
                });
        }

        function showCityDropdown(cities) {
            const dropdown = document.getElementById('city-dropdown');
            dropdown.innerHTML = '';
            
            cities.forEach(city => {
                const item = document.createElement('div');
                item.className = 'autocomplete-item';
                item.innerHTML = `
                    <div><strong>${city.name}</strong></div>
                    <div style="font-size: 0.875rem; color: var(--text-secondary);">
                        ${city.country}${city.admin1 ? ', ' + city.admin1 : ''}
                    </div>
                `;
                item.onclick = () => selectCity(city);
                dropdown.appendChild(item);
            });
            
            dropdown.style.display = 'block';
        }

        function hideCityDropdown() {
            const dropdown = document.getElementById('city-dropdown');
            dropdown.style.display = 'none';
        }

        function selectCity(city) {
            document.getElementById('city-input').value = city.name;
            document.getElementById('city-dropdown').style.display = 'none';
            
            // Show city info
            const cityInfo = document.getElementById('city-info');
            cityInfo.innerHTML = `
                <div class="city-details">
                    <strong>${city.name}, ${city.country}</strong><br>
                    Coordinates: ${city.latitude}, ${city.longitude}<br>
                    Timezone: ${city.timezone || 'Auto-detected'}
                </div>
            `;
            
            // Store coordinates as data attributes for display purposes
            cityInfo.setAttribute('data-latitude', city.latitude);
            cityInfo.setAttribute('data-longitude', city.longitude);
            
            // Update timezone if available
            if (city.timezone) {
                document.getElementById('timezone').value = city.timezone;
            }
            
            // Send only the coordinates to the ESP32
            updateCoordinates(city.latitude, city.longitude);
        }

      function getSettings() {
        return fetch("/&SETT")
          .then((response) => response.json())
          .then((data) => {
                    return data;
                })
                .catch((error) => {
                    console.error("Error fetching settings:", error);
                    showStatus("Failed to load settings.", "error");
                    return {}; // Return an empty object on error
                });
        }

        // Load general settings on page load
        loadGeneralSettings();
        
        // Load display modes on page load
        loadDisplayModes();
        
        // Load system information on page load
        loadSystemInfo();
        
        // Auto-refresh system info every 30 seconds
        setInterval(loadSystemInfo, 30000);
        
        // Add event listeners for automatic updates
        document.addEventListener('DOMContentLoaded', function() {
            // Display mode checkboxes - update immediately when changed
            const modeCheckboxes = document.querySelectorAll('#operation_mode_form input[type="checkbox"]');
            modeCheckboxes.forEach(checkbox => {
                checkbox.addEventListener('change', function() {
                    updateDisplayModes();
                });
            });
            
            // Message input - update with debouncing
            const messageInput = document.getElementById('message-input');
            if (messageInput) {
                let messageTimeout;
                messageInput.addEventListener('input', function() {
                    clearTimeout(messageTimeout);
                    messageTimeout = setTimeout(() => {
                        updateDisplayModes();
                    }, 500); // Wait 500ms after user stops typing
                });
            }
            
            // Brightness slider - update with debouncing, send only brightness
            const brightnessSlider = document.getElementById('brightness');
            if (brightnessSlider) {
                let brightnessTimeout;
                brightnessSlider.addEventListener('input', function() {
                    clearTimeout(brightnessTimeout);
                    brightnessTimeout = setTimeout(() => {
                        updateSingleSetting('brightness', parseInt(this.value));
                    }, 300); // Wait 300ms after user stops moving slider
                });
            }
            
            // Temperature unit radios - update immediately when changed, send only weatherUnits
            const tempRadios = document.querySelectorAll('input[name="weather-units"]');
            tempRadios.forEach(radio => {
                radio.addEventListener('change', function() {
                    updateSingleSetting('weatherUnits', this.value);
                });
            });
            
            // MDNS domain input - update when losing focus (blur event)
            const mdnsDomainInput = document.getElementById('mdns-domain');
            if (mdnsDomainInput) {
                mdnsDomainInput.addEventListener('blur', function() {
                    const domainValue = this.value.trim();
                    if (domainValue) {
                        updateSingleSetting('mdnsDomain', domainValue);
                    }
                });
            }
            
            // Initialize theme
            document.documentElement.setAttribute('data-theme', currentTheme);
            updateThemeIcon();
            
            // City search input
            const cityInput = document.getElementById('city-input');
            if (cityInput) {
                let searchTimeout;
                cityInput.addEventListener('input', function() {
                    clearTimeout(searchTimeout);
                    searchTimeout = setTimeout(() => {
                        searchCities(this.value);
                    }, 300);
                });
            }
            
            // Hide dropdown when clicking outside
            document.addEventListener('click', function(e) {
                if (!e.target.closest('.autocomplete-container')) {
                    hideCityDropdown();
                }
            });
        });
        
        // Debounce function for general use
        function debounce(func, wait) {
            let timeout;
            return function executedFunction(...args) {
                const later = () => {
                    clearTimeout(timeout);
                    func(...args);
                };
                clearTimeout(timeout);
                timeout = setTimeout(later, wait);
            };
        }
        
        function loadDisplayModes() {
            getSettings()
                .then(settings => {
                    if (settings.activeCards && Array.isArray(settings.activeCards)) {
                        // Reset all checkboxes first
                        document.getElementById('clock-checkbox').checked = false;
                        document.getElementById('date-checkbox').checked = false;
                        document.getElementById('weather-checkbox').checked = false;
                        document.getElementById('snake-checkbox').checked = false;
                        document.getElementById('ip-checkbox').checked = false;
                        document.getElementById('message-checkbox').checked = false;
                        
                        // Check the active ones
                        settings.activeCards.forEach(mode => {
                            switch(mode) {
                                case 'CLOCK':
                                    document.getElementById('clock-checkbox').checked = true;
                                    break;
                                case 'DATE':
                                    document.getElementById('date-checkbox').checked = true;
                                    break;
                                case 'WEATHER':
                                    document.getElementById('weather-checkbox').checked = true;
                                    break;
                                case 'SNAKE':
                                    document.getElementById('snake-checkbox').checked = true;
                                    break;
                                case 'IP':
                                    document.getElementById('ip-checkbox').checked = true;
                                    break;
                                case 'MESSAGE':
                                    document.getElementById('message-checkbox').checked = true;
                                    break;
                            }
                        });
                    }
                })
                .catch(error => {
                    console.error('Error loading display modes:', error);
                });
        }
        
        function loadGeneralSettings() {
            getSettings()
                .then(settings => {
                    // Set brightness
                    const brightnessSlider = document.getElementById('brightness');
                    if (brightnessSlider && settings.brightness) {
                        brightnessSlider.value = settings.brightness;
                    }
                    
                    // Set temperature units - use the correct field name from ESP32
                    const celsiusRadio = document.getElementById('celsius-radio');
                    const fahrenheitRadio = document.getElementById('fahrenheit-radio');
                    if (settings.weatherUnits === 'c' && celsiusRadio) {
                        celsiusRadio.checked = true;
                    } else if (settings.weatherUnits === 'f' && fahrenheitRadio) {
                        fahrenheitRadio.checked = true;
                    }
                    
                    // Set MDNS domain
                    const mdnsDomainInput = document.getElementById('mdns-domain');
                    if (mdnsDomainInput && settings.mdnsDomain) {
                        mdnsDomainInput.value = settings.mdnsDomain;
                    }
                    
                    // Set city info if available - use the correct field names from ESP32
                    if (settings.latitude !== undefined && settings.longitude !== undefined && 
                        settings.latitude !== 2147483647 && settings.longitude !== 2147483647) {
                        
                        // Show city info
                        const cityInfo = document.getElementById('city-info');
                        if (cityInfo) {
                            cityInfo.innerHTML = `
                                <div class="city-details">
                                    <strong>Location Set</strong><br>
                                    Coordinates: ${settings.latitude}, ${settings.longitude}<br>
                                    Timezone: ${settings.timezone || 'Auto-detected'}
                                </div>
                            `;
                            
                            // Store coordinates as data attributes for easy access
                            cityInfo.setAttribute('data-latitude', settings.latitude);
                            cityInfo.setAttribute('data-longitude', settings.longitude);
                        }
                    }
                })
                .catch(error => {
                    console.error('Error loading general settings:', error);
                });
        }
        
        function loadSystemInfo() {
            fetch('/&SYSINFO')
                .then(response => response.json())
                .then(data => {
                    // WiFi Status
                    document.getElementById('wifi-status').textContent = data.wifi_connected ? 'Connected' : 'Disconnected';
                    document.getElementById('wifi-ssid').textContent = data.wifi_ssid || 'Unknown';
                    
                    // IP Address
                    document.getElementById('ip-address').textContent = data.ip_address || 'Unknown';
                    const rssi = data.wifi_rssi || 0;
                    let strength = 'Unknown';
                    if (rssi >= -50) strength = 'Excellent';
                    else if (rssi >= -60) strength = 'Good';
                    else if (rssi >= -70) strength = 'Fair';
                    else if (rssi >= -80) strength = 'Poor';
                    else if (rssi < -80) strength = 'Very Poor';
                    document.getElementById('wifi-strength').textContent = `${strength} (${rssi} dBm)`;
                    
                    // Uptime
                    document.getElementById('uptime').textContent = data.uptime_formatted || 'Unknown';
                    document.getElementById('uptime-seconds').textContent = `${data.uptime_seconds || 0} seconds`;
                    
                    // Memory
                    const freeHeap = data.free_heap || 0;
                    const totalHeap = data.total_heap || 0;
                    const usagePercent = totalHeap > 0 ? Math.round(((totalHeap - freeHeap) / totalHeap) * 100) : 0;
                    document.getElementById('memory-usage').textContent = `${usagePercent}% used`;
                    document.getElementById('memory-details').textContent = `${(freeHeap / 1024).toFixed(1)}KB free / ${(totalHeap / 1024).toFixed(1)}KB total`;
                    
                    // Storage
                    const flashSize = data.flash_size || 0;
                    const flashSizeMB = (flashSize / (1024 * 1024)).toFixed(1);
                    document.getElementById('storage-usage').textContent = `${flashSizeMB} MB`;
                    document.getElementById('storage-details').textContent = 'Flash Storage';
                    
                    // Hardware
                    document.getElementById('chip-model').textContent = data.chip_model || 'Unknown';
                    document.getElementById('cpu-freq').textContent = `${data.cpu_freq_mhz || 0} MHz`;
                })
                .catch(error => {
                    console.error('Error loading system info:', error);
                    // Set error states
                    document.getElementById('wifi-status').textContent = 'Error';
                    document.getElementById('ip-address').textContent = 'Error';
                    document.getElementById('uptime').textContent = 'Error';
                    document.getElementById('memory-usage').textContent = 'Error';
                    document.getElementById('storage-usage').textContent = 'Error';
                    document.getElementById('chip-model').textContent = 'Error';
                });
        }
        
        function refreshSystemInfo() {
            const refreshBtn = document.querySelector('.btn-secondary');
            if (refreshBtn) {
                const originalText = refreshBtn.textContent;
                
                // Show loading state
                refreshBtn.textContent = "Refreshing...";
                refreshBtn.disabled = true;
                refreshBtn.innerHTML = '<span class="spinner"></span> Refreshing...';
                
                loadSystemInfo();
                
                // Restore button state after a short delay
                setTimeout(() => {
                    refreshBtn.textContent = originalText;
                    refreshBtn.disabled = false;
                    refreshBtn.innerHTML = originalText;
                }, 1000);
            } else {
                loadSystemInfo();
            }
            
            showStatus('System information refreshed!', 'success');
        }
        
        // New automatic update functions
        function updateDisplayModes() {
            const form = document.getElementById('operation_mode_form');
            let strLine = "";
            
            if (form["clock-checkbox"].checked) {
                strLine += "/&CLOCK";
            }
            if (form["date-checkbox"].checked) {
                strLine += "/&DATE";
            }
            if (form["weather-checkbox"].checked) {
                strLine += "/&WEATHER";
            }
            if (form["snake-checkbox"].checked) {
                strLine += "/&SNAKE";
            }
            if (form["ip-checkbox"].checked) {
                strLine += "/&IP";
            }
            if (form["message-checkbox"].checked) {
                const message = form["message-input"].value.trim();
                if (message === "") {
                    showStatus("Message cannot be empty when selected.", "error");
                    return;
                }
                strLine += "/&MSG=" + encodeURIComponent(message);
            }
            
            if (strLine === "") {
                showStatus("Please select at least one display mode.", "error");
                return;
            }
            
            // Add nocache parameter to prevent caching
            const nocache = "/&nocache=" + Math.random() * 1000000;
            
            fetch(strLine + nocache)
                .then(response => {
                    if (response.ok) {
                        showStatus("Display modes updated!", "success");
                    } else {
                        showStatus("Failed to update display modes.", "error");
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    showStatus("Error updating display modes.", "error");
                });
        }
        
        // Function to update a single setting
        function updateSingleSetting(settingName, value) {
            const settings = {};
            settings[settingName] = value;
            
            // Use GET request with URL-encoded JSON
            const jsonString = JSON.stringify(settings);
            const encodedJson = encodeURIComponent(jsonString);
            const url = `/&CNTL=${encodedJson}`;
            
            fetch(url)
            .then(response => {
                if (response.ok) {
                    showStatus(`${settingName} updated!`, 'success');
                } else {
                    showStatus(`Failed to update ${settingName}.`, 'error');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                showStatus(`Error updating ${settingName}.`, 'error');
            });
        }
        
        // Function to update coordinates when city is selected
        function updateCoordinates(latitude, longitude) {
            const timezone = document.getElementById('timezone').value;
            
            const settings = {
                latitude: latitude,
                longitude: longitude
            };
            
            // Add timezone if available
            if (timezone) {
                settings.timezone = timezone;
            }
            
            // Use GET request with URL-encoded JSON
            const jsonString = JSON.stringify(settings);
            const encodedJson = encodeURIComponent(jsonString);
            const url = `/&CNTL=${encodedJson}`;
            
            fetch(url)
            .then(response => {
                if (response.ok) {
                    showStatus('Location and timezone updated!', 'success');
                } else {
                    showStatus('Failed to update location.', 'error');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                showStatus('Error updating location.', 'error');
            });
        }
        
        function updateGeneralSettings() {
            const cityInput = document.getElementById('city-input');
            const brightnessSlider = document.getElementById('brightness');
            const celsiusRadio = document.getElementById('celsius-radio');
            
            // Get city coordinates from the city info data attributes
            const cityInfo = document.getElementById('city-info');
            let latitude = null;
            let longitude = null;
            
            if (cityInfo && cityInfo.hasAttribute('data-latitude') && cityInfo.hasAttribute('data-longitude')) {
                latitude = parseFloat(cityInfo.getAttribute('data-latitude'));
                longitude = parseFloat(cityInfo.getAttribute('data-longitude'));
            }
            
            const settings = {
                brightness: parseInt(brightnessSlider.value),
                weatherUnits: celsiusRadio.checked ? 'c' : 'f'
            };
            
            // Only add coordinates if we have them
            if (latitude !== null && longitude !== null) {
                settings.latitude = latitude;
                settings.longitude = longitude;
            }
            
            // Use GET request with URL-encoded JSON
            const jsonString = JSON.stringify(settings);
            const encodedJson = encodeURIComponent(jsonString);
            const url = `/&CNTL=${encodedJson}`;
            
            fetch(url)
            .then(response => {
                if (response.ok) {
                    showStatus('Settings updated!', 'success');
                } else {
                    showStatus('Failed to update settings.', 'error');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                showStatus('Error updating settings.', 'error');
            });
        }
    </script>
  </body>
</html>
)html";
#endif