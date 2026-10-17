# robot2D SlateTeal — v1

Геометрический робот в SlateTeal. Концепт создан встроенным image_gen;
для рабочего использования подготовлена чистая геометрическая SVG-реконструкция.

## Рабочие файлы

- [robot2d-mark.svg](robot2d-mark.svg): самостоятельный знак, без шрифтов.
- [robot2d-logo.svg](robot2d-logo.svg): горизонтальный знак + надпись robot2D;
  текст остаётся редактируемым, использует Segoe UI с Arial/sans-serif fallback.
- [robot2d-logo.png](robot2d-logo.png): фиксированная растровая версия надписи.
- `robot2d-vector-{32,64,128,256,512}.png`: прозрачные PNG из рабочего SVG.
- [robot2d-editor-vector.ico](robot2d-editor-vector.ico): Windows ICO,
  16/24/32/48/64/128/256 px, из рабочего SVG.

На сайте SVG-знак стоит рядом с живой надписью robot2D, включая Starlight,
и используется как favicon. Прежние logo/logo.png и ресурсы редактора сохранены.

Редактор загружает иконку окна из `res/textures/logo.png`
(`editor/include/editor/Application.hpp`). PNG 256/512 готов для этого потребителя;
рабочая сборка пользователя в этом шаге не изменяется. ICO готов для Windows
executable/ярлыка, но сам файл не меняет встроенную в exe иконку. Подключение
ресурсов в редакторе и native-проверка остаются следующим отдельным шагом.

## Исходный концепт

`robot2d-mark-master.png` — неизменённая генерация 1254 × 1254, RGBA;
`robot2d-mark-*.png` и `robot2d-editor.ico` — её первоначальные растровые экспорты.
Для интеграции используй рабочие SVG/vector-файлы выше. Prompt генерации —
[prompt.txt](prompt.txt), происхождение и SHA-256 — [provenance.json](provenance.json).
