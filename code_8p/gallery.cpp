// для того чтобы не было перекрестных инклюдов
#pragma once
#include "pthread.h"

/// Класс, имитирующий работу галереи
class Gallery {
public:
    Gallery() {
        pthread_mutex_init(&mutexCounter, nullptr);
        pthread_mutex_init(&mutexVector, nullptr);
    }

    ~Gallery() {
        // Мьютекс надо уничтожить при уничтожении галереи
        pthread_mutex_destroy(&mutexCounter);
        pthread_mutex_destroy(&mutexVector);
    }

    void AddToVisitorsCount(int value, bool isNew) {
        // Применяем мьютекс для того, чтобы случайно не затереть число.
        pthread_mutex_lock(&mutexCounter);
        visitorsNow += value;
        if (isNew) {
            total += 1;
        }
        pthread_mutex_unlock(&mutexCounter);
    }

    void AddHumanToPainting(int element, int value) {
        // Применяем мьютекс для того, чтобы не затереть число в массиве при одновременном обращении нескольким потокам
        pthread_mutex_lock(&mutexVector);
        paintingCounter[element] += value;
        pthread_mutex_unlock(&mutexVector);
    }

    int visitorsNow = 0;
    int paintingCounter[5]{};
    int total = 0;
private:
    pthread_mutex_t mutexCounter;
    pthread_mutex_t mutexVector;
};