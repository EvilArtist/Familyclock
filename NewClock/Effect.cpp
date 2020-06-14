#include "Effect.h"
namespace espace{

Effect::Effect()
{
    hold = 0;
}

Effect::~Effect(){}

void Effect::Assign(uint8_t* oldData, uint8_t* newData)
{
    for (uint8_t i = 0; i < 32; i++)
    {
        oldData[i] = newData[i];
    }
    
}

void Effect::Apply(uint8_t* oldData, uint8_t* newData)
{
    if (State == Begin)
    {
        TransitionIn(oldData, newData);
    } 
    else if (State == TransitingIn)
    {
        TransitionIn(oldData, newData);
    }
    else if (State == TransitingOut)
    {
        TransitionOut(oldData, newData);
    }
    else if ( State == Holding )
    {
        if(hold >= HoldingDuration)
        {
            TransitionOut(oldData, newData);
        }
        else
        {
            hold++;
        }
    }
}

RowTransition::RowTransition()
{
    State = Begin;
}

void RowTransition::TransitionIn(uint8_t* oldData, uint8_t* newData)
{
    uint8_t column = 0x00;
    State = TransitingIn;
    if (CurrentIndex < 9)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            if (i % 2 == 0)
                column = newData[i] >> (8 - CurrentIndex);
            else
                column = newData[i] << (8 - CurrentIndex);
            oldData[i] = column;
        }
    }
    else
    {
        State = Holding;
    }
    CurrentIndex++;
}

void RowTransition::TransitionOut(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingOut;
    uint8_t column = 0x00;
    if (CurrentIndex > 0)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            if (i % 2 == 0)
                column = newData[i] >> (9 - CurrentIndex);
            else
                column = newData[i] << (9 - CurrentIndex);
            oldData[i] = column;
        }
    }
    else 
    {
        State = End;
    }
    CurrentIndex--;
}

ScanTransition::ScanTransition()
{
    State = Begin;
}

void ScanTransition::TransitionIn(uint8_t* oldData, uint8_t* newData )
{
    State = TransitingIn;
    if (CurrentIndex < 33)
    {
        uint8_t updateIndex = CurrentIndex - 1;        
        
        if (CurrentIndex == 0)
        {
            oldData[31] = 0xFF;
            oldData[0] = 0xFF;
        }
        if (CurrentIndex > 0 && CurrentIndex < 16) 
        {
            oldData[updateIndex] = newData[updateIndex] & 0xAA;
            oldData[31 - updateIndex] = newData[31 - updateIndex] & 0x55;
            oldData[31-CurrentIndex] = 0xFF;
            oldData[CurrentIndex] = 0xFF;
        }
        else if (CurrentIndex > 16 && CurrentIndex < 32)  
        {
            oldData[31 - updateIndex] = newData[31 - updateIndex];
            oldData[updateIndex] = newData[updateIndex];
            oldData[31-CurrentIndex] = 0xFF;
            oldData[CurrentIndex] = 0xFF;
        }
        else if (CurrentIndex == 32)  
        {
            oldData[0] = newData[0];
            oldData[31] = newData[31];
        }
    }
    else
    {
        State = Holding;
    }
    CurrentIndex++;
}

void ScanTransition::TransitionOut(uint8_t* oldData, uint8_t* newData )
{
    State = TransitingOut;
    if (CurrentIndex > 0)
    {
        if (CurrentIndex < 33 && CurrentIndex > 17) // 33 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8
        {
            oldData[CurrentIndex - 2] = 0xFF;
            oldData[33 - CurrentIndex] = 0xFF; 
            oldData[CurrentIndex - 1] = newData[CurrentIndex - 1] & 0x55;
            oldData[32 - CurrentIndex] = newData[32 - CurrentIndex] & 0xAA;
        }
        else if (CurrentIndex < 17 && CurrentIndex > 1 ) //17 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1
        {
            oldData[CurrentIndex - 2] = 0xFF;
            oldData[33 - CurrentIndex] = 0xFF; 
            oldData[CurrentIndex - 1] = 0;
            oldData[32 - CurrentIndex] = 0;
        }
        else if (CurrentIndex == 1)
        {
            oldData[CurrentIndex - 1] = 0;
            oldData[32 - CurrentIndex] = 0;
        }
    }
    else
    {
        oldData[CurrentIndex] = 0;
        oldData[31 - CurrentIndex] = 0;
        State = End;
    }
    CurrentIndex--;
}

FlyTransition::FlyTransition()
{
    State = Begin;
}

void FlyTransition::TransitionIn(uint8_t* oldData, uint8_t* newData )
{
    State = TransitingIn;
    if (CurrentIndex < 39)
    {
        uint8_t maxShift = CurrentIndex > 7 ? 8 : CurrentIndex;
        for (uint8_t i = 0; i <= maxShift; i++)
        {
            if (CurrentIndex - i < 32)
            {
                oldData[CurrentIndex - i] = newData[CurrentIndex - i] << (7 - i);
            }
        }
    }
    else 
    {
        State = Holding;
    }
    CurrentIndex++;
}

void FlyTransition::TransitionOut(uint8_t* oldData, uint8_t* newData )
{
    State = TransitingOut;
    if (CurrentIndex > 0)
    {
        uint8_t Emax;
        Emax = CurrentIndex > 32 ? 40-CurrentIndex:8;
        for (uint8_t i = 0; i <= Emax; i++)
        {
            if (40 > CurrentIndex + i && CurrentIndex + i > 9 )
            {
                oldData[40 - CurrentIndex - i] = newData[40 - CurrentIndex - i] >> i;
            }
        }
        
    }
    else 
    {
        State = End;
    }
    CurrentIndex--;
}

RandomTransition::RandomTransition(){}

void RandomTransition::TransitionIn(uint8_t* oldData, uint8_t* newData )
{
    uint8_t mask;
    State = TransitingIn;
    if (CurrentIndex == 0)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = 0;
        }
    }
    else if (CurrentIndex < 10)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            mask = random(255);
            oldData[i] = oldData[i] | (mask & newData[i]);
        }
    }
    else if (CurrentIndex == 10 )
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = newData[i];
        }
    }
    else
    {
        State = Holding;
    }
    CurrentIndex++;
}

void RandomTransition::TransitionOut(uint8_t* oldData, uint8_t* newData )
{
    State = TransitingOut;
    if (CurrentIndex == 11)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = newData[i];
        }
    }       
    else if (CurrentIndex > 1)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            uint8_t Rmask = random(256);
            oldData[i] = oldData[i] & Rmask;
        }
    }
    else if (CurrentIndex == 1)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = 0;
        }
    }
    else
    {
        State = End;
    }
    CurrentIndex--;
}

ZoomTransition::ZoomTransition(){}

void ZoomTransition::TransitionIn(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingIn;
    if (CurrentIndex < 16)
    {
        int distance = 16 - CurrentIndex;
        if (CurrentIndex < 15)
        {
            for (int i = 0; i<32; i++)
            {
                oldData[i] = 0;
            }
        }
        
        for (int i = 1; 15 >= i * distance; i++)
        {
            oldData[16 - i * distance] = newData[16 - i];
            oldData[15 + i * distance] = newData[15 + i];
        }
    }
    else
    {
        State = Holding;
    }
    CurrentIndex++;
}

void ZoomTransition::TransitionOut(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingOut;
    if (CurrentIndex > 0 )
    {
        uint8_t distance = 16 - CurrentIndex + 2;
        
        for (uint8_t i = 0; i<32; i++)
        {
            oldData[i] = 0;
        }
        
        for (int i = 1; 15 >= i * distance; i++)
        {
            oldData[16 - i * distance] = newData[16 - i];
            oldData[15 + i * distance] = newData[15 + i];
        }
    }
    else
    {
        State = End;
    }
    CurrentIndex--;
}
InvertTransition::InvertTransition(){}
void InvertTransition::TransitionIn(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingIn;
    if (CurrentIndex < 8)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = (~newData[i] & 0xFF) >> (7 - CurrentIndex);
        }
    }
    else if (CurrentIndex < 16)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = (oldData[i] << 1) | (newData[i] >> (15- CurrentIndex)) ;
        }
    }
    else
    {
        State = Holding;
    }
    CurrentIndex++;
}

void InvertTransition::TransitionOut(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingOut;
    if (CurrentIndex > 9)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = (oldData[i] << 1) | ((~newData[i] & 0xFF)>> (CurrentIndex - 10)) ;
        }
    }
    else if (CurrentIndex > 0)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            oldData[i] = oldData[i] << 1;
        }
    }
    else
    {
        State = End;
    }
    CurrentIndex--;
}

ExpandTransition::ExpandTransition(){}

void ExpandTransition::TransitionIn(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingIn;
    if (CurrentIndex < 16)
    {
        for (uint8_t i = 0; i < CurrentIndex; i++)
        {
            oldData[16-i] = newData[CurrentIndex];
            oldData[15+i] = newData[31-CurrentIndex];
        }
    }
    else
    {
        State = Holding;
    }
    CurrentIndex++;
}

void ExpandTransition::TransitionOut(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingOut;
    if (CurrentIndex > 0)
    {
        for (uint8_t i = 0; i < 32 - CurrentIndex; i++)
        {
            oldData[16-i] = newData[CurrentIndex - 1];
            oldData[15+i] = newData[32-CurrentIndex];
        }
    }
    else
    {
        State = End;
    }
    CurrentIndex--;
}

WindowSlideTransition::WindowSlideTransition(){}

void WindowSlideTransition::TransitionIn(uint8_t* oldData, uint8_t* newData)
{
    State = TransitingIn;
    if (CurrentIndex < 8)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < CurrentIndex; j++)
            {
                oldData[i * 8 + 7 - j] = 0xFF;  
            }
        }
    }
    else if (CurrentIndex < 16)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < CurrentIndex; j++)
            {
                oldData[i * 8 + j] = newData[i * 8 + j];  
            }
        }
    }
}
}