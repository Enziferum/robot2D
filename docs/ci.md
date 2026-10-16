# CI и отчёты

Workflow [Engine Build](../.github/workflows/ci.yml) запускается только для pull request: при открытии, повторном
открытии и обновлении коммитов PR. Отдельные push, merge в dev и ручной запуск
workflow не создают дополнительных прогонов. Повторный запуск существующей
проверки доступен через Re-run jobs в Actions. Новый коммит в PR отменяет
предыдущий незавершённый прогон этого PR через concurrency.

Две независимые Release-сборки используют Ninja:

| Runner | Компилятор и зависимости | Проверки |
|---|---|---|
| Ubuntu 24.04 | GCC, пакеты apt, Mono | Зарегистрированные core-тесты, headless редактора и 9 graphics-сценариев через Xvfb/Mesa |
| Windows 2022 | MinGW64/MSYS2, Mono через Chocolatey | Зарегистрированные core-тесты и 18 headless-сценариев редактора |

Обе сборки собирают редактор и все три executable тестов редактора.
Graphics на Windows автоматически не запускаются: hosted runner не гарантирует
OpenGL 4.5. На Linux отсутствие контекста означает ошибку CI, даже если CTest
пометил graphics-тесты как skipped. MSVC этим workflow не проверяется.

Submodules загружаются рекурсивно на зафиксированных в Git ревизиях.
Зависимости apt/MSYS2 берутся из репозиториев пакетов; их версии не заморожены.
Старый install_deps.py в CI не используется.

## Где смотреть результат

Скачивание не требуется: основной отчёт уже отображается в **Summary** запуска.
В PR → **Checks** дополнительно публикуются **Tests (ubuntu-24.04)** и
**Tests (windows-2022)** через [dorny/test-reporter](https://github.com/dorny/test-reporter).
Выбрать нужную проверку: внутри счётчики и полный список тестов с результатами
и временем. Action закреплён на commit тега v3; результаты берутся из CTest JUnit.
Существующий Summary сохраняется, ZIP нужен только для исходных файлов и полных логов.

Публикация Checks требует checks:write, выданного только build job.
Для PR из fork этот шаг пропускается из-за read-only токена GitHub;
отчёт остаётся доступен в **Actions → запуск → Summary** без скачивания.
Отсутствие XML после ошибки сборки не скрывает её: job остаётся красной,
а существующий Summary показывает стадию и причину сбоя.

1. Открыть GitHub → Actions → Engine Build → нужный запуск.
2. В **Summary** видны результаты каждой сборки: стадии configure/build/test,
   таблица тестов, время, число passed/failed/skipped. Для ошибок раскрывается
   блок с диагностикой. Используется встроенный
   [GitHub Job Summary](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-commands#adding-a-job-summary),
   дополнительных токенов и прав записи не требуется, включая PR из fork.
3. В **Artifacts** скачать `test-report-ubuntu-24.04` или
   `test-report-windows-2022`. Хранение — 14 дней. Внутри:
   `reports/tests.xml` (JUnit), `reports/summary.md`, логи configure/build/test,
   CMakeConfigureLog, LastTest.log и оставшиеся fixtures неудачных тестов.

Падение конфигурации, сборки или тестов завершает job ошибкой. Пустой,
повреждённый или отсутствующий JUnit также не считается успехом. Отчёт формируется
и при ошибке сборки; артефакты загружаются через always(). Если ошибка случилась
до запуска scripts/ci.py, доступны обычные логи соответствующего шага Actions.
После отмены запуска полный отчёт не гарантируется.

## Повторить локально

Из корня проекта, с установленными зависимостями, Python 3.8+ и Ninja:

```text
python -m unittest discover -s scripts/tests -v
python scripts/ci.py --build-dir work/ci-release --parallel 4
```

Linux с Mesa/Xvfb:

```text
xvfb-run -a python3 scripts/ci.py --build-dir work/ci-release --graphics
```

Каталог должен быть новым либо уже настроенным на Ninja/Release.
Скрипт сохраняет отчёт в `<build-dir>/reports/summary.md`; переменная
GITHUB_STEP_SUMMARY нужна только для публикации в интерфейсе GitHub.

## Проверка изменений 2026-10-15

Workflow проверен actionlint 1.7.12. Пять проверок генератора отчёта прошли:
счётчики исходов, экранирование диагностики, пустой/повреждённый XML,
ограничение вывода и сохранение ошибки build.
Windows/GCC 15.2: полный Ninja/Release-прогон прошёл (35/35 core + headless),
дополнительный запуск с graphics — 44/44 без пропусков. Запись отчёта в
GITHUB_STEP_SUMMARY проверена. Linux локально не запускался, WSL-дистрибутива нет.
Удалённый запуск GitHub Actions требуется после публикации изменений.

### Подтверждённый удалённый запуск

2026-10-15, commit 26fdade, [PR #59](https://github.com/Enziferum/robot2D/pull/59):
[GitHub Actions](https://github.com/Enziferum/robot2D/actions/runs/38731059852)
успешен на Ubuntu 24.04 (43/43 теста, включая graphics) и Windows 2022
(35/35 core/headless), без пропусков. Артефакты обеих job скачаны;
summary.md, tests.xml и полные логи присутствуют и соответствуют результатам.
Чистая локальная Windows/Release-сборка с зафиксированными submodules — 44/44.
PR готов к ревью; слияние ещё не выполнялось.

Исправление yaml-cpp находится в editor/cmake/YamlCppPatching.cmake: GCC получает
недостающий cstdint только для emitterutils.cpp, без правок исходников submodule.

### Отчёты прямо в PR Checks

В PR #59 добавлен dorny/test-reporter v3 (закреплённый commit):
[Linux report](https://github.com/Enziferum/robot2D/runs/116417685976) и
[Windows report](https://github.com/Enziferum/robot2D/runs/116418553605).
Эти страницы содержат счётчики и полный список тестов; ничего скачивать не нужно.
Путь в интерфейсе: PR → Checks → Tests (нужная платформа).
Встроенный Summary запуска также сохраняется.

На финальном commit c6c657f [CI успешен](https://github.com/Enziferum/robot2D/actions/runs/38733201188):
Linux 43/43, Windows 35/35; оба Test Report Checks опубликованы и проверены.
Для PR из fork этот Action пропускается из-за read-only токена; таблица остаётся
доступна в Actions → запуск → Summary. Права checks:write ограничены build job.
