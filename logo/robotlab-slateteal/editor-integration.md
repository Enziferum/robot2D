# Robot Lab SlateTeal — логотип 02

Мастер — прозрачный RGBA PNG, созданный с помощью AI по игровому Player из
Robot Lab. Происхождение и SHA-256 хранятся в provenance.json. PNG редактора
512 × 512 и Windows ICO — только ресемплинг/упаковка,
без перерисовки. Сайт использует тот же логотип; маскот — вариант 03.

`editor/res/textures/logo.png` используется существующим Application::setup для
иконки окна и загрузчиком TextureID::Logo. Windows resource 1 ICON вшивает
`editor/res/branding/robot2d.ico` в robot2D.exe. ICO содержит 16/24/32/48/64/128/256 px.
CMake подключает RC только на Windows; Linux использует runtime PNG.

После сборки ресурсы лежат рядом с exe в res/. Ручная приёмка: иконки окна/taskbar,
File Explorer и Alt+Tab на разных DPI; свежий exe проверять в новом пути, поскольку
Windows может кэшировать старую иконку.
