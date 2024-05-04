#include <string.h>

char* gap(float distance_now){
    if (distance_now > 10)
        return "Stay out of trouble";
    if (distance_now > 3.5)
        return "Push";
        return "Gogogo";
}

char* fuel(float fuel_now){
    if (fuel_now > 80)
        return "Push Push Push";
    if (fuel_now > 50)
        return "You can go";
        return "Conserve Fuel";
}

char* tire(int tire_now){
    if (tire_now > 80)
        return "Go Push Go Push";
    if (tire_now > 50)
        return "Good Tire Wear";
    if (tire_now > 30)
        return "Conserve Your Tire";
        return "Box Box Box";
}

char* tire_change(char* type_now){
    if (strcmp(type_now, "Soft") == 0)
        return "Mediums Ready";
    if (strcmp(type_now, "Medium") == 0)
        return "Box for Softs";
}