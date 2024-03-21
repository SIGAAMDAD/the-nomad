array<string>@ ParseCSV( const string& in str )
{
    array<string> values;
    string data;

    data.reserve( MAX_STRING_CHARS );
    for ( uint i = 0; i < str.size(); i++ ) {
        switch ( str[i] ) {
        case ',':
            values.push_back( data );
            data = "";
            i++;
            break;
        case ' ':
            break; // ignore it
        default:
            data += str[i];
            break;
        };
    }
    
    return @values;
}