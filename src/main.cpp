#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

#include "../include/header.h"



bool cinNoFail()
{
    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid data!\n";
        return false;
    }
    else
    {
        if (std::cin.peek() != '\n')
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid data!\n";
            return false;
        }
    }

    return true;
}



std::ostream& operator<<(std::ostream& out, TrainType object)
{
    switch (object)
    {
        case TrainType::A: out << 'A';
            break;
        case TrainType::B: out << 'B';
            break;
        case TrainType::C: out << 'C';
            break;
        default:
            break;
    }

    return out;
}



void input(std::vector<Train>& trains, const int numberOfTrains)
{
    std::cout << "Travel time to the train station, seconds\n";

    int travelTime;

    for (int i{}; i < numberOfTrains; ++i)
    {
        while (true)
        {
            std::cout << "Train " << static_cast<TrainType>(i) << ": ";
            std::cin >> travelTime;

            if (cinNoFail()) break;
        }

        trains.emplace_back(static_cast<TrainType>(i), static_cast<std::chrono::seconds>(travelTime));
    }
}



void output(TrainType trainType, const std::string& str, std::mutex& mutexCout)
{
    std::lock_guard<std::mutex> lg(mutexCout);

    std::cout << "Train " << trainType << str << '\n';
}



void toTheTrainStation(Train& train, std::mutex& mutexStation, std::mutex& mutexCout,
                       std::condition_variable& condVarStation, bool& stationFree)
{
    //Поезд в пути к станции
    std::this_thread::sleep_for(train.getTravelTime());

    //Машинист проверяет, свободна ли станция

    {
        /*std::unique_lock<std::mutex> ul(mutexStation, std::defer_lock);

        bool first{ true };
        while (!ul.try_lock())
        {
            if (first)
            {
                first = false;

                output(train.getTrainType(), " is waiting for a free path", mutexCout);
            }
        }*/

        //Ничего лучше не придумал, чтобы убрать холостую загрузку с процессора

        std::unique_lock<std::mutex> ul(mutexStation, std::defer_lock);
        std::unique_lock<std::mutex> ul2(mutexStation, std::defer_lock);

        if (!ul.try_lock())
        {
            output(train.getTrainType(), " is waiting for a free path", mutexCout);

            ul2.lock();
            condVarStation.wait(ul2, [&] { return stationFree; });
        }

        stationFree = false;

        //Станция свободна, поезд заходит, выгружается-загружается и далее отправляется по команде диспетчера

        output(train.getTrainType(), " arrived at the station", mutexCout);

        while (true)
        {
            output(train.getTrainType(), ", depart - 1: ", mutexCout);

            int command;
            std::cin >> command;

            if (!cinNoFail()) continue;

            if (command == 1) break;
        }

        output(train.getTrainType(), " left the station", mutexCout);

        stationFree = true;
    }

    condVarStation.notify_one();
}



int main()
{
    const int numberOfTrains = static_cast<int>(TrainType::MAX);

    std::vector<Train> trains;
    trains.reserve(numberOfTrains);

    input(trains, numberOfTrains);

    std::vector<std::thread> trainThreads;
    trainThreads.reserve(numberOfTrains);

    //Ожидание свободного пути на станции
    std::mutex mutexStation;

    std::mutex mutexCout;

    //Для сигнализации машинистам, что станция освободилась
    std::condition_variable condVarStation;
    bool stationFree{ false };

    //Диспетчер отправляет поезда к станции назначения
    for (auto& train : trains)
        trainThreads.emplace_back(toTheTrainStation, std::ref(train), std::ref(mutexStation), std::ref(mutexCout),
                                  std::ref(condVarStation), std::ref(stationFree));

    //Диспетчер ожидает когда все поезда покинут станцию (станцию назначения)
    for (auto& thread : trainThreads)
        thread.join();

    std::cout << "\nAll trains have left the station\n";
}