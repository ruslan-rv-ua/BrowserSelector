# Інструкція з публікації Browser Selector у Scoop

Ця інструкція описує покрокову процедуру налаштування автоматичної публікації Browser Selector для встановлення через Scoop — популярний пакетний менеджер для Windows.

## Зміст

1. [Огляд архітектури](#1-огляд-архітектури)
2. [Створення Scoop Bucket репозиторію](#2-створення-scoop-bucket-репозиторію)
3. [Створення маніфесту застосунку](#3-створення-маніфесту-застосунку)
4. [Налаштування GitHub Actions для основного репозиторію](#4-налаштування-github-actions-для-основного-репозиторію)
5. [Налаштування GitHub Actions для bucket репозиторію](#5-налаштування-github-actions-для-bucket-репозиторію)
6. [Процес релізу](#6-процес-релізу)
7. [Тестування](#7-тестування)
8. [Найкращі практики](#8-найкращі-практики)
9. [Усунення проблем](#9-усунення-проблем)

---

## 1. Огляд архітектури

### Що таке Scoop?

[Scoop](https://scoop.sh/) — це командний інсталятор для Windows, що дозволяє легко встановлювати програми з командного рядка.

### Компоненти системи

```
┌─────────────────────────────────────────────────────────────────┐
│                    GitHub (ruslan-rv-ua)                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────┐        ┌─────────────────────────┐    │
│  │  BrowserSelector    │        │  scoop-bucket           │    │
│  │  (основний репо)    │───────▶│  (bucket репо)          │    │
│  │                     │        │                         │    │
│  │  • Вихідний код     │ Push   │  • browserselector.json │    │
│  │  • GitHub Actions   │ Update │  • (інші маніфести)     │    │
│  │  • Releases (.zip)  │        │                         │    │
│  └─────────────────────┘        └─────────────────────────┘    │
│                                              │                  │
└──────────────────────────────────────────────│──────────────────┘
                                               │
                                               ▼
                              ┌─────────────────────────────┐
                              │  Користувачі Scoop          │
                              │                             │
                              │  scoop bucket add ...       │
                              │  scoop install ...          │
                              └─────────────────────────────┘
```

### Потік автоматизації

1. **Розробник** створює тег релізу (наприклад, `v1.0.0`)
2. **GitHub Actions** в основному репозиторії:
   - Компілює застосунок
   - Створює ZIP архів з `.exe` та `config.json`
   - Публікує GitHub Release з архівом
   - Обчислює SHA256 хеш
   - Оновлює маніфест у bucket репозиторії
3. **Користувачі** встановлюють через `scoop install`

---

## 2. Створення Scoop Bucket репозиторію

### 2.1. Створення репозиторію на GitHub

1. Перейдіть на https://github.com/new
2. Налаштуйте репозиторій:
   - **Repository name:** `scoop-bucket`
   - **Description:** `Scoop bucket for Browser Selector and other applications`
   - **Public:** Обов'язково публічний
   - **Initialize with README:** ✓
   - **Add .gitignore:** None
   - **License:** MIT (рекомендовано)

3. Натисніть **Create repository**

### 2.2. Клонування та базова структура

```powershell
# Клонувати репозиторій
git clone https://github.com/ruslan-rv-ua/scoop-bucket.git
cd scoop-bucket
```

### 2.3. Створення структури bucket

```
scoop-bucket/
├── README.md
├── LICENSE
├── bucket/
│   └── browserselector.json    # Маніфест застосунку
├── bin/
│   └── checkver.ps1           # Скрипт перевірки версій (опціонально)
└── .github/
    └── workflows/
        └── ci.yml             # CI для валідації маніфестів
```

### 2.4. Створення README.md для bucket

```markdown
# Scoop Bucket

Personal Scoop bucket containing Browser Selector and other applications.

## Usage

### Add this bucket

```powershell
scoop bucket add ruslan-rv-ua https://github.com/ruslan-rv-ua/scoop-bucket
```

### Install applications

```powershell
scoop install browserselector
```

## Available Applications

| Application | Description |
|-------------|-------------|
| [browserselector](https://github.com/ruslan-rv-ua/BrowserSelector) | Accessible Windows browser selector |

## Contributing

Feel free to submit issues and pull requests.
```

### 2.5. Commit та push

```powershell
git add .
git commit -m "Initial bucket structure"
git push origin main
```

---

## 3. Створення маніфесту застосунку

### 3.1. Структура маніфесту

Створіть файл `bucket/browserselector.json`:

```json
{
    "version": "0.1.1",
    "description": "Accessible Windows browser selector - choose which browser opens your links",
    "homepage": "https://github.com/ruslan-rv-ua/BrowserSelector",
    "license": "MIT",
    "architecture": {
        "64bit": {
            "url": "https://github.com/ruslan-rv-ua/BrowserSelector/releases/download/v0.1.1/BrowserSelector-0.1.1-win64.zip",
            "hash": "SHA256_HASH_HERE"
        }
    },
    "extract_dir": "BrowserSelector",
    "bin": "BrowserSelector.exe",
    "shortcuts": [
        ["BrowserSelector.exe", "Browser Selector"]
    ],
    "persist": "config.json",
    "checkver": "github",
    "autoupdate": {
        "architecture": {
            "64bit": {
                "url": "https://github.com/ruslan-rv-ua/BrowserSelector/releases/download/v$version/BrowserSelector-$version-win64.zip"
            }
        }
    }
}
```

### 3.2. Пояснення полів маніфесту

| Поле | Призначення |
|------|-------------|
| `version` | Поточна версія застосунку |
| `description` | Короткий опис (один рядок) |
| `homepage` | Домашня сторінка проєкту |
| `license` | Ліцензія (SPDX ідентифікатор) |
| `architecture` | Архітектурно-специфічні налаштування |
| `url` | URL для завантаження |
| `hash` | SHA256 хеш архіву для верифікації |
| `extract_dir` | Папка всередині архіву для розпакування |
| `bin` | Виконувані файли для PATH |
| `shortcuts` | Ярлики в меню Пуск |
| `persist` | Файли/папки, що зберігаються між оновленнями |
| `checkver` | Метод перевірки нових версій |
| `autoupdate` | Налаштування автооновлення маніфесту |

### 3.3. Розширений маніфест з додатковими можливостями

```json
{
    "version": "0.1.1",
    "description": "Accessible Windows browser selector - choose which browser opens your links",
    "homepage": "https://github.com/ruslan-rv-ua/BrowserSelector",
    "license": "MIT",
    "notes": [
        "After installation, run 'BrowserSelector.exe' and click 'Set as Default' to register as default browser.",
        "Configuration file is stored in the app directory."
    ],
    "architecture": {
        "64bit": {
            "url": "https://github.com/ruslan-rv-ua/BrowserSelector/releases/download/v0.1.1/BrowserSelector-0.1.1-win64.zip",
            "hash": "SHA256_HASH_HERE"
        }
    },
    "extract_dir": "BrowserSelector",
    "bin": "BrowserSelector.exe",
    "shortcuts": [
        ["BrowserSelector.exe", "Browser Selector"]
    ],
    "persist": "config.json",
    "post_install": [
        "if (!(Test-Path \"$persist_dir\\config.json\")) {",
        "    Copy-Item \"$dir\\config.json\" \"$persist_dir\\config.json\"",
        "}"
    ],
    "checkver": "github",
    "autoupdate": {
        "architecture": {
            "64bit": {
                "url": "https://github.com/ruslan-rv-ua/BrowserSelector/releases/download/v$version/BrowserSelector-$version-win64.zip"
            }
        },
        "hash": {
            "url": "$url.sha256"
        }
    }
}
```

---

## 4. Налаштування GitHub Actions для основного репозиторію

### 4.1. Створення workflow файлу

Створіть файл `.github/workflows/release.yml` в основному репозиторії BrowserSelector:

```yaml
name: Release

on:
  push:
    tags:
      - 'v*'

env:
  APP_NAME: BrowserSelector
  
permissions:
  contents: write

jobs:
  build:
    runs-on: windows-latest
    
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Setup MSYS2
        uses: msys2/setup-msys2@v2
        with:
          msystem: MINGW64
          update: true
          install: >-
            mingw-w64-x86_64-gcc
            make

      - name: Build Release
        shell: msys2 {0}
        run: |
          make release

      - name: Get version from tag
        id: get_version
        shell: bash
        run: |
          VERSION=${GITHUB_REF#refs/tags/v}
          echo "VERSION=$VERSION" >> $GITHUB_OUTPUT
          echo "Building version: $VERSION"

      - name: Create release package
        shell: pwsh
        run: |
          $version = "${{ steps.get_version.outputs.VERSION }}"
          $packageDir = "BrowserSelector"
          $zipName = "BrowserSelector-$version-win64.zip"
          
          # Create package directory
          New-Item -ItemType Directory -Force -Path $packageDir
          
          # Copy files
          Copy-Item "bin/BrowserSelector.exe" -Destination $packageDir
          Copy-Item "config.json" -Destination $packageDir
          Copy-Item "LICENSE" -Destination $packageDir
          Copy-Item "README.md" -Destination $packageDir
          
          # Create ZIP archive
          Compress-Archive -Path $packageDir -DestinationPath $zipName -Force
          
          # Calculate SHA256
          $hash = (Get-FileHash -Path $zipName -Algorithm SHA256).Hash.ToLower()
          $hash | Out-File -FilePath "$zipName.sha256" -NoNewline
          
          Write-Host "Package: $zipName"
          Write-Host "SHA256: $hash"

      - name: Create GitHub Release
        uses: softprops/action-gh-release@v1
        with:
          files: |
            BrowserSelector-*.zip
            BrowserSelector-*.zip.sha256
          generate_release_notes: true
          draft: false
          prerelease: ${{ contains(github.ref, '-alpha') || contains(github.ref, '-beta') || contains(github.ref, '-rc') }}
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}

      - name: Calculate hash for Scoop
        id: hash
        shell: pwsh
        run: |
          $version = "${{ steps.get_version.outputs.VERSION }}"
          $zipName = "BrowserSelector-$version-win64.zip"
          $hash = (Get-FileHash -Path $zipName -Algorithm SHA256).Hash.ToLower()
          echo "HASH=$hash" >> $env:GITHUB_OUTPUT

      - name: Update Scoop bucket
        uses: peter-evans/repository-dispatch@v2
        with:
          token: ${{ secrets.SCOOP_BUCKET_TOKEN }}
          repository: ruslan-rv-ua/scoop-bucket
          event-type: update-manifest
          client-payload: |
            {
              "version": "${{ steps.get_version.outputs.VERSION }}",
              "hash": "${{ steps.hash.outputs.HASH }}",
              "url": "https://github.com/ruslan-rv-ua/BrowserSelector/releases/download/v${{ steps.get_version.outputs.VERSION }}/BrowserSelector-${{ steps.get_version.outputs.VERSION }}-win64.zip"
            }
```

### 4.2. Створення Personal Access Token

Для оновлення bucket репозиторію потрібен Personal Access Token:

1. Перейдіть на https://github.com/settings/tokens
2. Натисніть **Generate new token (classic)**
3. Налаштуйте:
   - **Note:** `Scoop Bucket Update`
   - **Expiration:** Виберіть відповідний термін
   - **Scopes:** 
     - ✓ `repo` (повний доступ до репозиторіїв)
4. Натисніть **Generate token**
5. **Скопіюйте токен** (він показується лише один раз!)

### 4.3. Додавання секрету до репозиторію

1. Перейдіть до Settings → Secrets and variables → Actions
2. Натисніть **New repository secret**
3. Налаштуйте:
   - **Name:** `SCOOP_BUCKET_TOKEN`
   - **Secret:** Вставте скопійований токен
4. Натисніть **Add secret**

---

## 5. Налаштування GitHub Actions для bucket репозиторію

### 5.1. Workflow для оновлення маніфесту

Створіть файл `.github/workflows/update-manifest.yml` в репозиторії scoop-bucket:

```yaml
name: Update Manifest

on:
  repository_dispatch:
    types: [update-manifest]
  workflow_dispatch:
    inputs:
      version:
        description: 'Version (without v prefix)'
        required: true
      hash:
        description: 'SHA256 hash'
        required: true
      url:
        description: 'Download URL'
        required: true

permissions:
  contents: write

jobs:
  update:
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout bucket
        uses: actions/checkout@v4

      - name: Get payload
        id: payload
        run: |
          if [ "${{ github.event_name }}" = "repository_dispatch" ]; then
            echo "VERSION=${{ github.event.client_payload.version }}" >> $GITHUB_OUTPUT
            echo "HASH=${{ github.event.client_payload.hash }}" >> $GITHUB_OUTPUT
            echo "URL=${{ github.event.client_payload.url }}" >> $GITHUB_OUTPUT
          else
            echo "VERSION=${{ github.event.inputs.version }}" >> $GITHUB_OUTPUT
            echo "HASH=${{ github.event.inputs.hash }}" >> $GITHUB_OUTPUT
            echo "URL=${{ github.event.inputs.url }}" >> $GITHUB_OUTPUT
          fi

      - name: Update manifest
        run: |
          VERSION="${{ steps.payload.outputs.VERSION }}"
          HASH="${{ steps.payload.outputs.HASH }}"
          URL="${{ steps.payload.outputs.URL }}"
          
          echo "Updating manifest to version $VERSION"
          echo "Hash: $HASH"
          echo "URL: $URL"
          
          # Update browserselector.json using jq
          jq --arg version "$VERSION" \
             --arg hash "$HASH" \
             --arg url "$URL" \
             '.version = $version | 
              .architecture."64bit".url = $url | 
              .architecture."64bit".hash = $hash' \
             bucket/browserselector.json > bucket/browserselector.json.tmp
          
          mv bucket/browserselector.json.tmp bucket/browserselector.json
          
          # Show updated manifest
          cat bucket/browserselector.json

      - name: Commit and push
        run: |
          git config user.name "github-actions[bot]"
          git config user.email "github-actions[bot]@users.noreply.github.com"
          
          git add bucket/browserselector.json
          git commit -m "Update browserselector to ${{ steps.payload.outputs.VERSION }}"
          git push
```

### 5.2. Workflow для валідації маніфестів

Створіть файл `.github/workflows/ci.yml` в репозиторії scoop-bucket:

```yaml
name: CI

on:
  push:
    paths:
      - 'bucket/*.json'
  pull_request:
    paths:
      - 'bucket/*.json'

jobs:
  validate:
    runs-on: windows-latest
    
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Install Scoop
        shell: pwsh
        run: |
          Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
          Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression

      - name: Add bucket and validate
        shell: pwsh
        run: |
          # Add this bucket locally
          scoop bucket add local $PWD
          
          # Validate each manifest
          Get-ChildItem bucket/*.json | ForEach-Object {
            $name = $_.BaseName
            Write-Host "Validating: $name"
            
            # Check JSON syntax
            try {
              $null = Get-Content $_.FullName | ConvertFrom-Json
              Write-Host "  ✓ Valid JSON"
            } catch {
              Write-Host "  ✗ Invalid JSON: $_"
              exit 1
            }
            
            # Try to get info (validates manifest structure)
            $info = scoop info $name 2>&1
            if ($LASTEXITCODE -eq 0) {
              Write-Host "  ✓ Valid manifest structure"
            } else {
              Write-Host "  ✗ Invalid manifest: $info"
              exit 1
            }
          }
          
          Write-Host "`nAll manifests validated successfully!"

      - name: Check hash accessibility
        shell: pwsh
        run: |
          Get-ChildItem bucket/*.json | ForEach-Object {
            $manifest = Get-Content $_.FullName | ConvertFrom-Json
            $url = $manifest.architecture.'64bit'.url
            
            if ($url) {
              Write-Host "Checking URL accessibility: $url"
              try {
                $response = Invoke-WebRequest -Uri $url -Method Head -TimeoutSec 10
                Write-Host "  ✓ URL is accessible (Status: $($response.StatusCode))"
              } catch {
                Write-Host "  ⚠ URL check failed (may be expected for new releases): $_"
              }
            }
          }
```

---

## 6. Процес релізу

### 6.1. Підготовка до релізу

1. **Оновіть версію** в файлах:
   - `resources/app.rc` (FILEVERSION, PRODUCTVERSION, FileVersion, ProductVersion)
   - `CHANGELOG.md`

2. **Commit зміни:**
   ```powershell
   git add .
   git commit -m "chore: bump version to X.Y.Z"
   ```

3. **Merge в main:**
   ```powershell
   git checkout main
   git merge develop
   git push origin main
   ```

### 6.2. Створення релізу

```powershell
# Створити та push тег
git tag -a v0.2.0 -m "Release v0.2.0"
git push origin v0.2.0
```

### 6.3. Що відбувається автоматично

1. GitHub Actions запускає workflow `release.yml`
2. Компілюється застосунок
3. Створюється ZIP архів
4. Публікується GitHub Release
5. Відправляється repository_dispatch до scoop-bucket
6. Оновлюється маніфест в bucket репозиторії

### 6.4. Ручне оновлення маніфесту (альтернатива)

Якщо автоматика не спрацювала:

```powershell
# В репозиторії scoop-bucket
cd scoop-bucket

# Отримати хеш нового релізу
$url = "https://github.com/ruslan-rv-ua/BrowserSelector/releases/download/v0.2.0/BrowserSelector-0.2.0-win64.zip"
$tempFile = "$env:TEMP\BrowserSelector.zip"
Invoke-WebRequest -Uri $url -OutFile $tempFile
$hash = (Get-FileHash -Path $tempFile -Algorithm SHA256).Hash.ToLower()
Write-Host "Hash: $hash"

# Оновити маніфест вручну в bucket/browserselector.json
# Змінити version, url, hash

git add bucket/browserselector.json
git commit -m "Update browserselector to 0.2.0"
git push
```

---

## 7. Тестування

### 7.1. Локальне тестування маніфесту

```powershell
# Встановити Scoop (якщо не встановлено)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression

# Додати bucket
scoop bucket add ruslan-rv-ua https://github.com/ruslan-rv-ua/scoop-bucket

# Пошук застосунку
scoop search browserselector

# Інформація про застосунок
scoop info browserselector

# Встановлення
scoop install browserselector

# Перевірка
BrowserSelector.exe --help

# Оновлення (після нового релізу)
scoop update browserselector

# Видалення
scoop uninstall browserselector
```

### 7.2. Тестування автооновлення

```powershell
# В репозиторії scoop-bucket
cd scoop-bucket

# Увімкнути debug режим Scoop
scoop config debug $true

# Перевірити наявність оновлень
.\bin\checkver.ps1 browserselector

# Оновити маніфест автоматично
.\bin\checkver.ps1 browserselector -u

# Перевірити зміни
git diff bucket/browserselector.json
```

### 7.3. Скрипт checkver.ps1

Створіть `bin/checkver.ps1` в scoop-bucket (опціонально, для ручної перевірки):

```powershell
# checkver.ps1 - Check for new versions and update manifests
param(
    [Parameter(Position = 0)]
    [string]$App = '*',
    
    [switch]$Update,
    [Alias('u')]
    [switch]$ForceUpdate
)

$bucketsDir = Join-Path $PSScriptRoot '..\bucket'

# Download Scoop's checkver if not available
$checkverUrl = 'https://raw.githubusercontent.com/ScoopInstaller/Scoop/master/bin/checkver.ps1'
$checkverPath = Join-Path $env:TEMP 'scoop-checkver.ps1'

if (-not (Test-Path $checkverPath)) {
    Invoke-WebRequest -Uri $checkverUrl -OutFile $checkverPath
}

# Run checkver
$params = @{
    App = $App
    Dir = $bucketsDir
}

if ($Update -or $ForceUpdate) {
    $params['Update'] = $true
}

if ($ForceUpdate) {
    $params['ForceUpdate'] = $true
}

& $checkverPath @params
```

---

## 8. Найкращі практики

### 8.1. Версіонування

- Використовуйте [Semantic Versioning](https://semver.org/)
- Формат тегів: `v1.2.3`
- Pre-release версії: `v1.2.3-beta.1`, `v1.2.3-rc.1`

### 8.2. Іменування архівів

Рекомендований формат:
```
{AppName}-{version}-{platform}.zip
```

Приклади:
- `BrowserSelector-0.1.1-win64.zip`
- `BrowserSelector-0.1.1-win32.zip`

### 8.3. Структура архіву

```
BrowserSelector-0.1.1-win64.zip
└── BrowserSelector/
    ├── BrowserSelector.exe
    ├── config.json
    ├── LICENSE
    └── README.md
```

### 8.4. Безпека

- **Завжди** включайте SHA256 хеш
- Використовуйте HTTPS для всіх URL
- Зберігайте токени в GitHub Secrets
- Регулярно оновлюйте Personal Access Token

### 8.5. Документація

- Додавайте `notes` до маніфесту з інструкціями
- Оновлюйте README в bucket репозиторії
- Ведіть CHANGELOG в основному репозиторії

### 8.6. Persist (збереження даних)

Для файлів, що повинні зберігатися між оновленнями:

```json
{
    "persist": [
        "config.json",
        "data"
    ]
}
```

### 8.7. Shortcuts та bin

```json
{
    "bin": "BrowserSelector.exe",
    "shortcuts": [
        ["BrowserSelector.exe", "Browser Selector"]
    ]
}
```

---

## 9. Усунення проблем

### 9.1. Помилка "Hash check failed"

**Причина:** Хеш в маніфесті не відповідає завантаженому файлу.

**Рішення:**
```powershell
# Отримати правильний хеш
$url = "https://github.com/.../BrowserSelector-0.1.1-win64.zip"
$tempFile = "$env:TEMP\test.zip"
Invoke-WebRequest -Uri $url -OutFile $tempFile
(Get-FileHash $tempFile -Algorithm SHA256).Hash.ToLower()

# Оновити маніфест з правильним хешем
```

### 9.2. Помилка "Could not find manifest"

**Причина:** Bucket не додано або маніфест відсутній.

**Рішення:**
```powershell
# Перевірити додані buckets
scoop bucket list

# Оновити buckets
scoop update

# Перевірити наявність маніфесту
scoop search browserselector
```

### 9.3. GitHub Actions не запускається

**Причина:** Workflow не налаштовано або немає прав.

**Рішення:**
1. Перевірте наявність файлу `.github/workflows/release.yml`
2. Перевірте синтаксис YAML
3. Перевірте права токена `SCOOP_BUCKET_TOKEN`
4. Перегляньте логи в Actions tab

### 9.4. repository_dispatch не працює

**Причина:** Недостатні права токена або неправильний payload.

**Рішення:**
1. Перевірте, що токен має scope `repo`
2. Перевірте назву репозиторію в workflow
3. Перевірте формат `client-payload` (valid JSON)

### 9.5. Застосунок не запускається після встановлення

**Причина:** Неправильний `extract_dir` або шлях до exe.

**Рішення:**
```powershell
# Перевірити структуру встановлення
ls "$env:USERPROFILE\scoop\apps\browserselector\current"

# Перевірити створені shims
ls "$env:USERPROFILE\scoop\shims" | Where-Object { $_.Name -like "*browser*" }
```

---

## Додаткові ресурси

- [Scoop Wiki](https://github.com/ScoopInstaller/Scoop/wiki)
- [App Manifests](https://github.com/ScoopInstaller/Scoop/wiki/App-Manifests)
- [Autoupdate](https://github.com/ScoopInstaller/Scoop/wiki/App-Manifest-Autoupdate)
- [Creating Buckets](https://github.com/ScoopInstaller/Scoop/wiki/Buckets)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)

---

## Чеклист для релізу

- [ ] Оновлено версію в `resources/app.rc`
- [ ] Оновлено `CHANGELOG.md`
- [ ] Код скомпільовано та протестовано локально
- [ ] Зміни закомічено та запушено в `develop`
- [ ] Зміни змержено в `main`
- [ ] Створено тег релізу
- [ ] GitHub Actions успішно виконав workflow
- [ ] GitHub Release створено з архівом
- [ ] Маніфест в scoop-bucket оновлено
- [ ] Тестове встановлення через Scoop успішне
