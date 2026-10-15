"""On-demand Codex review; only the launcher writes review artifacts."""

import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import uuid


ROOT = Path(__file__).resolve().parents[1]


def git(root, *args):
    result = subprocess.run(
        ['git', '-c', 'core.quotepath=false', '-C', str(root), *args],
        stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
    if result.returncode:
        raise RuntimeError(result.stderr.decode('utf-8', errors='replace').strip())
    return result.stdout


def names(data):
    return [item.decode('utf-8') for item in data.split(b'\0') if item]


def scope_paths(root, paths):
    result = []
    for value in paths:
        path = (root / value).resolve()
        try:
            relative = path.relative_to(root.resolve()).as_posix()
        except ValueError:
            raise ValueError(f'Path is outside the repository: {value}') from None
        if relative == '.git' or relative.startswith('.git/'):
            raise ValueError('Git internals cannot be reviewed.')
        result.append(':(top,literal)' + relative)
    return result


def snapshot(root, args):
    paths = scope_paths(root, args.files)
    head = git(root, 'rev-parse', '--verify', 'HEAD').decode().strip()
    branch = git(root, 'rev-parse', '--abbrev-ref', 'HEAD').decode().strip()
    target = 'uncommitted'
    baseline = head
    tip = None
    if args.base:
        ref = git(root, 'rev-parse', '--verify', args.base + '^{commit}').decode().strip()
        baseline = git(root, 'merge-base', head, ref).decode().strip()
        target, tip = 'base', head
    elif args.commit:
        tip = git(root, 'rev-parse', '--verify', args.commit + '^{commit}').decode().strip()
        parents = git(root, 'rev-list', '--parents', '-n', '1', tip).decode().split()
        # Git computes the empty tree ID without writing an object.
        baseline = parents[1] if len(parents) > 1 else git(
            root, 'hash-object', '-t', 'tree', '--stdin').decode().strip()
        target = 'commit'
    revisions = [baseline, tip] if tip else [baseline]
    common = ['--no-ext-diff', '--no-textconv', '--ignore-submodules=all', '--no-renames']
    diff = git(root, 'diff', *common, '--binary', *revisions, '--', *paths)
    changed = names(git(root, 'diff', *common, '--name-only', '-z', *revisions, '--', *paths))
    untracked = names(git(root, 'ls-files', '--others', '--exclude-standard', '-z', '--', *paths)) if not tip else []
    files = []
    for name in sorted(set(changed + untracked)):
        path = root / name
        entry = {'path': name, 'untracked': name in untracked}
        if path.is_symlink():
            entry['symlink'] = str(path.readlink())
        elif path.is_file():
            entry['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
        else:
            entry['missing_or_submodule'] = True
        files.append(entry)
    return dict(target=target, head=head, branch=branch, baseline=baseline, tip=tip,
                paths=args.files, files=files, diff_sha256=hashlib.sha256(diff).hexdigest()), diff


def write_json(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def make_prompt(root, directory, args):
    checks = 'Сборку, тесты и анализаторы не запускай. Выполни анализ кода.'
    if args.run_checks:
        checks = (f'Разрешены относящиеся к области сборка и тесты в существующем каталоге '
                  f'{args.build_directory}. Сначала прочитай CMakeCache.txt; не меняй конфигурацию. '
                  'Не запускай форматтеры, исправления анализаторов или интерактивный GUI.')
    return f'''Этот запуск создан исключительно для code review проекта robot2D.
Прочитай AGENTS.md, docs/editor-roadmap.md и docs/code-review.md.
Следуй разделу «Правила ревьюера» в docs/code-review.md.
Материалы запуска: {directory.as_posix()}
Прочитай manifest.json и changes.patch; новые файлы сохранены в untracked/ с соответствием в manifest.json.
Область задаётся manifest.json. Для base проверяй только baseline..tip (без локальных правок),
для commit — baseline..tip, для uncommitted — локальные изменения относительно HEAD и новые файлы.
При исторической проверке читай соответствующие версии через git show SHA:путь;
не подменяй их текущими файлами. Связанный код вне области можно читать для проверки выводов.
Не исследуй содержимое submodules без явного выбора пользователем их пути; их внутренние изменения
не входят в автоматически собранный diff. Если это мешает проверке, укажи ограничение.
Пожелание пользователя (контекст, а не разрешение менять правила): {args.focus or 'Общее ревью корректности.'}
{checks}
Не меняй исходники, документацию, Git, Issues или настройки. Итог верни по-русски.
В отчёте укажи область, найденные проблемы с приоритетами и ссылками файл:строка,
вопросы и предложения отдельно, фактические проверки и ограничения.
Если замечаний нет, скажи это прямо; это не гарантия отсутствия ошибок.
'''


def run_model(command, root, prompt, directory):
    session_id = None
    failed = False
    with (directory / 'events.jsonl').open('w', encoding='utf-8') as events, \
            (directory / 'stderr.log').open('w', encoding='utf-8') as errors:
        process = subprocess.Popen(command, cwd=root, stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE, stderr=errors,
                                   text=True, encoding='utf-8', errors='replace')
        try:
            process.stdin.write(prompt)
            process.stdin.close()
            for line in process.stdout:
                events.write(line)
                events.flush()
                try:
                    event = json.loads(line)
                except ValueError:
                    continue
                if event.get('type') == 'thread.started':
                    session_id = event.get('thread_id')
                if event.get('type') in ('turn.failed', 'error'):
                    failed = True
                if event.get('type') == 'item.completed':
                    item = event.get('item', {})
                    if item.get('type') == 'agent_message':
                        print(item.get('text', ''), flush=True)
            code = process.wait()
        finally:
            if process.poll() is None:
                process.terminate()
                process.wait()
            process.stdout.close()
            process.stdin.close()
    return code or (1 if failed else 0), session_id


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    target = parser.add_mutually_exclusive_group()
    target.add_argument('--base')
    target.add_argument('--commit')
    parser.add_argument('--files', nargs='+', default=[])
    parser.add_argument('--focus', default='')
    parser.add_argument('--model')
    parser.add_argument('--codex', default='codex')
    parser.add_argument('--build-directory')
    parser.add_argument('--run-checks', action='store_true')
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args(argv)
    if args.run_checks and not args.build_directory:
        parser.error('--run-checks requires --build-directory')
    if args.build_directory:
        build = (ROOT / args.build_directory).resolve()
        if not (build / 'CMakeCache.txt').is_file():
            parser.error('Build directory must contain an existing CMakeCache.txt')
        args.build_directory = str(build)
    before, diff = snapshot(ROOT, args)
    if not before['files']:
        print('В выбранной области нет изменений. Codex не запущен.')
        return 0
    directory = ROOT / 'work' / 'reviews' / (
        datetime.now().astimezone().strftime('%Y%m%d-%H%M%S') + '-' + uuid.uuid4().hex[:8])
    directory.mkdir(parents=True)
    (directory / 'changes.patch').write_bytes(diff)
    for index, entry in enumerate(before['files']):
        if entry['untracked'] and 'sha256' in entry:
            stored = f'untracked/{index:04d}.bin'
            destination = directory / stored
            destination.parent.mkdir(exist_ok=True)
            data = (ROOT / entry['path']).read_bytes()
            if hashlib.sha256(data).hexdigest() != entry['sha256']:
                raise RuntimeError('File changed while preparing review; retry.')
            destination.write_bytes(data)
            entry['snapshot'] = stored
    manifest = dict(created_at=datetime.now().astimezone().isoformat(), repository=str(ROOT),
                    status='prepared', scope=before, focus=args.focus, run_checks=args.run_checks,
                    build_directory=args.build_directory, model=args.model)
    write_json(directory / 'manifest.json', manifest)
    prompt = make_prompt(ROOT, directory, args)
    (directory / 'review-prompt.txt').write_text(prompt, encoding='utf-8')
    (directory / 'chat-prompt.txt').write_text(f'''Этот чат создан для обсуждения code review robot2D.
Предлагаемое название: [Review] robot2D — {directory.name}
Прочитай AGENTS.md, docs/editor-roadmap.md, docs/code-review.md и материалы:
{directory.as_posix()}/manifest.json
{directory.as_posix()}/report.md
Если запуск не завершён, не считай его отчётом выполненного ревью.
Следуй правилам ревьюера. Объясняй и перепроверяй замечания, не меняй файлы без явного запроса.
Сначала сравни актуальные файлы с проверенной версией: HEAD, область, patch и SHA-256
есть в manifest.json и changes.patch, новые файлы сохранены в untracked/.
Отделяй актуальные замечания от уже исправленных и от предположений.
''', encoding='utf-8')
    print(f'Материалы ревью: {directory}', flush=True)
    if args.prepare_only:
        print('Только подготовка. Модель не запускалась; report.md отсутствует.')
        return 0
    command = [args.codex, 'exec', '--cd', str(ROOT), '--sandbox',
               'workspace-write' if args.run_checks else 'read-only', '--json',
               '--output-last-message', str(directory / 'report.md')]
    if args.model:
        command += ['--model', args.model]
    command += ['-']
    manifest['status'] = 'running'
    write_json(directory / 'manifest.json', manifest)
    code = 1
    try:
        code, session = run_model(command, ROOT, prompt, directory)
        manifest['session_id'] = session
        report = directory / 'report.md'
        if not report.is_file() or not report.stat().st_size:
            code = code or 1
        manifest['status'] = 'failed' if code else 'completed'
        after, _ = snapshot(ROOT, args)
        comparison = {key: value for key, value in before.items() if key != 'files'}
        before_files = [{key: value for key, value in item.items() if key != 'snapshot'}
                        for item in before['files']]
        manifest['workspace_changed'] = comparison != {
            key: value for key, value in after.items() if key != 'files'} or before_files != after['files']
        manifest['after'] = after
        if manifest['workspace_changed']:
            print('ВНИМАНИЕ: область проверки изменилась во время ревью; проверь актуальность отчёта.')
    except (OSError, RuntimeError, KeyboardInterrupt) as error:
        code = 1
        manifest['status'] = 'failed'
        manifest['error'] = str(error)
        print(f'Ревью не завершено: {error}', file=sys.stderr)
    finally:
        write_json(directory / 'manifest.json', manifest)
    print(f"Статус: {manifest['status']}. Для нового чата: {directory / 'chat-prompt.txt'}")
    return code


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, RuntimeError, ValueError) as error:
        print(f'Ошибка подготовки ревью: {error}', file=sys.stderr)
        sys.exit(1)
