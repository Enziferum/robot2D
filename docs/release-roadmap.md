# Roadmap robot2D: 0.8.0 → 0.9.0 → 1.0.0

Этот релизный маршрут охватывает весь проект. Общая карта Core/ECS/Render/Editor,
межмодульные зависимости и правила синхронизации чатов —
[project-roadmap.md](project-roadmap.md).

Интеграция задач идёт через PR в `dev`; october2026_dev уже слита через PR #43.
`release/0.8.0` создаётся от dev ближе к стабилизации, выпуск — тег `v0.8.0`.
Правила ветвления — [project-workflow.md](project-workflow.md).

Согласованное направление, разложенное на Issues 2026-10-15. Даты релизов не назначены. Статусы и фактические результаты проверяются в GitHub Issues/Projects; наличие задачи не означает начало её реализации.

## Цели релизов

| Релиз | Обещание | Трекер |
|---|---|---|
| [0.8.0](https://github.com/Enziferum/robot2D/milestone/1) | Создать, собрать и передать простую 2D-игру | [PLAN-01](https://github.com/Enziferum/robot2D/issues/65) |
| [0.9.0](https://github.com/Enziferum/robot2D/milestone/2) | Развить архитектуру, переходы сцен, игровой UI и звук | [PLAN-02](https://github.com/Enziferum/robot2D/issues/66) |
| [1.0.0](https://github.com/Enziferum/robot2D/milestone/3) | Стабильный публичный инструмент с совместимостью, документацией и поставкой | [PLAN-03](https://github.com/Enziferum/robot2D/issues/67) |

Исторический milestone Version 0.8 переиспользован как 0.8.0; просроченная дата 2024 года снята. Текущая версия остаётся 0.8.0-dev. План не меняет версии приложения и форматов файлов. Правила dev/rc/stable — [versioning.md](versioning.md).

## Как делим работу

- Core и Editor используют одни Issues/Projects и milestones. Метка area:core
  выделяет Core-задачи, area:render — рендер. Milestone не запрещает начинать
  проектирование раньше целевого релиза; изменение общего API согласуется с потребителями.
- Цель — и развитие движка, и развитие навыков пользователя. Сложные задачи доступны обоим участникам.
- `owner:shared`: ведущий не назначен. Перед началом пользователь выбирает, реализует ли сам с поддержкой ассистента или поручает реализацию ассистенту.
- Второй участник помогает постановкой, разбором вариантов, ревью и проверками. Ассистент не забирает реализацию, если выбран режим подсказок/ревью.
- `owner:user`: Snapshot и CancellationToken закреплены за пользователем по прежнему соглашению. Помощь ассистента не означает передачу реализации.
- Поле Assignees содержит реальных пользователей GitHub. Для совместных задач не назначается фиктивный аккаунт Codex; выбранного ведущего фиксируем в Issue.
- В начале задачи записываем ведущего, формат помощи, первый шаг и контракт. В конце — результат, проверки, ограничения и ссылку на PR/commit.
- Реальные проверки UI, геймплея и поставки проводим с пользователем. Успешный CI не заменяет их.

Оформление задач — [project-workflow.md](project-workflow.md). На [доске](https://github.com/users/Enziferum/projects/5) релиз определяется Milestone, ответственность — меткой. Новые задачи помещаются в Backlog; существующие статусы сохраняются.

## Первая игра и границы 0.8

Одна арена из четырёх стен, игрок, движение и прицеливание, стрельба, здоровье и урон, волны с 2–3 типами врагов, спрайтовые анимации, минимальный HUD, проигрыш и новая партия. Сначала достаточно одного типа врага; затем добавляются ещё 1–2. Геймплей и баланс живут в C# проекта игры, а не в коде EditorLogic.

Build создаёт переносимую папку с Game.exe, ресурсами и зависимостями. Один exe без дополнительных файлов не требуется. Конечному игроку не нужны редактор, SDK или dev PATH.

Минимальные анимации входят уже в 0.8: спрайтшит → клип → preview → сохранение → C# → standalone. Полноценный UI-конструктор, универсальные переходы сцен, звук и большой архитектурный рефакторинг относятся к 0.9. Базовый Restart и необходимое отделение runtime от редактора нужны раньше.

## Порядок для быстрого результата

Уточнение по Core от 2026-10-15: до интеграции команд и runtime согласовать
[ECS-01](https://github.com/Enziferum/robot2D/issues/100) вместе с SNAP-01.
Необходимые изменения ComponentContainer/API, удаления и клонирования входят
в 0.8.0: ECS-04/05 → ECS-02/03. Обсуждение команд и Snapshot может идти параллельно;
старый маршрут ниже не означает, что изменения Core откладываются до готового Undo/Redo.

Большой 2D-рендер: [RENDER-04](https://github.com/Enziferum/robot2D/issues/98)
проектируется сейчас параллельно, основной объём
[RENDER-05](https://github.com/Enziferum/robot2D/issues/99) относится к 0.9.0.
RENDER-03 остаётся в 0.8.0; новый 3D renderer ещё не имеет назначенного релиза.

1. Выбрать ведущего [CMD-01](https://github.com/Enziferum/robot2D/issues/46) и согласовать контракт [SNAP-01](https://github.com/Enziferum/robot2D/issues/68); проверить Duplicate → Undo → Redo, затем остальные операции.
2. [CMD-02](https://github.com/Enziferum/robot2D/issues/70), [CMD-03](https://github.com/Enziferum/robot2D/issues/71), [CMD-04](https://github.com/Enziferum/robot2D/issues/72): закончить Copy/Paste, Duplicate, Delete и согласованную историю.
3. Подготовить минимум ресурсов и C#: [ASSET-02](https://github.com/Enziferum/robot2D/issues/73), [SCRIPT-01](https://github.com/Enziferum/robot2D/issues/76), [PREFAB-01](https://github.com/Enziferum/robot2D/issues/75); согласовать параллельную работу по [CANCEL-01](https://github.com/Enziferum/robot2D/issues/69).
4. Довести runtime, физику, ввод, анимации и HUD: [RUNTIME-01](https://github.com/Enziferum/robot2D/issues/77), [RUNTIME-02](https://github.com/Enziferum/robot2D/issues/37), [PHYS-01](https://github.com/Enziferum/robot2D/issues/78), [INPUT-01](https://github.com/Enziferum/robot2D/issues/79), [ANIM-01](https://github.com/Enziferum/robot2D/issues/74), [HUD-01](https://github.com/Enziferum/robot2D/issues/80).
5. Рано запустить минимальную сцену отдельно через [PLAYER-01](https://github.com/Enziferum/robot2D/issues/81); не ждать завершения всей демоигры. Финальная проверка анимаций в standalone выполняется после появления player.
6. На этой основе собирать [GAME-01](https://github.com/Enziferum/robot2D/issues/84), [GAME-02](https://github.com/Enziferum/robot2D/issues/85), [GAME-03](https://github.com/Enziferum/robot2D/issues/86) и доводить [BUILD-01](https://github.com/Enziferum/robot2D/issues/83); устранить подтверждённые ошибки [RENDER-03](https://github.com/Enziferum/robot2D/issues/82).
7. Закончить [LOAD-01](https://github.com/Enziferum/robot2D/issues/45)/[ASYNC-01](https://github.com/Enziferum/robot2D/issues/56) и пройти [QA-01](https://github.com/Enziferum/robot2D/issues/87); dev → rc → stable только после фактической приёмки.

Порядок задаёт основной маршрут, а не запрещает независимую параллельную работу. Зависимости каждой задачи записаны в Issue. Устаревший результат загрузки и отмена работы — разные механизмы.

Из ASSET-02, ANIM-01, PREFAB-01, PHYS-01 и RENDER-03 только PHYS-01 имеет
прямую зависимость от RUNTIME-01: полная приёмка безопасного создания/удаления
тел из physics callbacks требует контракта Instantiate/Destroy и жизни runtime-сущностей.
Это не мешает заранее проверять статические стены, фильтры и события столкновений.
Остальные четыре задачи не ждут RUNTIME-01; PREFAB-01 сама является его предшественником.

## Реестр релизных задач

Согласованный независимый bugfix для 0.8.0:
[IMGUI-01 #121](https://github.com/Enziferum/robot2D/issues/121) — безопасные
оконные scopes robot2D-imgui и сохранение dockspace. Реализация поручена
ассистенту; расширения Graphics/Window и весь CODE-01 этим шагом не запускаются.
Результаты и оставшаяся UI-приёмка — [разбор интеграции](imgui-backend-review.md).


### 0.8.0

Core дополняет существующий план следующими задачами. Объём уточняется в ECS-01
и ограничивается необходимой основой команд/runtime; новый backend хранения не выбран.

| Core / ECS | Ответственность | Предшественники |
|---|---|---|
| [ECS-01 #100](https://github.com/Enziferum/robot2D/issues/100) — согласовать контракты и объём | Совместно; инициатор — пользователь | — |
| [ECS-04 #101](https://github.com/Enziferum/robot2D/issues/101) — ComponentContainer | Совместно; ведущий перед стартом | ECS-01 |
| [ECS-05 #102](https://github.com/Enziferum/robot2D/issues/102) — API и миграция | Совместно; ведущий перед стартом | ECS-01, ECS-04 |
| [ECS-02 #103](https://github.com/Enziferum/robot2D/issues/103) — удаление | Совместно; ведущий перед стартом | ECS-01, ECS-04, ECS-05 |
| [ECS-03 #104](https://github.com/Enziferum/robot2D/issues/104) — клонирование | Совместно; ведущий перед стартом | ECS-01, ECS-04, ECS-05 |

| Задача | Ответственность | Предшественники |
|---|---|---|
| [SNAP-01](https://github.com/Enziferum/robot2D/issues/68) — Snapshot: контракт состояния для Undo/Redo | Пользователь; помощь по согласованию | — |
| [CANCEL-01](https://github.com/Enziferum/robot2D/issues/69) — CancellationToken и interrupt points загрузки | Пользователь; помощь по согласованию | — |
| [CMD-02](https://github.com/Enziferum/robot2D/issues/70) — Copy/Paste и Duplicate с корректными ссылками | Совместно; ведущий перед стартом | [SNAP-01](https://github.com/Enziferum/robot2D/issues/68), [ECS-03](https://github.com/Enziferum/robot2D/issues/104), [ECS-05](https://github.com/Enziferum/robot2D/issues/102) |
| [CMD-03](https://github.com/Enziferum/robot2D/issues/71) — Delete и восстановление иерархии | Совместно; ведущий перед стартом | [SNAP-01](https://github.com/Enziferum/robot2D/issues/68), [ECS-02](https://github.com/Enziferum/robot2D/issues/103), [ECS-05](https://github.com/Enziferum/robot2D/issues/102) |
| [CMD-04](https://github.com/Enziferum/robot2D/issues/72) — История команд и dirty-state относительно сохранения | Совместно; ведущий перед стартом | [CMD-02](https://github.com/Enziferum/robot2D/issues/70), [CMD-03](https://github.com/Enziferum/robot2D/issues/71) |
| [ASSET-02](https://github.com/Enziferum/robot2D/issues/73) — Минимальный импорт и переносимые ссылки на ресурсы | Ассистент; реализация поручена 2026-10-15 | — |
| [ANIM-01](https://github.com/Enziferum/robot2D/issues/74) — Спрайтшит → клип → preview → C# → standalone | Ассистент; реализация поручена 2026-10-15 | [ASSET-02](https://github.com/Enziferum/robot2D/issues/73) |
| [PREFAB-01](https://github.com/Enziferum/robot2D/issues/75) — Шаблоны врагов и пуль для runtime-спавна | Ассистент; реализация поручена 2026-10-15 | [ASSET-02](https://github.com/Enziferum/robot2D/issues/73) |
| [SCRIPT-01](https://github.com/Enziferum/robot2D/issues/76) — Воспроизводимая сборка C# и диагностика ошибок | Совместно; ведущий перед стартом | — |
| [RUNTIME-01](https://github.com/Enziferum/robot2D/issues/77) — C# Instantiate/Destroy и жизнь runtime-сущностей | Совместно; ведущий перед стартом | [SCRIPT-01](https://github.com/Enziferum/robot2D/issues/76), [PREFAB-01](https://github.com/Enziferum/robot2D/issues/75), [ECS-02](https://github.com/Enziferum/robot2D/issues/103), [ECS-03](https://github.com/Enziferum/robot2D/issues/104), [ECS-05](https://github.com/Enziferum/robot2D/issues/102) |
| [RUNTIME-02](https://github.com/Enziferum/robot2D/issues/37) — Play/Stop: изоляция и повторяемый запуск | Совместно; ведущий перед стартом | [SCRIPT-01](https://github.com/Enziferum/robot2D/issues/76), [ECS-03](https://github.com/Enziferum/robot2D/issues/104) |
| [PHYS-01](https://github.com/Enziferum/robot2D/issues/78) — Физика и события столкновений для арены | Ассистент; реализация поручена 2026-10-15 | [RUNTIME-01](https://github.com/Enziferum/robot2D/issues/77) |
| [INPUT-01](https://github.com/Enziferum/robot2D/issues/79) — Ввод, камера и прицеливание в Game View и standalone | Совместно; ведущий перед стартом | — |
| [HUD-01](https://github.com/Enziferum/robot2D/issues/80) — Минимальный HUD и отображение состояния из C# | Совместно; ведущий перед стартом | [SCRIPT-01](https://github.com/Enziferum/robot2D/issues/76), [ASSET-02](https://github.com/Enziferum/robot2D/issues/73) |
| [PLAYER-01](https://github.com/Enziferum/robot2D/issues/81) — Standalone player для существующей сцены | Совместно; ведущий перед стартом | [RUNTIME-01](https://github.com/Enziferum/robot2D/issues/77), [RUNTIME-02](https://github.com/Enziferum/robot2D/issues/37), [INPUT-01](https://github.com/Enziferum/robot2D/issues/79), [ASSET-02](https://github.com/Enziferum/robot2D/issues/73) |
| [RENDER-03](https://github.com/Enziferum/robot2D/issues/82) — Порядок sprite/text, камера и resize для игры | Ассистент; реализация поручена 2026-10-15 | [RENDER-02](https://github.com/Enziferum/robot2D/issues/63) |
| [BUILD-01](https://github.com/Enziferum/robot2D/issues/83) — Build: C# → проверка проекта → папка Game.exe | Совместно; ведущий перед стартом | [PLAYER-01](https://github.com/Enziferum/robot2D/issues/81), [SCRIPT-01](https://github.com/Enziferum/robot2D/issues/76), [ASSET-02](https://github.com/Enziferum/robot2D/issues/73), [PREFAB-01](https://github.com/Enziferum/robot2D/issues/75) |
| [GAME-01](https://github.com/Enziferum/robot2D/issues/84) — Арена: управление игроком, стрельба и попадания | Совместно; ведущий перед стартом | [RUNTIME-01](https://github.com/Enziferum/robot2D/issues/77), [PHYS-01](https://github.com/Enziferum/robot2D/issues/78), [INPUT-01](https://github.com/Enziferum/robot2D/issues/79), [ANIM-01](https://github.com/Enziferum/robot2D/issues/74) |
| [GAME-02](https://github.com/Enziferum/robot2D/issues/85) — Арена: 2–3 типа врагов и волны | Совместно; ведущий перед стартом | [GAME-01](https://github.com/Enziferum/robot2D/issues/84), [PREFAB-01](https://github.com/Enziferum/robot2D/issues/75) |
| [GAME-03](https://github.com/Enziferum/robot2D/issues/86) — Арена: уровень, HUD, проигрыш и повторная партия | Совместно; ведущий перед стартом | [CMD-01](https://github.com/Enziferum/robot2D/issues/46), [GAME-02](https://github.com/Enziferum/robot2D/issues/85), [HUD-01](https://github.com/Enziferum/robot2D/issues/80), [BUILD-01](https://github.com/Enziferum/robot2D/issues/83) |
| [QA-01](https://github.com/Enziferum/robot2D/issues/87) — 0.8 RC: полная арена и запуск на чистой Windows | Совместно; ведущий перед стартом | [GAME-03](https://github.com/Enziferum/robot2D/issues/86), [LOAD-01](https://github.com/Enziferum/robot2D/issues/45), [ASYNC-01](https://github.com/Enziferum/robot2D/issues/56), [RENDER-03](https://github.com/Enziferum/robot2D/issues/82) |
| [CMD-01](https://github.com/Enziferum/robot2D/issues/46) — CommandStack, клонирование, удаление и clipboard | Совместно; ведущий перед стартом | [SNAP-01](https://github.com/Enziferum/robot2D/issues/68), [CMD-02](https://github.com/Enziferum/robot2D/issues/70), [CMD-03](https://github.com/Enziferum/robot2D/issues/71), [CMD-04](https://github.com/Enziferum/robot2D/issues/72) |
| [LOAD-01](https://github.com/Enziferum/robot2D/issues/45) — Надёжная загрузка и завершение работы | Совместно; ведущий перед стартом | [CANCEL-01](https://github.com/Enziferum/robot2D/issues/69) |
| [ASYNC-01](https://github.com/Enziferum/robot2D/issues/56) — Исходы задач, остановка и завершение очереди | Совместно; ведущий перед стартом | [CANCEL-01](https://github.com/Enziferum/robot2D/issues/69) |
| [VERSION-01](https://github.com/Enziferum/robot2D/issues/51) — Единый источник версии и правила релизов | Совместно; ведущий перед стартом | — |
| [SAVE-01](https://github.com/Enziferum/robot2D/issues/52) — Проверить сохранение и Save / Discard / Cancel | Совместно; ведущий перед стартом | — |
| [LOG-01](https://github.com/Enziferum/robot2D/issues/53) — Диагностика запуска, загрузки и завершения | Совместно; ведущий перед стартом | — |
| [RENDER-01](https://github.com/Enziferum/robot2D/issues/61) — Диагностика OpenGL и проверка GLSL | Совместно; ведущий перед стартом | — |
| [RENDER-02](https://github.com/Enziferum/robot2D/issues/63) — Корректность ёмкости, переполнения и завершения batch | Совместно; ведущий перед стартом | [RENDER-01](https://github.com/Enziferum/robot2D/issues/61) |



### 0.9.0

| Core / Render | Ответственность | Предшественники |
|---|---|---|
| [RENDER-04 #98](https://github.com/Enziferum/robot2D/issues/98) — новый 2D-контракт; проектирование начинается сейчас | Совместно; ведущий перед стартом | RENDER-02 |
| [RENDER-05 #99](https://github.com/Enziferum/robot2D/issues/99) — реализация и адаптация Editor/player | Совместно; ведущий перед стартом | RENDER-04 |

| Задача | Ответственность | Предшественники |
|---|---|---|
| [ARCH-02](https://github.com/Enziferum/robot2D/issues/38) — Реализация согласованных границ editor/runtime и операций сцены | Совместно; ведущий перед стартом | [PLAN-01](https://github.com/Enziferum/robot2D/issues/65), [ARCH-01](https://github.com/Enziferum/robot2D/issues/55) |
| [SCENE-01](https://github.com/Enziferum/robot2D/issues/88) — Переходы меню → игра → результаты и жизнь объектов | Совместно; ведущий перед стартом | [ARCH-02](https://github.com/Enziferum/robot2D/issues/38), [LOAD-01](https://github.com/Enziferum/robot2D/issues/45), [ASYNC-01](https://github.com/Enziferum/robot2D/issues/56) |
| [AUDIO-01](https://github.com/Enziferum/robot2D/issues/89) — Звуковые эффекты, музыка и громкость | Совместно; ведущий перед стартом | [PLAN-01](https://github.com/Enziferum/robot2D/issues/65) |
| [GUI-01](https://github.com/Enziferum/robot2D/issues/90) — Игровые UI-компоненты, layout и ввод | Совместно; ведущий перед стартом | [HUD-01](https://github.com/Enziferum/robot2D/issues/80), [INPUT-01](https://github.com/Enziferum/robot2D/issues/79) |
| [ANIM-02](https://github.com/Enziferum/robot2D/issues/91) — Animation Panel: надёжное редактирование клипов | Совместно; ведущий перед стартом | [ANIM-01](https://github.com/Enziferum/robot2D/issues/74), [CMD-04](https://github.com/Enziferum/robot2D/issues/72), [ASSET-01](https://github.com/Enziferum/robot2D/issues/47) |
| [ASSET-01](https://github.com/Enziferum/robot2D/issues/47) — Ресурсы, кэши, Assets и preview | Совместно; ведущий перед стартом | [ASSET-02](https://github.com/Enziferum/robot2D/issues/73) |
| [UI-01](https://github.com/Enziferum/robot2D/issues/48) — Поиск, multiselection и ошибки Inspector | Совместно; ведущий перед стартом | [CMD-01](https://github.com/Enziferum/robot2D/issues/46) |
| [VIEW-01](https://github.com/Enziferum/robot2D/issues/49) — Frame Selected, snapping и групповая работа Guizmo | Совместно; ведущий перед стартом | [INPUT-01](https://github.com/Enziferum/robot2D/issues/79), [CMD-01](https://github.com/Enziferum/robot2D/issues/46) |
| [CODE-01](https://github.com/Enziferum/robot2D/issues/50) — Структура кода, общие UI-контролы и стиль | Совместно; ведущий перед стартом | [ARCH-02](https://github.com/Enziferum/robot2D/issues/38) |
| [ARCH-01](https://github.com/Enziferum/robot2D/issues/55) — Ответственности main, Interactors и EditorLogic | Совместно; ведущий перед стартом | [PLAN-01](https://github.com/Enziferum/robot2D/issues/65) |



### 1.0.0

| Задача | Ответственность | Предшественники |
|---|---|---|
| [API-01](https://github.com/Enziferum/robot2D/issues/92) — Контракт 1.0: C# API, форматы и миграции проектов | Совместно; ведущий перед стартом | [PLAN-02](https://github.com/Enziferum/robot2D/issues/66), [VERSION-01](https://github.com/Enziferum/robot2D/issues/51) |
| [RECOVERY-01](https://github.com/Enziferum/robot2D/issues/93) — Сохранность проекта и восстановление после сбоя | Совместно; ведущий перед стартом | [SAVE-01](https://github.com/Enziferum/robot2D/issues/52), [CMD-04](https://github.com/Enziferum/robot2D/issues/72), [API-01](https://github.com/Enziferum/robot2D/issues/92) |
| [DOC-01](https://github.com/Enziferum/robot2D/issues/94) — Документация: от пустого проекта до игры и C# API | Совместно; ведущий перед стартом | [PLAN-02](https://github.com/Enziferum/robot2D/issues/66), [API-01](https://github.com/Enziferum/robot2D/issues/92) |
| [DIST-01](https://github.com/Enziferum/robot2D/issues/95) — Поставка редактора: первый установщик и GitHub Release | Совместно; ведущий перед стартом | [PLAN-02](https://github.com/Enziferum/robot2D/issues/66), [API-01](https://github.com/Enziferum/robot2D/issues/92) |
| [UPDATE-01](https://github.com/Enziferum/robot2D/issues/96) — Проверка новой версии без автоматической замены файлов | Совместно; ведущий перед стартом | [DIST-01](https://github.com/Enziferum/robot2D/issues/95), [VERSION-01](https://github.com/Enziferum/robot2D/issues/51) |
| [QA-02](https://github.com/Enziferum/robot2D/issues/97) — 1.0 RC: матрица качества и решение о выпуске | Совместно; ведущий перед стартом | [PLAN-02](https://github.com/Enziferum/robot2D/issues/66), [API-01](https://github.com/Enziferum/robot2D/issues/92), [RECOVERY-01](https://github.com/Enziferum/robot2D/issues/93), [DOC-01](https://github.com/Enziferum/robot2D/issues/94), [DIST-01](https://github.com/Enziferum/robot2D/issues/95), [UPDATE-01](https://github.com/Enziferum/robot2D/issues/96) |



## Что не теряем при развитии

- Анимации проверяются в панели, при повторном открытии, в Play и собранной игре.
- Изменения сущностей учитывают иерархию, UUID/ссылки, историю, выделение, индекс и dirty-state.
- Импорт/сохранение ресурсов проверяются с переносимыми путями и упаковкой.
- HUD, ввод, камера, физика и спавн проходят один реальный игровой сценарий.
- 1.0 требует сохранности данных, документированной совместимости, проверенной установки и работающего примера; число функций само по себе не критерий.
- Проверка обновлений 1.0 сообщает версию/changelog/ссылку. Автоустановка и rollback не входят в первый объём.

## История и существующие задачи

План переиспользует существующие CMD/LOAD/ASYNC/ASSET/UI/VIEW/CODE/ARCH и Review-задачи. Issue #37 Scene Cloning продолжена как RUNTIME-02, #38 Refactoring — как ARCH-02. Исторические результаты не объявляются проверенными заново; открытые Review-задачи не закрываются этим планированием. Старые TOOL/TEST/CI/PROCESS остаются завершённой инфраструктурой; Config .ini #41 не добавляется в критический путь без отдельного решения.

Датированная история реализации — [editor-roadmap.md](editor-roadmap.md). Этот релизный маршрут заменяет прежнюю последовательность после TOOL-01 в части следующих приоритетов; архитектурные схемы из обсуждения не становятся утверждённым проектом классов.
