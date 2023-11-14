#ifndef __ABSTRACTDEVICE_H
#define __ABSTRACTDEVICE_H

#include "config.h"
#include "datetime.h"

class AbstractDevice {
    private:
        DateTime currentDateTime = DateTime(23, 11, 2020, 23, 59, 59);
    public:
        static int getBatteryLevel();
        static DateTime getDateTime();
        static void setDateTime(DateTime newDateTime);
};

#endif /*__ABSTRACTDEVICE_H */
