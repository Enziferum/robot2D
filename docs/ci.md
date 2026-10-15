# CI и отчёты

Workflow [Engine Build](../.github/workflows/ci.yml) запускается на push,
каждое обновление PR и вручную через Actions → Engine Build → Run workflow.

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
