# CORE-02: события, GUI и порядок доставки

Задача: [CORE-02 #119](https://github.com/Enziferum/robot2D/issues/119).
C++17, один главный поток и текущее основное окно. Core ECS не меняется.

## Event и получение кадра

`Event` владеет `std::variant` с отдельными типами нажатия и отпускания.
Публичный доступ к payload только const; `FilesDropped` владеет строками путей.
`type()` возвращает `Event::Type`, `getIf<T>()` — указатель или nullptr,
`get<T>()` при несовпадении бросает `std::bad_variant_access`.
Общие базы `KeyboardEvent`, `MouseButtonEvent` пригодны для чтения нескольких
альтернатив; подписка EventBinder требует конкретную альтернативу.

```cpp
robot2D::Event event(robot2D::Event::KeyPressed{{robot2D::Key::S, 31, 2}, false});
if(const auto* key = event.getIf<robot2D::Event::KeyPressed>()) {
    // key->code, key->scancode, key->modifiers, key->repeat
}
window.processEvents();
const auto frame = window.takeEvents(frameNumber);
for(const auto& received : frame.events) {
    // received.context(): sequence, frame, receivedAt
}
```

`EventQueue` сохраняет FIFO, монотонный sequence и время приёма относительно
создания очереди. Время отражает вызов callback, а не физический момент ввода ОС.
`Window::pollEvents` теперь только извлекает событие; GLFW вызывается отдельно
через `processEvents`. `display` только обменивает буферы. Ручные циклы должны
добавить pump перед dequeue; ожидания и sleep в callback/dequeue отсутствуют.
Исключение при сборе callback откладывается до возврата из GLFW pump.
Legacy drop callback вызывается при извлечении события вне GLFW callback.

Engine на каждой итерации собирает owning `FrameEvents`, вызывает
`Application::handleEventFrame`, проверяет решение о закрытии, затем выполняет
накопившиеся fixed updates, GUI и render. Базовый handleEventFrame вызывает
handleEvents в порядке batch. Поэтому кадр без fixed update всё равно доставляет
нажатие и отпускание. Обработчик Closed может отменить запрос через cancelClose.

## Типизированный EventBinder

```cpp
binder.bind<robot2D::Event::KeyPressed>(
    [](const robot2D::Event::KeyPressed& key, const robot2D::EventContext& context) {
        return editor::EventResult::Ignored;
    });
const auto result = binder.dispatch(event);
binder.unbind<robot2D::Event::KeyPressed>();
```

Context необязателен в callback; void означает Handled. Для каждого конкретного
типа одна подписка, повторный bind заменяет её. Несовпавший тип возвращает Ignored.
Порядок задаёт вызывающий цикл, unordered_map не обходится при доставке.
Исключения обработчика передаются вызывающему коду. Bind/unbind внутри dispatch
запрещены и бросают logic_error; повторный dispatch разрешён. Result не вводит
автоматическое всплытие через существующие контроллеры редактора.

## GUI и владение вводом

Application сначала передаёт **каждое** raw-событие GuiImpl. Адаптер использует
ImGuiIO::AddKeyEvent/AddMouseButtonEvent/AddMousePosEvent/AddMouseWheelEvent/
AddInputCharacter/AddFocusEvent; устаревшая запись KeysDown/MouseDown удалена.
GUI получает обе границы короткого нажатия. ImGui может распределить границы
между своими кадрами через собственную очередь, сохраняя порядок.

До NewFrame фиксируется GuiInputCapture предыдущего завершённого GUI кадра.
InputRouter отдельно решает вторичную доставку в Editor/Runtime; capture не
мешает первичной передаче GUI. События мыши проверяются по собственным координатам
относительно последнего layout Scene/Game viewport. Active item и popup блокируют
новые жесты. Runtime требует Game viewport; захват клавиатуры обходится только
для фокуса viewport без active item/popup. Открытый PopupManager блокирует ввод
сразу, включая оставшиеся события текущего batch.

Клавиша сохраняет владельца от Press до Release. Первая кнопка мыши владеет всем
жестом, включая move за пределами viewport и дополнительные кнопки. Release без
Press игнорируется вторичными потребителями. Потеря фокуса, смена Edit/Run и новый
popup отменяют жесты, сбрасывают shortcuts, gizmo и накопленный runtime UI input.
Это работает и при пустом batch. Cancel — сигнал очистки, он не создаёт raw input.

Shortcuts обрабатывают Press по потоку, а не только по финальному состоянию кадра;
repeat не повторяет команду. UISystem хранит координаты каждого Press до update.
Это ещё не общий InputState: C# polling и распределение по времени fixed tick
остаются [INPUT-01 #79](https://github.com/Enziferum/robot2D/issues/79).
Нельзя обещать сохранение короткого нажатия для старого C# polling API.

## История редактора

`EventHistory` — диагностическое кольцо: 2048 событий и 2 MiB логического payload
по умолчанию. Перезапись старых записей или пропуск слишком большого drop не
изменяют очередь доставки. Budget считает структуры и длины строк, а не точный
heap allocator footprint. Snapshot владеет копией и переживает перезапись кольца.
В меню Events открывается EventMonitorPanel: запись/пауза, очистка, фильтр типа,
sequence/frame/time, маршрут и отмена. GUI в колонке маршрута означает решение
роутера; это не отчёт о фактическом потреблении конкретным ImGui widget.
Панель не реализует replay и не является очередью fixed updates.

## Проверки и оставшаяся приёмка

`editor-event-*`: variant и владение путями, FIFO и метаданные, typed binder,
GUI Press/Release/Text/Focus, реальный headless InputText capture, владение
Release/drag, popup/focus/mode cancellation, короткий shortcut и история/budget.
Тесты не требуют native окна. Сборка примеров проверяет миграцию публичного API.

Ручная приёмка перед merge: InputText не двигает камеру и не вызывает shortcuts;
drag из viewport завершается за его границей; popup/Alt-Tab/Play во время drag
сбрасывают жест; два быстрых клика по разным UI hitbox сохраняют координаты;
drop нескольких файлов показывает принадлежащие событию пути; крестик окна
показывает запрос сохранения и Cancel оставляет редактор открытым; Event Monitor
сохраняет sequence при переполнении/паузе записи. Эти UI-сценарии не объявляются
пройденными по headless тестам.

Мультиокна, WindowID, DPI/IME и контекстная модель рендера требуют отдельного
контракта. Текущий шаг не меняет глобальное владение DesktopWindowImpl и GLFW.

Локальный результат 2026-10-16, реализация `588d469`: pre-commit пройден без обхода,
Python 47/47, CTest 58/58, строгий clang-tidy 179 единиц собственного кода; отдельный
строгий анализ изменённого ImGui-адаптера (3 единицы). Редактор и перечисленные
примеры собраны на MinGW GCC 16.2/Ninja Release. Graphics tests выключены;
native UI-сценарии выше остаются приёмкой перед merge.


Публикация: [draft PR #122](https://github.com/Enziferum/robot2D/pull/122), ветка
`CORE-02`. После объединения с dev @ c05a55b (ASSET-02 #118) сохранены обе записи
roadmap и оба набора тестов. Повторный обязательный pre-commit: Python 47/47,
сборка редактора/suite, headless CTest 60/60 и строгий clang-tidy 180 единиц.
UI/graphics-приёмка остаётся перед merge; результаты CI проверяются в PR.
Регрессия runtime UI: два клика по разным hitbox и последующий MouseMoved до update сохраняют координаты и порядок callback; повторный update не повторяет команды, LostFocus отменяет накопленный ввод. Тест подтверждён падением до исправления и проходит после него.


## Согласованный следующий контракт

2026-10-16 пользователь согласовал [fixed timestep и RuntimeSession](fixed-timestep-contract.md).
Там отдельно перечислены существующие возможности и будущие API. Сначала
завершаем CORE-02; этот документ не добавляет в Event PR реализацию scheduler,
OnFixedUpdate, Pause/Resume или fixed-tick InputState.


### CORE-02: результат пользовательской приёмки и startup cancellation — 2026-10-16

Пользователь сообщил: остальные проверки работают, но после выбора проекта в
инспекторе видна малая область GUI слева снизу, затем приложение падает при
переходе в редактор. Журнал этого запуска содержит layout_restored=false;
robot2D.ini без сохранённых ImGui секций. Восстановление старого layout не
подтверждается, resize-приёмка пока остаётся открытой.

Стек GDB в тестовом worktree воспроизвёл SIGSEGV:
Scene::handleEventsRuntime ← Editor::cancelInput ← Application::handleEventFrame.
EditorLogic пересылал синтетический LostFocus в ещё отсутствующую m_activeScene
во время async-load. Добавлена проверка наличия сцены на границе forwarding;
headless editor-event-capture проверяет отмену/Release без активной сцены в
Edit/Load/LoadFailed. Повторная интерактивная проверка перехода и resize нужна
после пересборки; весь ручной сценарий не объявляется пройденным.

CI документационного HEAD cb36f2a: Linux успешен; Windows не дошёл до сборки из-за
Chocolatey HTTP 504 при скачивании Mono. Повторена только упавшая job. Сбой
инфраструктуры не считается результатом проверки Event-кода.


Повторная приёмка startup fix `36b6a11`: пользователь подтвердил «Открылся,
GUI на всё окно» на той же сборке и проекте. Журнал подтверждает Resized
2560x1417 и успешное применение сцены после async-load. Малый GUI перед падением
не был подтверждённым сохранением layout: после исправления отмены ввода
переход Inspector → Editor завершается штатно без удаления конфигурации.
Пользователь также сообщил, что остальные проверки работают нормально.

Проверки исправления: event subset 8/8; обязательный pre-commit — Python 47/47,
сборка robot2D/core-tests/editor-suite, полный headless CTest 60/60, строгий
clang-tidy двух изменённых cpp. Ранее успешный полный анализ 180 единиц и CI
не выдаются за новый прогон. Следующий шаг — CI нового head и финальное ревью
PR #122; merge пока не выполнялся. Crash report записан как отдельная будущая
потребность в development-tools.md; тайминг и reporter не реализуются этим fix.
