(function() {
    const id = 'selection-style';
    const existing = document.getElementById(id);
    if (existing) {
        existing.remove();
        return "Selection highlighting enabled";
    } else {
        const style = document.createElement('style');
        style.id = id;
        style.textContent = `
            ::selection {
            background: transparent !important;
            color: inherit !important;
        }`;
        document.head.appendChild(style);
        return "Selection highlighting disabled";
    }
})();