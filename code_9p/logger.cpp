#include <fstream>
#include "pthread.h"
#include "gallery.cpp"
#include <ctime>

/// Thread-safety логгер для записи в файл
class ThreadLogger {
public:
    explicit ThreadLogger(Gallery* gallery) {
        pthread_mutex_init(&mutex, nullptr);
        gal = gallery; // получаем указатель, чтобы потом оперировать с объектом галереи
    }

    void OpenStream(const char* fileName) {
        // при создании необходимо открыть поток на запись в файл
        name = fileName;
        str.open(fileName);
    }

    ~ThreadLogger() {
        // при окончании программы надо закрыть поток записи в файл
        str.close();
    }

    void LogEnterTheGallery(int threadId) {
        // логируем вход потока в галерею
        pthread_mutex_lock(&mutex);
        str << "thread with id " << threadId << " entered the gallery. \n";
        WriteStatistics();
        pthread_mutex_unlock(&mutex);
    }

    void LogWatchPainting(int threadId, int paintingId, int duracity) {
        // логируем просмотр потоком картины в галерее
        pthread_mutex_lock(&mutex);
        str << "thread with id " << threadId << " watching at the painting with id = " << paintingId << " for " << duracity << " seconds.\n";
        WriteStatistics();
        pthread_mutex_unlock(&mutex);
    }

    void LogGoToAnotherPainting(int threadId, int paintingId) {
        // логируем завершение просмотра и переход к другой картине
        pthread_mutex_lock(&mutex);
        str << "thread with id " << threadId << " watched at the painting with id = " << paintingId << " and goes to another\n";
        WriteStatistics();
        pthread_mutex_unlock(&mutex);
    }

    void LogCameOut(int threadId) {
        // логируем уход потока из галереи
        pthread_mutex_lock(&mutex);
        str << "thread with id " << threadId << " came out from the gallery. " << "\n";
        WriteStatistics();
        pthread_mutex_unlock(&mutex);
    }

    void LogMessage(const char* message) {
        // логируем просто сообщение
        pthread_mutex_lock(&mutex);
        str << message << "\n\n";
        pthread_mutex_unlock(&mutex);
    }

private:
    void WriteStatistics()  {
        // Пишем статистику о галереи + время (там будет большое число в тактах)
        // Мьютекс здесь НЕ нужен, так как метод вызывается только другими методами, в которых есть мьютекс.
        str <<  "[" << (std::chrono::system_clock::now().time_since_epoch()).count() << "] "<< " now in gallery: " << gal->visitorsNow << ". statistics: " << gal->paintingCounter[0] << " "
            << gal->paintingCounter[1] << " " << gal->paintingCounter[2] << " " << gal->paintingCounter[3] << " "
            << gal->paintingCounter[4] << " --> total visitors for day: " << gal->total << "\n\n";
    }

    Gallery* gal;
    std::ofstream str;
    pthread_mutex_t mutex{};
    const char *name{};
};