# GuiRender / GuiImpl: однооконный backend

IMGUI-02 [#126](https://github.com/Enziferum/robot2D/issues/126), 2026-10-16.
Реализация первого шага по [согласованному контракту IMGUI-01](https://github.com/Enziferum/robot2D/blob/f4fc9c3b7fd290a1bf240d722972bc4ced877792/docs/imgui-backend-review.md).
Результат — [draft PR #130](https://github.com/Enziferum/robot2D/pull/130), ветка IMGUI-02.
Зависимость — CORE-02 / [PR #122](https://github.com/Enziferum/robot2D/pull/122).
Это текущий однооконный GL backend; GraphicsCommands и multi-viewports ещё не реализованы.

## Жизненный цикл и ошибки

```text
Uninitialized --initialize--> Ready --beginFrame--> BuildingFrame
BuildingFrame --endFrame--> DrawDataReady --render--> Ready
BuildingFrame --cancelFrame--> Ready
shutdown --> Uninitialized
```

Все операции выполняются на одном UI-потоке. initialize получает заимствованный
Window, владеющий font set и явную политику fallback. Повторная инициализация
работающего объекта отклоняется. Частичный отказ освобождает ресурсы и оставляет
Uninitialized; повторный shutdown безопасен, в том числе до initialize.
Сбой возвращается через bool с lastError(); ошибки порядка вызовов диагностируются.

beginFrame принимает только конечный положительный deltaSeconds. На период
построения UI собственный ImGuiContext текущий для прямых вызовов ImGui.
endFrame вызывает ImGui::Render ровно один раз и восстанавливает прежний context.
cancelFrame вызывает EndFrame без GPU исполнения. render потребляет подготовленный
кадр также при нулевом target или ошибке callback: объект возвращается в Ready,
повторное исполнение прежних draw data не предусмотрено. Draw data не сохраняют
между кадрами. shutdown отменяет открытый кадр и освобождает ресурсы.

Вне кадра для настроек, layout и вызовов editor используется GuiContextScope(gui.context()).
Контекст должен оставаться живым до завершения scope; scope не владеет контекстом.
inputCapture и доставка Event сами выбирают собственный context и восстанавливают
предыдущий. Сохраняется адаптер Add*Event CORE-02; capture снимается до NewFrame,
маршрутизация и владельцы Press/Release остаются в editor.

## Шрифты

GuiFontConfig владеет string path либо vector bytes, glyph ranges, FontId, размером
и mergeInto. ID ненулевые и уникальные; merge target должен быть объявлен раньше.
GuiConfiguration выбирает defaultFont. Пустой набор с явно разрешённым
allowDefaultFallback и defaultFont=0 выбирает встроенный шрифт; такой сценарий
использует example. Для именованных шрифтов defaultFont указывается явно. Источники копируются внутрь библиотеки;
исходную конфигурацию можно уничтожить сразу после initialize. Набор фиксирован
до shutdown. font(id) возвращает временный указатель для текущего набора;
потребитель хранит FontId. Поддержаны отдельные SFNT TTF/OTF, не коллекции TTC.
Проверка заголовка/границ таблиц не является валидатором произвольных недоверенных шрифтов.

Обязательный отсутствующий/неверный шрифт прерывает initialize с ID/path/reason.
Встроенный fallback разрешается явно через allowDefaultFallback; optional source
можно пропустить. Для merge fallback означает сохранение целевого базового шрифта,
без отсутствующих glyphs. Выбор Noto Sans, иконок, размеров и ranges находится
в editor; GuiRender не знает Font Awesome или editor/res.

## Core и граница отрисовки

Window::getLogicalSize() запрашивает фактический client size, getFramebufferSize()
— физические pixels. Старый getSize() сохраняет прежнюю кешированную семантику.
Разделение учитывает ограничения ОС при создании окна, даже до Resized batch.
FramebufferScale вычисляется из фактических размеров; при нулевом logical size
нет деления на ноль. Перемещение между DPI-мониторами и перестройка шрифтов отложены.

Вызывающий слой выбирает текущий GL context и target, завершает отложенную сцену
через RenderTarget::afterRender(), затем исполняет GUI. Backend не делает clear,
context switch или swap. Текущий Core хранит очередь draw в render cache,
но не кеширует GL bindings с пропуском повторных bind. Поэтому для этого шага
новый API invalidation не вводится; будущий Graphics pass обязан учитывать его.

Backend сохраняет затрагиваемое состояние один раз на проход: program, VAO,
array buffer, texture/sampler unit 0 и исходной active unit, framebuffer bindings,
viewport/scissor, polygon mode, blend, depth mask, color mask и enable flags.
Для atlas upload сохраняется unpack state, включая PBO/row length/alignment.
Callbacks исполняются по порядку; после произвольного callback и reset заново
устанавливаются необходимые UI bindings/state. Scissor использует DisplayPos и
FramebufferScale: начало округляется вниз, конец вверх, результат пересекается
с target; пустые области пропускаются. GL backend инвертирует Y.

Неиспользовавшийся дублирующий VAO/VBO путь удалён; текущая активная GL схема
сохранена. VtxOffset capability пока не заявляется; ненулевой offset отклоняется,
а поддержка больших списков относится к следующему Graphics шагу.

Texture использует пустой handle 0 и повторобезопасный release. ShaderHandler
получил явный release для эксклюзивного владельца GuiRender; общая семантика
копирования старых handlers и их destructor не менялась. Это ещё не универсальный
рефакторинг владения Shader/Texture во всём Core.

Штатный shutdown выполняется до уничтожения окна/GL context. Engine также
освобождает application перед локальным окном при раскрутке исключения, не меняя
scheduler. Гарантия не распространяется на произвольный внешний порядок уничтожения.

## Проверки и ручная приёмка

Постоянные сценарии — editor/tests/GuiBackendTests.cpp:

- headless math: углы проекции, разные/отрицательные X/Y, fractional clip,
  clamp, empty/zero-size, большие и нечисловые координаты;
- headless uninitialized: shutdown до initialize/дважды, неверный порядок,
  чужой context остаётся живым, пустой Texture не требует GL loader;
- headless fonts: memory source после освобождения конфигурации, явный default,
  merge в выбранный base после другого base, missing required и явный fallback;
- graphics lifecycle 4.3/4.5: previous/own context, finite dt, cancel/end/render,
  пустые draw data, повторный initialize/shutdown, отказ font setup и повторный успех;
- graphics renderer 4.3/4.5: реальные pixels, negative DisplayPos/scale 2,
  несколько lists/textures, alpha overlap, clipping, callbacks/reset и восстановление GL-state,
  исключение callback и следующий кадр, отрисовка после GUI;
- graphics rollback 4.3/4.5: отказ fragment shader после создания vertex/program/buffers,
  отсутствие оставшихся GPU объектов, повторный initialize и освобождение atlas.

Запуск включается в robot2D-editor-test-suite. Graphics без доступного контекста
помечаются Skipped (77), что не является прохождением. Инструкция — [editor-testing.md](editor-testing.md).

Ручная приёмка перед merge (пользователь подтвердил все пункты 2026-10-17):

1. Запустить editor из его build/editor, открыть проект из инспектора.
2. Проверить полный размер GUI и текст/иконки, изменение размера окна.
3. Свернуть/восстановить окно; продолжить ввод и перемещение панелей.
4. Ввести текст, выполнить drag за границы панели и отпустить кнопку; проверить capture.
5. Проверить Scene/Game после GUI и переход обратно к инспектору.
6. Закрыть редактор, повторно запустить и проверить layout и отсутствие startup crash.

Тайминг, multi-viewports, upstream update, горячая замена шрифтов, texture requests,
GraphicsCommands/streaming и обёртки IMGUI-01 остаются отдельными задачами.


## Регрессия SceneGrid / IndexBuffer при приёмке

На ручной приёмке Shooter пользователь обнаружил: GUI и сетка видны,
Quads/Draw calls ненулевые, объекты отсутствуют. Дефект воспроизведён на копии
сцены, затем сведён к тестовому проекту с прозрачной камерой перед видимым quad.

GuiRender сохраняет VAO вызывающего Core. OpenGLIndexBuffer при создании
использовал GL_ELEMENT_ARRAY_BUFFER и заменял attachment у этого VAO ещё до
явного VertexArray::setIndexBuffer(). Настройка SceneGrid подставляла в Core
буфер из шести индексов (24 байта). Первый quad камеры был прозрачным, а для
остальной сцены индексов не хватало. Зелёный CI 7ed90b2 не покрывал эту комбинацию.

Загрузка IndexBuffer теперь использует GL_COPY_WRITE_BUFFER с восстановлением
его прежней привязки. Привязка к VAO выполняется явно через существующий API;
новый Core API не вводится. Проверены существующие потребители: OpenGLRender,
QuadBatchRender и SceneGrid используют setIndexBuffer().

Три проверки падали до исправления и проходят после него:

- imgui-backend-renderer-43/45: сохранность чужого EBO и upload binding,
  создание без VAO; настоящие Core draws в Scene/Game framebuffer, затем GUI,
  проверка пикселей в нескольких кадрах;
- editor-scene-editor-gui-scene: изолированный проект, загрузка сцены с прозрачной
  камерой первой, полный Editor::render с сеткой/панелями и пиксели видимого quad
  после стабилизации docking. Пользовательский проект и layout не изменяются.

Локально редактор/core-tests/editor-suite собраны; CTest 216/216 без пропусков,
49 graphics. Ручная повторная проверка Shooter и CI нового commit фиксируются
отдельно; автоматический тест не объявляет всю UI-приёмку завершённой.

Повторная ручная проверка пользователем исправленного robot2D.exe: Shooter открыт,
объекты видны. Остальные пункты GUI/capture/layout проверяются отдельно перед merge.

Исправление: commit 7e45d6e. Обязательный hook: Python 53/53, headless 167/167,
строгий clang-tidy 189 единиц — успешно. Три графические регрессии повторно
прошли на окончательном коде.


2026-10-17: пользователь подтвердил успешную ручную приёмку всех пунктов и
поручил последовательное слияние #122, затем #130 при зелёном CI. Linux CI
4836bc1 обнаружил зависимость нового pixel-test от скрытого native back buffer:
Editor меняет размер окна, а X11/Xvfb применяет его асинхронно. Тест теперь
обрабатывает GLFW events и компонует GUI в явный framebuffer нужного размера;
проверка красного объекта после прозрачной камеры сохранена. Очистка singleton
GL-ресурсов выполняется до уничтожения создавшего их context. Результат
повторного CI фиксируется по новому head; прежний Linux failure не скрывается.


Дополнительная граница lifetime: EditorResourceManager владеет статическими
GPU-иконками (включая Movie для CameraManipulator). Добавлен явный повторобезопасный
releaseGraphicsResources(): Application вызывает его после уничтожения editor
module и до потери GL-контекста. Полный scene/GUI тест освобождает ресурсы после
потребителей и проверяет glIsTexture=false, отсутствие записи и повторный release.
Это не изменение ECS или общего Window/GLFW lifetime; итог Linux CI остаётся
обязательным условием интеграции.


Диагностический запуск 30a21d3 подтвердил: pixel-test проходит до shutdown,
затем процесс завершается SIGSEGV; на runner не было gdb. Linux CI теперь
устанавливает gdb для получения стека при повторном отказе. Lifetime-исправление
573e65f прошло локальный hook: editor/core-tests/suite, headless и строгий анализ;
подтверждение Linux и Windows выполняется на следующем опубликованном head.
