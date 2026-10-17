# Общее ревью 0.8.0-dev

Ветка 0.8.0-dev объединяет опубликованные изменения десяти PR для одного
ревью перед dev. Это интеграционный срез, а не выпуск 0.8.0 и не закрытие
всех перечисленных задач. Следующие исправления рассматриваются относительно
этой общей сборки. Основной checkout пользователя не переключается.

## Включённые исходные версии

| PR | Направление | Опубликованный SHA |
|---|---|---|
| [#123](https://github.com/Enziferum/robot2D/pull/123) | IMGUI-01 | `5feb554dd6ed968ee16f84fdddb2426411ade00d` |
| [#127](https://github.com/Enziferum/robot2D/pull/127) | ASSET-01 | `91f198184b94b41ab9399f25dbe8b779dccbd43a` |
| [#131](https://github.com/Enziferum/robot2D/pull/131) | RUNTIME-02 | `8cffb9ec614374ac1171dc96ab79855749263908` |
| [#132](https://github.com/Enziferum/robot2D/pull/132) | PLAYER-01 | `a97c9af73595535123e6a5cb61d484805e778447` |
| [#136](https://github.com/Enziferum/robot2D/pull/136) | BRAND-01 | `e6951ee49624d915fa1bdcbef67bcba0643404db` |
| [#139](https://github.com/Enziferum/robot2D/pull/139) | PLAN-01 | `2eb062662b65ba4cd6ec55bc954b08e6cd4da519` |
| [#140](https://github.com/Enziferum/robot2D/pull/140) | GAME-01 | `3491b2c21c3c3af9dd686d98bed11c8bc9122293` |
| [#141](https://github.com/Enziferum/robot2D/pull/141) | CORE-01 | `79369b2be5c76b875e69886ff6395a4681994f95` |
| [#134](https://github.com/Enziferum/robot2D/pull/134) | WEB-02 | `2a8d05bf89128e5717712f79ecc347ed2cd2e532` |
| [#109](https://github.com/Enziferum/robot2D/pull/109) | WEB-01 | `c8e009e2c40efb2627d262fc06b650c5b99f520b` |

RUNTIME-02 уже входит в ASSET-01; часть branding также перенесена туда.
История исходных коммитов сохраняется. Разрешение конфликтов должно сохранять
обе группы тестов и актуальные Core typed Event/Window/GUI контракты.
Незакоммиченные изменения других worktree и внешних проектов не включаются.

## Набор для ревью

Собственные build и готовые файлы находятся в work/ интеграционного worktree.
Точные пути EXE, Git SHA и результаты проверок публикуются в общем PR и
в сопровождающем manifest готовой сборки. Не использовать EXE из старых веток.

- Editor: общий UI/Assets/Preview, SlateTeal, branding, docking и runtime.
- Robot Lab: отдельная редактируемая копия проекта и standalone на общей ScriptCore.
- Player arena: техническая диагностика физики/Animator/HUD/C#.
- Core examples: First Sprite и Keyboard Movement из сайта.
- Sandbox и диагностические test executables: собраны отдельно от пользовательских приложений.
- Website: статическая сборка и инструкция локального просмотра; серверный deploy не выполняется.

## Короткий маршрут ручной приёмки

Каждый пункт можно проверять отдельно; полный прогон всех подсистем за один
сеанс не требуется. Фактические результаты записываются в PR, а не угадываются
по успешной сборке или автоматическим тестам.

1. Открыть общий Editor, добавить подготовленную копию RobotLabPlatformer.
   Проверить Inspector, Console и открытие Platformer.scene.
2. Проверить дерево Assets, breadcrumbs, плитку/список, alpha-preview и toolbar:
   Move/Scale/Rotate, Grid/Snap/Frame/Zoom. Проверить преобразования после zoom.
3. Закрыть Inspector, вернуть из меню, перестыковать панели, скрыть/вернуть
   dockspace и перезапустить редактор с сохранённым layout.
4. Три раза Play → Game → Stop → Play. Проверить мышь, фокус и отсутствие
   изменения исходной сцены. Во время Play перейти между Scene и Game.
5. Пройти Robot Lab START → 02 → 03 → EXIT; удержать прыжок при посадке,
   попробовать повторный прыжок в воздухе, упасть с обеих сторон, нажать R.
6. Проверить resize/maximize/minimize/restore. Если доступны разные DPI,
   перенести окно между мониторами 100/150/200%; записать конкретные условия.
7. Изменить сцену, сохранить/переоткрыть; отдельно проверить Save/Discard/Cancel
   при закрытии. Для испытаний использовать подготовленную копию.
8. Запустить standalone Robot Lab и техническую арену из готового набора.
   Проверить первый кадр, управление, resize и закрытие. Арена намеренно двигает
   синий квадрат к стене; это технический сценарий, не игровой контроллер.
9. Запустить First Sprite и Keyboard Movement; проверить картинку, движение,
   диагональ, потерю фокуса и выход.
10. Проверить Website: navigation, light/dark code blocks, docs/search,
    Core lessons и ссылки Downloads без выдуманных готовых релизов.

## Как фиксировать замечание

Указать EXE/сценарий, Git SHA из лога, шаги, ожидаемый и фактический результат,
лог и скриншот пользователя, если он помогает. Отличать блокирующий сбой от
визуального замечания и новой функциональности. UI проверяет пользователь;
computer-use и автоматическое управление редактором не применяются.

## Открытые границы

Robot Lab пока содержит первый одноэкранный этап. Ячейки, лазеры, checkpoint,
победа/рестарт полного уровня, анимации, pause/focus и расширенный ввод остаются
в исходных задачах. Scene transitions/Instantiate, полный Build exporter,
реальный multi-DPI/multi-viewports, серверный Nginx/TLS deploy и запуск на
второй чистой машине не считаются принятыми этой интеграцией.

Общий маршрут: [project-roadmap.md](project-roadmap.md),
[release-roadmap.md](release-roadmap.md), [Robot Lab](robot-lab-design.md),
[player](player.md), [editor tests](editor-testing.md).