namespace TheNomad::Util {
    bool IsSpace( char value ) {
        return ( value == ' ' );
    }

    string BoolToString( bool v ) {
        return v ? "true" : "false";
    }
};
