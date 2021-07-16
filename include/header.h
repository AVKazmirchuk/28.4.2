#pragma once
#pragma once

#include <chrono>



enum class TrainType
{
    A,
    B,
    C,
    MAX
};



class Train
{
private:
    //Тип поезда
    TrainType trainType;

    //Время в пути
    std::chrono::seconds travelTime;
public:
    Train(TrainType inTrainType, std::chrono::seconds inTravelTime) :
            trainType{inTrainType}, travelTime{inTravelTime} {}

    TrainType getTrainType() { return trainType; }

    std::chrono::seconds& getTravelTime() { return travelTime; }
};