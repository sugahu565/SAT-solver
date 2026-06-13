#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
SOLVER="$ROOT_DIR/build/bin/solver"
RESULTS_DIR="$ROOT_DIR/results"

if [ "$#" -ne 1 ]; then
    echo "Ошибка: Укажите путь к папке с датасетом."
    echo "Использование: ./measure_all.sh <путь_к_папке>"
    exit 1
fi

DIR=$(realpath "$1")
DIR_NAME=$(basename "$DIR")

if [ ! -x "$SOLVER" ]; then
    echo "Ошибка: Исполняемый файл $SOLVER не найден. Сначала выполните make build."
    exit 1
fi

if ! command -v hyperfine > /dev/null; then
    echo "Ошибка: Для замеров требуется hyperfine."
    exit 1
fi

CNF_FILES=("$DIR"/*.cnf)
if [ ! -e "${CNF_FILES[0]}" ]; then
    echo "В папке $DIR не найдено файлов с расширением .cnf"
    exit 1
fi

echo "=== Настройка CPU (может потребоваться пароль sudo) ==="
ORIGINAL_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor) #текущий режим

# Отключаем Turbo Boost (для Intel)
if [ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
    echo "1" | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo > /dev/null
fi
# Отключаем Turbo Boost (для AMD, на всякий случай)
if [ -f /sys/devices/system/cpu/cpufreq/boost ]; then
    echo "0" | sudo tee /sys/devices/system/cpu/cpufreq/boost > /dev/null
fi
# Переводим все ядра в режим минимальной базовой частоты
echo "powersave" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null

echo "Тактовая частота успешно зафиксирована на минимуме."

# Функция для автоматического восстановления частот
restore_cpu() {
    echo -e "\n=== Восстановление нормальной работы CPU ==="
    if [ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
        echo "0" | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo > /dev/null
    fi
    if [ -f /sys/devices/system/cpu/cpufreq/boost ]; then
        echo "1" | sudo tee /sys/devices/system/cpu/cpufreq/boost > /dev/null
    fi
    # Возвращаем стандартный планировщик Fedora
    # echo "balance_performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null
    echo "$ORIGINAL_GOV" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null
    echo "Настройки восстановлены."
}

# Если скрипт завершается (штатно или по Ctrl+C), вызываем restore_cpu
trap restore_cpu EXIT
# --------------------------------------

NAIVE_HEURISTIC=$("$SOLVER" --print-heuristic)
JWH_HEURISTIC=$("$SOLVER" -jwh --print-heuristic)
if [ "$NAIVE_HEURISTIC" != "naive" ] || [ "$JWH_HEURISTIC" != "jwh" ]; then
    echo "Ошибка: исполняемый файл не выбрал ожидаемые эвристики."
    echo "naive: $NAIVE_HEURISTIC, jwh: $JWH_HEURISTIC"
    exit 1
fi
echo "Проверка эвристик: naive и jwh выбраны корректно."

mkdir -p "$RESULTS_DIR"
echo "=== Запуск бенчмарка: $DIR_NAME (30 прогонов всей папки) ==="

hyperfine --warmup 1 --runs 30 \
    --export-markdown "$RESULTS_DIR/bench_results_${DIR_NAME}.md" \
    --export-json "$RESULTS_DIR/bench_results_${DIR_NAME}.json" \
    "\"$SOLVER\" \"$DIR\"/*.cnf > /dev/null" \
    "\"$SOLVER\" -jwh \"$DIR\"/*.cnf > /dev/null"

echo "=== Замеры завершены! ==="
