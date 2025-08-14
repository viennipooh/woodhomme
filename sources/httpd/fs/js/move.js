

window.addEventListener('DOMContentLoaded', function() {
    // Найдём все кнопки управления движением в контейнере
    var moveButtons = document.querySelectorAll('.container button[name^="move_"]');
    moveButtons.forEach(function(btn) {
        btn.addEventListener('click', function() {
            // Получаем имя команды, например move_up, move_down и т.д.
            var cmd = btn.name;
            // Формируем параметры для GET-запроса (или используйте POST, если нужно)
            var params = cmd + '=1';
            // Отправляем запрос на сервер (замените URL на ваш CGI-обработчик)
            fetch('/cgi-bin/system_move.cgi?' + params, {
                method: 'GET'
            })
            .then(response => response.text())
            .then(data => {
                console.log('Ответ сервера:', data);
            })
            .catch(error => {
                console.error('Ошибка отправки команды:', error);
            });
        });
    });
});



// // Функция обработки нажатия кнопки
// function handleButtonClick(direction) {
  // console.log(`Нажата кнопка: ${direction}`);
  // // Здесь можно добавить любую логику при нажатии кнопок
// }

// // Назначаем обработчики событий
// document.getElementById('btnUp').addEventListener('click', () => handleButtonClick('Вверх'));
// document.getElementById('btnDown').addEventListener('click', () => handleButtonClick('Вниз'));
// document.getElementById('btnLeft').addEventListener('click', () => handleButtonClick('Влево'));
// document.getElementById('btnRight').addEventListener('click', () => handleButtonClick('Вправо'));
