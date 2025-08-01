// OpenJE

#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define LOG_LOCATION __FILE__, __LINE__, __FUNCTION__

#if defined(LOGGER)
namespace {
    enum LogLevel {
        LINFO,
        LWARN,
        LERROR
    };

    #define COLOR_RESET     "\x1b[0m"
    #define COLOR_INFO      "\x1b[36m" // Cyan
    #define COLOR_WARN      "\x1b[33m" // Yellow
    #define COLOR_ERROR     "\x1b[31m" // Red

    inline const char* LogLevelToString( LogLevel level ) {
        switch ( level ) {
            case LINFO:  return "INFO";
            case LWARN:  return "WARN";
            case LERROR: return "ERROR";
            default:    return "UNKNOWN";
        }
    }

    inline const char* LogLevelToColor( LogLevel level ) {
        switch ( level ) {
            case LINFO:  return COLOR_INFO;
            case LWARN:  return COLOR_WARN;
            case LERROR: return COLOR_ERROR;
            default:    return COLOR_RESET;
        }
    }

    void _LOG( LogLevel level, const char* file, int line, const char* func, const char* fmt, va_list args ) {
        time_t t = time( 0 );
        char timebuf[ 20 ];
        strftime( timebuf, sizeof( timebuf ), "%Y-%m-%d %H:%M:%S", localtime( &t ) );

        char header[ 256 ];
        _snprintf( header, sizeof( header ), "[%s] [%s] (%s:%d %s): ", timebuf, LogLevelToString( level ), file, line, func );

        char message[ 1024 ];
        _vsnprintf( message, sizeof( message ), fmt, args );

        #if defined( LOG_TERMINAL )
        fprintf(
            stderr,
            "%s%s%s%s\n",
            LogLevelToColor( level ),
            header,
            COLOR_RESET,
            message
        );
        #endif // LOG_TERMINAL

        #if defined( LOG_FILE )
        static const char* LOG_FILE_PATH = "log.txt";
        FILE* logFile = fopen( LOG_FILE_PATH, "a" );
        if ( logFile ) {
            fprintf( logFile, "%s%s\n", header, message );
            fclose( logFile );
        }
        #endif // LOG_FILE
    } // _LOG
} // namespace
#endif // Logger

inline void LOG_INFO( const char* file, int line, const char* func, const char* fmt, ... ) {
    #if defined( LOGGER )
    va_list args;
    va_start( args, fmt );
    _LOG( LINFO, file, line, func, fmt, args );
    va_end( args );
    #endif // LOGGER
}

inline void LOG_WARN( const char* file, int line, const char* func, const char* fmt, ... ) {
    #if defined( LOGGER )
    va_list args;
    va_start( args, fmt );
    _LOG( LWARN, file, line, func, fmt, args );
    va_end( args );
    #endif // LOGGER
}

inline void LOG_ERROR( const char* file, int line, const char* func, const char* fmt, ... ) {
    #if defined( LOGGER )
    va_list args;
    va_start( args, fmt );
    _LOG( LERROR, file, line, func, fmt, args );
    va_end( args );
    #endif // LOGGER
}

#endif // LOGGER_H
