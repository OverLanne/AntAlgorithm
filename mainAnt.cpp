/********************************************************************************************
*                      Летняя практика     С  +  +                                          *
*-------------------------------------------------------------------------------------------*
*                                                                                           *
* Project Name  : \Курсы_С++\AntColony                                                      *
* File Name     : ЛетняяПрактикаМуравьиныйАлгоритм.cpp                                      *
* Programmer(s) : Шувалов Е.М.						                                                  *
* Modifyed By   :                                                                           *
* Created       : 15/07/22                                                                  *
* Last Revision : 23/07/22                                                                  *
* Comment(s)    : Муравьиный аллггоритм поиска оптимального маршрута с негативным ферамоном *
*                                                                                           *
********************************************************************************************/

#include <iostream>
#include <fstream>

using namespace std;
ofstream f("Proj.txt");


const int matrSize = 25;            // Количество всех городов

#define CC_MIN  3                   // Минимальное  количество всех городов
#define CC_MAX  5                   // Максимальное количество всех городов

#define ALPHA   1                   // Важность веса ребра
#define BETTA   4                   // Важность количества ферромонов | коэффициент эвристики

#define T_MAX   20                  // Время жизни колонии | общее количество итераций
#define Q       1                   // Константа
#define RHO     0.5                 // Коэффициент испарения феромона
#define VNP     0.1                 // Значение негативного ферамона

//  КЛАСС ГОРОДА-ВЕРШИНЫ В "ГРАФЕ"
class City {
public:
    City**      items;              // Массив элементов, к которым ведут дороги
    int         itemsCount,         // Число исходящих ветвей
                readability,        // Доступность к чтению
                itemsIndex;         // Индекс каждого города
    double      desireToGo;         // Желание перейти в город
    const char* name;               // "Имя" каждого города
};

//  КЛАСС МУРАВЬЯ, КОТОРЫЙ ПЕРЕДВИГАЕТСЯ
class AntUnit {
public:
    City*  curItem;                 // Ссылка на текущий элемент
    City** finalPath;               // Массив посещенных вершин
    double distancePath;            // Длина пути
    int    antCC;                   // Количество вершин
};

//  ОБЪЯВЛЕНИЕ ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
City** arrayAllItems = new City * [matrSize];
double matrixOfDistances[matrSize][matrSize];
double startValPheromone = 0.2;     // Входное значение ферамона
int    M                 = 25;       // Количество муравьев в колонии | после какого количества итераций будем обновлять ферамон


//  ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ПО НАХОЖДЕНИЮ ПОВТОНОГО ВХОЖДЕНИЯ: true - НАШЛА, false - НЕ НАШЛА ПОВТОРКУ
bool definitionOfReEntry(int to, AntUnit* itemAnt)
{
    int  from = itemAnt->curItem->itemsIndex;                                       // Индекс города, откуда мы идем
    bool flag = false;                                                              // Возвратная переменная
    for (int i = 0; i < itemAnt->antCC || from == to; i++) {
        if (arrayAllItems[to]->itemsIndex == itemAnt->finalPath[i]->itemsIndex || from == to) {
            flag = true;                                                            // Если Город, куда мы идем - уже есть в массиве посещенных городов (finalPath) ИЛИ
            break;                                                                  // Город откуда мы идем == Городу куда мы идем - Прервать, повторное значение (flag) = true
        }
    }
    return flag;
}

//  ФУНКЦИЯ ПО НАХОЖДЕНИЮ ВЕРОЯТНОСТИ ПЕРЕХОДА МУРАВЬЯ В ГОРОД
bool probability(AntUnit* itemAnt, double** Pheromons, double** NEGPheromons, double** Distance, int CountCity)
{
    double sum = 0.0,                                                               // Временная переменная Суммы
           a   = 0.0,
           b   = 0.0;
    City** outGoCity = itemAnt->curItem->items;                                     // Те города, с которыми у нас есть дороги
    int    from = itemAnt->curItem->itemsIndex;                                     // Индекс города, откуда мы идем
    int    itemsCount = itemAnt->curItem->itemsCount;                               // Количество городов, куда мы можем пойти

    for (int i = 0; i < itemsCount; i++)
    {
        if (definitionOfReEntry(outGoCity[i]->itemsIndex, itemAnt)) continue;       // Если проверяемый город (i) уже есть в нашем пути - игнорировать этот город
        sum += round(pow((Pheromons[from][outGoCity[i]->itemsIndex] - NEGPheromons[from][outGoCity[i]->itemsIndex]), ALPHA) * pow(Distance[from][outGoCity[i]->itemsIndex], BETTA) * 100000) / 100000;
    }                                                                               // Здесь ^ считаем сумму
    if (sum == 0) return 1;                                                         // ERR : если сумма = 0 - значит мы в тупике

    for (int to = 0; to < itemsCount; to++)                                         // Подбераем, поочереди, город, в который мы идем
    {
        if (definitionOfReEntry(outGoCity[to]->itemsIndex, itemAnt)) continue;      // Если проверяемый город (to) уже есть в нашем пути - игнорировать этот город
        
        a = pow((Pheromons[from][outGoCity[to]->itemsIndex] - NEGPheromons[from][outGoCity[to]->itemsIndex]), ALPHA);
        b = pow(Distance[from][outGoCity[to]->itemsIndex], BETTA);
        if (a * b < 0) a = 0;
        
        arrayAllItems[outGoCity[to]->itemsIndex]->desireToGo = round(a * b / sum * 1000) / 1000;
    }                                                                               // Здесь записываем для всех оставшихся городов - их вероятность перехода (окр до 3 знаков)
    return 0;
}

//  АЛГОРИТМ МУРАВЬИНОЙ КОЛОЛНИИ
AntUnit* AntColonyOptimization(int start, int finish, int CountCity, int* matrCountCounteraction, int matrCC)
{
    //  ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ
    arrayAllItems[start]->readability = 1;              // Устанавливаем признак того, что стартовый город уже посещен
    bool   ERR       = false;                           // Переменная тупика (если мы зашли в тупик)
    City** itemsPath = new City * [CountCity];          // Создаем новый массив с посещенными Городами
    itemsPath[0]     = arrayAllItems[start];            //      добавляем стартовый город в этот путь

    AntUnit* newAnt      = new AntUnit;                 // Происходит инициализация начальными значениями Массива Всех Муравьев
    newAnt->antCC        = 1;                           //      количество элементов в пути
    newAnt->distancePath = 0.0;                         //      общаяя дистанция
    newAnt->curItem      = arrayAllItems[start];        //      текущий элемент - стартовый
    newAnt->finalPath    = itemsPath;                   //      запоминание массива с посещенными городами
    AntUnit** Ants       = new AntUnit * [M];           // Добавление каждого newAnt в общий массив
    for (int i = 0; i < M; i++) Ants[i] = newAnt;

    City** winPath = new City * [CountCity];            // Создаем новый массив с посещенными Городам
    winPath[0] = arrayAllItems[start];                  //      добавляем стартовый город в этот путь
    AntUnit* winnerAnt   = new AntUnit;                 // Происходит инициализация начальными значениями Победного Муравья (он же короткий путь)
    winnerAnt->antCC     = 1;                           //      количество элементов в пути
    winnerAnt->distancePath = 0.0;                      //      общаяя дистанция
    winnerAnt->curItem   = arrayAllItems[start];        //      текущий элемент - стартовый
    winnerAnt->finalPath = winPath;                     //      запоминание массива с посещенными городами

    //  ФОРМИРОВАНИЕ МАТРИЦЫ БЛИЗОСТИ И МАТРИЦЫ ФЕРАМОНОВ
    double** matrDistance  = new double* [matrSize];
    for (int i = 0; i < matrSize; i++) {
        matrDistance[i] = new double [matrSize];
    }
    double** matrPheromons = new double* [matrSize];
    for (int i = 0; i < matrSize; i++) {
        matrPheromons[i] = new double[matrSize];
    }
    double** matrNEGPheromons = new double* [matrSize];
    for (int i = 0; i < matrSize; i++) {
        matrNEGPheromons[i] = new double[matrSize];
    }
    for (int i = 0; i < CountCity; i++)
    {
        for (int j = i; j < CountCity; j++)
        { // Если элемент в нашей исходной матрицы - не 0, то считаем по формуле (округляя до 3 знаков после ,), иначе  = 0
            if (matrixOfDistances[i][j] != 0) {
                matrDistance[i][j] = matrDistance[j][i]     = round((Q / matrixOfDistances[i][j]) * 1000) / 1000;
            }
            else {
                matrDistance[i][j] = matrDistance[j][i]     = 0.0;
            }
            matrPheromons[i][j]    = matrPheromons[j][i]    = startValPheromone;    // С позитивным ферамоном = стартовону значению
            matrNEGPheromons[i][j] = matrNEGPheromons[j][i] = -VNP;                 // С негативным ферамоном // чтобы сумма вероятностей = 1
        }
    }
    // С негативным ферамоном
    for (int i = 0; i < matrCC; i++)
    {
        for (int j = i; j < CountCity; j++)
        {
            if (matrCountCounteraction[i] - 1) {
                matrNEGPheromons[matrCountCounteraction[i] - 1][j] = matrNEGPheromons[j][matrCountCounteraction[i] - 1] = VNP;
            } // matr[i] - 1, поскольку в массиве я храню НЕ индексы, а НОМЕРА
        }
    }


    cout << "\n\t\tНачальное Значение ПОЗИТИВНОГО Ферамона :" << endl;
    for (int i = 0; i < CountCity; i++) {
        cout << endl;
        for (int j = 0; j < CountCity; j++) {
            cout << "  " << matrPheromons[i][j];
        }
    }
    cout << "\n\n";
    cout << "\n\t\tНачальное Значение НЕГАТИВНОГО Ферамона :" << endl;
    for (int i = 0; i < CountCity; i++) {
        cout << endl;
        for (int j = 0; j < CountCity; j++) {
            cout << "  " << matrNEGPheromons[i][j];
        }
    }
    cout << "\n";


    int    iter = 1;                            // Индекс в Финальном Пути (1 - т.к в 0 элементе находится наша начальная вершина)
    int    checkingForUsage = 0;                // Проверка на использование
    int    jMax = -1;                           // Индекс вершины В КОТОРУЮ мы переходим
    double pMax = 0.0,                          // Вероятность перехода в j вершину
        bufStep = 0.0,                          // Временное хранение вероятности, для определения в какую вершинну можно перейти
        randomTransition = 0.0;                 // Случайное значение от 0 до 1
    bool trig = false;                          // Флаг, для определения перехода в город (если перешли - прервать цикл)

    for (int t = 0; t < T_MAX; t++)
    { // Цикл по муравьям
        for (int k = 0; k < M; k++)
        { // Поиск маршрута для k-го муравья
            iter             = 1;               // Обнуление переменных на новой итерации
            checkingForUsage = 0;
            randomTransition = 0.0;

            while (Ants[k]->curItem->itemsIndex != finish) {
                trig = false;
                jMax = -1;          // Обнуление переменных на новой итерации
                pMax = 0.0,
                bufStep = 0.0,
                randomTransition = (double)rand() / (double)RAND_MAX * (1.0 - 0.0) + 0.0; // Генерация случайного числа от 0 до 1

                // Производится подсчет вероятностей перехода из текущего города во все возможные
                if (probability(Ants[k], matrPheromons, matrNEGPheromons, matrDistance, CountCity) || iter > CountCity) {
                    ERR = true;                 // Если мы уперлись в тупик, то ERR = true
                    break;
                }
                // Поочереди проверяются все вершины | проверка вероятности перехода в вершину j
                for (int j = 0; j < CountCity; j++)
                {
                    if (arrayAllItems[j] != Ants[k]->curItem && !arrayAllItems[j]->readability) {
                        // Если текущий элемент и проверяемый - не один и тот же && у проверяемого отсутствуют пометка посещенного - то
                        for (int to = 0; to < Ants[k]->curItem->itemsCount; to++) {
                            // Производится проверка на наличие дорог между Текущим городом и Проверяемым городом j
                            if (Ants[k]->curItem->items[to] == arrayAllItems[j] && arrayAllItems[j]->desireToGo > 0) {
                                bufStep += arrayAllItems[j]->desireToGo;            // Если да - увеличиваем значение вероятности
                                pMax = arrayAllItems[j]->desireToGo;    // Запоминаем вероятность
                                jMax = j;                               // Запоминаем индекс города
                                if (bufStep > randomTransition || (jMax == -1 && j + 1 == CountCity) || checkingForUsage == Ants[k]->curItem->itemsCount - 1) {
                                    // если мы попали в зону этой вероятнасти || проверяемый город - последний || мы проверили все исходящие города
                                    trig = true;                            // Флаг выхода из цикла
                                    break;
                                }
                            }
                            checkingForUsage++;                             // Увеличиваем индекс городов, с которыми у нас есть дороги (если Проверяемый город не подошел к Городу, с которым есть связь)
                        }
                        if (trig) break;                                    // Флаг выхода из цикла
                    }
                    checkingForUsage = 0;                                   // Обнуление индекса городов, с которыми у нас есть дороги
                    trig = false;                                           // Обнуление Флага выхода из цикла
                }
                if (!trig) {
                    pMax = 0;                                           // Запоминаем вероятность
                    jMax = 0;                                           // Запоминаем индекс города
                }

                Ants[k]->distancePath   += matrixOfDistances[Ants[k]->curItem->itemsIndex][jMax];   // Увеличение дистанции
                arrayAllItems[jMax]->readability = 1;                                               // Заполнение "посещенности"
                Ants[k]->finalPath[iter] = Ants[k]->curItem = arrayAllItems[jMax];                  // Обновление текущего элемента (На котором мы сейчас находимся)
                iter++;                     // Увеличение общего числа дорог
                Ants[k]->antCC = iter;      // Заполнение общего числа дорог
            }
            if (!ERR) {
                // Вывод промежуточных значений
                cout << endl << " |--------------------------------------------| " << endl;                                         // На консоль
                cout << endl << " Номер Итерации < t > " << t << " \tНомер Муравья < k > " << k << endl << "\t|текущий путь|\n\t";  // На консоль
                f << endl << " |--------------------------------------------| " << endl;                                            // В файл
                f << endl << " Номер Итерации < t > " << t << " \tНомер Муравья < k > " << k << endl << "\t|текущий путь|\n\t";     // В файл
                for (int tot = 0; tot < iter; tot++) {
                    cout << Ants[k]->finalPath[tot]->name;      // На консоль
                    f << Ants[k]->finalPath[tot]->name;         // В файл
                    if (tot != iter - 1) {
                        cout << "  ->  ";
                        f << "  ->  ";
                    }
                }
                cout << "\n |длина текущего пути| " << Ants[k]->distancePath << endl;       // На консоль
                cout << endl << " |--------------------------------------------| " << endl; // На консоль
                f << "\n |длина текущего пути| " << Ants[k]->distancePath << endl;          // В файл
                f << endl << " |--------------------------------------------| " << endl;    // В файл

                // Оставляем феромон на пути муравья
                for (int i = 0; i < Ants[k]->antCC; i++) {
                    int from = Ants[k]->finalPath[i % Ants[k]->antCC]->itemsIndex;                  // Получение индекса города ОТКУДА мы идем
                    int to = Ants[k]->finalPath[(i + 1) % Ants[k]->antCC]->itemsIndex;              // Получение индекса города КУДА мы идем
                    matrPheromons[from][to] += round((Q / Ants[k]->distancePath) * 1000) / 1000;    // Увеличение Позитивного ферамона на пути
                    matrPheromons[to][from] = matrPheromons[from][to];                              // Зеркальное увеличение Позитивного ферамона на пути

                    matrNEGPheromons[from][to] += VNP;                                              // Увеличение Негативного ферамона на пути
                    matrNEGPheromons[to][from] = matrNEGPheromons[from][to];                        // Зеркальное увеличение Негативного ферамона на пути
                }
                for (int i = 0; i < matrCC; i++)
                {
                    for (int j = i; j < CountCity; j++)
                    {
                        if (matrNEGPheromons[i][j] < 0) {
                            matrNEGPheromons[i][j] -= VNP;
                            matrNEGPheromons[j][i] = matrNEGPheromons[i][j];
                        }
                    }
                }

                // Проверка на лучшее решение
                if (Ants[k]->distancePath < winnerAnt->distancePath || winnerAnt->distancePath <= 0) {
                    // Если ТЕКУЩЕЕ значение пути меньше ЛУЧШЕГО - обновление ЛУЧШЕГО значения
                    winnerAnt->antCC = Ants[k]->antCC;
                    winnerAnt->distancePath = Ants[k]->distancePath;
                    cout << " |производится замена| " << winnerAnt->antCC << endl;          // На консоль
                    cout << " |производится замена| " << winnerAnt->distancePath << endl;   // На консоль
                    f << " |производится замена| " << winnerAnt->antCC << endl;             // В файл
                    f << " |производится замена| " << winnerAnt->distancePath << endl;      // В файл
                    for (int i = 0; i < winnerAnt->antCC; i++) {
                        winnerAnt->finalPath[i] = Ants[k]->finalPath[i];
                        cout << " |производится замена| " << winnerAnt->finalPath[i]->name << endl; // На консоль
                        f << " |производится замена| " << winnerAnt->finalPath[i]->name << endl;    // В файл
                    }
                }
                // Вывод пути
                for (int i = 0; i < winnerAnt->antCC; i++) {
                    cout << winnerAnt->finalPath[i]->name;
                    if (i != winnerAnt->antCC - 1) {
                        cout << "  ->  ";
                    }
                }
                cout << endl;
            }
            if (ERR) ERR = false;                                   // Обнуление значения, если ты попали в тупик

            // Обновление муравьев
            Ants[k]->antCC        = 1;
            Ants[k]->distancePath = 0.0;
            Ants[k]->curItem      = arrayAllItems[start];
            for (int i = 0; i < CountCity; i++)
                if (i != start) arrayAllItems[i]->readability = 0;
        }

        // Испарение ферамона
        for (int i = 0; i < CountCity; i++)
            for (int j = 0; j < CountCity; j++)
                if (i != j) matrPheromons[i][j] *= round((1 - RHO) * 1000) / 1000; // обновление феромона для ребра (i, j)

        //cout << "\n\n";
    }

    cout << "\n\t\tИтоговое Значение ПОЗИТИВНОГО Ферамона :" << endl;
    for (int i = 0; i < CountCity; i++) {
        cout << endl;
        for (int j = 0; j < CountCity; j++) {
            cout << "  " << matrPheromons[i][j];
        }
    }
    cout << "\n\n";
    cout << "\n\t\tИтоговое Значение НЕГАТИВНОГО Ферамона :" << endl;
    for (int i = 0; i < CountCity; i++) {
        cout << endl;
        for (int j = 0; j < CountCity; j++) {
            cout << "  " << matrNEGPheromons[i][j];
        }
    }
    cout << "\n";


    delete[] matrDistance;
    delete[] matrPheromons;
    delete[] matrNEGPheromons;

    return winnerAnt;
}

//  ГЛАВНЫЙ main()
int main()
{
    setlocale(LC_ALL, "RUSSIAN");

    int  CountCity  = 0,              // Вводимое количество городов
         widthGrid  = 0,              // Ширина сетки
         heightGrid = 0,              // Высота сетки
         countCounteraction = 0,      // Количество городов противодействий
         stepGrid   = 0,              // Шаг сетки
         startCity  = 0,              // Вводимое значение начального города
         endCity    = 0;              // Вводимое значение конечного  города
    bool ERR        = false;          // Переменная остановки программы


    //  ВЫВОД ОСНОВНЫХ ДАННЫХ НА КОНСОЛЬ
    cout << "\n\t\tЗначения По Умолчанию :";
    cout << "\nОбщее Количество Муравьев\t" << M << "\t - M";
    cout << "\nВажность Веса Ребра\t\t" << ALPHA << "\t - ALPHA";
    cout << "\nВажность Количества Феромонов\t" << BETTA << "\t - BETTA";
    cout << "\nВремя Жизни Колонии\t\t" << T_MAX << "\t - T_MAX";
    cout << "\nКоэффициент Испарения Феромона\t" << RHO << "\t - RHO";
    cout << "\nЗначение Негативного Феромона\t" << VNP << "\t - VNP";
    cout << endl << endl;

    // И В ФАЙЛ
    f << "\n\t\tЗначения По Умолчанию :";
    f << "\nОбщее Количество Муравьев\t" << M << "\t - M";
    f << "\nВажность Веса Ребра\t\t" << ALPHA << "\t - ALPHA";
    f << "\nВажность Количества Феромонов\t" << BETTA << "\t - BETTA";
    f << "\nВремя Жизни Колонии\t\t" << T_MAX << "\t - T_MAX";
    f << "\nКоэффициент Испарения Феромона\t" << RHO << "\t - RHO";
    f << "\nЗначение Негативного Феромона\t" << VNP << "\t - VNP";
    f << endl << endl;

    //  ВВОД ШИРИНЫ, ВЫСОТЫ И ШАГА СЕТКИ
    while (widthGrid < CC_MIN || widthGrid > CC_MAX)
    {
        cout << "\n\tВведите Ширину сетки [" << CC_MIN << ", " << CC_MAX << "] : "; cin >> widthGrid;
    }
    while (heightGrid < CC_MIN || heightGrid > CC_MAX)
    {
        cout << "\n\tВведите Высоту сетки [" << CC_MIN << ", " << CC_MAX << "] : "; cin >> heightGrid;
    }
    while (stepGrid < 1) {
        cout << "\n\tВведите Шаг сетки : "; cin >> stepGrid;
    }

    f << "\n\tВведите Ширину сетки [" << CC_MIN << ", " << CC_MAX << "] : " << widthGrid;
    f << "\n\tВведите Высоту сетки [" << CC_MIN << ", " << CC_MAX << "] : " << heightGrid;
    f << "\n\tВведите Шаг сетки : " << stepGrid;
    //heightGrid = widthGrid;
    CountCity = heightGrid * widthGrid;

    //  ИНИЦИАЛИЗАЦИЯ НАЧАЛЬНОЙ И КОНЕЧНОЙ ТОЧЕК
    while (startCity < 1 || startCity > CountCity)
    {
        cout << "\n\tВведите начальный город от 1 до " << CountCity << " : "; cin >> startCity;
    }
    while (endCity < 1 || endCity > CountCity || endCity == startCity)
    {
        cout << "\tВведите конечный город от 1 до " << CountCity << ", исключая " << startCity << " : "; cin >> endCity;
    }
    f << "\n\tВведите начальный город от 1 до " << CountCity << " : " << startCity;
    f << "\tВведите конечный город от 1 до " << CountCity << ", исключая " << startCity << " : " << endCity;

    //  ВВОД КОЛИЧЕСТВА ГОРОДОВ ПРОТИВОДЕЙСТВИЯ
    while (countCounteraction < 1 || countCounteraction > (widthGrid - 2) * (heightGrid - 2))
    {
        cout << "\tВведите количество городов противодействий от 1 до " << (widthGrid - 2) * (heightGrid - 2) << " : "; cin >> countCounteraction;
    }
    f << "\tВведите количество городов противодействий от 1 до " << (widthGrid - 2) * (heightGrid - 2) << " : " << countCounteraction;

    // Инициализация массива противодействующих городов и его заполнение
    int* matrCountCounteraction = new int [countCounteraction];
    for (int i = 0; i < countCounteraction; i++) {
        matrCountCounteraction[i] = -1;
    }
    cout << "\tВведите НОМЕРА < " << countCounteraction << " > городов противодействий, исключая СТАРТОВЫЙ и КОНЕЧНЫЙ город :" << endl;
    f << "\tВведите НОМЕРА < " << countCounteraction << " > городов противодействий, исключая СТАРТОВЫЙ и КОНЕЧНЫЙ город :" << endl;
    for (int i = 0; i < countCounteraction; i++) {
        while (matrCountCounteraction[i] < 0 || matrCountCounteraction[i] > 24 || matrCountCounteraction[i] == startCity || matrCountCounteraction[i] == endCity)
        {
            cout << "\t №" << i + 1 << "  = "; cin >> matrCountCounteraction[i];
            f << "\t №" << i + 1 << "  = " << matrCountCounteraction[i];
        }
    }

    startCity--;        // Вычитаем 1, чтобы получилось не НАЗВЕНИЕ а ИНДЕКС
    endCity--;          // Вычитаем 1, чтобы получилось не НАЗВЕНИЕ а ИНДЕКС


    // Заполнение матрицы расстояний
    for (int i = 0; i < CountCity; i++) {
        for (int j = 0; j < CountCity; j++) {
            if (i != j) matrixOfDistances[i][j] = stepGrid;
            else matrixOfDistances[i][j] = 0;
        }
    }

    //  ИНИЦИАЛИЗАЦИЯ ВСЕХ ГОРОДОВ
    City* city1 = new City,                     // для city1
        * city2 = new City,                     // для city2
        * city3 = new City,                     // для city3
        * city4 = new City,                     // для city4
        * city5 = new City,                     // для city5
        * city6 = new City,                     // для city6
        * city7 = new City,                     // для city7
        * city8 = new City,                     // для city8
        * city9 = new City,                     // для city9
        * city10 = new City,                    // для city10
        * city11 = new City,                    // для city11
        * city12 = new City,                    // для city12
        * city13 = new City,                    // для city13
        * city14 = new City,                    // для city14
        * city15 = new City,                    // для city15
        * city16 = new City,                    // для city16
        * city17 = new City,                    // для city17
        * city18 = new City,                    // для city18
        * city19 = new City,                    // для city19
        * city20 = new City,                    // для city20
        * city21 = new City,                    // для city21
        * city22 = new City,                    // для city22
        * city23 = new City,                    // для city23
        * city24 = new City,                    // для city24
        * city25 = new City;                    // для city25 // Поскольку MAX - 5х5

    //  ЗАПОЛНЯЕМ ИМЕНАМИ НАШИ ГОРОДА
    city1->name = "|1|";                        // для city1
    city2->name = "|2|";                        // для city2
    city3->name = "|3|";                        // для city3
    city4->name = "|4|";                        // для city4
    city5->name = "|5|";                        // для city5
    city6->name = "|6|";                        // для city6
    city7->name = "|7|";                        // для city7
    city8->name = "|8|";                        // для city8
    city9->name = "|9|";                        // для city9
    city10->name = "|10|";                      // для city10
    city11->name = "|11|";                      // для city11
    city12->name = "|12|";                      // для city12
    city13->name = "|13|";                      // для city13
    city14->name = "|14|";                      // для city14
    city15->name = "|15|";                      // для city15
    city16->name = "|16|";                      // для city16
    city17->name = "|17|";                      // для city17
    city18->name = "|18|";                      // для city18
    city19->name = "|19|";                      // для city19
    city20->name = "|20|";                      // для city20
    city21->name = "|21|";                      // для city21
    city22->name = "|22|";                      // для city22
    city23->name = "|23|";                      // для city23
    city24->name = "|24|";                      // для city24
    city25->name = "|25|";                      // для city25

    //  ЗАПОЛНЯЕМ ИНДЕКСАМИ НАШИ ГОРОДА
    city1->itemsIndex = 0;                      // для city1
    city2->itemsIndex = 1;                      // для city2
    city3->itemsIndex = 2;                      // для city3
    city4->itemsIndex = 3;                      // для city4
    city5->itemsIndex = 4;                      // для city5
    city6->itemsIndex = 5;                      // для city6
    city7->itemsIndex = 6;                      // для city7
    city8->itemsIndex = 7;                      // для city8
    city9->itemsIndex = 8;                      // для city9
    city10->itemsIndex = 9;                     // для city10
    city11->itemsIndex = 10;                    // для city11
    city12->itemsIndex = 11;                    // для city12
    city13->itemsIndex = 12;                    // для city13
    city14->itemsIndex = 13;                    // для city14
    city15->itemsIndex = 14;                    // для city15
    city16->itemsIndex = 15;                    // для city16
    city17->itemsIndex = 16;                    // для city17
    city18->itemsIndex = 17;                    // для city18
    city19->itemsIndex = 18;                    // для city19
    city20->itemsIndex = 19;                    // для city10
    city21->itemsIndex = 20;                    // для city21
    city22->itemsIndex = 21;                    // для city22
    city23->itemsIndex = 22;                    // для city23
    city24->itemsIndex = 23;                    // для city24
    city25->itemsIndex = 24;                    // для city25

    //  ЗАПОЛНЯЕМ СТАРТОВЫМИ ЗНАЧЕНИЯМИ ПОВТОРА НАШИ ГОРОДА
    city1->readability = 0;                     // для city1
    city2->readability = 0;                     // для city2
    city3->readability = 0;                     // для city3
    city4->readability = 0;                     // для city4
    city5->readability = 0;                     // для city5
    city6->readability = 0;                     // для city6
    city7->readability = 0;                     // для city7
    city8->readability = 0;                     // для city8
    city9->readability = 0;                     // для city9
    city10->readability = 0;                    // для city10
    city11->readability = 0;                    // для city11
    city12->readability = 0;                    // для city12
    city13->readability = 0;                    // для city13
    city14->readability = 0;                    // для city14
    city15->readability = 0;                    // для city15
    city16->readability = 0;                    // для city16
    city17->readability = 0;                    // для city17
    city18->readability = 0;                    // для city18
    city19->readability = 0;                    // для city19
    city20->readability = 0;                    // для city10
    city21->readability = 0;                    // для city21
    city22->readability = 0;                    // для city22
    city23->readability = 0;                    // для city23
    city24->readability = 0;                    // для city24
    city25->readability = 0;                    // для city25

    //  ЗАПОЛНЯЕМ СТАРТОВЫМИ ЗНАЧЕНИЯМИ ВЕРОЯТНОСТИ ПЕРЕХОДА В НАШИ ГОРОДА
    city1->desireToGo = 0;                     // для city1
    city2->desireToGo = 0;                     // для city2
    city3->desireToGo = 0;                     // для city3
    city4->desireToGo = 0;                     // для city4
    city5->desireToGo = 0;                     // для city5
    city6->desireToGo = 0;                     // для city6
    city7->desireToGo = 0;                     // для city7
    city8->desireToGo = 0;                     // для city8
    city9->desireToGo = 0;                     // для city9
    city10->desireToGo = 0;                    // для city10
    city11->desireToGo = 0;                    // для city11
    city12->desireToGo = 0;                    // для city12
    city13->desireToGo = 0;                    // для city13
    city14->desireToGo = 0;                    // для city14
    city15->desireToGo = 0;                    // для city15
    city16->desireToGo = 0;                    // для city16
    city17->desireToGo = 0;                    // для city17
    city18->desireToGo = 0;                    // для city18
    city19->desireToGo = 0;                    // для city19
    city20->desireToGo = 0;                    // для city10
    city21->desireToGo = 0;                    // для city21
    city22->desireToGo = 0;                    // для city22
    city23->desireToGo = 0;                    // для city23
    city24->desireToGo = 0;                    // для city24
    city25->desireToGo = 0;                    // для city25

    //  ФОРМИРУЕМ СОЕДИНЕНИЯ (ДОРОГИ) МЕЖДУ ВСЕМИ ГОРОДАМИ
    int N2 = 2,
        N3 = 3,
        N4 = 4;
    bool changeName = false;

    if (widthGrid == 3) {
        // Элемент city1
        City** itemsArray1 = new City * [N2];
        itemsArray1[0] = city2;
        itemsArray1[1] = city4;
        city1->items = itemsArray1;
        city1->itemsCount = N2;

        // Элемент city2
        City** itemsArray2 = new City * [N3];
        itemsArray2[0] = city1;
        itemsArray2[1] = city3;
        itemsArray2[2] = city5;
        city2->items = itemsArray2;
        city2->itemsCount = N3;

        // Элемент city3
        City** itemsArray3 = new City * [N2];
        itemsArray3[0] = city2;
        itemsArray3[1] = city6;
        city3->items = itemsArray3;
        city3->itemsCount = N2;

        // Элемент city4
        City** itemsArray4 = new City * [N3];
        itemsArray4[0] = city1;
        itemsArray4[1] = city5;
        itemsArray4[2] = city7;
        city4->items = itemsArray4;
        city4->itemsCount = N3;

        // Элемент city5
        City** itemsArray5 = new City * [N4];
        itemsArray5[0] = city2;
        itemsArray5[1] = city4;
        itemsArray5[2] = city6;
        itemsArray5[3] = city8;
        city5->items = itemsArray5;
        city5->itemsCount = N4;

        // Элемент city6
        City** itemsArray6 = new City * [N3];
        itemsArray6[0] = city3;
        itemsArray6[1] = city5;
        itemsArray6[2] = city9;
        city6->items = itemsArray6;
        city6->itemsCount = N3;

        if (heightGrid == 3) {
            // Элемент city7
            City** itemsArray7 = new City * [N2];
            itemsArray7[0] = city4;
            itemsArray7[1] = city8;
            city7->items = itemsArray7;
            city7->itemsCount = N2;

            // Элемент city8
            City** itemsArray8 = new City * [N3];
            itemsArray8[0] = city5;
            itemsArray8[1] = city7;
            itemsArray8[2] = city9;
            city8->items = itemsArray8;
            city8->itemsCount = N3;

            // Элемент city9
            City** itemsArray9 = new City * [N2];
            itemsArray9[0] = city6;
            itemsArray9[1] = city8;
            city9->items = itemsArray9;
            city9->itemsCount = N2;
        }
        else {
            // Элемент city7
            City** itemsArray7 = new City * [N3];
            itemsArray7[0] = city4;
            itemsArray7[1] = city8;
            itemsArray7[2] = city10;
            city7->items = itemsArray7;
            city7->itemsCount = N3;

            // Элемент city8
            City** itemsArray8 = new City * [N4];
            itemsArray8[0] = city5;
            itemsArray8[1] = city7;
            itemsArray8[2] = city9;
            itemsArray8[3] = city11;
            city8->items = itemsArray8;
            city8->itemsCount = N4;

            // Элемент city9
            City** itemsArray9 = new City * [N3];
            itemsArray9[0] = city6;
            itemsArray9[1] = city8;
            itemsArray9[2] = city12;
            city9->items = itemsArray9;
            city9->itemsCount = N3;

            if (heightGrid == 4) {
                // Элемент city10
                City** itemsArray10 = new City * [N2];
                itemsArray10[0] = city7;
                itemsArray10[1] = city11;
                city10->items = itemsArray10;
                city10->itemsCount = N2;

                // Элемент city11
                City** itemsArray11 = new City * [N3];
                itemsArray11[0] = city8;
                itemsArray11[1] = city10;
                itemsArray11[2] = city12;
                city11->items = itemsArray11;
                city11->itemsCount = N3;

                // Элемент city12
                City** itemsArray12 = new City * [N2];
                itemsArray12[0] = city9;
                itemsArray12[1] = city11;
                city12->items = itemsArray12;
                city12->itemsCount = N2;
            }
            else if (heightGrid == 5) {
                // Элемент city10
                City** itemsArray10 = new City * [N3];
                itemsArray10[0] = city7;
                itemsArray10[1] = city11;
                itemsArray10[2] = city13;
                city10->items = itemsArray10;
                city10->itemsCount = N3;

                // Элемент city11
                City** itemsArray11 = new City * [N4];
                itemsArray11[0] = city8;
                itemsArray11[1] = city10;
                itemsArray11[2] = city12;
                itemsArray11[3] = city14;
                city11->items = itemsArray11;
                city11->itemsCount = N4;

                // Элемент city12
                City** itemsArray12 = new City * [N3];
                itemsArray12[0] = city9;
                itemsArray12[1] = city11;
                itemsArray12[2] = city15;
                city12->items = itemsArray12;
                city12->itemsCount = N3;

                // Элемент city13
                City** itemsArray13 = new City * [N2];
                itemsArray13[0] = city10;
                itemsArray13[1] = city14;
                city13->items = itemsArray13;
                city13->itemsCount = N2;

                // Элемент city14
                City** itemsArray14 = new City * [N3];
                itemsArray14[0] = city11;
                itemsArray14[1] = city13;
                itemsArray14[2] = city15;
                city14->items = itemsArray14;
                city14->itemsCount = N3;

                // Элемент city15
                City** itemsArray15 = new City * [N2];
                itemsArray15[0] = city12;
                itemsArray15[1] = city14;
                city15->items = itemsArray15;
                city15->itemsCount = N2;
            }
        }
    }
    else if (widthGrid == 4) {
        // Элемент city1
        City** itemsArray1 = new City * [N2];
        itemsArray1[0] = city2;
        itemsArray1[1] = city5;
        city1->items = itemsArray1;
        city1->itemsCount = N2;

        // Элемент city2
        City** itemsArray2 = new City * [N3];
        itemsArray2[0] = city1;
        itemsArray2[1] = city3;
        itemsArray2[2] = city6;
        city2->items = itemsArray2;
        city2->itemsCount = N3;

        // Элемент city3
        City** itemsArray3 = new City * [N3];
        itemsArray3[0] = city2;
        itemsArray3[1] = city4;
        itemsArray3[2] = city7;
        city3->items = itemsArray3;
        city3->itemsCount = N3;

        // Элемент city4
        City** itemsArray4 = new City * [N2];
        itemsArray4[0] = city3;
        itemsArray4[1] = city8;
        city4->items = itemsArray4;
        city4->itemsCount = N2;

        // Элемент city5
        City** itemsArray5 = new City * [N3];
        itemsArray5[0] = city1;
        itemsArray5[1] = city6;
        itemsArray5[2] = city9;
        city5->items = itemsArray5;
        city5->itemsCount = N3;

        // Элемент city6
        City** itemsArray6 = new City * [N4];
        itemsArray6[0] = city2;
        itemsArray6[1] = city5;
        itemsArray6[2] = city7;
        itemsArray6[3] = city10;
        city6->items = itemsArray6;
        city6->itemsCount = N4;

        // Элемент city7
        City** itemsArray7 = new City * [N4];
        itemsArray7[0] = city3;
        itemsArray7[1] = city6;
        itemsArray7[2] = city8;
        itemsArray7[3] = city11;
        city7->items = itemsArray7;
        city7->itemsCount = N4;

        // Элемент city8
        City** itemsArray8 = new City * [N3];
        itemsArray8[0] = city4;
        itemsArray8[1] = city7;
        itemsArray8[2] = city12;
        city8->items = itemsArray8;
        city8->itemsCount = N3;

        if (heightGrid == 3) {
            // Элемент city9
            City** itemsArray9 = new City * [N2];
            itemsArray9[0] = city5;
            itemsArray9[1] = city10;
            city9->items = itemsArray9;
            city9->itemsCount = N2;

            // Элемент city10
            City** itemsArray10 = new City * [N3];
            itemsArray10[0] = city6;
            itemsArray10[1] = city9;
            itemsArray10[2] = city11;
            city10->items = itemsArray10;
            city10->itemsCount = N3;

            // Элемент city11
            City** itemsArray11 = new City * [N3];
            itemsArray11[0] = city7;
            itemsArray11[1] = city10;
            itemsArray11[2] = city12;
            city11->items = itemsArray11;
            city11->itemsCount = N3;

            // Элемент city12
            City** itemsArray12 = new City * [N2];
            itemsArray12[0] = city8;
            itemsArray12[1] = city11;
            city12->items = itemsArray12;
            city12->itemsCount = N2;
        }
        else {
            // Элемент city9
            City** itemsArray9 = new City * [N3];
            itemsArray9[0] = city5;
            itemsArray9[1] = city10;
            itemsArray9[2] = city13;
            city9->items = itemsArray9;
            city9->itemsCount = N3;

            // Элемент city10
            City** itemsArray10 = new City * [N4];
            itemsArray10[0] = city6;
            itemsArray10[1] = city9;
            itemsArray10[2] = city11;
            itemsArray10[3] = city14;
            city10->items = itemsArray10;
            city10->itemsCount = N4;

            // Элемент city11
            City** itemsArray11 = new City * [N4];
            itemsArray11[0] = city7;
            itemsArray11[1] = city10;
            itemsArray11[2] = city12;
            itemsArray11[3] = city15;
            city11->items = itemsArray11;
            city11->itemsCount = N4;

            // Элемент city12
            City** itemsArray12 = new City * [N3];
            itemsArray12[0] = city8;
            itemsArray12[1] = city11;
            itemsArray12[2] = city16;
            city12->items = itemsArray12;
            city12->itemsCount = N3;

            if (heightGrid == 4) {
                // Элемент city13
                City** itemsArray13 = new City * [N2];
                itemsArray13[0] = city9;
                itemsArray13[1] = city14;
                city13->items = itemsArray13;
                city13->itemsCount = N2;

                // Элемент city14
                City** itemsArray14 = new City * [N3];
                itemsArray14[0] = city10;
                itemsArray14[1] = city13;
                itemsArray14[2] = city15;
                city14->items = itemsArray14;
                city14->itemsCount = N3;

                // Элемент city15
                City** itemsArray15 = new City * [N3];
                itemsArray15[0] = city11;
                itemsArray15[1] = city14;
                itemsArray15[2] = city16;
                city15->items = itemsArray15;
                city15->itemsCount = N3;

                // Элемент city16
                City** itemsArray16 = new City * [N2];
                itemsArray16[0] = city12;
                itemsArray16[1] = city15;
                city16->items = itemsArray16;
                city16->itemsCount = N2;
            }
            else if (heightGrid == 5) {
                // Элемент city13
                City** itemsArray13 = new City * [N3];
                itemsArray13[0] = city9;
                itemsArray13[1] = city14;
                itemsArray13[2] = city17;
                city13->items = itemsArray13;
                city13->itemsCount = N3;

                // Элемент city14
                City** itemsArray14 = new City * [N4];
                itemsArray14[0] = city10;
                itemsArray14[1] = city13;
                itemsArray14[2] = city15;
                itemsArray14[3] = city18;
                city14->items = itemsArray14;
                city14->itemsCount = N4;

                // Элемент city15
                City** itemsArray15 = new City * [N4];
                itemsArray15[0] = city11;
                itemsArray15[1] = city14;
                itemsArray15[2] = city16;
                itemsArray15[3] = city19;
                city15->items = itemsArray15;
                city15->itemsCount = N4;

                // Элемент city16
                City** itemsArray16 = new City * [N3];
                itemsArray16[0] = city12;
                itemsArray16[1] = city15;
                itemsArray16[2] = city20;
                city16->items = itemsArray16;
                city16->itemsCount = N3;

                // Элемент city17
                City** itemsArray17 = new City * [N2];
                itemsArray17[0] = city13;
                itemsArray17[1] = city18;
                city17->items = itemsArray17;
                city17->itemsCount = N2;

                // Элемент city18
                City** itemsArray18 = new City * [N3];
                itemsArray18[0] = city14;
                itemsArray18[1] = city17;
                itemsArray18[2] = city19;
                city18->items = itemsArray18;
                city18->itemsCount = N3;

                // Элемент city19
                City** itemsArray19 = new City * [N3];
                itemsArray19[0] = city15;
                itemsArray19[1] = city18;
                itemsArray19[2] = city20;
                city19->items = itemsArray19;
                city19->itemsCount = N3;

                // Элемент city20
                City** itemsArray20 = new City * [N2];
                itemsArray20[0] = city16;
                itemsArray20[1] = city19;
                city20->items = itemsArray20;
                city20->itemsCount = N2;
            }
        }
    }
    else if (widthGrid == 5) {
        // Элемент city1
        City** itemsArray1 = new City * [N2];
        itemsArray1[0] = city2;
        itemsArray1[1] = city6;
        city1->items = itemsArray1;
        city1->itemsCount = N2;

        // Элемент city2
        City** itemsArray2 = new City * [N3];
        itemsArray2[0] = city1;
        itemsArray2[1] = city3;
        itemsArray2[2] = city7;
        city2->items = itemsArray2;
        city2->itemsCount = N3;

        // Элемент city3
        City** itemsArray3 = new City * [N3];
        itemsArray3[0] = city2;
        itemsArray3[1] = city4;
        itemsArray3[2] = city8;
        city3->items = itemsArray3;
        city3->itemsCount = N3;

        // Элемент city4
        City** itemsArray4 = new City * [N3];
        itemsArray4[0] = city3;
        itemsArray4[1] = city5;
        itemsArray4[2] = city9;
        city4->items = itemsArray4;
        city4->itemsCount = N3;

        // Элемент city5
        City** itemsArray5 = new City * [N2];
        itemsArray5[0] = city4;
        itemsArray5[1] = city10;
        city5->items = itemsArray5;
        city5->itemsCount = N2;

        // Элемент city6
        City** itemsArray6 = new City * [N3];
        itemsArray6[0] = city1;
        itemsArray6[1] = city7;
        itemsArray6[2] = city11;
        city6->items = itemsArray6;
        city6->itemsCount = N3;

        // Элемент city7
        City** itemsArray7 = new City * [N4];
        itemsArray7[0] = city2;
        itemsArray7[1] = city6;
        itemsArray7[2] = city8;
        itemsArray7[3] = city12;
        city7->items = itemsArray7;
        city7->itemsCount = N4;

        // Элемент city8
        City** itemsArray8 = new City * [N4];
        itemsArray8[0] = city3;
        itemsArray8[1] = city7;
        itemsArray8[2] = city9;
        itemsArray8[3] = city13;
        city8->items = itemsArray8;
        city8->itemsCount = N4;

        // Элемент city9
        City** itemsArray9 = new City * [N4];
        itemsArray9[0] = city4;
        itemsArray9[1] = city8;
        itemsArray9[2] = city10;
        itemsArray9[3] = city14;
        city9->items = itemsArray9;
        city9->itemsCount = N4;

        // Элемент city10
        City** itemsArray10 = new City * [N3];
        itemsArray10[0] = city5;
        itemsArray10[1] = city9;
        itemsArray10[2] = city15;
        city10->items = itemsArray10;
        city10->itemsCount = N3;

        if (heightGrid == 3) {
            // Элемент city11
            City** itemsArray11 = new City * [N2];
            itemsArray11[0] = city6;
            itemsArray11[1] = city12;
            city11->items = itemsArray11;
            city11->itemsCount = N2;

            // Элемент city12
            City** itemsArray12 = new City * [N3];
            itemsArray12[0] = city7;
            itemsArray12[1] = city11;
            itemsArray12[2] = city13;
            city12->items = itemsArray12;
            city12->itemsCount = N3;

            // Элемент city13
            City** itemsArray13 = new City * [N3];
            itemsArray13[0] = city8;
            itemsArray13[1] = city12;
            itemsArray13[2] = city14;
            city13->items = itemsArray13;
            city13->itemsCount = N3;

            // Элемент city14
            City** itemsArray14 = new City * [N3];
            itemsArray14[0] = city9;
            itemsArray14[1] = city13;
            itemsArray14[2] = city15;
            city14->items = itemsArray14;
            city14->itemsCount = N3;

            // Элемент city15
            City** itemsArray15 = new City * [N2];
            itemsArray15[0] = city10;
            itemsArray15[1] = city14;
            city15->items = itemsArray15;
            city15->itemsCount = N2;


        }
        else {
            // Элемент city11
            City** itemsArray11 = new City * [N3];
            itemsArray11[0] = city6;
            itemsArray11[1] = city12;
            itemsArray11[2] = city16;
            city11->items = itemsArray11;
            city11->itemsCount = N3;

            // Элемент city12
            City** itemsArray12 = new City * [N4];
            itemsArray12[0] = city7;
            itemsArray12[1] = city11;
            itemsArray12[2] = city13;
            itemsArray12[3] = city17;
            city12->items = itemsArray12;
            city12->itemsCount = N4;

            // Элемент city13
            City** itemsArray13 = new City * [N4];
            itemsArray13[0] = city8;
            itemsArray13[1] = city12;
            itemsArray13[2] = city14;
            itemsArray13[3] = city18;
            city13->items = itemsArray13;
            city13->itemsCount = N4;

            // Элемент city14
            City** itemsArray14 = new City * [N4];
            itemsArray14[0] = city9;
            itemsArray14[1] = city13;
            itemsArray14[2] = city15;
            itemsArray14[3] = city19;
            city14->items = itemsArray14;
            city14->itemsCount = N4;

            // Элемент city15
            City** itemsArray15 = new City * [N3];
            itemsArray15[0] = city10;
            itemsArray15[1] = city14;
            itemsArray15[2] = city20;
            city15->items = itemsArray15;
            city15->itemsCount = N3;

            if (heightGrid == 4) {
                // Элемент city16
                City** itemsArray16 = new City * [N2];
                itemsArray16[0] = city11;
                itemsArray16[1] = city17;
                city16->items = itemsArray16;
                city16->itemsCount = N2;

                // Элемент city17
                City** itemsArray17 = new City * [N3];
                itemsArray17[0] = city12;
                itemsArray17[1] = city16;
                itemsArray17[2] = city18;
                city17->items = itemsArray17;
                city17->itemsCount = N3;

                // Элемент city18
                City** itemsArray18 = new City * [N3];
                itemsArray18[0] = city13;
                itemsArray18[1] = city17;
                itemsArray18[2] = city19;
                city18->items = itemsArray18;
                city18->itemsCount = N3;

                // Элемент city19
                City** itemsArray19 = new City * [N3];
                itemsArray19[0] = city14;
                itemsArray19[1] = city18;
                itemsArray19[2] = city20;
                city19->items = itemsArray19;
                city19->itemsCount = N3;

                // Элемент city20
                City** itemsArray20 = new City * [N2];
                itemsArray20[0] = city15;
                itemsArray20[1] = city19;
                city20->items = itemsArray20;
                city20->itemsCount = N2;






            }
            else if (heightGrid == 5) {
                // Элемент city16
                City** itemsArray16 = new City * [N3];
                itemsArray16[0] = city11;
                itemsArray16[1] = city17;
                itemsArray16[2] = city21;
                city16->items = itemsArray16;
                city16->itemsCount = N3;

                // Элемент city17
                City** itemsArray17 = new City * [N4];
                itemsArray17[0] = city12;
                itemsArray17[1] = city16;
                itemsArray17[2] = city18;
                itemsArray17[3] = city22;
                city17->items = itemsArray17;
                city17->itemsCount = N4;

                // Элемент city18
                City** itemsArray18 = new City * [N4];
                itemsArray18[0] = city13;
                itemsArray18[1] = city17;
                itemsArray18[2] = city19;
                itemsArray18[3] = city23;
                city18->items = itemsArray18;
                city18->itemsCount = N4;

                // Элемент city19
                City** itemsArray19 = new City * [N4];
                itemsArray19[0] = city14;
                itemsArray19[1] = city18;
                itemsArray19[2] = city20;
                itemsArray19[3] = city24;
                city19->items = itemsArray19;
                city19->itemsCount = N4;

                // Элемент city20
                City** itemsArray20 = new City * [N3];
                itemsArray20[0] = city15;
                itemsArray20[1] = city19;
                itemsArray20[2] = city25;
                city20->items = itemsArray20;
                city20->itemsCount = N3;

                // Элемент city21
                City** itemsArray21 = new City * [N2];
                itemsArray21[0] = city16;
                itemsArray21[1] = city22;
                city21->items = itemsArray21;
                city21->itemsCount = N2;

                // Элемент city22
                City** itemsArray22 = new City * [N3];
                itemsArray22[0] = city17;
                itemsArray22[1] = city21;
                itemsArray22[2] = city23;
                city22->items = itemsArray22;
                city22->itemsCount = N3;

                // Элемент city23
                City** itemsArray23 = new City * [N2];
                itemsArray23[0] = city18;
                itemsArray23[1] = city22;
                city23->items = itemsArray23;
                city23->itemsCount = N2;

                // Элемент city24
                City** itemsArray24 = new City * [N3];
                itemsArray24[0] = city19;
                itemsArray24[1] = city23;
                itemsArray24[2] = city25;
                city24->items = itemsArray24;
                city24->itemsCount = N3;

                // Элемент city25
                City** itemsArray25 = new City * [N2];
                itemsArray25[0] = city20;
                itemsArray25[1] = city24;
                city25->items = itemsArray25;
                city25->itemsCount = N2;
            }
        }
    }


    //  ЗАПОЛНЯЕМ МАССИВ ВСЕХ ЭЛЕМЕНТОВ НАШИМИ ГОРОДАМИ
    arrayAllItems[city1->itemsIndex] = city1;       // для city1
    arrayAllItems[city2->itemsIndex] = city2;       // для city2
    arrayAllItems[city3->itemsIndex] = city3;       // для city3
    arrayAllItems[city4->itemsIndex] = city4;       // для city4
    arrayAllItems[city5->itemsIndex] = city5;       // для city5
    arrayAllItems[city6->itemsIndex] = city6;       // для city6
    arrayAllItems[city7->itemsIndex] = city7;       // для city7
    arrayAllItems[city8->itemsIndex] = city8;       // для city8
    arrayAllItems[city9->itemsIndex] = city9;       // для city9
    arrayAllItems[city10->itemsIndex] = city10;     // для city10
    arrayAllItems[city11->itemsIndex] = city11;     // для city11
    arrayAllItems[city12->itemsIndex] = city12;     // для city12
    arrayAllItems[city13->itemsIndex] = city13;     // для city13
    arrayAllItems[city14->itemsIndex] = city14;     // для city14
    arrayAllItems[city15->itemsIndex] = city15;     // для city15
    arrayAllItems[city16->itemsIndex] = city16;     // для city16
    arrayAllItems[city17->itemsIndex] = city17;     // для city17
    arrayAllItems[city18->itemsIndex] = city18;     // для city18
    arrayAllItems[city19->itemsIndex] = city19;     // для city19
    arrayAllItems[city20->itemsIndex] = city20;     // для city20
    arrayAllItems[city21->itemsIndex] = city21;     // для city21
    arrayAllItems[city22->itemsIndex] = city22;     // для city22
    arrayAllItems[city23->itemsIndex] = city23;     // для city23
    arrayAllItems[city24->itemsIndex] = city24;     // для city24
    arrayAllItems[city25->itemsIndex] = city25;     // для city25

    // Вывод сетки городов
    int imtirrat = 0;
    for (int i = 0; i < heightGrid * 4; i++) {
        cout << "\n\t";
        if (i % 4 == 0) {
            for (int j = 0; j < widthGrid; j++) {
                if (imtirrat == startCity) {
                    cout << "<S>";
                }
                else if (imtirrat == endCity) {
                    cout << "<F>";
                }
                else {
                    cout << arrayAllItems[imtirrat]->name;
                }
                imtirrat++;
                if (j + 1 != widthGrid) {
                    cout << "  --" << stepGrid << "--  ";
                }
            }
        }
        else if (i % 4 == 2) {
            if (i + 3 < heightGrid * 4) {
                for (int j = 0; j < widthGrid; j++) {
                    cout << " " << stepGrid << " ";
                    for (int im = 0; im < 9; im++) {
                        cout << " ";
                    }
                }
            }
        }
        else {
            if (i + 3 < heightGrid * 4) {
                for (int j = 0; j < widthGrid; j++) {
                    cout << " | ";
                    for (int im = 0; im < 9; im++) {
                        cout << " ";
                    }
                }
            }
        }
    }
    cout << endl;

    // Вызов функции пересчета
    AntUnit* Ant = AntColonyOptimization(startCity, endCity, CountCity, matrCountCounteraction, countCounteraction);

    //  ВЫВОД РЕЗУЛЬТАТА

    cout << "\n|||||||||||||||||||| ИТОГО ||||||||||||||||||||\n\tДлинна пути : " << Ant->distancePath;     // На консоль
    cout << "\n\tПуть: \t" << Ant->finalPath[0]->name;                                                      // На консоль
    f << "\n|||||||||||||||||||| ИТОГО ||||||||||||||||||||\n\tДлинна пути : " << Ant->distancePath;        // В Файл
    f << "\n\tПуть: \t" << Ant->finalPath[0]->name;                                                         // В Файл
    for (int i = 1; i < Ant->antCC; ++i) {
        cout << "  ->  " << Ant->finalPath[i]->name;                    // На консоль
        f << "  ->  " << Ant->finalPath[i]->name;                       // В Файл
    }
    cout << endl;

    imtirrat = 0;
    bool uuu = false;
    for (int i = 0; i < heightGrid * 4; i++)
    {
        cout << "\n\t";
        if (i % 4 == 0) {
            for (int j = 0; j < widthGrid; j++)
            {
                if (imtirrat == startCity) {
                    cout << "<S>";
                }
                else if (imtirrat == endCity) {
                    cout << "<F>";
                }
                else {
                    cout << arrayAllItems[imtirrat]->name;
                    arrayAllItems[imtirrat]->readability = 2;
                }
                if (j + 1 != widthGrid) {
                    for (int rrr = 0; rrr < Ant->antCC; rrr++)
                    {
                        if (imtirrat == Ant->finalPath[rrr]->itemsIndex) {
                            //(imtirrat + 1 == Ant->finalPath[rrr + 1]->itemsIndex || (rrr - 1 >= 0) ? () : (0))) {
                            if (imtirrat + 1 == Ant->finalPath[rrr + 1]->itemsIndex) {
                                uuu = true;
                            }
                            else if (rrr - 1 >= 0) {
                                uuu = (imtirrat + 1 == Ant->finalPath[rrr - 1]->itemsIndex);
                            }
                            if (uuu) {
                                break;
                            }
                        }
                    }
                    if (uuu) {
                        cout << " * * * * ";
                        uuu = false;
                    }
                    else {
                        cout << "  --" << stepGrid << "--  ";
                    }
                }
                imtirrat++;
            }
        }
        else if (i + 3 < heightGrid * 4) {
            for (int j = widthGrid; j > 0; --j)
            {
                for (int rrr = 0; rrr < Ant->antCC; rrr++)
                {
                    if (imtirrat - j == Ant->finalPath[rrr]->itemsIndex && (imtirrat - j + widthGrid) >= 0 && (imtirrat - j + widthGrid) < heightGrid * widthGrid) {
                        if (arrayAllItems[imtirrat - j + widthGrid]->itemsIndex == Ant->finalPath[rrr + 1]->itemsIndex) {
                            uuu = true;
                            break;
                        }
                    }
                }
                if (uuu) {
                    cout << " * ";
                    uuu = false;
                }
                else {
                    if (i % 4 == 2) {
                        cout << " " << stepGrid << " ";
                    }
                    else {
                        cout << " | ";
                    }
                }
                cout << "         ";
            }
        }
    }
    cout << endl;

    f.close();
    cout << "Файл лежит в том же каталоге, что и этот проект, под именем 'Shuvalov_Proj'\n\n";

    system("pause");
    return 0;
}
