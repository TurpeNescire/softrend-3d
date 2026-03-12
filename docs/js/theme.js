(function () {
  var STORAGE_KEY = 'sr3d-theme';
  var DEFAULT = 'theme-dark';
  var themes = ['theme-dark', 'theme-terminal', 'theme-docs', 'theme-editorial'];

  function apply(theme) {
    themes.forEach(function (t) { document.body.classList.remove(t); });
    document.body.classList.add(theme);
    var picker = document.getElementById('theme-picker');
    if (picker) picker.value = theme;
    localStorage.setItem(STORAGE_KEY, theme);
  }

  function saved() {
    return localStorage.getItem(STORAGE_KEY) || DEFAULT;
  }

  document.addEventListener('DOMContentLoaded', function () {
    apply(saved());
    var picker = document.getElementById('theme-picker');
    if (picker) {
      picker.addEventListener('change', function () {
        apply(this.value);
      });
    }
  });
})();
