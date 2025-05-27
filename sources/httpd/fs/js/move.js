// Функция обработки нажатия кнопки
function handleButtonClick(direction) {
  console.log(`Нажата кнопка: ${direction}`);
  // Здесь можно добавить любую логику при нажатии кнопок
}

// Назначаем обработчики событий
document.getElementById('btnUp').addEventListener('click', () => handleButtonClick('Вверх'));
document.getElementById('btnDown').addEventListener('click', () => handleButtonClick('Вниз'));
document.getElementById('btnLeft').addEventListener('click', () => handleButtonClick('Влево'));
document.getElementById('btnRight').addEventListener('click', () => handleButtonClick('Вправо'));
