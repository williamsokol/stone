//////////////////////////////////////////////////////////////////////////
//
// This file is part of Stone's source files.
// Stone is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef UTILS_H
#define UTILS_H


#include "myassert.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <limits>




using namespace std;




template<class T>
void clear_delete_elements(T & Container)
{
    for (typename T::iterator i = Container.begin() ; i != Container.end() ; i++)
        delete *i;

    Container.clear();
}



template<class T>
inline void push_back_if_not_found(T & Container, const typename T::value_type & Element)
{
    if (!contains(Container, Element)) Container.push_back(Element);
}




template<class T>
inline void push_back_assert_does_not_contain(T & Container, const typename T::value_type & Element)
{
    assert_throw(!contains(Container, Element));
	Container.push_back(Element);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  bc_type
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//
//  A simple bound-checked type
//

template<class TInt, TInt Min, TInt Max>
class bc_type
{
public:
                    bc_type(const TInt & value) : m_value(value)    { assert_plus(Valid(), "bound check fail"); }

    enum
    {
        min = Min,
        max = Max,
        card = max-min+1
    };

    const TInt &    ToInt() const       { return m_value; }

private:
    bool            Valid() const       { return (Min <= m_value) && (m_value <= Max); }

    TInt            m_value;
};


template<class TInt, TInt Min, TInt Max>
inline bool operator==(const bc_type<TInt, Min, Max> & a, const bc_type<TInt, Min, Max> & b) { return a.ToInt() == b.ToInt(); }

template<class TInt, TInt Min, TInt Max>
inline bool operator!=(const bc_type<TInt, Min, Max> & a, const bc_type<TInt, Min, Max> & b) { return a.ToInt() != b.ToInt(); }


//////////////////////////////////////////////////////////////////////////////////////////////
//  bc_type specializations

template<int Min, int Max>
class bc_int : public bc_type<int, Min, Max>
{ public: bc_int(int value) : bc_type<int, Min, Max>(value) {} };














#endif

