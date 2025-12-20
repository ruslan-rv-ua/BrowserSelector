# Про встановлення браузера за замовчуванням в Windows 10/11

## Правда про "автоматичне" встановлення

### Що роблять Firefox та Chrome?

**Коротка відповідь:** Вони ТАКОЖ відкривають Windows Settings і просять користувача вибрати вручну.

### Детальніше:

#### 1. **Firefox**
```
- Показує діалог "Set Firefox as default browser"
- При натисканні відкриває Windows Settings (ms-settings:defaultapps)
- Користувач повинен вручну вибрати Firefox зі списку
- Це саме те, що робить BrowserSelector
```

#### 2. **Chrome**
```
- Показує банер "Make Chrome your default browser"  
- При натисканні відкриває Windows Settings
- Користувач вибирає Chrome вручну
- Аналогічно до BrowserSelector
```

#### 3. **Edge** (вбудований Windows)
```
- Єдиний браузер з "особливими правами" від Microsoft
- Може встановлювати себе програмно (тому що це системний компонент)
- Інші браузери не мають таких привілеїв
```

## Чому не можна встановити автоматично?

### Windows 10/11 Security Model

Microsoft заблокував програмне встановлення браузера за замовчуванням починаючи з Windows 10:

**Причини:**
1. **Захист від malware** - шкідливі програми не можуть змінити браузер без відома користувача
2. **Антимонопольні вимоги** - всі браузери повинні конкурувати на рівних умовах
3. **User Choice Protection** - тільки користувач може змінити default apps

### Що сталося з UserChoice хешем?

**Попередні спроби обходу:**
- Firefox та інші пробували використовувати UserChoice hash
- Microsoft постійно змінює алгоритм
- З Windows 10 версії 1803+ це практично неможливо
- Навіть якщо hash правильний, Windows може ігнорувати його

## Методи які використовуються

### 1. **Shell32.dll ordinal 144** (використовується в BrowserSelector)
```c
LaunchAdvancedAssociationUIProc pLaunchUI = 
    GetProcAddress(hShell32, MAKEINTRESOURCEA(144));
pLaunchUI(L"http");
```
**Результат:** Відкриває спрощений діалог вибору (на деяких версіях Windows)

### 2. **ms-settings: URI** (fallback)
```
ms-settings:defaultapps
```
**Результат:** Відкриває повні налаштування Windows

### 3. **IApplicationAssociationRegistration** (застарілий)
```cpp
// Працював до Windows 8
// Заблокований на Windows 10+
SetAppAsDefault(appName, protocol, AT_URLPROTOCOL);
```
**Результат:** ERROR_ACCESS_DENIED на Windows 10+

## Висновок

### Що робить BrowserSelector:

✅ Реєструється як валідний browser handler  
✅ З'являється в списку доступних браузерів  
✅ Відкриває Windows Settings для вибору  
✅ Перевіряє чи користувач встановив його за замовчуванням  

### Що НЕ можна зробити без прав адміністратора або цифрового підпису Microsoft:

❌ Автоматично встановити себе браузером за замовчуванням  
❌ Обійти Windows Security Model  
❌ Змінити UserChoice без підтвердження користувача  

## Це нормально!

**Всі браузери** (крім Edge) працюють так само:
- Firefox ✓ відкриває Settings
- Chrome ✓ відкриває Settings  
- Opera ✓ відкриває Settings
- Brave ✓ відкриває Settings

**BrowserSelector робить все правильно** згідно з Windows вимогами безпеки.

## Джерела

- [Microsoft Docs: Default Apps](https://learn.microsoft.com/en-us/windows/client-management/default-apps)
- [Firefox Source: SetDefaultBrowser](https://searchfox.org/mozilla-central/source/toolkit/xre/nsWindowsWMain.cpp)
- [Chromium Source: shell_integration_win](https://source.chromium.org/chromium/chromium/src/+/main:chrome/browser/shell_integration_win.cc)

## Тестування

Спробуйте самі:
1. Встановіть чистий Firefox на Windows 10/11
2. Натисніть "Set as default browser"
3. Firefox відкриє Settings
4. Вам потрібно вибрати вручну

**Це саме те, що робить BrowserSelector!**
