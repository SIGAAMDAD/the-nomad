using TheNomad;

namespace TheNomad.Math {
    class vec3 {
        vec3() {
            x = 0;
            y = 0;
            x = 0;
        }
        vec3( const vec3@ other ) {
            x = other.x;
            y = other.y;
            z = other.z;
        }

        vec3@ operator=( const vec3@ other ) {
            x = other.x;
            y = other.y;
            z = other.z;
            return @this;
        }

        private float x, y, z;
    };

    class vec2 {
        vec2() {
            x = 0;
            y = 0;
        }
        vec2( const vec2@ other ) {
            x = other.x;
            y = other.y;
        }
        
        vec2@ operator=( const vec2@ other ) {
            x = other.x;
            y = other.y;
            return @this;
        }

        private float x, y;
    };

    public float DotProduct( const vec3@ a, const vec3@ b );
    public float DotProduct( const vec2@ a, const vec2@ b );
};