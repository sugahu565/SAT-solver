import sys
import os
import subprocess

def main():
    if len(sys.argv) < 3:
        print("Ожидается: run_tests.py <путь_к_solver> <папка_с_тестами> [доп_аргументы...]")
        sys.exit(1)

    solver_path = sys.argv[1]
    tests_dir = sys.argv[2]
    extra_args = sys.argv[3:]
    
    passed = 0
    total = 0

    for filename in sorted(os.listdir(tests_dir)):
        total += 1
        cnf_path = os.path.join(tests_dir, filename)
        expected_output = "UNSAT" if filename.startswith("uuf") else "SAT"
        result = subprocess.run([solver_path, cnf_path] + extra_args, capture_output=True, text=True)
        actual_output = result.stdout.strip()
        status = "OK" if expected_output in actual_output else "FAIL"
        
        if status == "OK":
            passed += 1
        else:
            print(f"ОШИБКА: {filename} (Ожидалось {expected_output}, получено {actual_output})")

        if total % 10 == 0:
            print(f"Обраработано {total} тестов...")

    print(f"\nГотово! Пройдено {passed}/{total}")
    if passed != total:
        sys.exit(1)

if __name__ == "__main__":
    main()

    
