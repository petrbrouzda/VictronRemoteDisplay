#include "ChartDataSource.h"
#include <math.h>


RingBufCDS::RingBufCDS(int numItems)
{
    this->data = RingBuf_new(sizeof(float), numItems );
    this->version = 0;
}

void RingBufCDS::put(float item )
{
    if( this->data->isFull(this->data) ) {
        float tmp;
        this->data->pull(this->data, &tmp);
    }
    this->data->add( this->data, &item );
    this->version++;
}

int RingBufCDS::getNumItems() 
{
    return this->data->numElements(this->data);
}

float RingBufCDS::getItem( int pos ) 
{
    int pos2 = this->data->numElements(this->data) - pos - 1;
    float * f = (float*)(this->data->peek(this->data, pos2 ));
    if( f!=NULL ) {
        return *f ;
    } else {
        return NAN;
    }
}



ArrayCDS::ArrayCDS(int numItems)
{
    this->data = (float*)malloc(sizeof(float) * (numItems+1) );
    this->maxItems = numItems;
    this->items = 0;
    this->version = 0;
    for( int i = 0; i<numItems; i++ ) {
        this->data[i] = NAN;
    }
}

void ArrayCDS::setItem(int pos, float item)
{
    if( pos < this->maxItems ) {
        this->data[pos] = item;
        if( pos+1 > this->items ) {
            this->items = pos+1;
        }
        this->version++;
    }
}

int ArrayCDS::getNumItems() 
{
    return this->items;
}

int ArrayCDS::getMaxItems()
{
    return this->maxItems;
}

float ArrayCDS::getItem( int pos ) 
{
    if( pos < this->maxItems ) {
        return this->data[pos];
    } else {
        return NAN;
    }
}
