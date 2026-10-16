# GuiRender / GuiImpl: однооконный backend

IMGUI-02 [#126](https://github.com/Enziferum/robot2D/issues/126), 2026-10-16.
Реализация первого шага по [согласованному контракту IMGUI-01](https://github.com/Enziferum/robot2D/blob/f4fc9c3b7fd290a1bf240d722972bc4ced877792/docs/imgui-backend-review.md).
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
GuiConfiguration выбирает defaultFont. Источники копируются внутрь библиотеки;
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
  несколько lists/textures, clipping, callbacks/reset и восстановление GL-state,
  исключение callback и следующий кадр, отрисовка после GUI;
- graphics rollback 4.3/4.5: отказ fragment shader после создания vertex/program/buffers,
  отсутствие оставшихся GPU объектов, повторный initialize и освобождение atlas.

Запуск включается в robot2D-editor-test-suite. Graphics без доступного контекста
помечаются Skipped (77), что не является прохождением. Инструкция — [editor-testing.md](editor-testing.md).

Ручная приёмка перед merge (ещё не выполнена пользователем):

1. Запустить editor из его build/editor, открыть проект из инспектора.
2. Проверить полный размер GUI и текст/иконки, изменение размера окна.
3. Свернуть/восстановить окно; продолжить ввод и перемещение панелей.
4. Ввести текст, выполнить drag за границы панели и отпустить кнопку; проверить capture.
5. Проверить Scene/Game после GUI и переход обратно к инспектору.
6. Закрыть редактор, повторно запустить и проверить layout и отсутствие startup crash.

Тайминг, multi-viewports, upstream update, горячая замена шрифтов, texture requests,
GraphicsCommands/streaming и обёртки IMGUI-01 остаются отдельными задачами.
