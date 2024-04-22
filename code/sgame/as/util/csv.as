array<string>@ ParseCSV( const string& in str )
{
    array<string> values;
    string data;

    for ( uint i = 0; i < str.size(); i++ ) {
        switch ( str[i] ) {
        case ',':
            values.Add( data );
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