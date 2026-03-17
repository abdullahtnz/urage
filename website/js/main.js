function switchTab(id) {
  document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
  document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
  document.getElementById('tab-' + id).classList.add('active');
  event.target.classList.add('active');
}

function copyCode(btn) {
  const code = btn.nextElementSibling.innerText;
  navigator.clipboard.writeText(code).then(() => {
    btn.textContent = 'COPIED!';
    btn.style.color = 'var(--accent)';
    setTimeout(() => { btn.textContent = 'COPY'; btn.style.color = ''; }, 2000);
  });
}