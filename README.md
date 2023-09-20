# AntAlgorithm

"Рассмотреть возможность применения «негативного» феромона. У целевой
функции остается только параметр длины пути. Параметр, связанный с
противодействие заносит другой феромон. Этот феромон уменьшает
вероятности перехода по дугам, у которых значение большое. По моему должна
возникнуть проблема нормировки, могу рассказать отдель."

По программе :

1) С начала вводятся значения:
Ширина сетки, Шаг сетки, Стартовый и Финишный Города, а так же количество городов противодействий и их номера

------------------------ Пример -------------------------

Введите Ширину сетки [3, 5] : 5
Введите Высоту сетки [3, 5] : 5
Введите Шаг сетки : 2
Введите начальный город от 1 до 25 : 4
Введите конечный город от 1 до 25, исключая 4 : 24
Введите количество городов противодействий от 1 до 9 : 2
Введите НОМЕРА < 2 > городов противодействий, исключая СТАРТОВЫЙ и КОНЕЧНЫЙ город :

№1 = 16
№2 = 18

----------------------------------------------------------

2) После чего, программа выводит карту дорог, а после чего начинает проводить итерации по поиску маршрута,
если "муравей" дошел из стартового города в финишный, то такой путь выводится, с указанием номера
"муравья" и номера итерации :

----------------------------- Пример -----------------------------

Номер Итерации < t > 0 Номер Муравья < k > 0
|текущий путь|
|4| -> |3| -> |8| -> |7| -> |12| -> |13| -> |14| -> |9| -> |10| -> |15| -> |20| -> |19| -> |24|
|длина текущего пути| 24

------------------------------------------------------------------

3) После всех итераций выводится итоговый, самый короткий путь

----------------------------- Пример -----------------------------

|||||||||||||||||||| ИТОГО ||||||||||||||||||||
Длинна пути : 8

Путь: |4| -> |9| -> |14| -> |19| -> |24|
|1| --2-- |2| --2-- |3| --2-- <S> --2-- |5|
| | | * |
2 2 2 * 2
| | | * |

|6| --2-- |7| --2-- |8| --2-- |9| --2-- |10|

| | | * |
2 2 2 * 2
| | | * |

|11| --2-- |12| --2-- |13| --2-- |14| --2-- |15|

| | | * |
2 2 2 * 2
| | | * |

|16| --2-- |17| --2-- |18| --2-- |19| --2-- |20|

| | | * |
2 2 2 * 2
| | | * |

|21| --2-- |22| --2-- |23| --2-- <F> --2-- |25|

------------------------------------------------------------------
