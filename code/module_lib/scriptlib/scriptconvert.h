#ifndef __SCRIPT_CONVERT__
#define __SCRIPT_CONVERT__

#pragma once

#include "../module_public.h"

class CScriptConvert
{
public:
    CScriptConvert( void ) = default;
    ~CScriptConvert() = default;

    static string_t *BoolToString( bool b ) {
        return new string_t( b ? "true" : "false" );
    }
    static string_t *ShortToString( int16_t i ) {
        return new string_t( va( "%hi", i ) );
    }
    static string_t *IntToString( int32_t i ) {
        return new string_t( va( "%i", i ) );
    }
    static string_t *LongToString( int64_t l ) {
        return new string_t( va( "%li", l ) );
    }
    static string_t *FloatToString( float f ) {
        return new string_t( va( "%.3f", f ) );
    }
    static string_t *DoubleToString( double d ) {
        return new string_t( va( "%.3lf", d ) );
    }
    static string_t *UShortToString( uint16_t i ) {
        return new string_t( va( "%hu", i ) );
    }
    static string_t *UIntToString( uint32_t i ) {
        return new string_t( va( "%u", i ) );
    }
    static string_t *ULongToString( uint64_t l ) {
        return new string_t( va( "%lu", l ) );
    }

    static int32_t HexStringToInt( const string_t *str ) {
        return Com_HexStrToInt( str->c_str() );
    }
    static uint64_t HexStringToULong( const string_t *str ) {
        char buf[64];
        return strtoul( str->c_str(), (char **)&buf, 16 );
    }

    static double StringToDouble( const string_t *str ) {
        char buf[64];
        return strtod( str->c_str(), (char **)&buf );
    }
    static float StringToFloat( const string_t *str ) {
        return N_atof( str->c_str() );
    }
    static int16_t StringToShort( const string_t *str ) {
        return atoi( str->c_str() );
    }
    static uint16_t StringToUShort( const string_t *str ) {
        return (uint16_t)atoi( str->c_str() );
    }
    static int32_t StringToInt( const string_t *str ) {
        return N_atoi( str->c_str() );
    }
    static uint32_t StringToUInt( const string_t *str ) {
        return (uint32_t)atol( str->c_str() );
    }
    static int64_t StringToLong( const string_t *str ) {
        return atol( str->c_str() );
    }
    static uint64_t StringToULong( const string_t *str ) {
        char buf[64];
        return strtoul( str->c_str(), (char **)&buf, 10 );
    }
    static bool StringToBool( const string_t *str ) {
        return *str == "true" ? true : false;
    }

    template<typename T>
    static uint16_t ToUShort( T v ) { return (uint16_t)v; }
    template<typename T>
    static uint32_t ToUInt( T v ) { return (uint32_t)v; }
    template<typename T>
    static uint64_t ToULong( T v ) { return (uint64_t)v; }
    template<typename T>
    static int32_t ToShort( T v ) { return (int32_t)v; }
    template<typename T>
    static int32_t ToInt( T v ) { return (int32_t)v; }
    template<typename T>
    static int64_t ToLong( T v ) { return (int64_t)v; }
    template<typename T>
    static double ToDouble( T v ) { return (double)v; }
    template<typename T>
    static float ToFloat( T v ) { return (float)v; }
    template<typename T>
    static bool ToBool( T v ) { return (bool)v; }

    static void Register( asIScriptEngine *pEngine )
    {
        CheckASCall( pEngine->RegisterObjectType( "Convert", sizeof( CScriptConvert ), asOBJ_VALUE ) );
        CheckASCall( pEngine->RegisterObjectBehaviour( "Convert", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( CScriptConvert, ( void ) ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectBehaviour( "Convert", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( CScriptConvert ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 StringToHexInt( const string& in ) const", WRAP_FN( CScriptConvert::HexStringToInt ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 FromHexString( const string& in ) const", WRAP_FN( CScriptConvert::HexStringToULong ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( bool ) const", WRAP_FN( CScriptConvert::BoolToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( int64 ) const", WRAP_FN( CScriptConvert::LongToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( uint64 ) const", WRAP_FN( CScriptConvert::ULongToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( int32 ) const", WRAP_FN( CScriptConvert::IntToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( uint32 ) const", WRAP_FN( CScriptConvert::UIntToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( int16 ) const", WRAP_FN( CScriptConvert::ShortToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "string ToString( uint16 ) const", WRAP_FN( CScriptConvert::UShortToString ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( const string& in ) const", WRAP_FN( CScriptConvert::StringToFloat ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( double ) const", WRAP_FN( CScriptConvert::ToFloat<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( int8 ) const", WRAP_FN( CScriptConvert::ToFloat<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( int16 ) const", WRAP_FN( CScriptConvert::ToFloat<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( int32 ) const", WRAP_FN( CScriptConvert::ToFloat<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( int64 ) const", WRAP_FN( CScriptConvert::ToFloat<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( uint8 ) const", WRAP_FN( CScriptConvert::ToFloat<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( uint16 ) const", WRAP_FN( CScriptConvert::ToFloat<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( uint32 ) const", WRAP_FN( CScriptConvert::ToFloat<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( uint64 ) const", WRAP_FN( CScriptConvert::ToFloat<uint64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "float ToFloat( bool ) const", WRAP_FN( CScriptConvert::ToFloat<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( const string& in ) const", WRAP_FN( CScriptConvert::StringToLong ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( float ) const", WRAP_FN( CScriptConvert::ToLong<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( double ) const", WRAP_FN( CScriptConvert::ToLong<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( int8 ) const", WRAP_FN( CScriptConvert::ToLong<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( int16 ) const", WRAP_FN( CScriptConvert::ToLong<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( int32 ) const", WRAP_FN( CScriptConvert::ToLong<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( uint8 ) const", WRAP_FN( CScriptConvert::ToLong<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( uint16 ) const", WRAP_FN( CScriptConvert::ToLong<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( uint32 ) const", WRAP_FN( CScriptConvert::ToLong<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( uint64 ) const", WRAP_FN( CScriptConvert::ToLong<uint64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int64 ToLong( bool ) const", WRAP_FN( CScriptConvert::ToLong<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( const string& in ) const", WRAP_FN( CScriptConvert::StringToULong ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( float ) const", WRAP_FN( CScriptConvert::ToULong<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( double ) const", WRAP_FN( CScriptConvert::ToULong<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( int8 ) const", WRAP_FN( CScriptConvert::ToULong<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( int16 ) const", WRAP_FN( CScriptConvert::ToULong<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( int32 ) const", WRAP_FN( CScriptConvert::ToULong<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( int64 ) const", WRAP_FN( CScriptConvert::ToULong<int64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( uint8 ) const", WRAP_FN( CScriptConvert::ToULong<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( uint16 ) const", WRAP_FN( CScriptConvert::ToULong<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( uint32 ) const", WRAP_FN( CScriptConvert::ToULong<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint64 ToULong( bool ) const", WRAP_FN( CScriptConvert::ToULong<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( const string& in ) const", WRAP_FN( CScriptConvert::StringToInt ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( float ) const", WRAP_FN( CScriptConvert::ToInt<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( double ) const", WRAP_FN( CScriptConvert::ToInt<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( int8 ) const", WRAP_FN( CScriptConvert::ToInt<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( int16 ) const", WRAP_FN( CScriptConvert::ToInt<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( int64 ) const", WRAP_FN( CScriptConvert::ToInt<int64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( uint8 ) const", WRAP_FN( CScriptConvert::ToInt<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( uint16 ) const", WRAP_FN( CScriptConvert::ToInt<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( uint32 ) const", WRAP_FN( CScriptConvert::ToInt<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( uint64 ) const", WRAP_FN( CScriptConvert::ToInt<uint64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int32 ToInt( bool ) const", WRAP_FN( CScriptConvert::ToInt<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( const string& in ) const", WRAP_FN( CScriptConvert::StringToUInt ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( float ) const", WRAP_FN( CScriptConvert::ToUInt<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( double ) const", WRAP_FN( CScriptConvert::ToUInt<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( int8 ) const", WRAP_FN( CScriptConvert::ToUInt<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( int16 ) const", WRAP_FN( CScriptConvert::ToUInt<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( int32 ) const", WRAP_FN( CScriptConvert::ToUInt<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( int64 ) const", WRAP_FN( CScriptConvert::ToUInt<int64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( uint8 ) const", WRAP_FN( CScriptConvert::ToUInt<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( uint16 ) const", WRAP_FN( CScriptConvert::ToUInt<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( uint64 ) const", WRAP_FN( CScriptConvert::ToUInt<uint64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint32 ToUInt( bool ) const", WRAP_FN( CScriptConvert::ToUInt<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( const string& in ) const", WRAP_FN( CScriptConvert::ShortToString ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( float ) const", WRAP_FN( CScriptConvert::ToShort<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( double ) const", WRAP_FN( CScriptConvert::ToShort<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( int8 ) const", WRAP_FN( CScriptConvert::ToShort<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( int32 ) const", WRAP_FN( CScriptConvert::ToShort<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( int64 ) const", WRAP_FN( CScriptConvert::ToShort<int64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( uint8 ) const", WRAP_FN( CScriptConvert::ToShort<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( uint16 ) const", WRAP_FN( CScriptConvert::ToShort<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( uint32 ) const", WRAP_FN( CScriptConvert::ToShort<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( uint64 ) const", WRAP_FN( CScriptConvert::ToShort<uint64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "int16 ToShort( bool ) const", WRAP_FN( CScriptConvert::ToShort<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( const string& in ) const", WRAP_FN( CScriptConvert::StringToUShort ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( float ) const", WRAP_FN( CScriptConvert::ToUShort<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( double ) const", WRAP_FN( CScriptConvert::ToUShort<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( int8 ) const", WRAP_FN( CScriptConvert::ToUShort<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( int16 ) const", WRAP_FN( CScriptConvert::ToUShort<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( int32 ) const", WRAP_FN( CScriptConvert::ToUShort<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( int64 ) const", WRAP_FN( CScriptConvert::ToUShort<int64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( uint8 ) const", WRAP_FN( CScriptConvert::ToUShort<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( uint32 ) const", WRAP_FN( CScriptConvert::ToUShort<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( uint64 ) const", WRAP_FN( CScriptConvert::ToUShort<uint64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "uint16 ToUShort( bool ) const", WRAP_FN( CScriptConvert::ToUShort<bool> ), asCALL_GENERIC ) );

        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( const string& in ) const", WRAP_FN( CScriptConvert::StringToBool ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( float ) const", WRAP_FN( CScriptConvert::ToBool<float> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( double ) const", WRAP_FN( CScriptConvert::ToBool<double> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( int8 ) const", WRAP_FN( CScriptConvert::ToBool<int8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( int16 ) const", WRAP_FN( CScriptConvert::ToBool<int16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( int32 ) const", WRAP_FN( CScriptConvert::ToBool<int32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( int64 ) const", WRAP_FN( CScriptConvert::ToBool<int64_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( uint8 ) const", WRAP_FN( CScriptConvert::ToBool<uint8_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( uint16 ) const", WRAP_FN( CScriptConvert::ToBool<uint16_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( uint32 ) const", WRAP_FN( CScriptConvert::ToBool<uint32_t> ), asCALL_GENERIC ) );
        CheckASCall( pEngine->RegisterObjectMethod( "Convert", "bool ToBool( uint64 ) const", WRAP_FN( CScriptConvert::ToBool<uint64_t> ), asCALL_GENERIC ) );
    }
};

#endif