#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdlib>

void initialize_matrix(std::vector<double>& a, int m, int n, int thread_id, int num_threads) {
    int items_per_thread = m / num_threads;
    int lb = thread_id * items_per_thread;
    int ub = (thread_id == num_threads - 1) ? m : lb + items_per_thread;

    for (int i = lb; i < ub; i++) {
        for (int j = 0; j < n; j++) {
            a[i * n + j] = i + j; 
        }
    }
}

void initialize_vector(std::vector<double>& b, int n, int thread_id, int num_threads) {
    int items_per_thread = n / num_threads;
    int lb = thread_id * items_per_thread;
    int ub = (thread_id == num_threads - 1) ? n : lb + items_per_thread;

    for (int j = lb; j < ub; j++) {
        b[j] = j; 
    }
}

void matrix_vector_product(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c, int m, int n, int thread_id, int num_threads) {
    int items_per_thread = m / num_threads;
    int lb = thread_id * items_per_thread;
    int ub = (thread_id == num_threads - 1) ? m : lb + items_per_thread;

    for (int i = lb; i < ub; i++) {
        c[i] = 0.0;
        for (int j = 0; j < n; j++) {
            c[i] += a[i * n + j] * b[j];
        }
    }
}

void run_parallel(int m, int n, int num_threads) {
    std::vector<double> a;
    a.reserve(m * n); // Резервируем память для матрицы
    std::vector<double> b;
    b.reserve(n); // Резервируем память для вектора b
    std::vector<double> c;
    c.reserve(m); // Резервируем память для результата

    std::vector<std::thread> threads;

    // Инициализация матрицы в параллельном режиме
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(initialize_matrix, std::ref(a), m, n, i, num_threads);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    threads.clear();

    // Инициализация вектора b в параллельном режиме
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(initialize_vector, std::ref(b), n, i, num_threads);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    threads.clear();

    // Умножение матрицы на вектор в параллельном режиме
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(matrix_vector_product, std::ref(a), std::ref(b), std::ref(c), m, n, i, num_threads);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

void run_sequential(int m, int n) {
    std::vector<double> a;
    a.reserve(m * n); // Резервируем память для матрицы
    std::vector<double> b;
    b.reserve(n); // Резервируем память для вектора b
    std::vector<double> c;
    c.reserve(m); // Резервируем память для результата

    // Инициализация матрицы последовательно
    for (int i = 0; i < m; i++) {
        a.push_back(0); // Заполняем нулями для корректного доступа по индексу
        for (int j = 0; j < n; j++) {
            a[i * n + j] = i + j;
        }
        c.push_back(0.0); // Заполняем нулями
    }

    // Инициализация вектора b последовательно
    for (int j = 0; j < n; j++) {
        b.push_back(j);
    }

    // Умножение матрицы на вектор последовательно
    matrix_vector_product(a, b, c, m, n, 0, 1); // Здесь один поток
}
int main() {
    const int sizes[] = { 20000, 40000 };
    const int num_threads_options[] = { 1, 2, 4, 7, 8, 16, 20, 40 };

    for (int size : sizes) {
        std::cout << "Matrix size: " << size << "x" << size << "\n";

        for (int num_threads : num_threads_options) {
            auto start = std::chrono::high_resolution_clock::now();
            run_parallel(size, size, num_threads);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_time = end - start;

            std::cout << "Time taken with " << num_threads << " threads: " << elapsed_time.count() << " seconds\n";
        }

        std::cout << "\n";
    }

    return 0;
}

