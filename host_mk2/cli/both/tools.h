#ifndef __ZEJUNK_H__
#define __ZEJUNK_H__

#include <cstdio>
#include <cstdlib>

#include <stdexcept>

// Todo:
// Decimal can't detect overflow

class numchar {

    // 'char' is unsigned on some platforms (ARM, PPC..)
    // Thus it's just easier to cast it to a known type and stick with that
    const uint8_t *m_ptr;

    void setPtr( const char *ptr ) {
        m_ptr = (uint8_t*)ptr;
    }

    // Skip tab and space up to the first character
    static const uint8_t *scanTo( const uint8_t *ch ) {
        if ( ch == nullptr ) return nullptr;
        while ( *ch != 0 && (*ch == ' ' || *ch == 0x09 ))
            ch++;
        return ch;
    }

public:
    explicit numchar( const char *ptr = nullptr ) {
        setPtr( ptr );
    }

    template <typename T>
    bool isDec( ) {
        const uint8_t *p = scanTo( m_ptr );
        int sLen = 0;
        if ( p == nullptr ) return false;
        if ( *p == '-' ) {
            if ( std::is_signed<T>::value == false )
                return false;
            p++;
        }
        while ( *p >= '0' && *p <= '9' ) {
            p++;
            sLen++;
        }
        return ( sLen > 0 && (*p == 0 || *p == ' ' || *p == 0x09) );
    }

    template <typename T>
    T asDec() {
        const uint8_t *p = scanTo( m_ptr );
        bool isNeg = false;
        size_t sLen = 0;
        T num = 0;
        if ( p == nullptr )
            throw std::logic_error("You must check if a number is decimal before attempting conversion");
        if ( *p == '-' ) {
            if ( std::is_signed<T>::value == false )
                throw std::logic_error("Specified data type can't deal with negative numbers");
            isNeg = true;
            p++;
        }
        while ( *p >= '0' && *p <= '9' ) {
            num *= 10;
            num += (*p - '0');
            p++;
            sLen++;
        }
        if ( (*p != 0 && *p != ' ' && *p != 0x09) || sLen == 0 )
            throw std::logic_error("You must check if a number is decimal before attempting conversion");
        return ( isNeg ) ? -num : num;
    }

    template <typename T>
    bool isHex() {
        const uint8_t *p = scanTo( m_ptr );
        size_t sLen = 0;
        if ( p == nullptr ) return false;
        if ( *p == '-' ) return false; // No such thing as negative hex numbers
        // Do not expect $ AND 0x / x, just one of
        if ( *p == '$' ) p++;
        else if ( *p == 'x' || *p == 'X' ) p++;
        else if ( *p == '0' && ( p[1] == 'x' || p[1] == 'X' ) ) p += 2;
        uint8_t msn = *p;
        while ( (*p >= '0' && *p <= '9') ||
                (*p >= 'a' && *p <= 'f') ||
                (*p >= 'A' && *p <= 'F') ) {
            p++;
            sLen++;
        }
        if ( sLen == 0 ) return false;
        if ( sizeof(T) < ((sLen + 1) / 2) ) return false;
        // If using the whole size, make sure the most significant nibble is less than 7 to prevent overflow
        if ( sizeof(T) == ((sLen + 1) / 2) && std::is_signed<T>::value == true && msn > '7' )
            return false;
        return ( *p == 0 || *p == ' ' || *p == 0x09 );
    }

    template <typename T>
    T asHex() {
        const uint8_t *p = scanTo( m_ptr );
        size_t sLen = 0;
        T num = 0;
        if ( p == nullptr )
            throw std::logic_error("You must check if a number is hex before attempting conversion");
        if ( *p == '-' )
            throw std::logic_error("There is no such thing as negative hex");
        // Do not expect $ AND 0x / x, just one of
        if ( *p == '$' ) p++;
        else if ( *p == 'x' || *p == 'X' ) p++;
        else if ( *p == '0' && ( p[1] == 'x' || p[1] == 'X' ) ) p += 2;
        uint8_t msn = *p;
        while ( *p ) {
            num <<= 4;
            if /**/ ( *p >= '0' && *p <= '9' ) num |= (*p - '0');
            else if ( *p >= 'a' && *p <= 'f' ) num |= (*p - 0x57);
            else if ( *p >= 'A' && *p <= 'F' ) num |= (*p - 0x37);
            else throw std::logic_error("You must check if a number is hex before attempting conversion (Illegal character)");
            p++;
            sLen++;
        }
        if ( sLen == 0 ) throw std::logic_error("You must check if a number is hex before attempting conversion (No size)");
        if ( sizeof(T) < ((sLen + 1) / 2) ) throw std::logic_error("You must check if a number is hex before attempting conversion (Too large)");
        // If using the whole size, make sure the most significant nibble is less than 7 to prevent overflow
        // It seems std:: will forcefully convert it to unsigned 
        if ( sizeof(T) == ((sLen + 1) / 2) && std::is_signed<T>::value == true && msn > '7' )
            throw std::logic_error("You must check if a number is hex before attempting conversion (Signed overflow)");
        if ( (*p != 0 && *p != ' ' && *p != 0x09) || sLen == 0 )
            throw std::logic_error("You must check if a number is hex before attempting conversion");
        return num;
    }

    const char *c_str() {
        return (char*)m_ptr;
    }

    void operator = ( const char *ptr ) { setPtr ( ptr ); }
};

#endif
