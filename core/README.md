# robot2D Core и ImGui

Core можно собирать отдельно от редактора и Mono. Требуются C++17, CMake,
Ninja, GLFW, spdlog и FreeType. На macOS используйте Apple Clang и нативные
зависимости одной архитектуры (`brew install ninja glfw spdlog freetype`).

```sh
cmake -S . -B build-core -G Ninja -DCMAKE_BUILD_TYPE=Release -DRB2D_BUILD_EDITOR=OFF -DRB2D_BUILD_IMGUI=ON
cmake --build build-core --target robot2D-core robot2D-imgui
```

Подключение из другого CMake-проекта:

```cmake
set(RB2D_BUILD_EDITOR OFF CACHE BOOL "" FORCE)
set(RB2D_BUILD_IMGUI ON CACHE BOOL "" FORCE)
add_subdirectory(external/robot2D)
target_link_libraries(MyTool PRIVATE robot2D-core robot2D-imgui)
```

Для чистого Core задайте `RB2D_BUILD_IMGUI=OFF` и линкуйте только
`robot2D-core`. Сабмодули редактора и Mono при этом не конфигурируются.
По умолчанию корневая сборка сохраняет сборку редактора.

```cpp
robot2D::DesktopRuntime desktop;
robot2D::WindowContext context(false, false,
    robot2D::WindowContext::RenderApi::OpenGL4_1);
robot2D::RenderWindow window(desktop, {1280, 720}, "MyTool", context);
```

На macOS контекст по умолчанию — OpenGL 4.1 Core Profile, forward-compatible.
На Windows/Linux сохранён default 4.3; 4.1 можно выбрать явно, 4.5 также
поддерживается. Явный запрос версии не заменяется другой версией: при
недоступности контекста создание окна бросает `WindowCreationError`.
GLAD загружается на всех трёх платформах. Путь 4.1 использует GLSL 410,
bind-based текстуры и framebuffer без обязательных DSA/immutable-storage
расширений; ImGui использует GLSL 330.

GL-ресурсы создавайте и освобождайте с текущим живым контекстом. GUI заимствует
окно: вызовите `gui.shutdown()` до уничтожения окна. Выбранная версия рендера
хранится глобально: одновременная работа окон с разными версиями API не является
поддерживаемым контрактом.

Размер окна и курсор выражаются в logical pixels, framebuffer — в физических.
ImGui получает scale из их отношения. Для Core перед рисованием в окно обновляйте
поверхность через `window.resize()` с `window.getFramebufferSize()`; при нулевом
framebuffer пропускайте кадр. View и координаты сцены задавайте отдельно согласно
выбранной системе координат приложения.

## Проверки

`RB2D_BUILD_CORE_TESTS=ON` включает штатные headless Core tests и сборку
`robot2D-core-imgui-smoke`, если ImGui включён. `RB2D_CORE_GRAPHICS_TESTS=ON`
дополнительно регистрирует этот графический тест в CTest; отсутствие контекста
считается ошибкой. Тест создаёт скрытое окно 4.1 core/forward-compatible, рисует
Core sprite и ImGui, проверяет framebuffer с depth/stencil, MSAA и resize,
кириллический шрифт, viewport и framebuffer scale, освобождает GUI до окна.
Указатели DSA и glTexStorage2D отключаются внутри отдельного процесса теста.

CI Linux запускает его через Xvfb/Mesa с ограничением GL 4.1/GLSL 410.
Windows и macOS CI собирают smoke и запускают headless tests. Графический
запуск на hosted macos-15-arm64 проверен в Actions run 46006344388: NSGL
отклонил создание контекста 4.1 с `Failed to find a suitable pixel format`.
Этот runner не подтверждает графику или Retina; smoke запускайте отдельно
на Mac с доступным OpenGL renderer. macOS job нацелен на Apple silicon;
Intel и universal binary требуют отдельной проверки.

Ручная приёмка на Mac: соберите и запустите `robot2D-core-imgui-smoke`, затем
в приложении Core+ImGui проверьте видимый кадр с кириллицей, изменение размера,
перенос окна между Retina и обычным монитором, сворачивание/восстановление и
закрытие с shutdown GUI до окна. Проверьте отсутствие GL ошибок и соответствие
viewport физическому framebuffer. Скрытый smoke не заменяет эту приёмку.
