/* date = October 2nd 2024 1:43 pm */

#ifndef _PAIRS_H
#define _PAIRS_H

template <typename Type1, typename Type2>
struct pair
{
    Type1 First;
    Type2 Second;
    
    pair() : First(Type1()), Second(Type2()) {}
    
    pair(const Type1 &A, const Type2 &B) 
        : First(A), Second(B) {}
    
    
    bool operator==(const pair &Comparitor)
    {
        return(First == Comparitor.First && Second == Comparitor.Second);
    }
    
    bool operator!=(const pair &Comparitor)
    {
        return(!(*this == Comparitor));
    }
};

template <typename Type1, typename Type2>
internal inline pair<Type1, Type2> 
MakePair(const Type1 &A, const Type2 &B)
{
    return(pair<Type1, Type2>(A, B));
}

#endif //_PAIRS_H
