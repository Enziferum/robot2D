# Согласованный контракт Engine / Application / RuntimeSession

Пользователь согласовал контракт 2026-10-16. Этот документ фиксирует направление
следующего шага, **не объявляет API реализованным и не разрешает начинать его
реализацию сейчас**. Сначала завершаем CORE-02 и его приёмку.

Связанные задачи: [CORE-02 #119](https://github.com/Enziferum/robot2D/issues/119),
[PHYS-01 #78](https://github.com/Enziferum/robot2D/issues/78),
[INPUT-01 #79](https://github.com/Enziferum/robot2D/issues/79),
[RUNTIME-01 #77](https://github.com/Enziferum/robot2D/issues/77),
[RUNTIME-02 #37](https://github.com/Enziferum/robot2D/issues/37).
Общий маршрут остаётся в [project-roadmap.md](project-roadmap.md),
релизные границы — в [release-roadmap.md](release-roadmap.md).

## Что уже существует, а что только согласовано

| Область | Подтверждённое состояние | Согласовано для следующего шага |
|---|---|---|
| Engine | Есть накопитель и постоянный шаг `1 / maxFrameRate`; `Application::update(float)` вызывается из этого цикла | Независимый fixedDt и отдельные кадровый/fixed callbacks |
| Перегрузка | В существующем цикле нет ограничения догоняющих шагов/входного интервала | maxStepsPerFrame, maxFrameDt, отбрасывание целого долга |
| C# | Есть OnUpdate(float), сейчас вызываемый из runtime update на шагах Engine | OnFixedUpdate и перевод OnUpdate на кадровую частоту |
| Физика | Адаптер получает dt аргументом update и вызывает Box2D Step | Тот же адаптер подключается к единому fixed tick; своего накопителя не получает |
| Ввод CORE-02 | В ветке CORE-02 реализованы FIFO/frame batch до fixed updates, typed Event, queued GUI input, routing/ownership/cancel и история | Независимые frame/fixed представления ввода и распределение границ по ticks ещё не реализованы |
| Жизненный цикл | Есть существующий Play/Stop; отдельного общего SimulationControl/RuntimeSession API из этого документа нет | Running/Paused/Stopped, очереди переходов, SessionId, сброс игрового времени |
| Player | Engine/Application уже являются точкой запуска приложения | Один scheduler и один общий игровой runtime для редактора и будущего Game.exe |

Сверка исходного состояния: реализация CORE-02 @ `1703688`,
[draft PR #122](https://github.com/Enziferum/robot2D/pull/122), ещё не merged на
момент записи. Windows и Linux CI на этом commit завершились успешно; это
проверки Event-реализации, **не будущего тайминга**. Ручная приёмка CORE-02
остаётся отдельным шагом. Ранее выполненные проверки не запускались заново
из-за добавления этого документа.

## Ответственности и конфигурация

Engine владеет главным runLoop, измерением кадрового времени, fixed-step
scheduler, накопителем, состоянием контроллера симуляции и выдачей SessionId.
Application подключает runtime и интерфейс конкретного приложения.
RuntimeSession владеет игровым миром, физикой, скриптовыми экземплярами и
связанными ресурсами, выполняет полученные вызовы. Второго накопителя и runLoop
в RuntimeSession, физическом адаптере или Game.exe не будет.

Редактор и Game.exe используют один Engine и контракт Application. Редактор
добавляет панели и инструменты, player запускает ту же игровую сессию без них.
Core не должен зависеть от классов editor или заголовков Box2D.

Согласованный эскиз конфигурации; это не существующие объявления заголовков:

```cpp
struct SimulationSettings {
    double fixedDt = 1.0 / 60.0; // Секунды.
    unsigned maxStepsPerFrame = 8;
    double maxFrameDt = 0.25;   // Секунды.
};

struct EngineConfiguration {
    // Существующие настройки окна и контекста.
    SimulationSettings simulation;
};

struct RuntimeSettings {
    PhysicsSettings physics;
};

struct GameConfiguration {
    EngineConfiguration engine;
    RuntimeSettings runtime;
};
```

У fixedDt один источник — EngineConfiguration. У PhysicsSettings нет своего
fixedDt. Gravity и параметры физического backend принадлежат runtime settings;
Box2D-specific solver iterations не превращаются в зависимость Core от Box2D.
Точную декларацию PhysicsSettings и формат её сохранения согласуем при интеграции.

Настройки игры читаются редактором из проекта и передаются в RuntimeSession при
Play. При сборке player они сохраняются и читаются Game.exe. Настройки окна
самого редактора и расположение панелей не являются настройками игрового мира.
Поставка Game.exe и этот конфигурационный путь пока не объявляются готовыми.

Накопитель использует double; dt в C++/C# callbacks остаётся float в секундах.
Интервалы должны быть конечными и положительными, maxStepsPerFrame — ненулевым.
Настройки симуляции фиксируются при Play, изменение применяется при новом запуске.
VSync и ограничение FPS независимы от fixedDt. Миграцию неоднозначного старого
maxFrameRate выполняем явно, без молчаливого изменения его смысла.

## Контексты времени и управление

```cpp
using SessionId = std::uint64_t;

struct FixedTick {
    SessionId sessionId;
    std::uint64_t index; // Первый tick — 0.
    float deltaTime;     // Настроенный fixedDt.
    double startTime;    // Игровое время в начале tick.
};

struct FrameTime {
    std::uint64_t index;
    double realDeltaTime; // Измеренный кадровый интервал.
    float deltaTime;      // После ограничения maxFrameDt.
};

enum class SimulationState { Stopped, Running, Paused };

struct SimulationStatus {
    SimulationState state;
    SessionId sessionId;
    std::uint64_t completedTicks;
    double elapsedTime;
};

class SimulationControl {
public:
    void requestPlay();
    void requestPause();
    void requestResume();
    void requestStop();
    SimulationStatus status() const noexcept;
};

struct EngineContext {
    SimulationControl& simulation;
};
```

request-методы ставят команды в очередь. Это не синхронное создание/удаление
мира и не подтверждение успешного перехода. status сообщает уже применённое
состояние. Контроллер принадлежит Engine, доступен Application до его завершения.
Исходная модель — главный поток; потокобезопасная очередь этим контрактом не обещана.

SessionId не переиспользуется в течение жизни Engine. После неудачной попытки
старта допустим пропуск номера. Это идентификатор поколения сессии, а не замена
CancellationToken или правилам времени жизни объектов. Последние остаются в
согласованных пользовательских задачах.

## Эскиз Application и RuntimeSession

```cpp
class Application {
public:
    virtual void setup(EngineContext& context) = 0;
    virtual void destroy();
    virtual void handleEventFrame(const FrameEvents& events);
    virtual void update(const FrameTime& frame);
    virtual void fixedUpdate(const FixedTick& tick);
    virtual void guiUpdate(float deltaTime);
    virtual void render() = 0;

    virtual SimulationStartResult beginSimulation(SessionId id);
    virtual void endSimulation();
    virtual void simulationStateChanged(
        const SimulationStatus& before,
        const SimulationStatus& after);
};

class RuntimeSession {
public:
    SimulationStartResult start(SessionId id, const RuntimeSettings& settings);
    void fixedUpdate(const FixedTick& tick);
    void update(const FrameTime& frame);
    void cancelInput();
    void stop();
    bool isActive() const noexcept;
};
```

SimulationStartResult сообщает успех или причину ошибки старта. Точное устройство
результата, значения по умолчанию виртуальных методов и размещение новых типов
уточним перед реализацией; эти фрагменты показывают согласованный внешний смысл,
не являются полным компилируемым header.

Application::update вызывается каждый кадр, в том числе Stopped/Paused, чтобы
работали сервисы приложения и редактор. fixedUpdate вызывается только в Running,
0…N раз за кадр. Application вызывает RuntimeSession::update только в Running.
guiUpdate/render продолжают работать при паузе. RuntimeSession::isActive означает
наличие runtime, включая Paused; второй контроллер Running/Paused внутри него
не должен расходиться с Engine.

Игровой C# OnUpdate(float) выполняется один раз за игровой кадр только в Running,
после fixed ticks; OnFixedUpdate(float) — один раз на каждый tick. Физическое
управление переносится в OnFixedUpdate. Существующий OnUpdate сейчас фактически
fixed, поэтому смена частоты является несовместимым изменением поведения и требует
аудита скриптов. Не вводим скрытый двойной вызов старого OnUpdate.

## Порядок кадра и физического шага

```text
GLFW pump → owning FIFO batch
→ raw GUI input и маршрутизация игрового ввода
→ применить запрошенные переходы на безопасной границе
→ проверить закрытие приложения
→ если Running: 0…N fixed ticks
→ Application::update(frame), внутри игровой OnUpdate только в Running
→ применить команды после update
→ GUI, render, swap; при необходимости ограничение FPS
```

Один RuntimeSession::fixedUpdate выполняет:

```text
Подготовка fixed input и разрешённых отложенных операций
→ C# OnFixedUpdate(fixedDt)
→ подготовленные физические команды
→ один Box2D Step(fixedDt)
→ синхронизация физических transforms
→ collision/trigger callbacks после разблокировки мира
→ безопасное завершение отложенных операций
```

Один OnFixedUpdate и один Step относятся к одному tick с одинаковым dt.
События контактов доставляются после каждого выполненного Step, не один раз
за render frame. Детали создания/удаления сущностей и физических тел сохраняют
контракты PHYS-01/RUNTIME-01; новый scheduler не разрешает менять мир внутри Step.
Core ECS этим документом не перерабатывается.

## Накопитель и перегрузка

В Running поступивший реальный интервал ограничивается maxFrameDt и добавляется
в накопитель. Выполняется не больше maxStepsPerFrame полных шагов. После лимита
оставшийся долг на целые fixedDt отбрасывается, дробный остаток сохраняется.
Ни один Step не получает увеличенный dt для компенсации долга.

При 60 Hz и пустом накопителе кадр 200 ms даёт максимум 8 шагов: около 133.33 ms
выполненной симуляции и 66.67 ms отброшенного долга. Игровое время отстаёт от
реального при перегрузке; это выбранный компромисс для отзывчивости редактора и
локальной игры. Таймеры игры используют выполненное время симуляции.

Диагностика содержит число шагов, остаток и отброшенное время; не добавляем INFO
на каждый кадр. Точный способ измерения/представления счётчиков — при реализации.

## Play / Pause / Resume / Stop

| Переход | Ресурсы и время |
|---|---|
| Stopped → Play | Новый SessionId, создание runtime; после успешного start — Running, tick 0, elapsed 0, пустой накопитель |
| Running → Pause | Мир, скорости и контакты сохраняются; игровое время заморожено, дробный накопитель очищен |
| Paused → Resume | Та же сессия и следующий tick, без времени паузы в накопителе |
| Running/Paused → Stop | Завершение и освобождение runtime, сброс накопителя, Stopped |

Ошибка старта оставляет Stopped, освобождает частично созданные ресурсы и
возвращает причину для UI/журнала. Running публикуется только после успеха.
Время загрузки при Play не попадает в первый игровой интервал.

Повторный Play в Running/Paused не пересоздаёт сессию: продолжение выполняется
Resume. Повторный Pause в Paused и Stop в Stopped безопасны. Команды обрабатываются
последовательно: Stop → Play за один кадр создаёт новую сессию с новым ID и
обнулённым игровым временем, даже если внешнее состояние снова стало Running.

Безопасные границы: после входного batch, между завершёнными ticks и после
Application::update. Команды из построения GUI применяются на следующем кадре.
Stop/Pause из OnFixedUpdate или контактного callback ставятся в очередь; текущий
tick безопасно завершается. После применённого Stop/Pause оставшиеся ticks и
игровой OnUpdate старой сессии в этом кадре не выполняются. Это приоритет Stop
над продолжением шага, а не перестановка FIFO-команд Stop → Play.

## Ввод и последующие шаги

Frame input и fixed input имеют независимые отметки чтения: чтение Press в
OnUpdate не отнимает его у OnFixedUpdate. Down сохраняется между ticks, границы
Press/Release выдаются одному tick, упорядоченный список сохраняет несколько
быстрых переходов. Отбрасывание долга не должно терять эти переходы: их перенос
на следующую выполняемую временную границу требует отдельной реализации INPUT-01.
Она ещё не выполнена в CORE-02; старый C# polling не получает этих гарантий.

При Pause/Stop игровой ввод отменяется. Ввод во время паузы не воспроизводится
при Resume; удержанная через паузу кнопка требует нового Press. Во время Paused
нет игровых OnUpdate/OnFixedUpdate и контактных callbacks, окно и GUI работают.

Интерполяция изображения, timeScale и ручной одиночный Step не входят в первый
шаг реализации. Сначала завершаем CORE-02, затем отдельно согласуем объём и
ведущего реализации scheduler/Engine/Application, после этого — подключение общего
RuntimeSession, миграцию C# callbacks и fixed input. Нового конкурирующего
roadmap и разрешения на ECS/Snapshot/CancellationToken рефакторинг здесь нет.

## На что PHYS-01 может опираться сейчас

PHYS-01 сохраняет физический адаптер, принимающий dt, и доставку контактов после
Step. Он не добавляет собственный scheduler, OnFixedUpdate или новый runLoop и
не меняет частоту существующего OnUpdate в рамках физической задачи.
Общий scheduler, независимый fixedDt, SimulationControl, SessionId и Pause/Resume
из этого документа **согласованы, но пока не реализованы**. Документ дополняет
границы [core-events.md](core-events.md), не расширяет реализацию CORE-02.
