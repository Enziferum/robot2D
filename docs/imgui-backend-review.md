# Разбор robot2D ImGui и требования к Core

Дата: 2026-10-16. Статус: контракт renderer согласован пользователем для передачи
в Core; контракт GuiImpl и план Api/обёрток зафиксированы пользователем.
Пользователь отдельно поручил первый оконный шаг IMGUI-01; его результат
описан в конце. Остальные контракты не являются разрешением на реализацию.
База чтения: локальный `dev` @ `3e3cfe3`. Результаты ниже получены чтением кода;
они не заменяют воспроизведение дефектов и проверки будущих исправлений.

`robot2D-imgui` сохраняет собственные platform/render backend и использует Core
Graphics. Собственный MultiSelection редактора и SelectionService сохраняются.
Window/Event развиваются в другом чате; этот документ фиксирует согласованный
контракт потребителя. Имена API и способ реализации согласуются с Core отдельно.

Общий маршрут — [project-roadmap.md](project-roadmap.md), релизы —
[release-roadmap.md](release-roadmap.md). Связанные задачи:
[CORE-01 #110](https://github.com/Enziferum/robot2D/issues/110),
[CORE-02 #119](https://github.com/Enziferum/robot2D/issues/119),
[RENDER-04 #98](https://github.com/Enziferum/robot2D/issues/98),
[RENDER-05 #99](https://github.com/Enziferum/robot2D/issues/99),
[CODE-01 #50](https://github.com/Enziferum/robot2D/issues/50),
[UI-01 #48](https://github.com/Enziferum/robot2D/issues/48).
Новый TASK-ID здесь не назначается; перед реализацией согласовать регистрацию
конкретного объёма, ведущего и релиз. Это разбор потребителя общего маршрута.

## Синхронизация с работой над Core

Прочитан чат «Составить план ревью Core/Util». На момент чтения CORE-02 в работе
в отдельном worktree `core-02-events`, ветка `CORE-02`. Пользователь поручил там
Event на std::variant, типизированный EventBinder, адаптацию GuiImpl и тесты
порядка событий и захвата ввода. Сообщение чата о внесённых изменениях не означает
завершение проверок или интеграцию в dev.

В согласованном там направлении GuiImpl подаёт переходы через Add*Event;
маршрутизатор приложения решает capture и сохраняет владельца Press/Release.
Эти обязанности не переносить в renderer и не реализовывать второй раз здесь.
В прочитанном рабочем Event.hpp CORE-02 есть sequence/frame/receivedAt,
но нет WindowID. Это наблюдение о текущем незавершённом файле, не итоговый контракт.

Отправлен запрос о плане Window, WindowID, координатах, DPI, shared contexts и
границах Graphics. Дальнейшие требования из таблиц ниже требуют согласования
с владельцем Core. Код и рабочие файлы его worktree этим разбором не меняются.

## Карта интеграции

| Участок | Текущая ответственность | Предлагаемая граница |
|---|---|---|
| [Gui.cpp](../editor/extlibs/robot2D-imgui/src/robot2D/Gui.cpp), [GuiImpl.cpp](../editor/extlibs/robot2D-imgui/src/robot2D/GuiImpl.cpp) | Context, ввод, курсоры, NewFrame, запуск render | Gui управляет жизненным циклом; platform часть адаптирует Core Window/Event |
| [Render.cpp](../editor/extlibs/robot2D-imgui/src/robot2D/Render.cpp), [OrthoView.cpp](../editor/extlibs/robot2D-imgui/src/robot2D/OrthoView.cpp) | GL state, геометрия, shader, шрифты | Исполнение ImDrawData через низкоуровневые операции Graphics |
| [Api.cpp](../editor/extlibs/robot2D-imgui/src/robot2D/Api.cpp), [Sugar.hpp](../editor/extlibs/robot2D-imgui/include/robot2D/imgui/Sugar.hpp) | Окна, scopes, изображения, текст | Небольшие проверяемые обёртки без знания editor UUID/команд |
| [TreeHierarchy.cpp](../editor/src/panels/TreeHierarchy.cpp), SelectionService | Выделение и иерархия | Остаются ответственностью editor |

Названия новых классов и окончательные сигнатуры пока не выбраны; ниже приведён
эскиз согласованного контракта. Сам Dear ImGui — vendored
код; собственная интеграция — отдельная область. Обновление upstream не должно
смешиваться с незадокументированными локальными изменениями его исходников.

## Render и OrthoView

Приоритеты: P1 — исправить до опоры на backend в новых сценариях;
P2 — устранить при приведении интеграции в порядок. Это приоритеты плана,
а не утверждение, что каждый случай уже воспроизведён в UI.

| ID | Наблюдение в текущем коде | Исправление и наблюдаемая проверка |
|---|---|---|
| R1 P1 | OrthoView::getMatrix берёт `T = m_viewport.lx`, хотя верх задаёт `ly` | Использовать Y; проверить проекцию при DisplayPos=(120,350) и отрицательной позиции. Проверка только (0,0) не ловит дефект |
| R2 P1 | OpenGLContext сохраняет часть state. Shader в конце сбрасывается через unUse, старые program/VAO/texture/sampler не восстанавливаются. Sampler uniform=0, но active texture unit не устанавливается | Явный UI pass и договорённость о сохранении либо переустановке state; проверить ненулевой texture unit, чужой sampler/program/VAO до UI и корректный рисунок после UI |
| R3 P1 | Draw loop не обрабатывает UserCallback и reset render state | Исполнять callbacks в порядке команд, обрабатывать reset согласно закреплённой версии ImGui; регрессия callback между двумя рисунками |
| R4 P2 | IdxOffset используется, VtxOffset нет; RendererHasVtxOffset сейчас не объявлен | Добавить baseVertex или эквивалентное смещение vertex binding, затем объявить поддержку. Без флага текущий код ограничен размером списка, а не автоматически неверен для любого UI. Проверить draw list свыше 64K вершин |
| R5 P1 | setupGL создаёт сырые VBO/EBO, default destructor их не удаляет; отказ shader setup не передаётся GuiImpl | RAII, освобождение при частичном отказе, явный результат setup; проверить циклы setup/shutdown и отказ инициализации |
| R6 P2 | Параллельно создаются Core VertexArray/VertexBuffer, не используемые draw loop. Их layout задаёт цвет Float4 вместо packed RGBA8 | Оставить один путь через Graphics, с layout sizeof/offsetof(ImDrawVert). Удалять дубли после перевода; сравнить цвет/alpha по пикселям |
| R7 P2 | Для каждого CmdList вызывается glBufferData; VAO создаётся на render | Измерить upload bytes/calls и CPU/GPU время. Предложение: ёмкость буферов с ростом и загрузка геометрии крупными блоками; VAO на context. Не считать glBufferData само по себе доказанной проблемой скорости |
| R8 P2 | ClipRect учитывает DisplayPos/FramebufferScale, но нет clamp к target и пропуска равных границ | Определить преобразование/округление scissor, пересечение с framebuffer и пропуск пустых прямоугольников; проверить край target, fractional scale, minimized window |
| R9 P1 | Шрифты загружаются из внешнего vector, при этом FontDataOwnedByAtlas=true | Выбрать явное владение: копия в памяти ImGui либо хранение исходного буфера с правильным флагом. Не передавать vector::data как владение allocator ImGui; проверить memory-font и shutdown |
| R10 P2 | Renderer включает файл иконок из editor через относительный путь; собирает atlas и знает merge иконок | Набор шрифтов/иконок задаёт потребитель, renderer обслуживает GPU-текстуры. Локальная политика шрифтов может остаться в Gui; самостоятельный example не должен требовать editor/src |

Сохранять порядок CmdLists и CmdBuffer. Для объединённой загрузки учитывать
накопленные index/vertex offsets между списками плюс смещения команды.
Произвольный callback остаётся барьером. RenderTarget со слоями и камерой сцены
не использовать для UI без доказанного сохранения порядка, scissor и lifetime.

VAO на каждый render сейчас обеспечивает работу в текущем контексте; его
переиспользование допустимо только с привязкой к context и очисткой до уничтожения
этого context. Texture/buffer sharing не означает sharing VAO/FBO.

## GuiImpl и жизненный цикл

| ID | Текущее состояние | План |
|---|---|---|
| G1 | В dev используются KeyMap/KeysDown, polling мыши и ручные modifier fields | Принять и проверить результат CORE-02; не делать параллельную миграцию этого же файла |
| G2 | CreateContext вызывается без хранения собственного ImGuiContext*. Shutdown безусловно обращается к GetIO/DestroyContext, включая объект без setup | Явное владение context, определённая политика повторного setup, безопасный shutdown до setup/после отказа; все операции обращаются к своему context |
| G3 | GuiRender::setup возвращает void; дальнейший NewFrame/render возможен после отказа GPU setup | Публиковать ready только после успешного setup; откатывать частичные ресурсы, определить контракт ошибок и повторного запуска |
| G4 | DisplaySize берётся из Window::getSize, framebuffer scale не вычисляется; dt передаётся без проверки | Отделить logical size от framebuffer size, обновлять DPI/scale по согласованному Window API; определить обработку dt<=0 и minimized состояния |
| G5 | Clipboard handlers закомментированы, IME не интегрирован; часть курсоров заменена Arrow | Добавить clipboard/IME через Core platform сервисы и честные capabilities; проверить текст, Ctrl+C/V и позицию IME в отдельном окне |
| G6 | Один Window*, render только главного viewport, нет PlatformIO handlers | Сначала стабильный однооконный backend; multi-viewport подключить после Window-контракта из следующего раздела |

### Согласованный контракт GuiImpl

Зафиксирован по запросу пользователя 2026-10-16 для передачи в чат Core.
Это контракт потребителя, без запуска реализации; окончательные имена API,
механизм ошибок и связи с будущими сервисами Window/Graphics уточняются отдельно.
GuiImpl координирует ImGui context/кадр, platform backend и шрифтовый набор.
GuiRender исполняет GPU-команды, editor управляет панелями и маршрутизацией ввода.

### Учёт CORE-02

При последнем чтении рабочего дерева CORE-02 уже выделены
`feedGuiEvent(ImGuiIO&, const Event&)` и `GuiInputCapture` с полями mouse,
keyboard, activeItem, popup. События подаются через Add*Event; перезапись
KeysDown/MouseDown через polling удалена. Это состояние незавершённой ветки,
а не утверждение о merge или повторно выполненных проверках.

Сохранить адаптер, пригодный для headless-проверок без GPU renderer. Capture
снимается до NewFrame и не является результатом обработки отдельного события.
Все события ввода поступают Gui независимо от capture, строго по порядку.
Editor сохраняет владельца Press/Release; GuiImpl не дублирует маршрутизатор.
WantTextInput можно предоставить отдельно для запроса текстового ввода,
не подменяя им keyboard capture. Сохранить регрессии порядка, коротких
нажатий, InputText и смены capture из CORE-02.

### Состояния и кадр

```text
Uninitialized → Ready → BuildingFrame → DrawDataReady → Ready
                     ↘ cancelFrame() → Ready
Ready → shutdown() → Uninitialized
```

| Состояние | Разрешённые действия |
|---|---|
| Uninitialized | Настройка и initialize |
| Ready | Приём Event, capture snapshot и начало кадра |
| BuildingFrame | Построение UI прямыми вызовами ImGui и обёрток |
| DrawDataReady | Исполнение подготовленного UI для нужных targets |

Эскиз публичного API:

```cpp
bool initialize(const GuiConfiguration& configuration);
void shutdown();
void handleEvents(const Event& event);
GuiInputCapture inputCapture() const;
bool beginFrame(float deltaSeconds);
void endFrame();
void cancelFrame();
bool render(/* согласованные зависимости Graphics */);
```

EndFrame завершает построение через ImGui::Render, а cancelFrame заканчивает
кадр без подготовки рисунка. GPU render отделён от подготовки: Render вызывается
один раз для кадра и координирует исполнение по нужным viewport, не повторяя
ImGui::Render для каждого окна. Ссылки на draw data не переносятся в следующий
кадр. Порядок освобождения DrawDataReady после пропуска/ошибки GPU render
нужно явно определить при выборе API ошибок; застревать в этом состоянии нельзя.

Повторный initialize работающего Gui возвращает ошибку. Shutdown повторобезопасен,
в том числе до initialize; частичный отказ инициализации освобождает ресурсы
и возвращает объект в Uninitialized. Ready публикуется после успешной настройки
context, platform части, шрифтов и renderer. Ошибки порядка вызовов диагностируются.
BeginFrame требует конечный положительный deltaSeconds. Нулевой framebuffer
исключает рисование в этот target, но не обработку событий других окон.
Все операции Gui/ImGui выполняются на одном UI-потоке; фоновые задачи не меняют
ImGui или GPU-данные напрямую.

Порядок кадра: capture snapshot предыдущего кадра → ОС/Event batch → доставка
каждого события Gui и router editor → beginFrame → панели → endFrame →
исполнение UI в нужных targets. Перед исполнением UI завершить предшествующие
отложенные команды сцены согласно контракту Graphics.

### Владение context

Gui хранит и владеет собственным ImGuiContext*. Все его операции обращаются
к нему. Во время построения UI он текущий для прямых вызовов ImGui; после
завершения работы восстанавливается предыдущий context. Backend data принадлежит
экземпляру Gui. Для editor используется один ImGui context на группу окон;
дополнительная панель ОС не создаёт второй ImGui context.

### Шрифты и atlas

Font configuration владеет путём/исходными bytes. Не хранить заимствованные
string_view/vector* из текущего GuiFontConfig как долгоживущий источник.
Memory-font копируется в управляемую библиотекой память; она сохраняется
столько, сколько требует ImGui. Не передавать vector::data как владение
allocator ImGui. Потребитель использует FontId, а не сохраняет ImFont* между
перестройками. Merge явно указывает целевой FontId; default выбирается явно.

| Владелец | Ответственность |
|---|---|
| Editor | Роли шрифтов, Noto Sans/иконки, базовые размеры, пользовательский масштаб |
| Gui/шрифтовый набор | Источники, FontId, merge/default, ошибки и lifetime |
| ImGui | Формирование glyph atlas |
| GuiRender | GPU create/update/destroy atlas textures через Graphics |

Renderer не знает Font Awesome и editor/res. Для нового upstream учитывать
динамические atlas updates вместо контракта «один Build и одна texture навсегда»;
не сохранять предположение о неизменности GPU texture handle atlas.
В первом объёме набор шрифтов задаётся при initialize, горячая замена всего
набора отложена. Ошибка обязательного шрифта прерывает initialize с диагностикой
пути/FontId/причины. Встроенный fallback включается явной настройкой.

### DPI

Размеры шрифтов: базовый логический размер × пользовательский масштаб × DPI окна.
Пользовательский масштаб задаёт editor, DPI конкретного окна предоставляет Core.
Framebuffer scale отдельно выражает отношение pixel size к logical size;
не применять один коэффициент повторно на разных уровнях.
Хранить базовую тему и размеры, чтобы смена DPI не накапливала масштабирование.
Конкретные поля ImGui выбрать после закрепления upstream, с проверкой переноса
окон между мониторами разного DPI. Разовый getDPI при запуске не является
контрактом поддержки нескольких мониторов.

### Подготовка Multi-Viewports

Platform backend ведёт соответствие ImGuiViewportID → Core WindowID →
target/context и ownership/state. Главное окно заимствовано; Gui его не
уничтожает. Дополнительные окна создаются и уничтожаются через Core.
PlatformIO адаптирует create/destroy/show/move/resize/title/focus; renderer
исполняет draw data viewport через выбранный Graphics pass.

Для реализации нужны source WindowID и общий порядок Event, signed
client/desktop преобразования, logical/framebuffer sizes, DPI/monitor changes,
cursor/hover/drag capture и IME/clipboard нужного окна. Все координаты ввода
Multi-Viewports согласуются с desktop space; нельзя смешивать их с client
координатами текущего адаптера CORE-02.

Фокус окна отличать от фокуса всей группы Gui. Переход из главного окна в
откреплённую панель не сбрасывает ввод всего Gui. Прямое преобразование каждого
LostFocus в AddFocusEvent(false) из однооконной основы CORE-02 требует
адаптации с учётом событий группы. Межоконный drag сохраняет владельца
нажатия и получает завершение/отмену даже за границей исходного окна.

Multi-Viewports выбирается при initialize после проверки capabilities Core.
Если режим запрошен, но недоступен, вернуть понятную ошибку; однооконный режим
выбирается явно. Runtime-переключение режима в первый объём не входит.
Обновление platform windows и выбор/восстановление GL context координируются
внешним по отношению к GuiRender слоем; сам GuiRender context не переключает.
Event pump работает отдельно от swap и обслуживает дополнительные окна
при minimized главном окне.

### Завершение и первый объём

Shutdown требует живых сервисов Core platform/Graphics. Сначала завершить
или отменить открытый кадр, остановить доставку событий и отменить активный
ввод editor. Затем удалить дополнительные окна вместе с их context-local
ресурсами, освободить общие GPU-ресурсы при живом совместимом GL context,
удалить ImGui context и font sources. После этого приложение может уничтожить
основное окно и завершить Core. Внутри удаления окна context-local ресурсы
всегда освобождаются до его context. Точный механизм согласовать с Window.

Деструктор — запасная безопасная очистка, нормальный shutdown явный.
Это не гарантия удаления GPU-ресурсов после уничтожения GL context:
правильный порядок lifetime обязан обеспечить вызывающий код.

Первый объём: владение context и состояния кадра, безопасный initialize/shutdown,
передача ошибок setup, владеющий фиксированный font set и сохранение адаптера
CORE-02. Реестр viewport/PlatformIO подключить после согласования Window API.
Горячая замена font set, runtime-переключение multi-viewport и новый router
в этот объём не входят. Реализация ни одного из этих шагов пока не поручена.

Приёмка будущей реализации: shutdown до setup/дважды и после частичного отказа;
повторный initialize; собственный/предыдущий ImGui context; ошибочный dt;
cancelFrame и пропуск GPU render; память memory-font после освобождения буфера
вызывающего кода, missing required font и явный fallback; atlas region updates;
DPI без двойного масштаба; переход фокуса/drag между окнами, закрытие панели
без завершения приложения, minimized главное окно и восстановление GL context.
Сохранить сценарии CORE-02; эти проверки в данном разборе не запускались.

## Api и обёртки

| ID | Наблюдение | Предлагаемая правка |
|---|---|---|
| A1 | createWindow игнорирует bool Begin, всегда вызывает callback; windowOptions.disable выполняется до отложенного End и не выполнится при исключении callback | RAII для обеих обязанностей; callback только при видимости. Отдельно определить порядок style scope и End. Для скрытого dockspace сохранить KeepAliveOnly вне пропущенного содержимого |
| A2 | ImageButton использует старую перегрузку без явного строкового ID | Передавать независимый ID; проверить две кнопки с одной текстурой. Обновить вызывающие места при миграции upstream |
| A3 | RenderFrameBuffer безусловно разыменовывает Ptr | Определить предусловие либо безопасный пустой результат, политику UV flip и lifetime текстуры до исполнения draw data |
| A4 | DrawAnimatedImage не получает textureSize, а делит UV на его компоненты. Обе sprite-функции deprecated | Сначала проверить потребителей. Предложить единый Image(Sprite/TextureRegion), определить texture rect, zero-size и flip; не чинить неиспользуемый API без решения о сохранении |
| A5 | InputText повторяет стандартный resize adapter для std::string | Сохранить удобный API, сверить с misc/cpp/imgui_stdlib выбранного тега; проверить рост строки, UTF-8 и chaining callback. Сам const_cast не объявляется отдельным дефектом |
| A6 | Sugar хранит различие always-End/conditional-End, но TreeNode scope безусловно делает TreePop при open, даже с NoTreePushOnOpen. Несколько macros ссылаются на ImGuiSugar вместо robot2D::Sugar | Проверить именно используемые обёртки; учитывать flags, исправить namespace; compile smoke для публичных scopes плюс баланс стеков |
| A7 | DragDropTarget::unpackPayload<T> делает cast без DataSize/ограничения типа; PayloadBuffer хранит чужой адрес | Для простых значений проверять тип/размер, копировать trivially copyable payload; документировать borrowed lifetime. Для строк использовать имеющийся size-aware unpack, для сущностей — UUID, без знания editor в библиотеке |

### Зафиксированный план Api и обёрток

План обсуждён и зафиксирован по запросу пользователя 2026-10-16. Он определяет
границы и первый объём будущей работы, без разрешения начать реализацию.
Имена и сигнатуры ниже — эскизы. Прямой доступ к ImGui сохраняется: обёртки
добавляют безопасный контракт или повторяемое поведение, а не дублируют весь API.

| Область | robot2D-imgui | Editor |
|---|---|---|
| Окна | Корректные Begin/End scopes и параметры окна | IPanel, видимость, запрос фокуса, docking layout и сохранение настроек |
| Стиль | ScopedStyleColor/Var, ScopedID, font/disabled scopes | Тема, цвета ошибок, оформление конкретных панелей |
| Изображения | ImageRegion, Image/ImageButton, адаптация Texture/Sprite/FBO | ImagePreview с aspect fit и шахматным фоном |
| Текст | InputText для std::string, resize и callback adapter | RenameField с edit buffer, validation и политикой подтверждения |
| Drag/drop | Scopes, проверка типа/размера, копирование payload, preview/delivery | EntityPayload/AssetPayload, UUID и пути, правила допустимого переноса |
| Свойства | Универсальные примитивы ImGui и типы движка | PropertyGrid, vector controls, EditResult, mixed values, команды и dirty-state |
| Сцена | Не знает ECS, SelectionService и Snapshot | ComponentSection, ссылки на сущности, MultiSelection, Undo/Redo |

### Что сохранить и переделать

Сохранить RAII ScopedStyleColor/Var и работу с несколькими значениями;
добавить условное применение с именованным временем жизни. В текущем Inspector
временный ScopedStyleColor в ветке if уничтожается до следующего Selectable:
такое использование не окрашивает виджет. Условный scope должен охватывать
сам вызов рисования, а не только выражение создания guard.

**Окна и Sugar.** Основой сделать некопируемые WindowScope/ChildScope,
PopupScope/TableScope/ComboScope и TreeScope. Window/Child всегда завершают
Begin, остальные — по правилам своего Begin; TreeScope учитывает
NoTreePushOnOpen. Bool scope означает возможность рисовать содержимое.
Sugar может сохранить привычный синтаксис поверх этой основы; исправить
namespace и проверить совместимость с выбранным upstream. Новые macros,
которые лишь сокращают обычный if, добавлять по реальной пользе.

IPanel::renderWindow уже учитывает Begin, закрытие и focus; сохранить эту
политику в editor и переиспользовать библиотечный scope. Callback-вариант
возможен как тонкий адаптер, желательно без обязательного std::function.
WindowOptions разделить: title/flags/закрытие описывают окно, а типизированные
style scopes применяются отдельно. Нынешний map<int,vec2f> не выражает
скалярные style variables. Скрытый host dockspace продолжает обслуживаться.

**Изображения.** Общая основа вместо отдельных путей для обычного и
анимированного спрайта:

```cpp
struct ImageRegion {
    TextureView texture;
    robot2D::vec2f uv0;
    robot2D::vec2f uv1;
};

void Image(const ImageRegion& image, robot2D::vec2f size);
bool ImageButton(const char* id, const ImageRegion& image,
                 robot2D::vec2f size);
```

Texture/Sprite/FBO преобразуются в region с явными UV; анимация предоставляет
текущий region. ID кнопки независим от texture. Lifetime TextureView следует
согласованному Graphics-контракту. RenderFrameBuffer можно временно сохранить
как адаптер с прежним flip. Deprecated DrawImage/DrawAnimatedImage выводить
после проверки потребителей и наличия замены. Поведение пустого view,
нулевого размера и aspect fit определить отдельно.

**Текст.** Сохранить InputText с std::string; предпочтительный базовый API —
ссылка на строку, flags по умолчанию и отдельная перегрузка с callback.
Resize adapter сверить с misc/cpp/imgui_stdlib закреплённой версии. RenameField
в editor владеет буфером сессии: Enter подтверждает, Escape отменяет,
validation сообщает ошибку; политику потери фокуса выбрать явно. Само поле
не переименовывает файлы и не меняет компоненты.

**Payload.** DragDropTarget сохранить как scope. Вместо непроверенного T*
предусмотреть acceptValue<T>, возвращающий optional копии допустимого
trivially copyable значения после проверки типа и точного DataSize через
memcpy. Пригодность конкретного T для payload проверяется отдельно: один
trivially copyable не гарантирует отсутствие указателей или корректный lifetime.
Для текста возвращать владеющий std::string. Preview и окончательный delivery
различимы; подсветка drop не запускает действие. Entity/Asset payload задаёт
editor; библиотека не получает зависимость от его моделей. SimpleDragDrop и
colorButton остаются кандидатами на удаление после проверки потребителей:
их заменяют безопасный payload API и style scopes с обычной кнопкой.

### Контролы свойств редактора

Развивать существующие vector controls из
[ScenePanelUI.cpp](../editor/src/panels/ScenePanelUI.cpp), сохранив reset
и адаптивную раскладку. Вместо void возвращать результат взаимодействия:

```cpp
struct EditResult {
    bool changed;
    bool started;
    bool finished;
    bool cancelled;
};
```

Changed означает изменение в этом кадре, started/finished — границы сессии,
cancelled — явную отмену. Finished не гарантирует отличия от исходного значения.
Составной vector control агрегирует события осей и reset-кнопок. Перед
реализацией определить завершение при смене selection/закрытии панели и поведение
клавиатурного ввода/сброса. Границы считывать у внутренних виджетов, а не у
последнего элемента после EndTable. Снимок, применение команды, восстановление
значения при отмене и markModified выполняет editor. Snapshot не менять.

Для будущего группового Inspector предусмотреть mixed state по каждой оси
и маску изменённых компонент: изменение X не должно затирать разные Y.
Полная реализация multiedit не входит в первый объём.

| Следующий составной контрол editor | Назначение |
|---|---|
| PropertyGrid/PropertyRow | Label/value, адаптивная ширина и место для validation |
| AssetField | Preview/имя, drop, clear, reveal; возвращает действие |
| EntityReferenceField | Выбор/очистка UUID и отображение отсутствующей сущности |
| ComponentSection | Заголовок, enabled, меню Remove/Reset; возвращает запрос, не удаляет компонент внутри рисунка |
| IconButton | ID, tooltip, selected/disabled состояние toolbar |
| SearchField | Очистка и фокус; состояние фильтра принадлежит панели |
| ValidationMessage | Единое отображение ошибки рядом с полем |

Эти контролы развиваются после базовых примитивов по конкретным потребителям;
весь список не является одним поручением на реализацию.

### Первый объём будущей работы

1. Безопасные scopes: исправить A1/A6, определить style lifetime, переиспользовать
   основу в createWindow/IPanel, добавить нужные ScopedID/условные style guards.
2. Изображения: явный ID ImageButton и общий ImageRegion/UV-контракт;
   адаптировать существующие вызовы Scene/Game/toolbar. Окончательный TextureView
   зависит от Core Graphics; не заводить конкурирующий тип. Исправление ID/scopes
   не требует ожидания всего рефакторинга Core.
3. Payload: проверяемые копируемые значения и владеющий текст, явный delivery;
   адаптировать используемые drop-пути Assets/Inspector/Scene/Viewport/Animation.
4. EditResult у существующих vector controls и применение changed в Inspector;
   проверить reset и сессию drag. Команды Undo, Snapshot, групповой Inspector,
   PropertyGrid и весь набор новых составных контролов в этот шаг не входят.

Перед реализацией зарегистрировать/уточнить TASK-ID и ведущего, согласовать
точные сигнатуры и совместимость потребителей. Делить объём на проверяемые шаги;
не совмещать его с самостоятельной миграцией Window/Event из CORE-02.

Приёмка: баланс Begin/End и Push/Pop при false/раннем выходе, TreeNode с
NoTreePushOnOpen, закрытие/focus панели и скрытый dockspace; две ImageButton
с одной texture, UV Sprite/FBO; отклонение неверных payload type/size,
preview без действия и жизнь скопированного значения после drop; EditResult
для drag/reset/ввода и отсутствия изменения. Сохранить сценарии SelectionService
и MultiSelection. Проверки пока не выполнялись.

## Согласованный контракт рендера через Core Graphics

Пользователь согласовал фиксацию следующего контракта 2026-10-16 и передачу
его в чат Core для учёта при рефакторинге Graphics и Window. Это разрешение
на документацию и координацию; реализацию пользователь пока не поручал.
Контракт задаёт поведение и границы ответственности. Приведённые типы и имена
служат эскизом API, а не готовыми объявлениями существующих классов.

### Граница GuiRender и Core

`robot2D-imgui` переводит ImDrawData в последовательные операции Graphics.
Core предоставляет ресурсы и исполнение команд; он не включает заголовки ImGui
и не знает о ImDrawData, ImDrawCmd, ImTextureData или MultiSelection.

```cpp
class GuiRender {
public:
    bool initialize();
    void shutdown();
    bool render(const ImDrawData& drawData, GraphicsCommands& commands);
};
```

GuiRender владеет shader, streaming-буферами и служебными GPU-текстурами ImGui.
Он исполняет texture requests, готовит геометрию, проекцию, clip и draw ranges.
Ресурсы конкретного GL context учитываются отдельно. Выбор шрифтов и иконок
остаётся политикой потребителя/Gui, а не Graphics.

GuiRender получает команды уже открытого прохода. Он не выбирает окно,
не переключает GL context, не очищает target и не делает swap. Перед UI должны
быть исполнены предшествующие отложенные команды сцены. Вызывающий слой выбирает
target/context через согласованный API Core и затем открывает проход:

```cpp
auto pass = graphics.beginPass(target, passOptions);
guiRender.render(drawData, pass.commands());
```

UI-проход сохраняет существующее содержимое target. Размер target задан в
framebuffer pixels. Завершение прохода не делает swap. Этот путь Graphics
существует рядом со сценовым RenderTarget: UI не зависит от layer/entityID,
камеры или transform сцены. Точное место API согласовать с RENDER-04.

### Последовательное исполнение

```cpp
class GraphicsCommands {
public:
    void setViewport(PixelRect viewport);
    void setScissor(PixelRect rectangle);
    void setBlendState(const BlendState& state);
    void setDepthState(const DepthState& state);
    void setCullMode(CullMode mode);
    void bindShader(ShaderHandler& shader);
    void bindGeometry(const VertexArray& geometry);
    void bindTexture(unsigned slot, TextureView texture,
                     const SamplerState& sampler);
    void drawIndexed(const IndexedDraw& draw);
};
```

Вызовы сохраняют порядок; привязки и состояние относятся к следующему draw.
Исполнение синхронно по порядку CPU-команд, без требования завершать GPU после
каждого вызова. Публичный контракт первого шага не требует сохранять commands
между кадрами, вводить render thread или отложенный command buffer.
Обработка stencil, color mask и invalidation state cache также входит в
полноту прохода, хотя вспомогательные методы не перечислены в эскизе.

### Indexed draw и единицы измерения

```cpp
enum class IndexType { UInt16, UInt32 };

struct IndexedDraw {
    PrimitiveRenderType topology;
    IndexType indexType;
    std::uint32_t indexCount;
    std::uint32_t firstIndex;
    std::int32_t baseVertex;
};
```

`indexCount` — число индексов, `firstIndex` — смещение в индексах от начала
привязанного IBO, `baseVertex` — смещение в вершинах. Перевод firstIndex в
байтовый offset делает Graphics backend. Поддерживаются UInt16 и UInt32.
Для объединённой геометрии GuiRender складывает listIndexStart с IdxOffset,
listVertexStart с VtxOffset; сами индексы переписывать не требуется.
Проверять переполнение и представимость при переводе размеров ImGui в Core/GL.

### Streaming-буферы и layout

Для существующих VertexBuffer/IndexBuffer нужны общие по смыслу операции:

```cpp
std::size_t capacityBytes() const noexcept;
bool reserveBytes(std::size_t minimumCapacity);
bool updateBytes(std::size_t destinationOffset,
                 const void* source, std::size_t byteCount);
```

Reserve гарантирует минимальную ёмкость; сохранение прежних данных при росте
streaming-буфера не требуется. Update не растит capacity и сообщает об отказе
при выходе за границы. После успешного upload исходная CPU-память больше
не нужна. Идентичность объекта буфера для потребителя сохраняется; Core
обеспечивает корректные привязки после перевыделения storage. Актуальный объём
геометрии хранится отдельно от capacity. Правила состояния после отказа
выделения и единый способ диагностики уточнить при проектировании API ошибок.

Layout поддерживает явные stride/offset/location и packed нормализованный цвет:

```cpp
VertexLayout layout{
    sizeof(ImDrawVert),
    {
        {0, VertexFormat::Float2, offsetof(ImDrawVert, pos)},
        {1, VertexFormat::Float2, offsetof(ImDrawVert, uv)},
        {2, VertexFormat::UNorm8x4, offsetof(ImDrawVert, col)}
    }
};
```

Это код адаптера robot2D-imgui: Core видит только универсальный layout.
UNorm8x4 — четыре unsigned byte с нормализацией в [0,1] для float shader input.
Существующий автоматически упакованный layout можно сохранить как удобный API.
Persistent mapping, ring buffer и indirect/multi-draw не являются условием
первого этапа; оптимизации вводятся по измерениям с сохранением контракта.

### Текстуры и представления

Texture владеет ресурсом. TextureView — невладеющее представление для sampling,
получаемое от Texture или выбранного цветового attachment FBO. View не удаляет
ресурс; владелец сохраняет его до исполнения UI-команд. При пересоздании
attachment после resize старый view недействителен, нужен актуальный.

```cpp
bool updateRegion(PixelRect destination, const PixelDataView& source);
```

PixelDataView содержит адрес, размер доступных данных, формат и row pitch
в байтах. Region update проверяет границы, учитывает pitch, сохраняет texture
identity, не пересоздаёт texture и не требует полной CPU-копии Image.
Filter/wrap задаются явно SamplerState; Core может кешировать GPU samplers.
GuiRender связывает texture ID выбранной версии ImGui с допустимым view,
а также обслуживает создание, обновление и удаление atlas textures.

### Состояние и callbacks

| Состояние UI | Контракт |
|---|---|
| Topology | Triangles |
| Depth test / write | Выключены |
| Stencil test | Выключен |
| Culling | Выключен |
| Scissor | Включён |
| RGB blend | SrcAlpha, OneMinusSrcAlpha, Add |
| Alpha blend | One, OneMinusSrcAlpha, Add |
| Texture | Явная привязка к slot 0 и sampler |

PixelRect для viewport/scissor — целочисленный прямоугольник в framebuffer
pixels с началом сверху слева. Graphics OpenGL backend преобразует Y.
GuiRender учитывает DisplayPos/FramebufferScale, пересекает clip с target
и пропускает пустой результат. Правило округления fractional clip уточнить
до реализации и закрепить тестами, сохранив этот контракт координат.

Для первого этапа затрагиваемое GPU state сохраняется и восстанавливается
один раз на проход. Восстановление включает согласованность state cache Core;
оно не заменяет завершение ранее накопленной отрисовки сцены.

ImGui callbacks исполняет GuiRender в исходном порядке. Reset-команда повторно
устанавливает UI state. После произвольного callback, включая прямые GL-вызовы,
нужно инвалидировать затронутый cache Graphics и восстановить необходимые
привязки UI. Точный вспомогательный API invalidation/reset согласуется в Core;
он не должен требовать знания ImGui.

### Контексты и остающиеся решения Core

Buffer/texture/shader относятся к совместимой share group. VAO/FBO обслуживаются
для конкретного context и удаляются до его уничтожения. Context identity
предоставляет Graphics; GuiRender не выводит её из адреса Window. Core задаёт
видимость и синхронизацию записи в одном context перед чтением в другом.
GuiRender не переключает contexts самостоятельно.

С Core остаётся согласовать окончательные имена/сигнатуры, target/context
handles и capabilities, API ошибок и состояние ресурсов после отказа,
полноту сохраняемого state, invalidation после callback, округление clip,
thread/lifetime ограничения и межконтекстную синхронизацию. Эти детали
не означают разрешение начать реализацию Graphics или Window.

## Чего не хватает в Core Graphics

Это перечень операций, а не требование создать классы с указанными именами.
Реализацию и границы API согласовать с RENDER-04. Core не должен зависеть от
ImDrawData, ImGuiContext, ImTextureData или заголовков ImGui.

| Требование | Что уже есть | Недостающее и минимальная приёмка |
|---|---|---|
| Layout внешней вершины | Element/AttributeLayout, normalized, VertexArray | UNorm8x4 и явные stride/offset/location для packed/паддированных структур. Сейчас layout вычисляется как сумма размеров, byte4 нет. Проверка позиции/UV/цвета стандартного ImDrawVert без перекодировки в float4 |
| Обновляемая геометрия | VertexBuffer::Create(size), setData(data,size) | IndexBuffer upload после создания; явные byte capacity/used size; рост и update subrange, результат отказа. VBO сейчас пишет с offset=0 и только в существующую ёмкость. Uint16/Uint32 либо документированный вариант конвертации, без потери диапазона |
| Indexed draw диапазона | RenderTarget::draw(VertexArray, RenderStates), indexCount | Index type, firstIndex/byteOffset, baseVertex или эквивалент. Текущий OpenGL renderCache рисует UInt32 с offset=nullptr и отложенными states. Проверить несколько команд и несколько списков из общих буферов |
| UI pass и state | ShaderHandler, Texture, RenderTarget, blend enums | Target, viewport, scissor, отдельные blend factors/equations, sampler, depth/cull и граница чужого state. На первом этапе сохранять/восстанавливать state один раз на проход с согласованным cache. clearScissor не заменяет scissor отдельного draw |
| Texture create/update/destroy | Texture::create, bind, RAII destructor | Update rectangle без смены texture identity, source pitch/alignment, явные filter/wrap, безопасное владение GPU handle. Нынешний create пересоздаёт texture и копирует Image; для частого atlas update нужен иной путь. Проверить patch с ненулевым offset и padded source rows |
| Context-local ресурсы | VertexArray и FBO | Согласованный context identity/lifetime; создание и удаление VAO/FBO в правильном context. RenderAPI сейчас хранит глобальную версию API — отдельно определить допустимость для группы совместимых окон |
| Межконтекстный доступ | Texture/Buffer как GPU-объекты | Общая share group и политика видимости/синхронизации записи texture перед чтением из другого context. Sharing и синхронизация — разные обязанности; точный механизм выбрать в Core |
| Измерение стоимости UI | RenderStats сцены | Отдельные UI draw calls, upload calls/bytes, capacity growth и context switches; сравнение одинакового layout до/после. Общее устройство профайлера не является условием первого исправления |

Минимально важны layout, dynamic indices, draw ranges, scissor/state и texture
updates. Persistent mapping, ring buffers, indirect draws и multi-draw сейчас
не требуются: сначала корректный путь на OpenGL 4.3/4.5 и измерения.

## Запрос к рефакторингу Window

| Потребность robot2D-imgui | Что нужно определить в Core |
|---|---|
| Событие от конкретного окна | Стабильный WindowID и lifetime; окно-источник в event/envelope; единый порядок событий нескольких окон |
| Координаты desktop | Signed position, явные client/screen преобразования; монитор слева/сверху главного имеет отрицательные координаты |
| DPI и несколько мониторов | Logical size, framebuffer pixels, content scale; monitor bounds/work area; уведомления о смене DPI/мониторов |
| Дополнительное окно панели | Независимые create/show/hide/move/resize/title/focus/minimize/destroy; close request не завершает GLFW и приложение целиком |
| Свойства platform window | Decoration, resizable, topmost, taskbar, owner/parent и no-focus/no-input — по capabilities ОС; native handle для IME и платформенных требований |
| Ввод и курсор | Фокус по окнам, cursor enter/leave, захват drag за границами, hovered window; warp мыши в выбранное окно. Clipboard/IME — сервисы платформы |
| OpenGL | Создание context с sharing, получить/сделать current/восстановить прежний, swap конкретного окна, уничтожить в определённом порядке |
| Event pump | Обработка ОС отдельно от swap; обслуживать панели, даже когда главное окно minimized; не собирать несколько независимых несогласованных event loops |

ImGui context остаётся один, GL contexts может быть несколько. Нельзя напрямую
превращать LostFocus главного окна в потерю фокуса всего UI при переходе в другую
панель. При отсутствии capability backend должен иметь определённое поведение
и не заявлять соответствующий BackendFlag.

## Обновление Dear ImGui

Локально: `1.90 WIP`, `IMGUI_VERSION_NUM=18994`; README прямо указывает mix docking
и multiselection. Цель обсуждения — закреплённый upstream docking release,
актуальный при начале миграции. На дату разбора releases/latest указывает 1.93.0;
релиз перечисляет `v1.93.0-docking`. Тег и SHA проверить перед загрузкой.

Сначала инвентаризировать изменения vendored ImGui/imconfig и потребителей
imgui_internal.h, ImGuizmo, Spinner и Sugar. Отделить настройки интеграции от
патчей upstream; обновить исходники и заголовки согласованным набором. Сохранить
источник/лицензию/версию/SHA. CMake сейчас связывает даже цель imgui с Core и
использует корневые cmake helpers; независимость библиотеки проверять отдельно.

Для нового renderer предусмотреть texture requests create/update/destroy вместо
опоры только на разовый Build/GetTexData/SetTexID. BackendFlags выставлять после
реализации capability. Обновление upstream не требует перевода TreeHierarchy
на upstream MultiSelect. Публичные вызовы и собственные draw-list эффекты
проверить на изменения выбранной версии.

Источники для проверки контракта, прочитаны 2026-10-16:

- [Релиз 1.93.0](https://github.com/ocornut/imgui/releases/tag/v1.93.0).
- [Контракт собственного backend и texture requests](https://github.com/ocornut/imgui/blob/docking/docs/BACKENDS.md).
- [Multi-Viewports](https://github.com/ocornut/imgui/wiki/Multi-Viewports).
- [Официальный OpenGL backend как справочный пример](https://github.com/ocornut/imgui/blob/docking/backends/imgui_impl_opengl3.cpp).

## Предлагаемые шаги реализации

| Шаг | Граница результата | Зависимость и приёмка |
|---|---|---|
| 1. Согласовать Core | Ответ на таблицы Graphics/Window и регистрация небольшого объёма | Сначала актуальный итог CORE-02; не переносить его незавершённые файлы вручную |
| 2. Корректность текущего backend | R1–R3, R5, R9, G2/G3; необходимые A1/guards | Одно окно, воспроизводящие регрессии, graphics suite. Не ждать всей многооконности для локальных дефектов |
| 3. Поднять upstream | Закреплённый docking release и адаптация своего backend/API/texture lifecycle | Минимальные Graphics texture операции согласованы; редактор собирается, старые сценарии сохранены |
| 4. Единый Graphics путь | Один набор GPU-ресурсов, точный layout, draw ranges/state, VtxOffset | API Core из согласованных таблиц; R6/R4 и устранение дублирующего GL-пути |
| 5. Оптимизировать по измерениям | Буферы с ёмкостью, меньше uploads, VAO на context при готовом lifecycle | R7 и одинаковые нагрузки до/после, без изменения изображения и порядка |
| 6. Подключить multi-viewport | Собственный PlatformIO, дополнительные окна, восстановление context | Готовый Window-контракт, сначала Console/Inspector, затем Scene/Game; два монитора и DPI |
| 7. Развивать обёртки | Выбранные A2–A7, документация API, самостоятельный example | Карта потребителей и согласованный объём, без замены MultiSelection |

Контракт renderer выше зафиксирован пользователем; шаги реализации остаются
предложением для обсуждения. Расширения Core могут объединяться с его
рефакторингом; оптимизация не должна задерживать исправление корректности.
При миграции нельзя оставлять новый ImGui с несовместимым старым renderer.

## Проверки будущей реализации

- Headless: проекция с разными X/Y и отрицательным DisplayPos, scissor/scale,
  расчёт смещений/overflow и роста ёмкости, lifecycle до setup и после отказа.
- Graphics OpenGL 4.3/4.5: RGBA vertex color, alpha overlap, texture switches,
  несколько CmdLists, 16/32-bit индексы в поддерживаемых конфигурациях, >64K
  вершин, callbacks/reset, пустые/minimized draw data, patch atlas и state isolation.
- UI: dock/undock и сохранение layout, окна без видимого содержимого, ImageButton
  с общей texture, длинный UTF-8 InputText, clipboard, memory-font shutdown.
- После Window: второе окно с отрицательной позицией, разные DPI 100/150/200%,
  drag между окнами, фокус/IME, закрытие панели без завершения приложения,
  главный viewport minimized, Scene/Game texture из общей share group.
- Существующие регрессии CORE-02 на порядок/capture/Press–Release сохранить;
  отдельно проверить неизменность MultiSelection и команд editor.

Постоянные проверки размещать в существующей инфраструктуре
[editor/tests](../editor/tests) и [core/tests](../core/tests), согласно
[editor-testing.md](editor-testing.md). Для своей интеграции под extlibs явно
выбрать область форматтера/анализатора по [development-tools.md](development-tools.md);
не запускать анализ vendored Dear ImGui как собственного кода по умолчанию.

В исходном исследовательском разборе сборка, runtime-тесты, benchmarks и ручные UI-сценарии не запускались; проверки IMGUI-01 записаны отдельно ниже.
Следующий шаг — согласовать ответ Core и первый объём исправлений, затем выбрать
исполнителя. Сам исследовательский разбор не менял C++/Window/Event/MultiSelection.


## IMGUI-01 первый шаг реализации 2026-10-16

Задача [IMGUI-01 #121](https://github.com/Enziferum/robot2D/issues/121), ветка
`IMGUI-01` от удалённого dev @ c05a55b, отдельный worktree imgui-01-window-scopes.
Пользователь поручил только scopes окон/стилей и сохранение dockspace.

Добавлены WindowStyleScope и WindowScope. Стили WindowOptions удерживаются
до End, затем восстанавливаются; callback вызывается только при видимом Begin,
исключения не подавляются. createWindow и IPanel используют одну основу,
сигнатуры и политика видимости/focus панелей сохранены. UIManager поддерживает
скрытый dockspace через KeepAliveOnly с тем же ID в стеке host-окна.

Четыре headless-сценария imgui-scope-windows/unwind/panels/docking проверяют
видимое/свёрнутое окно, false open, ранний выход, исключение, вложенные scopes,
восстановление стилей, закрытие/показ/focus панели и сохранение DockId при
нескольких скрытых кадрах host. Тесты вызывают реальную общую функцию host,
используемую UIManager, без native окна и GPU context. На первом появлении
ImGui может измерять свёрнутое окно: тест false Begin задаёт фиксированный размер.

Windows GCC 16.2/Ninja Release: robot2D, robot2D-core-tests и
robot2D-editor-test-suite собраны; CTest 56/56 без пропусков (graphics выключены).
WindowScope прошёл отдельный строгий clang-tidy в явной области своей библиотеки.
Обязательный pre-commit прошёл: Python 47/47, формат, сборка, CTest 56/56
и строгий clang-tidy 179 собственных единиц. Проверки CORE-02 сюда не переносятся. GuiImpl/Event, Core Graphics/Window и vendored ImGui не менялись.

Ручная приёмка ещё нужна: закрыть Inspector крестиком, показать через меню,
проверить focus; сменить dock tabs, свернуть/восстановить host, сохранить layout,
перезапустить и проверить расположение/видимость панелей. Эти действия не
объявляются выполненными по headless-тестам. Следующий шаг — публикация PR в dev, CI и ручная приёмка. Остальные контракты в этом документе — планы.
