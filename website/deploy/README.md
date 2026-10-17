# Деплой robot2D: Linux VPS + Nginx

Резервный вариант: обычный Nginx и папка сайта, без Docker. Текущий выбор
пользователя — [GitHub Pages](github-pages.md); VPS пока покупать не требуется.
Astro/Starlight
собираются локально или в CI, на VPS отправляется **только website/dist/**.
Node.js, pnpm, исходники C++ и сборочные зависимости не нужны для раздачи сайта.
Сервер/домен ещё не куплены, поэтому внешнее подключение и установка не выполнялись.

## Структура сервера

```text
/srv/robot2d/
  releases/
    <commit-or-release-id>/     # содержимое dist/, неизменяемый готовый выпуск
  current -> releases/<id>     # root Nginx
  shared/_astro/              # накопленные content-hashed CSS/JS
```

Один отдельный deploy-пользователь владеет этой папкой, Nginx имеет права чтения
и прохода по каталогам. Не выдавай deploy-пользователю общий root-доступ ради копирования
статических файлов. Удаление старых выпусков в скрипт не включено: сначала держим
несколько предыдущих сборок для отката, лимит хранения решим после выбора диска.

## Первая установка — после появления VPS

1. Выбрать дистрибутив Linux, домен и SSH-доступ. Установить Nginx и rsync пакетным менеджером ОС.
2. Подготовить /srv/robot2d/releases и права deploy-пользователя; убедиться, что
   Nginx читает эти файлы. На системах с SELinux/AppArmor проверить контекст/профиль.
3. Скопировать nginx.conf.example в конфигурацию сайта, заменить пример домена,
   проверить `nginx -t`, затем reload службы. Этот файл — HTTP bootstrap, **не готовый TLS-конфиг**.
4. Настроить DNS и HTTPS-сертификат с автоматическим продлением. Конкретные команды
   зависят от ОС и домена; перед публичным запуском проверить HTTPS, redirect с HTTP
   и обновление сертификата. Сертификаты и SSH-ключи не храним в Git.
5. Скопировать activate-release.sh в принадлежащий deploy-пользователю служебный путь,
   например /srv/robot2d-tools/activate-release.sh. Nginx не должен раздавать эту папку.

## Собрать и выложить первый вариант

Сборка выполняется в checkout репозитория на локальной машине или runner CI.
Сначала задать настоящий SITE_URL, SITE_BASE=/ для отдельного домена:

```sh
cd website
SITE_URL=https://YOUR-DOMAIN SITE_BASE=/ pnpm build
SITE_BASE=/ pnpm verify
```

`YOUR-DOMAIN`, `YOUR-HOST` и `RELEASE_ID` ниже — заменяемые значения, не реальные адреса.
Используй уникальный полный Git SHA для каждой сборки; повторно не перезаписывай
активный каталог. Пример для Linux/macOS/WSL-клиента с ssh и rsync:

```sh
ssh deploy@YOUR-HOST 'mkdir /srv/robot2d/releases/RELEASE_ID'
rsync -a --chmod=D755,F644 dist/ deploy@YOUR-HOST:/srv/robot2d/releases/RELEASE_ID/
ssh deploy@YOUR-HOST 'sh /srv/robot2d-tools/activate-release.sh /srv/robot2d RELEASE_ID'
```

Не запускай активацию после ошибки rsync. Скрипт проверяет обязательные страницы
и Pagefind, затем атомарно заменяет current на Linux через symlink + GNU mv -T.
Перед переключением он пополняет shared/\_astro: старые хэшированные CSS/JS
остаются доступны клиентам с предыдущим HTML. Локальный rsync с checksum и
отложенными rename пополняет shared до переключения current; --delete не используется. Он не копирует HTML поверх активного сайта, не удаляет releases и отказывается
заменять реальную папку current. Одновременные активации исключает lock-каталог.
Если процесс был убит через SIGKILL, lock останется: прежде чем удалить пустой lock
вручную, убедиться, что другого deployment-процесса нет.

После переключения проверить HTTPS главной, /docs/core/, /docs/learn/, поиск,
реальный 404 и загрузку изображений. Nginx reload для каждого обновления контента
не требуется: root указывает на current. Новую конфигурацию Nginx применяют отдельно.

## Откат

```sh
ssh deploy@YOUR-HOST 'sh /srv/robot2d-tools/activate-release.sh /srv/robot2d PREVIOUS_RELEASE_ID'
```

Используется уже сохранённая папка. Отдельно проверь сайт после отката.
Бэкап репозитория и конфигурации сервера — отдельные задачи; symlink не заменяет бэкап.

## Как вести обновления

- **Текст/уроки:** небольшой PR → CI check/build/links → merge в dev → сборка
  проверенного commit с реальным SITE_URL → ручная выкладка → HTTP/ручная приёмка.
- **Релиз движка:** выпуск реального тега/артефактов → pnpm sync:releases → проверить
  снимок и файлы → PR сайта → обычная выкладка. Каталог автоматически исключает всё до 0.8.0.
- **Зависимости сайта:** отдельный PR с lockfile и проверками, независимо от контента.
- **API Core:** правка кода сопровождается ревью связанной страницы; в статье обновляются
  проверенный commit и ограничения. Полную версионность /docs/0.8/ добавим при выпуске
  стабильной 0.8, когда появится второй поддерживаемый вариант документации.

CI сайта сохраняет корневую static-сборку как artifact robot2d-website-<sha> на 7 дней.
Это preview PR: для production использовать принятый commit и пересобрать с реальным SITE_URL.
В архиве нужное содержимое dist/; подпапочная тестовая сборка в artifact не попадает.

Сначала ручной процесс; после проверки реального сервера можно добавить отдельный
ручной workflow deploy с GitHub Environment approval, pinned SSH known_hosts и deploy-ключом.
Сборка на PR никогда не получает SSH-ключи и не выкладывает непроверенный код.
Автоматический production-deploy на каждый push сейчас не настроен.

## Nginx и кэш

Конфиг раздаёт настоящий static tree и настоящий 404 — это не SPA fallback.
HTML, Pagefind manifest и публичные файлы переоцениваются по Cache-Control: no-cache.
Только content-hashed /\_astro/ имеет годовой immutable cache. Не включай глобальный
immutable для brand PNG или Pagefind entry: имена могут остаться прежними при обновлении.

Nginx на целевом Linux проверяется через nginx -t после подстановки домена и путей;
настоящего Nginx/VPS в текущей Windows-среде нет. Linux fixture-тест активации
входит в CI; локально проверяется shell-синтаксис. Для размещения в подпапке
нужно согласованно поменять Nginx mount, SITE_BASE и 404 location; этот конфиг рассчитан на /.

Первичные справочники: [Astro deployment](https://docs.astro.build/en/guides/deploy/),
[Nginx static files](https://docs.nginx.com/nginx/admin-guide/web-server/serving-static-content/),
[Nginx headers](https://nginx.org/en/docs/http/ngx_http_headers_module.html).
