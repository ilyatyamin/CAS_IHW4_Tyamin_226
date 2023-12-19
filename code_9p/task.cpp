#include "pthread.h"
#include <vector>
#include <random>
#include <unistd.h>
#include "logger.cpp"
#include "gallery.cpp"

// Генератор случайных чисел на C++
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<int> timeGen(1, 10);

// Набор из мьютекса и условной переменной для входа в галерею
pthread_cond_t entranceCond;
pthread_mutex_t entranceMutex;

pthread_cond_t condition[5];
pthread_mutex_t mutexPainting[5];

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
    int num = *(int *) args;

    // Используем conditional variable вместе с мьютексом
    pthread_mutex_lock(&entranceMutex);
    while (gallery.visitorsNow >= 50) {
        // Ждем пока количество людей в галерее станет < 50
        pthread_cond_wait(&entranceCond, &entranceMutex);
    }
    gallery.visitorsNow += 1;
    gallery.total += 1;
    pthread_mutex_unlock(&entranceMutex);

    logger.LogEnterTheGallery(num);

    // Перемешиваем и получаем индивидуальный порядок обхода картин
    std::vector<int> paintings{0, 1, 2, 3, 4};
    std::shuffle(paintings.begin(), paintings.end(), std::random_device());

    for (const int &element: paintings) {
        PrintToConsole();

        // Используем conditional variable вместе с мьютексом
        pthread_mutex_lock(&mutexPainting[element]);
        while (gallery.paintingCounter[element] >= 5) {
            // Ждем пока количество потоков, смотрящих на картину станет < 5.
            pthread_cond_wait(&condition[element], &mutexPainting[element]);
        }
        gallery.paintingCounter[element] += 1;
        pthread_mutex_unlock(&mutexPainting[element]);

        int timeDuracity = timeGen(gen);
        logger.LogWatchPainting(num, element, timeDuracity);
        sleep(timeDuracity); // сколько секунд смотрит на картину

        pthread_mutex_lock(&mutexPainting[element]);
        gallery.paintingCounter[element] -= 1;
        // Вышли от картины -> освободилось место -> оповещаем первый в очереди поток
        pthread_cond_signal(&condition[element]);
        pthread_mutex_unlock(&mutexPainting[element]);

        logger.LogGoToAnotherPainting(num, element);
    }

    // Блокируем мьютексом, чтобы никто не поменял переменную
    pthread_mutex_lock(&entranceMutex);
    gallery.visitorsNow -= 1;
    // Вышли из галереи -> освободилось место -> оповещаем первый в очереди поток
    pthread_cond_signal(&entranceCond);
    pthread_mutex_unlock(&entranceMutex);

    logger.LogCameOut(num);

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
        pthread_create(&visitors[i], nullptr, VisitorsActions, (void *) (&numbers[i]));
    }

    // Закрываем потоки
    for (int i = 0; i < numOfThreads; ++i) {
        std::string message = "Thread with id = " + std::to_string(i) + " finished working.";
        pthread_join(visitors[i], nullptr);
    }

    return nullptr;
}

void SolveTheTask(size_t numberOfThreads, const char *nameFile) {
    // Открываем логер на чтение файла с именем nameFile
    logger.OpenStream(nameFile);

    pthread_t watchman;

    // Инициализируем все, что нужно
    pthread_cond_init(&entranceCond, nullptr);
    pthread_mutex_init(&entranceMutex, nullptr);
    for (int i = 0; i < 5; ++i) {
        pthread_cond_init(&condition[i], nullptr);
        pthread_mutex_init(&mutexPainting[i], nullptr);
    }

    pthread_create(&watchman, nullptr, WatchManAction, &numberOfThreads);
    pthread_join(watchman, nullptr);

    pthread_cond_destroy(&entranceCond);
    pthread_mutex_destroy(&entranceMutex);
    for (int i = 0; i < 5; ++i) {
        pthread_cond_destroy(&condition[i]);
        pthread_mutex_destroy(&mutexPainting[i]);
    }
}