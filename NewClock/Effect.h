#ifndef Effect_h
#define Effect_h
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#endif
namespace espace{
enum EffectState{
    Begin,
    TransitingIn,
    TransitingOut,
    Holding,
    End,
} ;

class Effect
{
    private:
        uint8_t hold = 0;
        
    public:
        Effect(/* args */);
        ~Effect();
        
        virtual void TransitionIn(uint8_t* oldData, uint8_t* newData );
        virtual void TransitionOut(uint8_t* oldData, uint8_t* newData );
        void Assign(uint8_t* oldData, uint8_t* newData );
        void Apply(uint8_t* oldData, uint8_t* newData);
        uint8_t CurrentIndex = 0;
        EffectState State = Begin;
        uint8_t HoldingDuration = 30;
};

class RowTransition : public Effect
{
    public:
        RowTransition(/* args */);
        ~RowTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData );
        void TransitionOut(uint8_t* oldData, uint8_t* newData );
};

class ScanTransition : public Effect
{
    public:
        ScanTransition(/* args */);
        ~ScanTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData );
        void TransitionOut(uint8_t* oldData, uint8_t* newData );
};

class FlyTransition : public Effect
{
    public:
        FlyTransition(/* args */);
        ~FlyTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData );
        void TransitionOut(uint8_t* oldData, uint8_t* newData );
};

class RandomTransition : public Effect
{
    public:
        RandomTransition(/* args */);
        ~RandomTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData );
        void TransitionOut(uint8_t* oldData, uint8_t* newData );
};

class ZoomTransition : public Effect
{
    public:
        ZoomTransition(/* args */);
        ~ZoomTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData);
        void TransitionOut(uint8_t* oldData, uint8_t* newData);
};


class InvertTransition : public Effect
{
    public:
        InvertTransition(/* args */);
        ~InvertTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData);
        void TransitionOut(uint8_t* oldData, uint8_t* newData);
};

class ExpandTransition : public Effect
{
    public:
        ExpandTransition(/* args */);
        ~ExpandTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData);
        void TransitionOut(uint8_t* oldData, uint8_t* newData);
};

class WindowSlideTransition : public Effect
{
    public:
        WindowSlideTransition(/* args */);
        ~WindowSlideTransition();
        void TransitionIn(uint8_t* oldData, uint8_t* newData);
        void TransitionOut(uint8_t* oldData, uint8_t* newData);
};
}
#endif