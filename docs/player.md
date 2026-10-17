# Standalone player — PLAYER-01

Реализация 2026-10-16: ветка PLAYER-01 от dev ab0f3a5 (после #125/#128/#129),
отдельный worktree player-01. Ведущий — ассистент по поручению пользователя.

## Что запускается

CMake target robot2D-player создаёт Game.exe на Windows, Game на Linux в
`<build>/player`. Это отдельный native entry point. Общий static target
robot2D-runtime содержит существующие Scene, сериализаторы, Box2D adapter,
Mono/C# bridge, текст, анимацию и RenderSystem. Редактор линкуется с тем же target.
Game не линкуется с robot2D-editor-objects/robot2D-imgui, не создаёт Editor,
панели, project browser, Gui, файловые диалоги и editor configuration/cache.
Legacy namespace editor и интерфейс IEditorCamera пока сохранены для совместимости;
PlayerCamera адаптирует преобразование координат окна к существующему scripting API.

CPU-загрузка и GL-привязка ресурсов вынесены из SceneLoadTask/EditorLogic в
общие SceneAssets. ClassID перенесён из ICommand в общий runtime без изменения ID API.
В TextSystem и UISystem добавлен cloneSelf: без него Scene::onRuntimeStart
не переносил эти системы в игровой мир. Исходные исходники Editor/UI не дублируются.

## Launch manifest, версия 1

По умолчанию player ищет game-launch.yaml рядом со своим executable.
`--manifest <path>` явно выбирает другой manifest; package root — его каталог.
Working directory не участвует в разрешении ресурсов. Пример:

```yaml
Version: 1
Title: My game
StartScene: assets/scenes/main.scene
ScriptAssembly: assets/scripts/bin/MyGame.dll
ScriptCore: runtime/robot2D_ScriptCore.dll
MonoLib: runtime/mono/lib
MonoEtc: runtime/mono/etc
Window:
  Width: 1280
  Height: 720
  Fullscreen: false
```

Все пять путей относительны package root, с нормализацией и проверкой canonical
границы, включая symlinks. Абсолютные пути, выход за root, NUL, неизвестная версия,
отсутствующие файлы/каталоги и Mono mscorlib дают ошибку до открытия окна.
Размер окна — 1..16384 по каждой оси. Сцена и project assets продолжают использовать
свои прежние версии/форматы; Version=1 здесь относится только к launch manifest.
`--check` проверяет структуру и наличие входов без окна/Mono JIT; это не проверка
содержимого всех DLL/ресурсов. `--hidden` запускает настоящее скрытое GLFW окно
для автоматических graphics сценариев. `--help` показывает CLI.

## Сборка и пробный запуск

В отдельном проверенном Ninja/Release build с RB2D_BUILD_EDITOR_TESTS=ON:

```text
cmake --build <build> --target robot2D-player -j 4
cmake --build <build> --target robot2D-player-arena -j 4
```

Второй target доступен вместе с тестами и создаёт `<build>/player-arena`:
четыре стены, динамический объект, trigger, клип Idle, HUD и C# PlayerSmoke.
Эта диагностическая арена проверяет инфраструктуру; gameplay/баланс проекта игры
остаются в C#. Без test environment арена остаётся открытой до закрытия окна.
Её можно запустить полным путём из другого cwd:

```powershell
& 'C:/path/to/build/player-arena/Game.exe'
```

Для своего проекта создать новую папку игры, скопировать шаблон из `<build>/player`,
свои assets и launch manifest. Пользовательская DLL должна быть предварительно
собрана против этой ScriptCore через общий SCRIPT-01 builder. Player не запускает
Python/mcs и не компилирует исходники. Автоматическая сборка/упаковка из редактора
остаётся BUILD-01; ExportTask пока продолжает явно отказывать.
Нельзя выбирать существующую авторскую папку под diagnostic-arena target: это
генератор тестовых данных в собственном build tree.

## Runtime и завершение

Порядок: manifest -> Mono из package -> свежая игровая DLL -> deserialize ->
проверка классов и единственной primary Camera с Drawable/положительным размером ->
CPU assets -> GL assets -> Scene::onRuntimeStart -> штатный Engine loop.
Первый render происходит после первого настоящего update, когда созданы GPU
буферы текста и состояние камеры; фиктивный zero-dt script tick не вводится.

Scene::updateRuntime вызывает scripting, Box2D, animation/text/UI systems.
Input использует существующее окно, клавиатуру/мышь и viewport той же runtime камеры.
C# Engine.ExitEngine сигнализирует завершение; сцена останавливается, managed instances
освобождаются, затем сцена/GL ресурсы, Mono и окно. Ошибки load/update/render имеют
границы операции, путь и причину, дают ненулевой exit code. Исключения setup ловятся
внутри Application: cleanup остаётся под живым GL context. Частично начатая физика
допускает Stop. Модель игровых C# исключений остаётся существующей моделью Mono wrapper;
новая система обработки script exceptions этим шагом не вводится.

Источник cadence — текущий Engine. Новый fixed scheduler/OnFixedUpdate из контракта
CORE-02 не реализуется здесь; player станет его потребителем после завершения Core.
ECS, Snapshot и CancellationToken не изменены.

Переходы сцен пока возвращают false/диагностику; async переход использует тот же отказ.
Полный Instantiate/Destroy/restart принадлежит RUNTIME-01/02. Player сообщает, что
Instantiate недоступен; bridge не запускает managed OnCreate для недействительного clone.
Это не заменяет требования runtime lifecycle заглушками успешных операций.

## Runtime template

После сборки рядом с Game находятся native зависимости, runtime/ScriptCore и
runtime/mono/{lib,etc}. Копируются Mono 4.5 profile/GAC/config и native helpers;
пока консервативно, без trimming. CMake получает транзитивные native зависимости,
не копирует Windows OS DLL; VC runtime включается. Дубли одинаковых DLL допустимы
только при совпадении SHA256, конфликт разных бинарных файлов блокирует bundle.
На Linux сохраняется зависимость от совместимых системных ABI/GL/display driver.

Mono directories передаются player явно, вместо compile-time SDK путей редактора.
Тест переносит executable вместе с runtime, удаляет Mono/compiler из Windows PATH,
очищает MONO_PATH/MONO_CFG_DIR/MONO_GAC_PREFIX/MONO_ENV_OPTIONS. DLL загружается
из новой папки. Чистая вторая машина, полный dependency/license inventory,
минимизация размера и окончательный exporter относятся к BUILD-01/QA-01.
Наличие локального bundle не объявляется подтверждённой поставкой на второй машине.

## Проверки

Постоянные `player-inputs` (headless) и `player-runtime` (graphics) запускают сам Game
из чужого cwd после перемещения папки; Windows — путь с кириллицей, пробелами и &.
Проверяются версия/размер/абсолютные и выходящие пути, отсутствующие core/Mono/BCL,
symlink escape, успешная C# сцена, повреждённая DLL/сцена и отсутствующий шрифт.
PlayerSmoke подтверждает serialized Int, движение, trigger, Animator, HUD, input
и Engine.ExitEngine через report, который не может остаться от прошлого запуска.

Unicode сценарий выявил narrow fopen в Image/Font/TinyXML. Собственные обёртки теперь
открывают fs::u8path потоком: stb декодирует память, FreeType держит backing bytes
для memory face, AnimationParser читает/пишет XML потоком. Сторонний код не менялся.

Локально собраны редактор, Game, core-tests и editor-test-suite; полный CTest
110/110 (43 graphics) прошёл после финальной проверки bundle/первого render.
Финальные результаты hook и CI публикуются в Issue #81/PR.
Ручная приёмка: запустить свою сцену, проверить клавиатуру/мышь и курсор при resize,
HUD/анимацию/коллизии, закрытие крестиком; перенести папку на вторую Windows без SDK.
Эта пользовательская приёмка остаётся открытой.

### Регрессия белого окна — 2026-10-17

После afterRender player вызывает Window::display: завершение batch само по себе
не показывает back buffer в окне. Постоянный player-runtime дополнен native probe
через настоящий Engine: GL_FRONT должен содержать синюю стену и чёрный фон арены.
При временном удалении display проверка воспроизводимо падает. Это дополняет
C# проверки состояния проверкой реально представленного кадра.

### Замеры старта — 2026-10-17

PlayerStartup пишет длительность этапа и total_ms (steady_clock) от создания
PlayerApplication перед созданием окна до первого display. Чтение manifest до
создания приложения и запуск процесса OS в total_ms не входят. Метки пишутся
только при старте: window_ready, mono_core, game_assembly, scene_deserialize,
scene_validate, assets_cpu, assets_gpu, runtime_start, first_update_begin,
first_update_complete, first_render_begin, first_frame_presented.

Причина долгого ожидания первого update — потеря дробных микросекунд в частых
Clock::restart. Остаток теперь сохраняется; это исправление точности существующего
таймера, без второго scheduler или изменения контракта fixed tick. Регрессия
RapidRestartsPreserveElapsedMicroseconds проверяет сохранение прошедшего времени,
а не лимит скорости CPU. До исправления: wall=4671 мкс, сумма=83 мкс, тест падает.

Локально три старых запуска ожидали первый update 592–771 мс; пять исправленных
ожидали 16,84–16,89 мс. После исправления окно/OpenGL занимали 175–243 мс,
Mono+game DLL 9–10 мс, deserialize/validate 1,7–1,9 мс, CPU/GPU assets 3–4 мс,
runtime start 41–42 мс, первый update 2–3 мс и render/display около 1 мс.
Первый показ — 251–320 мс. Замеры через скрытое окно на текущей Windows/NVIDIA;
они не обещают время холодного запуска на другой машине. Пользовательский лог
до исправления содержал паузу 9,58 с до первого TextSystem/GLSL сообщения;
точные новые этапы позволяют разделять такие случаи без догадок о шейдере.

После синхронизации PLAYER-01 с dev f4121a7 сохранён steady_clock CORE-02 и
все его тесты; player использует typed Event для resize/close. Presentation probe
проверяет эти события. Полный CTest объединённого кода: 219/219. Последняя проверка
конечного exe до синхронизации: SHA 521887d, первый кадр 596,95 мс, ожидание tick
16,93 мс; runtime report PASS 3 12. Финальные hook/CI и SHA сборки — в Issue #81.

### Mono config и CI — 2026-10-17

MonoEtc должен содержать mono/config, --check проверяет его наличие до окна.
Host явно применяет dllmap из этой конфигурации. Файл читается через fs::u8path
и передаётся в mono_config_parse_memory: Mono filename parser не применял config
из Unicode Windows пути. SDK конфигурация и пользовательские проекты не меняются.
Временное состояние ScriptEngineData освобождается при ошибке чтения до JIT init.

Это часть обязанности embedding host, а не пользовательского игрового скрипта:
[Mono embedding](https://www.mono-project.com/docs/advanced/embedding/) и
[DLL mappings](https://www.mono-project.com/docs/advanced/pinvoke/dllmap/).
Тестовый пакет дополняет свою копию config alias Robot2DPlayerNativeProbe на
msvcrt.dll/libc.so.6; C# вызывает abs(-37) и проверяет 37. Так тест доказывает
применение именно упакованного config на обеих OS. File.WriteAllText report
дополнительно проверяет native поддержку файлового I/O на Linux.

Без config parsing регрессия воспроизводимо падает. После memory parser локально
пройдены 227/227 тестов (56 graphics), включая перенос в Unicode путь, missing
config, новый DesktopRuntime/instance polling dev 4132acf и показ GL_FRONT.
Итог обязательного hook и серверного CI публикуется в Issue #81/PR #132.

Presentation probe на Windows читает GL_FRONT после display. В Linux CI
Mesa/Xvfb возвращал нули из GL_FRONT и у скрытого, и у отображённого GLX окна.
Linux test executable использует GNU ld --wrap=glfwSwapBuffers: читает пиксели
GL_BACK непосредственно перед настоящим swap и проверяет ровно один вызов.
Проверяются framebuffer 0, отсутствие GL ошибок, синяя стена и чёрная внутренняя
область. Если display пропущен, wrapper не вызывается и тест падает. Game и
общая библиотека не получают тестовых перехватчиков. Все process окна снова скрыты.
Механизм wrapper: [GNU ld](https://sourceware.org/binutils/docs/ld/Options.html).