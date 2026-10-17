# GitHub Pages для robot2D

Основной хостинг сайта — GitHub Pages. Публикуется только `website/dist/`:
Astro/Starlight, документация, Pagefind и публичные ресурсы. Установщики/архивы
остаются в GitHub Releases. VPS, Nginx, Docker и SSH-ключи для этого не нужны.
Workflow: [Website](../../.github/workflows/website.yml).

## Однократная настройка владельцем

1. Открыть Settings → Pages → Build and deployment → Source → **GitHub Actions**.
2. В Settings → Environments → **github-pages** проверить Deployment branches and
   tags. Для автопубликации разрешить `dev`, для первого теста также `WEB-02`.
   Если ограничения уже установлены, добавлять конкретные нужные ветки; снимать
   остальные защиты ради деплоя не требуется. При обязательном review подтвердить
   публикацию в Actions. Для PR-label запуска при ограничениях может потребоваться
   разрешить PR refs (`refs/pull/*/merge`) согласно подсказке ошибки GitHub.
3. Оставить Enforce HTTPS включённым. Не выбирать Deploy from a branch/папку docs:
   сборкой и публикацией управляет Actions.

По API до первой настройки Pages отсутствовал; после действия пользователя
подтверждены `build_type: workflow`, HTTPS и адрес
**https://enziferum.github.io/robot2D/**. У существовавшего окружения github-pages
ограничений веток/обязательного review на момент проверки не было. Сам workflow
не включает Pages и не меняет настройки репозитория (`enablement: false`).
Токен GitHub выдаётся Actions автоматически, PAT/секрет для Pages не нужен.

## После merge в dev

Push в `dev`, затрагивающий `website/**`, корневой CMakeLists.txt (версия движка)
или Website workflow, запускает проверки и затем deploy. Обычные PR выполняют
проверки и сохраняют preview artifact, но не публикуют сайт.

Проверки: frozen pnpm install, release-policy tests, Astro check, формат,
сборка/ссылки для `/` и `/robot2D/`, Linux fixture-тест Nginx-выкладки/отката.
Перед upload выполняется дополнительная сборка и проверка с **реальными**
`origin` и `base_path` из `actions/configure-pages`. Фиктивный `example.invalid`
используется только в проверке подкаталога и не попадает в Pages artifact.
Публикуется именно проверенный `dist/` того же запуска; deploy зависит от check.
Build имеет read-only contents/pages; только deploy получает pages:write и
id-token:write. Публикации сериализуются через github-pages concurrency group.

## Ручной деплой выбранной ветки

В Actions → Website → Run workflow выбрать **Branch: WEB-02** (или другую ветку,
в которой есть этот workflow), затем Run workflow. Из CLI:

```sh
gh workflow run website.yml --repo Enziferum/robot2D --ref WEB-02
```

Это настоящая публикация по **одному общему адресу**, не отдельный preview URL.
Следующий успешный deploy из dev или вручную выбранной ветки обновит этот сайт.
Для отката запустить workflow на ветке с нужным проверенным состоянием.

### Первый показ до merge workflow в dev

GitHub документирует наличие workflow в default branch как условие ручного
запуска, поэтому до первого merge кнопка Run workflow может отсутствовать.
Зарегистрированный PR-проверками Website также можно попробовать запустить CLI
командой выше. Альтернатива без merge — **однократно добавить метку `deploy-pages`
на PR #134**:

```sh
gh pr edit 134 --repo Enziferum/robot2D --add-label deploy-pages
```

Событие labeled запускает проверки и публикацию точного head SHA PR. Это работает
только для PR из того же репозитория; fork PR не получает разрешение публикации.
Наличие метки само по себе не включает автодеплой каждого последующего push в PR.
Чтобы опубликовать обновлённую ветку повторно через метку, снять и добавить её
заново. Другие метки не запускают эти дополнительные проверки/публикацию.

Статус — Actions → Website → check/deploy; после успеха адрес показан в summary
и deployment github-pages. Только успешный deploy и HTTP-проверка подтверждают,
что сайт доступен: один upload artifact ещё не означает публикацию.

## Свой домен позднее

Зарегистрировать домен отдельно, затем Settings → Pages → Custom domain,
подтверждение владения/DNS по инструкции GitHub и проверка HTTPS. Workflow берёт
origin/base_path из Pages: для project URL это `/robot2D/`, для собственного домена
обычно `/`. После изменения настроек повторно запустить Website, чтобы заново
собрались ссылки, Pagefind, canonical URL и sitemap. Само добавление домена не
меняет файлы уже опубликованной сборки. При workflow-deployment CNAME в dist не
используется как замена настройки Custom domain в GitHub.

Ручная приёмка: главная, оба урока, навигация, изображения, поиск, 404 и доступность
с разных сетей. Computer-use не применяется. На Pages заголовками/cache/TLS
управляет GitHub; правила Nginx из резервного VPS-варианта сюда не переносятся.

Справочники: [custom workflows](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages),
[manual workflow runs](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/manually-run-a-workflow),
[custom domains](https://docs.github.com/en/pages/configuring-a-custom-domain-for-your-github-pages-site/about-custom-domains-and-github-pages).
