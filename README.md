# CFT

Утилита обработки данных `cft_proc_util` выполняет слияние и сортировку файлов.
Утилита тестирования `cft_test_util` выполняет проверку утилиты обработки данных.

## Настройка окружения (Ubuntu)

```
sudo apt install meson
meson setup build
```

## Компиляция

```
ninja -C build
```

## Запуск

```
./build/cft_test_util build/cft_proc_util in1 in2 out
```
