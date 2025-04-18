import math

def check_sin_results(filename):
    print(f"Проверка {filename}:")
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 2:
                print(f"Неверный формат строки: {line.strip()}")
                continue
            
            try:
                x = float(parts[0])
                expected_result = float(parts[1])
                calculated_result = math.sin(x)

                # Округление до 5 знаков после запятой
                expected_result_rounded = round(expected_result, 6)
                calculated_result_rounded = round(calculated_result, 6)

                if abs(calculated_result_rounded - expected_result_rounded) < 1e-5:  # Точность сравнения
                    print(f"Правильно: sin({x}) = {expected_result_rounded}")
                else:
                    print(f"Ошибка: sin({x}) = {calculated_result_rounded}, ожидалось {expected_result_rounded}")

            except ValueError:
                print(f"Ошибка преобразования: {line.strip()}")

def check_sqrt_results(filename):
    print(f"\nПроверка {filename}:")
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 2:
                print(f"Неверный формат строки: {line.strip()}")
                continue
            
            try:
                x = float(parts[0])
                expected_result = float(parts[1])
                calculated_result = math.sqrt(x)

                # Округление до 5 знаков после запятой
                expected_result_rounded = round(expected_result, 6)
                calculated_result_rounded = round(calculated_result, 6)

                if abs(calculated_result_rounded - expected_result_rounded) < 1e-5:  # Точность сравнения
                    print(f"Правильно: sqrt({x}) = {expected_result_rounded}")
                else:
                    print(f"Ошибка: sqrt({x}) = {calculated_result_rounded}, ожидалось {expected_result_rounded}")

            except ValueError:
                print(f"Ошибка преобразования: {line.strip()}")




def check_pow_results(filename):
    print(f"\nПроверка {filename}:")
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 3:
                print(f"Неверный формат строки: {line.strip()}")
                continue
            
            try:
                base = float(parts[0])
                exponent = float(parts[1])
                expected_result = float(parts[2])
                calculated_result = base**(exponent)

                # Округление до 6 знаков после запятой
                expected_result_rounded = round(expected_result + 0.01, 1)
                calculated_result_rounded = round(calculated_result + 0.01, 1)

                # Форматированный вывод
                if abs(calculated_result_rounded - expected_result_rounded) < 1e-6:  # Точность сравнения
                    print(f"Правильно: pow({base}, {exponent}) = {expected_result_rounded:.6f}")
                else:
                    print(f"Ошибка: pow({base}, {exponent}) = {calculated_result_rounded:.6f}, ожидалось {expected_result_rounded:.6f}")

            except ValueError:
                print(f"Ошибка преобразования: {line.strip()}")

if __name__ == "__main__":  # Исправлено условие
    check_sin_results("sin_results.txt")
    check_sqrt_results("sqrt_results.txt")
    check_pow_results("pow_results.txt")





