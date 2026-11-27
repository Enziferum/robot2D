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
`Window::getMetrics()` возвращает фактические logical/framebuffer размеры,
content scale и минимизацию. Content scale и отношение framebuffer/logical
могут различаться. `getSize()` сохраняет совместимость и возвращает фактический
logical размер, а не последний запрос `setSize()` или старое событие resize.

Перед рисованием в окно вызывайте `window.synchronizeSize()`: default view
остаётся в logical координатах, viewport получает физические размеры,
пользовательские камеры сохраняются. При минимизации или нулевых размерах метод
возвращает false; пропускайте вывод и swap, продолжая pump/update. Если GUI-кадр
уже начат, завершите его через `cancelFrame()`. ImGui render также потребляет
готовый кадр без вывода в недоступное окно. `resize(vec2i)` оставлен как явная
настройка render surface 1:1 для совместимости; он не меняет native окно.

`mapClientToCoords()` переводит logical mouse position через физический viewport
в координаты заданного View. `FramebufferResized`, `ContentScaleChanged` и
`MinimizedChanged` — отдельные типизированные события. Они сохраняются в FIFO;
callbacks не меняют GL-состояние и не подменяют фактические размеры историческими
значениями. Window, DesktopRuntime, cursors и GUI используйте на одном UI-потоке.

## События и Engine

`robot2D::EventBinder` доступен из `<robot2D/Core/EventBinder.hpp>` без Editor.
Один обработчик привязывается к конкретному payload, например `Event::KeyPressed`;
можно принимать payload и `EventContext` или только payload. Callback возвращает
`EventResult::Ignored/Handled`; callback с void считается Handled. Callbacks
должны быть копируемыми. bind/unbind во время dispatch запрещены; исключение из
callback передаётся вызывающему коду, состояние dispatch восстанавливается.
Существующие `editor::EventBinder/EventResult` остаются совместимыми aliases.

`Window::takeEvents(frame)` возвращает владеющий batch с sequence, временем
приёма и номером кадра. События доставляйте в исходном порядке один раз.
`Gui::handleEvents()` должен получать все события независимо от capture;
snapshot `inputCapture()` берите перед `beginFrame()`. GUI capture определяет
обработку прикладных shortcuts/мыши, но не отменяет системные события и release.
Состояние игровых действий для fixed ticks — отдельный уровень поверх raw input.

Можно использовать собственный цикл с DesktopRuntime/RenderWindow/Gui либо
унаследовать Application и вызвать Engine. `Engine::run()` отвергает null
application, неположительный maxFrameRate и нулевые/непредставимые размеры до
инициализации desktop. При таком отказе переданный application освобождается.
После прикрепления окна destroy вызывается один раз, включая ошибку setup/кадра,
и application уничтожается с живым текущим GL-контекстом. Остановка из
handleMessages/update/guiUpdate прекращает последующие callbacks этого кадра.

Текущее имя maxFrameRate сохранено для совместимости: оно задаёт частоту fixed
update (`1 / maxFrameRate`), а не отдельный limiter рендера. Vsync задаётся через
WindowContext. Новый timestep/RuntimeSession и ограничение догоняющих шагов
не входят в этот контракт.

Для инструмента Core+ImGui не требуется ECS Scene. Текущий ECS можно использовать
отдельно; новый storage/clone/undo контракт не является условием создания окна,
ресурсов, рендера и GUI. Текущий renderer сохраняется; ресурсы отложенного batch
должны жить до `afterRender()`. Аудио-модуль можно подключать со стороны приложения.

## Проверки

`RB2D_BUILD_CORE_TESTS=ON` включает штатные headless Core tests и сборку
`robot2D-core-imgui-smoke`, если ImGui включён. Отдельная Core+ImGui конфигурация
также проверяет GUI math, CPU font atlas и uninitialized lifecycle без Editor/Mono.
`RB2D_CORE_GRAPHICS_TESTS=ON`
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
