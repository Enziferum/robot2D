# BUILD-01: подготовка переносимой папки игры

2026-10-16, ветка BUILD-01 от dev df823bb, отдельный worktree build-01.
Ведущий — ассистент по поручению пользователя. Документ различает реализованный
защитный шаг и предлагаемый контракт следующей интеграции с player.

## Что сейчас есть

ExportOptions хранит выбранный outputFolder, startScene и fullscreen.
ExportTask был заглушкой с sleep(2s): TaskQueue присваивал ему Succeeded, хотя он
не создавал ни одного файла. Теперь пустые параметры дают явную причину, а при
заполненных параметрах задача возвращает Failed: PLAYER-01 и интегрированный
script/package pipeline ещё не готовы. Путь/сцена/причина видны в Console/логе.
Результат не называется готовой игрой; output не создаётся и не изменяется.
Параметры async-задачи копируются, поскольку UI очищает свой ExportOptions после enqueue.

Постоянный task-queue-completion включает регрессию реального ExportTask через
очередь: три причины ошибки, доставка completion, сохранение существующего output
и его содержимого. Тест не заменяет будущую проверку готовой Game.exe. Редактор/core-tests/suite
собраны Windows/Ninja/Release; полный CTest 103/103, включая 41 graphics.

В dev уже есть переносимые assets/... ссылки и copyProjectAssets (ASSET-02),
сцены/prefab с относительными зависимостями (PREFAB-01). SCRIPT-01
[PR #125](https://github.com/Enziferum/robot2D/pull/125) добавляет общий
buildProjectScripts(root,name,tools) -> ScriptBuildResult: success, assembly,
diagnostic. Эта ветка основана на dev и не копирует незамерженный сборщик.
Интеграция компиляции после его merge должна использовать этот же сервис.

## Предлагаемый конвейер следующего шага

Это план реализации, а не уже исполняемый exporter и не новый утверждённый формат
player. Consumer-side чтение launch manifest надо сверить с PLAYER-01 #81.

| Этап | Вход и наблюдаемый результат | Условие ошибки |
|---|---|---|
| Проверка запроса | Копия Project root/name и ExportOptions; одна выбранная стартовая сцена | Нет проекта, output, сцены, сохранённого входа; Play активен |
| Проверка путей | Canonical project/assets и output, project-relative стартовая сцена | Output внутри проекта/assets, совпадение входа/выхода, symlink escape, существующий final output |
| C# | Тот же buildProjectScripts, что у Play; success и новая assembly обязательны | Компилятор/API/исходники отсутствуют либо compile/load validation failed |
| Player bundle | Артефакт PLAYER-01 для конкретной OS/архитектуры с известным составом runtime | Нет player, ScriptCore, Mono runtime/config/framework или native DLL/so |
| Staging | Новый соседний временный каталог; copyProjectAssets сохраняет дерево assets | Недоступный/исчезнувший ресурс, ошибка копирования, внешний абсолютный путь |
| Launch manifest | Только относительные package paths, стартовая сцена, scripts assembly, fullscreen | Нет сцены/assembly в staging, неподдерживаемая версия manifest |
| Проверка результата | Запуск staging player без source root/editor/PATH; затем публикация каталога | Player не стартует, отсутствующая зависимость, load error |
| Публикация | Rename staging в выбранный новый final каталог, вывод итогового пути | Rename/права; частичная папка не объявляется успешной |

Первый шаг отказывает при существующем final output: явный overwrite/backup можно
добавить отдельным решением. Не удалять неизвестное содержимое пользовательской папки.
Staging удаляется только при доказанном владении task и проверенном canonical пути.
Устаревший результат/отмена используют существующую очередь; новый CancellationToken
остаётся работой пользователя. Не заменять её отдельным механизмом.

## Предлагаемый layout

```text
Game/
  Game.exe                     # либо platform player executable
  game-launch.yaml             # новый launch contract, версия согласуется с player
  assets/                      # сохранённые project-relative сцены/prefab/fonts/textures/clips
    scripts/bin/<project>.dll  # свежий результат общего сборщика
  runtime/
    robot2D_ScriptCore.dll
    mono/...                   # Mono runtime + config + профиль managed framework
  <platform native libraries>  # рядом с exe либо по согласованному loader path
```

В launch manifest не сохранять исходный абсолютный project root, пути редактора,
путь CMake Python/mcs, dev PATH и локальные cache keys. Player вычисляет package root
от своего executable, а не process cwd. Имена и позиции Mono/native файлов надо
подтвердить запуском bundle; наличие одной Mono DLL не доказывает переносимость.
Два проекта с одинаковыми basename ресурсов сохраняют разные assets подкаталоги.

Консервативный copyProjectAssets копирует все assets, включая prefab зависимости
и шрифты. На первом шаге не внедряем вычисление только достижимых ресурсов.
Сборщик C# выполняется до snapshot/copy assets, поэтому assets/scripts/bin содержит
результат именно этого запроса. .robot2d/scripts-build.json остаётся editor/build
метаданными и не является разрешением запускать произвольную старую DLL.

## Кто что предоставляет

- SCRIPT-01 #76: общий генератор/сборщик и диагностика; Build вызывает его,
  не повторяет mcs flags и не выбирает DLL только по существованию.
- PLAYER-01 #81: executable без Editor/UI, bootstrap сцены/ScriptCore/Mono,
  расположение runtime/native зависимостей и чтение согласованного manifest.
- RUNTIME-01 #77: владение игровой сценой, Play/Stop/restart, безопасный lifecycle.
- ASSET-02 #73 / PREFAB-01 #75: разрешение относительных ссылок, проверки ресурсов,
  переносимость сцен/prefab. BUILD-01 сообщает контекст отсутствующей зависимости.
- BUILD-01 #83: UI запроса, preflight, staging/copy, launch manifest, итог/ошибка,
  успешная публикация только после полной проверки.

## Матрица приёмки после интеграции

1. Новый C# класс автоматически входит в сборку, старый DLL не маскирует compile error.
2. Пустая/невалидная сцена, отсутствующий шрифт/prefab/texture/player/Mono дают ошибку
   с этапом и путём; final output отсутствует.
3. Проект/output с пробелами и Unicode; игра запускается из иного cwd.
4. Перенести Game на другую машину/чистое окружение без editor, SDK и dev PATH.
5. Запустить сцену, увидеть HUD/шрифты, анимацию, физику; проиграть и перезапустить.
6. Удалить/переименовать исходный проект и установленный редактор, повторить запуск.
7. Сбой записи/копирования не меняет старую поставку и исходные файлы.
8. В конфигурации/сценах/prefab не остаётся абсолютных ссылок source/editor root.
9. Зафиксировать OS/архитектуру, compiler/runtime версии и фактический состав bundle.

Эти проверки ещё не выполнялись: player отсутствует. BUILD-01 остаётся открытой;
данный PR — подготовка и устранение ложного успеха, не выпуск готовой игры.


## PLAYER-01: первый consumer

[Standalone player](player.md) реализует launch manifest v1 с именами полей
Version/Title/StartScene/ScriptAssembly/ScriptCore/MonoLib/MonoEtc/Window.
Относительные пути ограничены package root. `<build>/player` содержит Game,
ScriptCore, Mono/runtime helpers и native зависимости; `<build>/player-arena`
доступен для диагностического запуска при включённых тестах.
Предыдущие разделы сохраняют историю подготовки BUILD-01. Следующая интеграция
экспортера должна формировать этот manifest, использовать общий C# builder и
проверять артефакт player; заглушка ExportTask данным PLAYER шагом не заменяется.
Чистая машина и готовая поставка ещё требуют приёмки.
