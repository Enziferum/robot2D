# Разбор robot2D ImGui и требования к Core

Обновлено: 2026-10-17. База сверки: удалённый `dev` @
`4132acf3120da33eebb7334a3d96d00ef7f40860`, после CORE-02 #122,
IMGUI-02 #130 и Window/Desktop #137. Основной checkout под IDE не переключался.
Первичный аудит выполнен 2026-10-16 по `3e3cfe3`; таблицы ниже обновлены по dev.
Контракты renderer/GuiImpl и план Api ранее зафиксированы пользователем.
Текущее поручение — актуализация документа и передача остатка требований в Core,
без новой реализации. Выполненные шаги и открытые предложения различаются явно.
Проверки прежних PR здесь не объявляются запущенными заново.

`robot2D-imgui` сохраняет собственные platform/render backend и использует Core
Graphics остаётся целевым путём: текущий GuiRender пока напрямую вызывает OpenGL.
Собственный MultiSelection редактора и SelectionService сохраняются.
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

Координация — чат «Составить план ревью Core/Util». Фактический результат:

| Область | Состояние на базе сверки |
|---|---|
| Event/input | [CORE-02 #122](https://github.com/Enziferum/robot2D/pull/122) merged: variant Event, typed EventBinder, FIFO/frame batch, Add*Event и capture; владельцы Press/Release остаются в editor |
| GuiRender/GuiImpl | [IMGUI-02 #130](https://github.com/Enziferum/robot2D/pull/130) merged: однооконная корректность, lifecycle, владеющий font set; контракт и прежние проверки — [gui-backend.md](gui-backend.md) |
| Window/Desktop | [PR #137](https://github.com/Enziferum/robot2D/pull/137) merged: DesktopRuntime, RAII native window, ошибки/rollback, polling с явным окном, общий pump и корректный GL shutdown |
| Scopes Api | [IMGUI-01 #121 / PR #123](https://github.com/Enziferum/robot2D/pull/123) открыт, реализован в отдельной ветке; конфликты с dev разрешены в ветке IMGUI-01, локальные повторные проверки пройдены; интеграция ещё впереди |
| Graphics / Multi-Viewports | Минимальный Core API ещё согласуется; собственного PlatformIO backend и нескольких GUI-окон нет |
| Payload / прочие обёртки | A7 обсуждён без реализации; A2–A6 остаются отдельными шагами |

У Event есть sequence/frame/receivedAt, но нет WindowID. Sequence отдельных
EventQueue не задаёт общий порядок нескольких окон. Наличие независимого
backend у каждого Window не означает готовность multi-viewports/context sharing.
getLogicalSize/getFramebufferSize уже есть: повторно заказывать их не требуется.

Оставшиеся запросы перечислены в таблицах Graphics/Window и пакете передачи ниже.
Имена API, capabilities и объём следующего шага согласует ведущий Core отдельно.
Код и рабочие файлы Core этим документом не меняются. Состояние Issue #110
не использовать как доказательство завершения всех требований: при сверке она
закрыта, хотя roadmap и сообщения Core ещё называют общий остаток открытым.

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

| ID | Состояние в dev | Оставшееся / приёмка |
|---|---|---|
| R1 P1 | Исправлено в IMGUI-02: проекция использует Y | Сохранить проверки разных X/Y и отрицательного DisplayPos |
| R2 P1 | Исправлено в текущем GL backend: state сохраняется один раз на проход, включая active texture/sampler/program/VAO | Перенести гарантию в Graphics pass; учитывать будущий binding cache |
| R3 P1 | UserCallback и reset обработаны, UI state восстанавливается после callback | Сохранить порядок/barrier и регрессии при переносе на Graphics |
| R4 P2 | IdxOffset поддержан; ненулевой VtxOffset отклоняется, RendererHasVtxOffset не заявлен | Нужны baseVertex или эквивалент и тест >64K вершин, затем capability |
| R5 P1 | Исправлено: VBO/EBO освобождаются, частичный отказ откатывается, ошибка передаётся GuiImpl | Сохранить setup/shutdown/rollback при переходе на Core ресурсы |
| R6 P2 | Неиспользуемый дублирующий VAO/VBO путь удалён; активный путь остаётся прямым GL | Нужен layout Core с packed RGBA8, sizeof/offsetof(ImDrawVert) для единого Graphics пути |
| R7 P2 | Upload через glBufferData для CmdList и временный VAO сохранены | Сначала измерить uploads/bytes/рост; capacity и VAO на context после Core API/lifetime |
| R8 P2 | Исправлено: clip округляется floor/ceil, пересекается с target; пустой пропускается | Сохранить fractional scale/minimized/DisplayPos регрессии |
| R9 P1 | Исправлено владение memory-font; конфигурация копируется | Сохранить font/source lifetime при миграции upstream |
| R10 P2 | Font set и merge задаёт потребитель, renderer не выбирает иконки editor | Добавить texture lifecycle выбранного upstream при его миграции |

Сохранять порядок CmdLists и CmdBuffer. Для объединённой загрузки учитывать
накопленные index/vertex offsets между списками плюс смещения команды.
Произвольный callback остаётся барьером. RenderTarget со слоями и камерой сцены
не использовать для UI без доказанного сохранения порядка, scissor и lifetime.

VAO на каждый render сейчас обеспечивает работу в текущем контексте; его
переиспользование допустимо только с привязкой к context и очисткой до уничтожения
этого context. Texture/buffer sharing не означает sharing VAO/FBO.

## GuiImpl и жизненный цикл

| ID | Состояние в dev | Оставшееся |
|---|---|---|
| G1 | Выполнено CORE-02: feedGuiEvent/Add*Event и capture snapshot, без прежней перезаписи polling | Сохранить router/Press–Release; адаптация к группе окон после WindowID |
| G2 | Выполнено IMGUI-02: собственный context, восстановление предыдущего, безопасный shutdown | Сохранить lifetime при добавлении platform windows |
| G3 | Выполнено: initialize/ready только после успеха; bool/lastError, rollback и состояния кадра | Не дублировать реализацию lifecycle |
| G4 | Logical/framebuffer size и scale, finite positive dt и пропуск нулевого target реализованы | Content scale/monitor/DPI changes и перенос окна между мониторами ещё нужны |
| G5 | Курсоры привязаны к DesktopRuntime, capabilities выставляются по готовности | Clipboard/IME и platform capabilities остаются запросом к Core |
| G6 | Стабилизирован однооконный backend, один заимствованный Window | PlatformIO, registry viewport→WindowID→context/target и multi-viewports не реализованы |

### Согласованный контракт GuiImpl

Зафиксирован по запросу пользователя 2026-10-16 для передачи в чат Core.
Это контракт потребителя, без запуска реализации; окончательные имена API,
механизм ошибок и связи с будущими сервисами Window/Graphics уточняются отдельно.
GuiImpl координирует ImGui context/кадр, platform backend и шрифтовый набор.
GuiRender исполняет GPU-команды, editor управляет панелями и маршрутизацией ввода.

### Учёт CORE-02

CORE-02 уже в dev через #122. Выделены `feedGuiEvent(ImGuiIO&, const Event&)`
и `GuiInputCapture` с mouse/keyboard/activeItem/popup. События подаются через
Add*Event; прежняя перезапись KeysDown/MouseDown через polling удалена.
Это реализованная однооконная основа, а не ожидаемая зависимость.

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
в этот объём не входят. Первый однооконный объём уже реализован IMGUI-02;
следующие пункты не являются поручением на реализацию.

Приёмка lifecycle/font set выполнена в IMGUI-02; сохранить регрессии: shutdown до setup/дважды и после частичного отказа;
повторный initialize; собственный/предыдущий ImGui context; ошибочный dt;
cancelFrame и пропуск GPU render; память memory-font после освобождения буфера
вызывающего кода, missing required font и явный fallback; atlas region updates;
DPI без двойного масштаба; переход фокуса/drag между окнами, закрытие панели
без завершения приложения, minimized главное окно и восстановление GL context.
Сохранить сценарии CORE-02. Atlas updates, DPI и межоконные сценарии остаются
будущей приёмкой; при актуализации документа тесты заново не запускались.

## Api и обёртки

| ID | Наблюдение | Предлагаемая правка |
|---|---|---|
| A1 | В dev ещё старый путь; исправление WindowScope/styles/createWindow/IPanel и dockspace реализовано в IMGUI-01 #123 | PR не merged; в IMGUI-01 разрешены конфликты с dev, сохранён GuiImpl и пройдены повторные локальные проверки; не считать завершённым в dev |
| A2 | ImageButton использует старую перегрузку без явного строкового ID | Передавать независимый ID; проверить две кнопки с одной текстурой. Обновить вызывающие места при миграции upstream |
| A3 | RenderFrameBuffer безусловно разыменовывает Ptr | Определить предусловие либо безопасный пустой результат, политику UV flip и lifetime текстуры до исполнения draw data |
| A4 | DrawAnimatedImage не получает textureSize, а делит UV на его компоненты. Обе sprite-функции deprecated | Сначала проверить потребителей. Предложить единый Image(Sprite/TextureRegion), определить texture rect, zero-size и flip; не чинить неиспользуемый API без решения о сохранении |
| A5 | InputText повторяет стандартный resize adapter для std::string | Сохранить удобный API, сверить с misc/cpp/imgui_stdlib выбранного тега; проверить рост строки, UTF-8 и chaining callback. Сам const_cast не объявляется отдельным дефектом |
| A6 | Sugar хранит различие always-End/conditional-End, но TreeNode scope безусловно делает TreePop при open, даже с NoTreePushOnOpen. Несколько macros ссылаются на ImGuiSugar вместо robot2D::Sugar | Проверить именно используемые обёртки; учитывать flags, исправить namespace; compile smoke для публичных scopes плюс баланс стеков |
| A7 | DragDropTarget::unpackPayload<T> делает cast без DataSize/ограничения типа; PayloadBuffer хранит чужой адрес | Предложены optional копии допустимого значения, владеющий текст и delivery-only accept; отдельный inspect для preview. UUID/пути остаются в editor; точный API пока не утверждён и не реализован |

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

### Объём Api и следующий независимый шаг

1. IMGUI-01: scopes окон/стилей и dockspace уже реализованы в PR #123;
   синхронизация и интеграция ещё впереди. A6/Sugar не объявлять исправленным этим PR.
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
и MultiSelection. Для A1 есть прежние проверки ветки IMGUI-01; остальные пункты
не объявляются проверенными, после синхронизации #123 повторный полный CTest прошёл 228/228, включая 55 graphics.

### Payload: обсуждённое предложение A7, без реализации

Следующий независимый от Core шаг: сохранить DragDropTarget как RAII scope;
acceptValue<T>/acceptText возвращают optional владеющего результата только при
delivery. Отдельные inspectValue/inspectText возвращают копию и явные состояния
preview/delivery; наличие preview не запускает действие. Raw AcceptBeforeDelivery
не должен менять контракт accept. В одном target сохранить проверку нескольких ID.

Предложены симметричные setValuePayload/setTextPayload. Для value недостаточно
trivially_copyable: нужен явный допуск типа; первый формат — uint64_t, editor
сам преобразует UUID. Проверяются ID, DataSize, адрес и точный размер; чтение
через memcpy не зависит от выравнивания. Текст копируется по длине, допускает
один завершающий NUL и сохраняет внутренние NUL. Editor-адаптер пути отклоняет
пустое значение и внутренний NUL до файловой операции.

Миграция: Assets/Inspector/Scene/Viewport/Animation и прямой drag/drop TreeHierarchy,
без изменения MultiSelection, очереди reorder/reparent или модели команд.
Предложено удалить старые unpackPayload<T>/PayloadBuffer/unpackPayload2Buffer
и deprecated SimpleDragDrop после миграции всех потребителей; это изменение
публичного API ещё требует согласования, как и окончательные сигнатуры.

Постоянные headless-тесты: type/size/null/недопустимый размер, невыровненный вход,
сохранность копии после изменения источника; текст с/без конечного NUL и встроенным
NUL, отказ editor-пути; настоящий ImGui context и несколько кадров drag/preview/
delivery/cancel, несколько ID одного target и баланс scopes при раннем выходе.
Это предложенный объём, не новая зарегистрированная IMGUI-задача и не реализация.

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

Уточнение по dev: Core сейчас хранит очередь draws, но не кеширует GL bindings
с пропуском bind. Поэтому текущий IMGUI-02 обходится без нового invalidation API;
вызывающий слой завершает очередь через RenderTarget::afterRender(). Требование
согласованности cache относится к будущему Graphics pass, если такой cache появится.

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

Уже реализованы и повторно не запрашиваются: DesktopRuntime owned/borrowed,
единоличное владение native window каждого Window, ошибки создания/rollback,
разрушение GPU до native context, makeCurrent конкретного окна, polling с явным
окном и единый processEvents отдельно от swap. Logical/framebuffer sizes тоже
уже доступны. Ни один из этих пунктов сам по себе не включает Multi-Viewports.

| Потребность robot2D-imgui | Что нужно определить в Core |
|---|---|
| Событие от конкретного окна | Стабильный WindowID и lifetime; окно-источник в event/envelope; единый порядок событий нескольких окон |
| Координаты desktop | Signed position, явные client/screen преобразования; монитор слева/сверху главного имеет отрицательные координаты |
| DPI и несколько мониторов | Сохранить существующие logical/framebuffer sizes; добавить content scale, monitor bounds/work area и уведомления о смене DPI/мониторов |
| Дополнительное окно панели | Независимое владение/create/close уже есть; дополнить show/hide/focus/state и операции platform window, не завершать весь runtime при закрытии панели |
| Свойства platform window | Decoration, resizable, topmost, taskbar, owner/parent и no-focus/no-input — по capabilities ОС; native handle для IME и платформенных требований |
| Ввод и курсор | Фокус по окнам, cursor enter/leave, захват drag за границами, hovered window; warp мыши в выбранное окно. Clipboard/IME — сервисы платформы |
| OpenGL | makeCurrent/display и порядок уничтожения уже есть; нужны shared creation, context identity, получение/восстановление прежнего current context, согласование lifetime с Graphics |
| Event pump | Единый pump уже реализован; проверить обслуживание панелей при minimized главном окне и сохранить один цикл, не добавлять конкурирующие loops |

ImGui context остаётся один, GL contexts может быть несколько. Нельзя напрямую
превращать LostFocus главного окна в потерю фокуса всего UI при переходе в другую
панель. При отсутствии capability backend должен иметь определённое поведение
и не заявлять соответствующий BackendFlag.

### Пакет оставшихся требований для чата Core — 2026-10-17

Это передача требований потребителя для согласования контрактов, без поручения
реализовать Core или менять GuiImpl/CORE-02. Последовательность предлагается,
а не считается принятым исполнителем планом:

1. Graphics, совместно с RENDER-04 #98: выбрать минимальный последовательный
   pass/target API; packed vertex layout; streaming VBO/IBO; UInt16/UInt32,
   firstIndex/baseVertex; texture views/region updates/row pitch. Определить
   ошибки, lifetime до исполнения, callback reset и границу состояния.
2. Window/platform: WindowID и межоконный порядок Event; signed desktop/client
   coordinates; content scale/monitors/DPI notifications; platform window
   properties/capabilities, hover/focus/drag capture, clipboard/IME.
3. Общая граница Window/Graphics: shared context creation, context identity,
   current-context restore, context-local VAO/FBO cleanup и видимость/synchronization
   записей общих textures/buffers перед чтением в другом context.

Минимальная приёмка Graphics: packed color/alpha, несколько draw lists/ranges,
оба index type и >64K вершин, clip с отрицательным DisplayPos/fractional scale,
atlas patch с offset/padded rows, callbacks/state и Core draw после GUI.
Минимальная приёмка Window: монитор слева/сверху, разные DPI и перенос панели,
межоконный drag и фокус/IME, закрытие панели без выхода приложения, minimized
главное окно, общий texture из Scene/Game и правильный shutdown обоих contexts.

Однооконные исправления #122/#130/#137 сохраняются. Backend не переключает
contexts внутри GuiRender; это координирует platform/вызывающий слой. ImGuiContext
один на группу окон, Core не зависит от ImGui. Отдельно согласовать имена API,
capabilities, следующий TASK-ID/ведущего и объём; не создавать дубли существующих
RENDER-04/05. Upstream upgrade и собственный MultiSelection остаются работой GUI.

## Обновление Dear ImGui

В dev базы сверки: `1.90 WIP`, `IMGUI_VERSION_NUM=18994`; README прямо указывает mix docking
и multiselection. Цель обсуждения — закреплённый upstream docking release,
актуальный при начале миграции. В первичном разборе рассматривался 1.93.0;
при этой актуализации внешний upstream не перепроверялся. Тег, SHA и контракт
texture updates проверить перед загрузкой, не считать прежний номер актуальным автоматически.

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
| 1. Согласовать Core | Ответ на таблицы Graphics/Window и регистрация небольшого объёма | CORE-02 и Window/Desktop уже merged; согласовать именно оставшиеся операции |
| 2. Корректность текущего backend | R1–R3, R5, R8–R10, G1–G3 и однооконная часть G4 выполнены в dev | A1 — отдельный открытый PR #123; Graphics migration и Multi-Viewports этим результатом не закрыты |
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

При актуализации документа сборка, runtime-тесты, benchmarks и ручные UI-сценарии
заново не запускались. Прежние результаты IMGUI-02/CORE-02/Window описаны в их
PR и gui-backend.md/core-api.md. Далее согласовать ответ Core и отдельные объёмы
Graphics/Window; payload остаётся независимым предложенным GUI-шагом.
C++/Window/Event/MultiSelection этим документом не изменяются.

## IMGUI-01: синхронизация с dev — 2026-10-17

По поручению пользователя в отдельный worktree IMGUI-01 объединён origin/dev
@ 4132acf. Конфликты реестров задач и документации разрешены с сохранением
обоих наборов записей; все регистрации headless/graphics тестов сохранены.
GuiImpl.cpp/.hpp не отличаются от dev, CORE-02 и Core API не менялись.
WindowScope сохраняет требование живого текущего context на срок scope.
В imgui-scope-unwind добавлена проверка двух contexts: исключение callback
в GuiContextScope восстанавливает окна/стили выбранного context, затем прежний
context и его стеки остаются пригодными для продолжения кадра.

Windows Ninja/Release GCC 16.2.0: robot2D, robot2D-core-tests и
robot2D-editor-test-suite собраны; полный CTest 228/228 без пропусков,
включая 55 graphics и четыре imgui-scope сценария. Локальные ссылки проверены.
Обязательный hook и CI актуального commit фиксируются в PR #123; прежний CI
f4fc9c3 не является проверкой объединённой версии. Native UI-приёмка окон,
крестика Inspector и layout после перезапуска остаётся отдельным шагом.
