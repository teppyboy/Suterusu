(function() {
    const id = 'selection-style';
    const storageKey = 'selection-toggle-enabled';
    
    // Function to toggle selection
    function toggleSelection() {
        const existing = document.getElementById(id);
        const isEnabled = !existing;
        
        if (existing) {
            existing.remove();
        } else {
            const style = document.createElement('style');
            style.id = id;
            style.textContent = `
                ::selection {
                background: transparent !important;
                color: inherit !important;
            }`;
            (document.head || document.documentElement).appendChild(style);
        }
        
        // Save state to localStorage
        localStorage.setItem(storageKey, isEnabled);
    }
    
    // Function to apply saved state
    function applySavedState() {
        const isEnabled = localStorage.getItem(storageKey) === 'true';
        if (isEnabled) {
            const style = document.createElement('style');
            style.id = id;
            style.textContent = `
                ::selection {
                background: transparent !important;
                color: inherit !important;
            }`;
            (document.head || document.documentElement).appendChild(style);
        }
    }
    
    // Apply saved state on page load
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', applySavedState);
    } else {
        applySavedState();
    }
    
    // Expose toggle function globally for manual triggering if needed
    window.toggleSelection = toggleSelection;
    toggleSelection();
})();