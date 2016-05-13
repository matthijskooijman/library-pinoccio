#ifndef SCOUT_BACKPACK_H_
#define SCOUT_BACKPACK_H_

//#include "util/Callback.h"

namespace pinoccio {
    class Backpack {
    public:
        void setup();

        bool isVccEnabled();

        void enableVcc();

        void disableVcc();

//            CallbackList<void, bool> toggleBackpackVccCallbacks;
    protected:
        bool vccEnabled;
    };
}


#endif
