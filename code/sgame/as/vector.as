//
// vector.as -- basic math vector implementations
//

shared class vec3
{
    vec3( float _x, float _y, float _z ) {
        x = _x;
        y = _y;
        z = _z;
    }
    vec3( const vec3& other ) {
        x = other.x;
        y = other.y;
        z = other.z;
    }

    vec3& operator=( const vec3& other ) {
        x = other.x;
        y = other.y;
        z = other.z;
    }

    float& operator[]( uint nIndex ) {
        return @(this)[nIndex];
    }
    const float& operator[](uint nIndex) const {
        return @(this)[nIndex];
    }

    float x;
    float y;
    float z;
};