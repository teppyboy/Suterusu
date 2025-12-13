(function() {
    const id = 'selection-style';
    
    // Function to toggle selection
    function toggleSelection() {
        const existing = document.getElementById(id);
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
    }
    
    // Run immediately if DOM is ready, otherwise wait
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', toggleSelection);
    } else {
        toggleSelection();
    }
})();