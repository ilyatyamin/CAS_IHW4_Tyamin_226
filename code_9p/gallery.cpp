// для того чтобы не было перекрестных инклюдов
#pragma once

/// Класс, имитирующий работу галереи
class Gallery {
public:
    int visitorsNow = 0;
    int paintingCounter[5]{};
    int total = 0;
};