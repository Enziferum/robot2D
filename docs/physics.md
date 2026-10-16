# Физика 2D и события контактов

PHYS-01, первый шаг от `dev` @ `c05a55b`, 2026-10-16.
Задача: [#78](https://github.com/Enziferum/robot2D/issues/78).

## Геометрия и параметры

`Box2DPhysicsAdapter` владеет миром, телами и фикстурами. Одной сущности
соответствуют одно тело и не более одной прямоугольной фикстуры; дети обходятся
рекурсивно. Повторный `addRuntime` не создаёт дубликаты.

Координаты Transform и offset коллайдера заданы в пикселях, масштаб — 32 px/m.
`Collider2DComponent::size` задаёт долю размера Transform для **полуразмера**:
значение `(0.5, 0.5)` даёт прямоугольник полного размера Transform.
Локальный центр равен `size * (0.5 - origin) + offset`; размеры и центр
переводятся в метры. Transform хранит угол в градусах, Box2D — в радианах.
После Step угол присваивается абсолютно, без накопления.

Учитываются Static/Dynamic/Kinematic, fixedRotation, bullet, gravityScale,
linear/angular damping, density, friction, restitution и restitutionThreshold.
Невалидные параметры тела/геометрии отклоняются с UUID и причиной в журнале.
Неположительный или нечисловой dt не запускает Step.

Матрица слоёв разрешает Player/Enemy взаимодействовать с Ground/Wall,
Projectile — с Enemy/Wall/Ground. Trigger взаимодействует с Player/Enemy/Projectile;
для событий нужен `isTrigger=true`. Player/Projectile по умолчанию исключены.
При `useDefaultMask=false` используется собственная маска; `markFilterDirty`
применяет обновлённые category/mask/group к фикстуре перед следующим Step.

## Доставка событий

- На одну пару фикстур: один Enter при начале контакта, затем один Stay на
  каждый последующий Step и один Exit при завершении. Спящие контакты и сенсоры
  сохраняют Stay. Повторное касание после Exit начинает новую пару событий.
- Доставка выполняется после Box2D Step, когда мир разблокирован, отдельно
  каждому живому участнику: `callback(contact, self, other)`.
- Порядок A/B канонический по UUID; нормаль направлена A → B. C# получает
  `normalToMe`: для A — отрицательную нормаль, для B — положительную.
- Идентификаторы фикстур ненулевые, уникальны в пределах запущенного мира
  и стабильны до удаления фикстуры. Они не являются указателями или сохранёнными ID.
- Точки контакта и separation заданы в метрах, импульсы — в единицах Box2D.
  Сенсоры не имеют manifold: pointCount, точки и нормаль нулевые.
  Exit не содержит актуальных точек/импульсов; нормаль сохраняется из контакта.
- Native/managed ABI остаётся прежним: Pack=1, PhysicsContact2D — 84 байта,
  PhysicsContactPoint2D — 12 байт, enum — byte, bool — I1.
  Пакет копируется для каждого C# получателя, поэтому изменение `ref` локально.
  Каждый ScriptInstance имеет собственный MonoClassWrapper/объект даже для одного класса.

Урон от одного касания следует наносить в Enter, а не Stay. Удаление/пометка
тела на удаление подавляет оставшиеся Enter/Stay с ним в текущей доставке.
Исключение C# обработчика регистрируется с участниками и типом события.

## Создание, удаление и Stop

`addRuntime`/`removeRuntime` из callback ставятся в очередь и применяются после
доставки событий. Remove имеет приоритет перед Add той же сущности в одном пакете.
Вне callback операции выполняются сразу. Удаляется также физическое поддерево;
у оставшегося участника контакта будет один Exit на следующем Step.
`Scene::processRemovals(Runtime)` снимает тела до удаления сущностей из ECS.
Это контракт удаления физики, а не новый API создания/удаления C# объектов.

Stop из callback прекращает дальнейшую доставку и освобождает мир после выхода
из callback. Stop/деструктор очищают runtimeBody/runtimeFixture всех ещё доступных
компонентов, контакты, очереди и callback. Повторный Stop безопасен.
Вложенный update игнорируется, Start из callback отклоняется с диагностикой.

Сцены сохраняют IsTrigger, CategoryBits/MaskBits/GroupIndex, GravityScale,
LinearDamping и AngularDamping. Поля опциональны при чтении старых сцен;
версия схемы не повышается. Файлы пользовательских проектов этот шаг не меняет.

## Проверка и границы

Постоянные `editor-physics-*`: geometry, walls, bullet, triggers, filters,
lifecycle, callbacks, invalid, serialization, managed. Последний сценарий
загружает свежую ScriptCore и PhysicsDriver через настоящий ScriptEngine/Mono:
два экземпляра одного класса, ABI, Enter/Stay/Exit, триггеры, направления нормалей
и изоляция ref-пакетов. Все сценарии доступны в обычной headless suite.

На Windows/Release в `work/phys-01-build` собраны редактор, core-tests и
editor-test-suite; прошли 103/103 CTest, включая 41 graphics и 10 новых physics.
Полный обязательный hook дополнительно проверяет формат и строгий clang-tidy.

Ручная приёмка: в тестовой игровой сцене поставить статические стены, Dynamic
Player/Enemy, Projectile с bullet и сенсор Trigger. Проверить остановку у стен,
один урон на Enter, trigger callbacks, удаление пули и два цикла Play/Stop.
Эта интерактивная приёмка и standalone ещё не выполнены.

Полный путь C# Instantiate/Destroy и его lifecycle остаётся в
[RUNTIME-01 #77](https://github.com/Enziferum/robot2D/issues/77); поэтому PHYS-01
не закрывается этим шагом. Нет новой реализации one-way/drop-through,
круглых коллайдеров, фиксированного physics timestep или переработки Core ECS.
