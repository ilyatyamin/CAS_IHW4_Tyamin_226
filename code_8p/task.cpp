#include "pthread.h"
#include <vector>
#include <random>
#include <unistd.h>
#include <semaphore>
#include "logger.cpp"
#include "gallery.cpp"

// Генератор случайных чисел на C++
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<int> timeGen(1, 10);

// Набор семафоров для работы программы
// Семафор для вахтера: не впускает больше 50 потоков
std::counting_semaphore<50> semaphoreForEntering(50);

// Вектор из семафор для удобной индексации (опишу в отчете)
std::vector<void *> semForPainting;
// Мьютекс для блокирования записи в вектор (чтобы только один поток мог обращатсья)
pthread_mutex_t blockingVector;

// Объекты галереи и логгера (глобальные, чтобы все могли видеть)
Gallery gallery;
ThreadLogger logger(&gallery);

void PrintToConsole() {
    printf("Now is %d visitors. 1 pic: %d, 2 pic: %d, 3 pic: %d, 4 pic: %d, 5 pic: %d \n", gallery.visitorsNow,
           gallery.paintingCounter[0], gallery.paintingCounter[1], gallery.paintingCounter[2],
           gallery.paintingCounter[3], gallery.paintingCounter[4]);
}

/// Стартовая функция потока посетителя
void *VisitorsActions(void *args) {
    // Блокируем семафор входа. Если он станет -1 (изначально 50), то поток заблокируется
    // ВНИМАНИЕ! здесь sem.acquire() аналогичен sem_wait() в терминах semaphore.h (см отчет)
    semaphoreForEntering.acquire();
    int num = *(int*)args;

    pthread_mutex_lock(&blockingVector);
    gallery.AddToVisitorsCount(1, true);
    logger.LogEnterTheGallery(num);
    pthread_mutex_unlock(&blockingVector);

    // Перемешиваем и получаем индивидуальный порядок обхода картин
    std::vector<int> paintings{0, 1, 2, 3, 4};
    std::shuffle(paintings.begin(), paintings.end(), std::random_device());

    for (const int &element: paintings) {
        // По индексу элемента получаем отвечающий за него семафор.
        // Удобнее сделать массив из семафор и обращаться к ним по индексам, чем делать 5 отдельных объектов
        auto *x = static_cast<std::counting_semaphore<5> *>(semForPainting[element]);
        PrintToConsole();

        // Если на картину смотрит более 5 человек, то ждем
        // Внимание! здесь sem.acquire() аналогичен sem_wait() в терминах semaphore.h (см отчет)
        x->acquire();

        gallery.AddHumanToPainting(element, 1);
        int timeDuracity = timeGen(gen);
        logger.LogWatchPainting(num, element, timeDuracity);
        sleep(timeDuracity); // сколько секунд смотрит на картину
        gallery.AddHumanToPainting(element, -1); // уходит от картины

        logger.LogGoToAnotherPainting(num, element);

        // Освобождаем место для картины
        // Внимание! здесь sem.release() аналогичен sem_post() в терминах semaphore.h (см отчет)
        x->release();
    }

    gallery.AddToVisitorsCount(-1, false);
    logger.LogCameOut(num);

    // Уходим из галереи
    // Внимание! здесь sem.release() аналогичен sem_post() в терминах semaphore.h (см отчет)
    semaphoreForEntering.release();

    return nullptr;
}

/// Стартовая функция вахтера
void *WatchManAction(void *args) {
    // принимает количество посетителей как аргумент
    int numOfThreads = *(int *) args;
    pthread_t visitors[numOfThreads];

    // Запускает посетителей. Если в галерее >= 50 человек, то срабатывает семафор
    std::vector<int> numbers;
    for (int i = 0; i < numOfThreads; ++i) {
        numbers.emplace_back(i);
    }

    for (int i = 0; i < numOfThreads; ++i) {
        std::string message = "Thread with id = " + std::to_string(i) + " started working.";
        pthread_create(&visitors[i], nullptr, VisitorsActions, (void*)(&numbers[i]));
    }

    // Закрываем потоки
    for (int i = 0; i < numOfThreads; ++i) {
        std::string message = "Thread with id = " + std::to_string(i) + " finished working.";
        pthread_join(visitors[i], nullptr);
    }

    return nullptr;
}

void SolveTheTask(size_t numberOfThreads, const char* nameFile) {
    logger.OpenStream(nameFile);

    pthread_t watchman;

    pthread_mutex_init(&blockingVector, nullptr);

    // Семафор для вахтера: не подпускает к каждой из картин более 5 человек.
    std::counting_semaphore<5> semForPainting1(5);
    std::counting_semaphore<5> semForPainting2(5);
    std::counting_semaphore<5> semForPainting3(5);
    std::counting_semaphore<5> semForPainting4(5);
    std::counting_semaphore<5> semForPainting5(5);

    semForPainting = {&semForPainting1, &semForPainting2, &semForPainting3, &semForPainting4,
                      &semForPainting5};

    pthread_create(&watchman, nullptr, WatchManAction, &numberOfThreads);
    pthread_join(watchman, nullptr);

    pthread_mutex_destroy(&blockingVector);
}