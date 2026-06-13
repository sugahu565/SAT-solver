#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
SOLVER="$ROOT_DIR/build/bin/solver"
RESULTS_DIR="$ROOT_DIR/results"
CPU_LIMITER_ACTIVE=0
GOVERNOR_CHANGED=0
INTEL_TURBO_CHANGED=0
AMD_BOOST_CHANGED=0
ORIGINAL_GOV=""
INTEL_NO_TURBO_FILE="/sys/devices/system/cpu/intel_pstate/no_turbo"
AMD_BOOST_FILE="/sys/devices/system/cpu/cpufreq/boost"
CPU_GOVERNOR_FILES=()

run_privileged_write() {
    local value="$1"
    local path="$2"

    if [ ! -e "$path" ]; then
        return 1
    fi

    if [ -w "$path" ]; then
        printf '%s\n' "$value" > "$path"
        return 0
    fi

    if [ "${EUID:-$(id -u)}" -eq 0 ]; then
        printf '%s\n' "$value" > "$path"
        return 0
    fi

    if command -v sudo > /dev/null 2>&1; then
        printf '%s\n' "$value" | sudo tee "$path" > /dev/null
        return $?
    fi

    return 1
}

warn_cpu_limiter() {
    echo "Предупреждение: ограничение CPU недоступно ($1). Продолжаю замеры без фиксации частоты."
}

configure_cpu_limiter() {
    local cpu_dir

    echo "=== Настройка CPU (best effort) ==="

    for cpu_dir in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
        if [ -e "$cpu_dir" ]; then
            CPU_GOVERNOR_FILES+=("$cpu_dir")
        fi
    done

    if [ "${#CPU_GOVERNOR_FILES[@]}" -eq 0 ]; then
        warn_cpu_limiter "не найдены governor-файлы cpufreq"
        return 0
    fi

    if ! ORIGINAL_GOV=$(cat "${CPU_GOVERNOR_FILES[0]}" 2> /dev/null); then
        warn_cpu_limiter "не удалось прочитать текущий governor"
        return 0
    fi

    if [ -f "$INTEL_NO_TURBO_FILE" ]; then
        if run_privileged_write "1" "$INTEL_NO_TURBO_FILE"; then
            INTEL_TURBO_CHANGED=1
            CPU_LIMITER_ACTIVE=1
        else
            echo "Предупреждение: не удалось отключить Intel Turbo Boost."
        fi
    fi

    if [ -f "$AMD_BOOST_FILE" ]; then
        if run_privileged_write "0" "$AMD_BOOST_FILE"; then
            AMD_BOOST_CHANGED=1
            CPU_LIMITER_ACTIVE=1
        else
            echo "Предупреждение: не удалось отключить AMD boost."
        fi
    fi

    for cpu_dir in "${CPU_GOVERNOR_FILES[@]}"; do
        if ! run_privileged_write "powersave" "$cpu_dir"; then
            warn_cpu_limiter "не удалось переключить governor в powersave"
            return 0
        fi
        GOVERNOR_CHANGED=1
        CPU_LIMITER_ACTIVE=1
    done

    echo "Тактовая частота CPU зафиксирована настолько, насколько это поддерживает текущая система."
}

restore_cpu() {
    local cpu_dir

    if [ "$CPU_LIMITER_ACTIVE" -eq 0 ]; then
        return 0
    fi

    echo -e "\n=== Восстановление нормальной работы CPU ==="

    if [ "$INTEL_TURBO_CHANGED" -eq 1 ] && ! run_privileged_write "0" "$INTEL_NO_TURBO_FILE"; then
        echo "Предупреждение: не удалось вернуть Intel Turbo Boost."
    fi

    if [ "$AMD_BOOST_CHANGED" -eq 1 ] && ! run_privileged_write "1" "$AMD_BOOST_FILE"; then
        echo "Предупреждение: не удалось вернуть AMD boost."
    fi

    if [ "$GOVERNOR_CHANGED" -eq 1 ]; then
        for cpu_dir in "${CPU_GOVERNOR_FILES[@]}"; do
            if ! run_privileged_write "$ORIGINAL_GOV" "$cpu_dir"; then
                echo "Предупреждение: не удалось восстановить governor для $cpu_dir."
            fi
        done
    fi

    echo "Настройки CPU восстановлены."
}

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

trap restore_cpu EXIT
configure_cpu_limiter

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
