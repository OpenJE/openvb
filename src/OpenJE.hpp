// Copyright 2024 OpenJE

#ifndef OPENJE_H
#define OPENJE_H

#include <stdio.h>
#include <time.h>

namespace OpenJE {
    void Debug( const char *__restrict _Format ) {
        printf( _Format );
    }
}

#endif // OPENJE_H
