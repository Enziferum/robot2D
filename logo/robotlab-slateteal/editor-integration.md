# Robot Lab SlateTeal — выбранный логотип 02

Выбран пользователем 2026-10-17. Мастер — оригинальный прозрачный RGBA PNG,
сгенерированный image_gen по игровому Player из Robot Lab. Prompt и происхождение
хранятся рядом. PNG редактора 512 × 512 и Windows ICO — только ресемплинг/упаковка,
без перерисовки. Сайт использует тот же вариант в WEB-02/#133, маскот — вариант 03.

`editor/res/textures/logo.png` используется существующим Application::setup для
иконки окна и загрузчиком TextureID::Logo. Windows resource 1 ICON вшивает
`editor/res/branding/robot2d.ico` в robot2D.exe. ICO содержит 16/24/32/48/64/128/256 px.
CMake подключает RC только на Windows; Linux использует runtime PNG.

После сборки ресурсы лежат рядом с exe в res/. Ручная приёмка: иконки окна/taskbar,
File Explorer и Alt+Tab на разных DPI; свежий exe проверять в новом пути, поскольку
Windows может кэшировать старую иконку. Computer-use не применяется.
