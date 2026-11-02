document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll("[data-js]").forEach((elemento) => {
        elemento.textContent = "JavaScript cargado correctamente.";
    });
});
